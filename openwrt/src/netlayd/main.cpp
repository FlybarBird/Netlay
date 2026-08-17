/* netlayd — Netlay/SonoBus-compatible AOO listen client with ALSA USB output
 * and Group Control target (/sb remote mix). Playback only.
 */

#include "aoo/aoo.hpp"
#include "aoo/aoo_net.hpp"
#include "aoo/aoo_opus.h"

#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"

#include <alsa/asoundlib.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <strings.h>
#include <syslog.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr const char *kDefaultServer = "aoo.sonobus.net";
constexpr int kDefaultServerPort = 10998;
constexpr int kDefaultUdpPort = 28780;
constexpr int kSampleRate = 48000;
constexpr int kChannels = 2;
constexpr int kBlockFrames = 256;
constexpr int kSinkBufMs = 200;
constexpr const char *kCtlPath = "/var/run/netlay.ctl";
constexpr const char *kStatusPath = "/var/run/netlay.status";
constexpr const char *kLevelsPath = "/var/run/netlay.levels";

constexpr int kRmProtoVer = 1;
constexpr int kRmMuteFlag = 1;
constexpr int kRmMaxRetries = 8;
constexpr int kRmNackDenied = 1;
constexpr int kRmNackUnknown = 2;
constexpr int kRmNackBadVer = 3;
constexpr double kRmControllerHoldMs = 4000.0;

std::string json_escape(const std::string &s) {
    std::string o;
    o.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
        case '"': o += "\\\""; break;
        case '\\': o += "\\\\"; break;
        case '\n': o += "\\n"; break;
        case '\r': o += "\\r"; break;
        case '\t': o += "\\t"; break;
        default:
            if (c < 0x20) {
                char buf[8];
                snprintf(buf, sizeof(buf), "\\u%04x", c);
                o += buf;
            } else {
                o += static_cast<char>(c);
            }
        }
    }
    return o;
}

std::string json_get(const std::string &body, const std::string &key) {
    const std::string pat = "\"" + key + "\"";
    auto p = body.find(pat);
    if (p == std::string::npos) return {};
    p = body.find(':', p + pat.size());
    if (p == std::string::npos) return {};
    p = body.find_first_not_of(" \t", p + 1);
    if (p == std::string::npos) return {};
    if (body[p] == '"') {
        ++p;
        std::string out;
        for (; p < body.size() && body[p] != '"'; ++p) {
            if (body[p] == '\\' && p + 1 < body.size()) {
                out += body[++p];
            } else {
                out += body[p];
            }
        }
        return out;
    }
    auto e = body.find_first_of(",} \t\n", p);
    return body.substr(p, e == std::string::npos ? std::string::npos : e - p);
}

bool json_truthy(const std::string &v) {
    return v == "1" || v == "true" || v == "on" || v == "yes";
}

bool users_equal(const std::string &a, const std::string &b) {
    return strcasecmp(a.c_str(), b.c_str()) == 0;
}

float osc_as_float(osc::ReceivedMessageArgumentIterator &it) {
    if (it->IsFloat()) return (it++)->AsFloat();
    if (it->IsDouble()) return static_cast<float>((it++)->AsDouble());
    if (it->IsInt32()) return static_cast<float>((it++)->AsInt32());
    ++it;
    return 1.f;
}

double now_ms() {
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double, std::milli>(clock::now().time_since_epoch()).count();
}

std::string sockaddr_key(const sockaddr *sa, socklen_t len) {
    char host[INET6_ADDRSTRLEN] = {0};
    uint16_t port = 0;
    if (sa->sa_family == AF_INET && len >= sizeof(sockaddr_in)) {
        auto *in = reinterpret_cast<const sockaddr_in *>(sa);
        inet_ntop(AF_INET, &in->sin_addr, host, sizeof(host));
        port = ntohs(in->sin_port);
    } else if (sa->sa_family == AF_INET6 && len >= sizeof(sockaddr_in6)) {
        auto *in6 = reinterpret_cast<const sockaddr_in6 *>(sa);
        inet_ntop(AF_INET6, &in6->sin6_addr, host, sizeof(host));
        port = ntohs(in6->sin6_port);
    }
    return std::string(host) + ":" + std::to_string(port);
}

struct Endpoint {
    int *sock = nullptr;
    sockaddr_storage addr{};
    socklen_t addrlen = 0;
};

int32_t endpoint_reply(void *user, const char *data, int32_t n) {
    auto *e = static_cast<Endpoint *>(user);
    if (!e || !e->sock || *e->sock < 0) return 0;
    int r = sendto(*e->sock, data, n, 0,
                   reinterpret_cast<sockaddr *>(&e->addr), e->addrlen);
    return r < 0 ? 0 : r;
}

int32_t client_sendfn(void *user, const char *data, int32_t n, void *addr) {
    int fd = *static_cast<int *>(user);
    auto *sa = static_cast<sockaddr *>(addr);
    socklen_t len = (sa->sa_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    int r = sendto(fd, data, n, 0, sa, len);
    return r < 0 ? 0 : r;
}

struct Peer {
    std::string group;
    std::string user;
    std::string address;
    Endpoint *ep = nullptr;
    aoo::isink::pointer sink;
    float gain = 1.f;
    bool muted = false;
};

struct PendingRm {
    int seq = 0;
    std::vector<char> packet;
    sockaddr_storage addr{};
    socklen_t addrlen = 0;
    double next_retry_ms = 0;
    int tries = 0;
    int retry_delay_ms = 40;
};

struct AlsaDevice {
    std::string id;
    std::string name;
};

std::string trim_copy(std::string s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    return s.substr(i);
}

std::string read_first_line(const std::string &path) {
    std::ifstream f(path);
    std::string line;
    if (!f || !std::getline(f, line)) return {};
    return trim_copy(line);
}

std::string card_display_name(int card) {
    const std::string base = "/sys/class/sound/card" + std::to_string(card);
    std::string product = read_first_line(base + "/device/../product");
    if (!product.empty()) return product;
    std::string id = read_first_line(base + "/id");
    if (!id.empty()) return id;
    return "USB audio";
}

std::vector<AlsaDevice> list_alsa_devices() {
    std::vector<AlsaDevice> out;
    std::ifstream f("/proc/asound/cards");
    std::string line;
    while (std::getline(f, line)) {
        const char *s = line.c_str();
        while (*s == ' ' || *s == '\t') ++s;
        if (*s < '0' || *s > '9') continue;
        int idx = -1;
        char name[128] = {0};
        char rest[256] = {0};
        if (sscanf(s, "%d [%127[^]]]: %255[^\n]", &idx, name, rest) < 2) continue;
        AlsaDevice d;
        d.id = "hw:" + std::to_string(idx) + ",0";
        d.name = card_display_name(idx);
        std::string extra = trim_copy(rest);
        auto dash = extra.rfind(" - ");
        if (dash != std::string::npos) {
            std::string prod = trim_copy(extra.substr(dash + 3));
            if (!prod.empty()) d.name = prod;
        }
        if (d.name.find("Dummy") != std::string::npos) continue;
        out.push_back(d);
    }
    if (!out.empty()) return out;

    DIR *dir = opendir("/dev/snd");
    if (!dir) return out;
    while (dirent *ent = readdir(dir)) {
        int card = -1, dev = -1;
        char kind = 0;
        if (sscanf(ent->d_name, "pcmC%dD%d%c", &card, &dev, &kind) != 3 || kind != 'p')
            continue;
        AlsaDevice d;
        d.id = "hw:" + std::to_string(card) + "," + std::to_string(dev);
        d.name = card_display_name(card);
        out.push_back(d);
    }
    closedir(dir);
    return out;
}

std::string first_usb_device() {
    for (const auto &d : list_alsa_devices()) {
        if (d.name.find("USB") != std::string::npos || d.name.find("usb") != std::string::npos)
            return d.id;
    }
    auto all = list_alsa_devices();
    return all.empty() ? std::string() : all.front().id;
}

class AlsaOut {
public:
    ~AlsaOut() { close(); }

    bool try_open(const std::string &device, int want_ch, std::string &err) {
        close();
        int e = snd_pcm_open(&pcm_, device.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
        if (e < 0) {
            err = device + ": " + snd_strerror(e);
            return false;
        }
        const snd_pcm_format_t formats[] = {
            SND_PCM_FORMAT_S16_LE,
            SND_PCM_FORMAT_S32_LE,
            SND_PCM_FORMAT_S24_LE,
            SND_PCM_FORMAT_S24_3LE
        };
        std::string last = "no supported ALSA format";
        for (auto fmt : formats) {
            snd_pcm_hw_params_t *hw = nullptr;
            snd_pcm_hw_params_alloca(&hw);
            if (snd_pcm_hw_params_any(pcm_, hw) < 0) continue;
            if (snd_pcm_hw_params_set_access(pcm_, hw, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
                continue;
            if (snd_pcm_hw_params_set_format(pcm_, hw, fmt) < 0) continue;
            unsigned int rate = kSampleRate;
            if (snd_pcm_hw_params_set_rate(pcm_, hw, rate, 0) < 0) continue;
            unsigned int ch = static_cast<unsigned int>(want_ch);
            if (snd_pcm_hw_params_set_channels(pcm_, hw, ch) < 0) continue;
            snd_pcm_uframes_t period = kBlockFrames;
            snd_pcm_hw_params_set_period_size_near(pcm_, hw, &period, nullptr);
            snd_pcm_uframes_t buf = period * 4;
            snd_pcm_hw_params_set_buffer_size_near(pcm_, hw, &buf);
            e = snd_pcm_hw_params(pcm_, hw);
            if (e < 0) {
                last = snd_strerror(e);
                continue;
            }
            format_ = fmt;
            rate_ = rate;
            channels_ = static_cast<int>(ch);
            period_ = period;
            device_ = device;
            snd_pcm_prepare(pcm_);
            return true;
        }
        err = last;
        close();
        return false;
    }

    bool open(const std::string &device, std::string &err) {
        if (try_open(device, 2, err)) return true;
        if (device.rfind("hw:", 0) == 0) {
            if (try_open("plughw:" + device.substr(3), 2, err)) return true;
        }
        if (try_open(device, 1, err)) return true;
        return false;
    }

    void close() {
        if (pcm_) {
            snd_pcm_drop(pcm_);
            snd_pcm_close(pcm_);
            pcm_ = nullptr;
        }
    }

    bool write_s16(const int16_t *interleaved, int frames) {
        if (!pcm_) return false;
        const void *ptr = interleaved;
        const int n = frames * channels_;
        if (format_ == SND_PCM_FORMAT_S32_LE || format_ == SND_PCM_FORMAT_S24_LE) {
            conv32_.resize(static_cast<size_t>(n));
            const int shift = (format_ == SND_PCM_FORMAT_S32_LE) ? 16 : 8;
            for (int i = 0; i < n; ++i)
                conv32_[static_cast<size_t>(i)] = static_cast<int32_t>(interleaved[i]) << shift;
            ptr = conv32_.data();
        } else if (format_ == SND_PCM_FORMAT_S24_3LE) {
            conv24_.resize(static_cast<size_t>(n) * 3);
            for (int i = 0; i < n; ++i) {
                const int32_t v = static_cast<int32_t>(interleaved[i]) << 8;
                conv24_[static_cast<size_t>(i) * 3 + 0] = static_cast<uint8_t>(v);
                conv24_[static_cast<size_t>(i) * 3 + 1] = static_cast<uint8_t>(v >> 8);
                conv24_[static_cast<size_t>(i) * 3 + 2] = static_cast<uint8_t>(v >> 16);
            }
            ptr = conv24_.data();
        }
        snd_pcm_sframes_t w = snd_pcm_writei(pcm_, ptr, frames);
        if (w == -EPIPE) {
            snd_pcm_prepare(pcm_);
            w = snd_pcm_writei(pcm_, ptr, frames);
        } else if (w == -ESTRPIPE) {
            while ((w = snd_pcm_resume(pcm_)) == -EAGAIN) usleep(10000);
            if (w < 0) snd_pcm_prepare(pcm_);
            w = snd_pcm_writei(pcm_, ptr, frames);
        }
        return w >= 0;
    }

    int channels() const { return channels_; }
    const std::string &device() const { return device_; }
    bool ok() const { return pcm_ != nullptr; }

private:
    snd_pcm_t *pcm_ = nullptr;
    snd_pcm_format_t format_ = SND_PCM_FORMAT_S16_LE;
    unsigned int rate_ = kSampleRate;
    int channels_ = 2;
    snd_pcm_uframes_t period_ = kBlockFrames;
    std::string device_;
    std::vector<int32_t> conv32_;
    std::vector<uint8_t> conv24_;
};

class Daemon {
public:
    explicit Daemon(int udp_port) : udp_port_(udp_port) {
        aoo_initialize();
    }

    ~Daemon() {
        stop();
        client_.reset();
    }

    void set_volume(float v) { volume_.store(std::clamp(v, 0.f, 2.f)); }
    float volume() const { return volume_.load(); }

    void set_allow_remote_mix(bool on) {
        allow_remote_mix_.store(on);
        if (!on) {
            std::lock_guard<std::mutex> lk(rm_mu_);
            controller_.clear();
            controller_until_ms_ = 0;
        }
    }

    bool set_device(const std::string &dev, std::string &err) {
        std::string want = dev;
        if (want.empty() || want == "auto" || want == "default")
            want = first_usb_device();
        if (want.empty() || want == "default") {
            err = "no ALSA playback device";
            std::lock_guard<std::mutex> lk(alsa_mu_);
            alsa_.close();
            device_ = "auto";
            return false;
        }
        std::lock_guard<std::mutex> lk(alsa_mu_);
        bool ok = alsa_.open(want, err);
        device_ = want;
        return ok;
    }

    std::string connect(const std::string &server, int port,
                        const std::string &user, const std::string &user_pwd,
                        const std::string &group, const std::string &group_pwd) {
        std::lock_guard<std::mutex> lk(net_mu_);
        disconnect_locked();

        server_ = server.empty() ? kDefaultServer : server;
        server_port_ = port > 0 ? port : kDefaultServerPort;
        username_ = user.empty() ? "Slate7" : user;
        user_pwd_ = user_pwd;
        group_ = group;
        group_pwd_ = group_pwd;

        if (group_.empty()) return "group name is required";
        if (!ensure_udp()) return "failed to bind UDP port " + std::to_string(udp_port_);

        client_.reset(aoo::net::iclient::create(&udp_fd_, client_sendfn, udp_port_));
        if (!client_) return "failed to create AOO client";

        running_ = true;
        last_error_.clear();
        state_ = "connecting";
        client_thread_ = std::thread([this] { client_->run(); });
        recv_thread_ = std::thread([this] { recv_loop(); });
        send_thread_ = std::thread([this] { send_loop(); });
        audio_thread_ = std::thread([this] { audio_loop(); });

        int rc = client_->connect(server_.c_str(), server_port_,
                                  username_.c_str(), user_pwd_.c_str());
        if (rc < 0) return "connect() failed";
        write_status();
        return {};
    }

    void disconnect() {
        std::lock_guard<std::mutex> lk(net_mu_);
        disconnect_locked();
        write_status();
    }

    std::string status_json() {
        std::lock_guard<std::mutex> lk(peer_mu_);
        auto devices = list_alsa_devices();
        std::string controller;
        {
            std::lock_guard<std::mutex> rlk(rm_mu_);
            if (!controller_.empty() && now_ms() < controller_until_ms_)
                controller = controller_;
            else
                controller_.clear();
        }
        std::ostringstream o;
        o << "{\"ok\":true"
          << ",\"state\":\"" << json_escape(state_) << "\""
          << ",\"error\":\"" << json_escape(last_error_) << "\""
          << ",\"server\":\"" << json_escape(server_) << "\""
          << ",\"port\":" << server_port_
          << ",\"udp_port\":" << udp_port_
          << ",\"group\":\"" << json_escape(group_) << "\""
          << ",\"username\":\"" << json_escape(username_) << "\""
          << ",\"device\":\"" << json_escape(device_) << "\""
          << ",\"alsa_ok\":" << (alsa_.ok() ? "true" : "false")
          << ",\"alsa_channels\":" << (alsa_.ok() ? alsa_.channels() : 0)
          << ",\"src_channels\":" << src_channels_.load()
          << ",\"volume\":" << volume_.load()
          << ",\"allow_remote_mix\":" << (allow_remote_mix_.load() ? "true" : "false")
          << ",\"controller\":\"" << json_escape(controller) << "\""
          << ",\"levels\":" << levels_json_object()
          << ",\"peers\":[";
        for (size_t i = 0; i < peers_.size(); ++i) {
            if (i) o << ",";
            o << "{\"user\":\"" << json_escape(peers_[i].user)
              << "\",\"group\":\"" << json_escape(peers_[i].group)
              << "\",\"address\":\"" << json_escape(peers_[i].address)
              << "\",\"gain\":" << peers_[i].gain
              << ",\"mute\":" << (peers_[i].muted ? "true" : "false") << "}";
        }
        o << "],\"peer_count\":" << peers_.size()
          << ",\"encrypted\":false"
          << ",\"devices\":[";
        for (size_t i = 0; i < devices.size(); ++i) {
            if (i) o << ",";
            o << "{\"id\":\"" << json_escape(devices[i].id)
              << "\",\"name\":\"" << json_escape(devices[i].name) << "\"}";
        }
        o << "]}";
        return o.str();
    }

    void write_status() {
        auto json = status_json();
        std::string tmp = std::string(kStatusPath) + ".tmp";
        std::ofstream f(tmp);
        if (f) {
            f << json << "\n";
            f.close();
            rename(tmp.c_str(), kStatusPath);
        }
    }

    std::string levels_json() const {
        return std::string("{\"ok\":true,\"levels\":") + levels_json_object() + "}";
    }

    void write_levels() {
        auto json = levels_json();
        std::string tmp = std::string(kLevelsPath) + ".tmp";
        std::ofstream f(tmp);
        if (f) {
            f << json << "\n";
            f.close();
            rename(tmp.c_str(), kLevelsPath);
        }
    }

    void stop() { disconnect(); }

private:
    bool ensure_udp() {
        if (udp_fd_ >= 0) return true;
        udp_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_fd_ < 0) return false;
        int yes = 1;
        setsockopt(udp_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        int buf = 256 * 1024;
        setsockopt(udp_fd_, SOL_SOCKET, SO_RCVBUF, &buf, sizeof(buf));
        setsockopt(udp_fd_, SOL_SOCKET, SO_SNDBUF, &buf, sizeof(buf));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(static_cast<uint16_t>(udp_port_));
        if (bind(udp_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
            close(udp_fd_);
            udp_fd_ = -1;
            return false;
        }
        return true;
    }

    void disconnect_locked() {
        running_ = false;
        send_cv_.notify_all();
        if (client_) client_->quit();
        if (udp_fd_ >= 0) {
            int fd = udp_fd_;
            sockaddr_in self{};
            self.sin_family = AF_INET;
            self.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            self.sin_port = htons(static_cast<uint16_t>(udp_port_));
            sendto(fd, "", 0, 0, reinterpret_cast<sockaddr *>(&self), sizeof(self));
        }
        if (client_thread_.joinable()) client_thread_.join();
        if (recv_thread_.joinable()) recv_thread_.join();
        if (send_thread_.joinable()) send_thread_.join();
        if (audio_thread_.joinable()) audio_thread_.join();
        {
            std::lock_guard<std::mutex> lk(aoo_mu_);
            std::lock_guard<std::mutex> plk(peer_mu_);
            for (auto &p : peers_) {
                if (p.sink) p.sink->uninvite_all();
                p.sink.reset();
            }
            peers_.clear();
            pending_mix_.clear();
            master_gain_ = 1.f;
            master_mute_ = false;
        }
        client_.reset();
        {
            std::lock_guard<std::mutex> lk(ep_mu_);
            endpoints_.clear();
        }
        {
            std::lock_guard<std::mutex> lk(rm_mu_);
            pending_.clear();
            last_applied_.clear();
            last_live_.clear();
            controller_.clear();
        }
        src_channels_.store(0);
        reset_meter();
        write_levels();
        state_ = "idle";
    }

    Endpoint *endpoint_for(const sockaddr *sa, socklen_t len) {
        auto key = sockaddr_key(sa, len);
        std::lock_guard<std::mutex> lk(ep_mu_);
        auto it = endpoints_.find(key);
        if (it != endpoints_.end()) return it->second.get();
        auto ep = std::make_unique<Endpoint>();
        ep->sock = &udp_fd_;
        memcpy(&ep->addr, sa, len);
        ep->addrlen = len;
        Endpoint *raw = ep.get();
        endpoints_[key] = std::move(ep);
        return raw;
    }

    void invite_sources(Peer &p) {
        if (!p.ep || !p.sink) return;
        for (int32_t id = 0; id <= 8; ++id)
            p.sink->invite_source(p.ep, id, endpoint_reply);
    }

    void uninvite_sources(Peer &p) {
        if (!p.ep || !p.sink) return;
        for (int32_t id = 0; id <= 8; ++id)
            p.sink->uninvite_source(p.ep, id, endpoint_reply);
    }

    void apply_mix_to_peer(Peer &p, float gain, bool muted) {
        p.gain = std::clamp(gain, 0.f, 4.f);
        const bool was = p.muted;
        p.muted = muted;
        if (muted && !was) uninvite_sources(p);
        else if (!muted && was) invite_sources(p);
    }

    void apply_master_mix(float gain, bool muted) {
        master_gain_ = std::clamp(gain, 0.f, 4.f);
        const bool was = master_mute_;
        master_mute_ = muted;
        if (muted && !was) {
            for (auto &p : peers_) uninvite_sources(p);
        } else if (!muted && was) {
            for (auto &p : peers_)
                if (!p.muted) invite_sources(p);
        }
    }

    // PEER_JOIN's advertised address often differs from the UDP source after
    // hole-punch. Re-bind the matching sink so invites and audio use the same endpoint.
    void bind_peer_endpoint(Endpoint *ep, int32_t packet_sink_id) {
        if (!ep) return;
        for (auto &p : peers_) {
            if (!p.sink || p.ep == ep) continue;
            int32_t sid = 0;
            if (p.sink->get_id(sid) <= 0) continue;
            if (packet_sink_id == sid
                || (packet_sink_id == AOO_ID_NONE && peers_.size() == 1)) {
                p.ep = ep;
                invite_sources(p);
                return;
            }
        }
        if (peers_.size() == 1 && peers_[0].sink && peers_[0].ep != ep) {
            peers_[0].ep = ep;
            invite_sources(peers_[0]);
        }
    }

    void recv_loop() {
        char buf[AOO_MAXPACKETSIZE];
        while (running_) {
            pollfd pfd{udp_fd_, POLLIN, 0};
            int pr = poll(&pfd, 1, 250);
            if (pr <= 0) continue;
            sockaddr_storage ss{};
            socklen_t slen = sizeof(ss);
            int n = recvfrom(udp_fd_, buf, sizeof(buf), 0,
                             reinterpret_cast<sockaddr *>(&ss), &slen);
            if (n <= 0) continue;

            int32_t type = 0, id = 0;
            Endpoint *ep = endpoint_for(reinterpret_cast<sockaddr *>(&ss), slen);

            std::lock_guard<std::mutex> lk(aoo_mu_);
            if (aoo_parse_pattern(buf, n, &type, &id) > 0) {
                if (type == AOO_TYPE_SINK) {
                    std::lock_guard<std::mutex> plk(peer_mu_);
                    bind_peer_endpoint(ep, id);
                    for (auto &p : peers_) {
                        if (p.sink)
                            p.sink->handle_message(buf, n, ep, endpoint_reply);
                    }
                    send_cv_.notify_one();
                }
            } else if (aoonet_parse_pattern(buf, n, &type) > 0) {
                if (client_ && (type == AOO_TYPE_CLIENT || type == AOO_TYPE_PEER)) {
                    client_->handle_message(buf, n, &ss);
                    send_cv_.notify_one();
                }
            } else if (n >= 3 && memcmp(buf, "/sb", 3) == 0) {
                handle_sb(buf, n, ep);
            }
            drain_events_locked();
        }
    }

    void send_loop() {
        while (running_) {
            {
                std::unique_lock<std::mutex> lk(send_mu_);
                send_cv_.wait_for(lk, std::chrono::milliseconds(20));
            }
            {
                std::lock_guard<std::mutex> lk(aoo_mu_);
                if (client_) client_->send();
                {
                    std::lock_guard<std::mutex> plk(peer_mu_);
                    for (auto &p : peers_) {
                        if (p.sink) while (p.sink->send()) {}
                    }
                }
                drain_events_locked();
            }
            process_rm_retries();
        }
    }

    void audio_loop() {
        std::vector<float> mix0(kBlockFrames), mix1(kBlockFrames);
        std::vector<float> t0(kBlockFrames), t1(kBlockFrames);
        float *tplanes[2] = {t0.data(), t1.data()};
        std::vector<int16_t> interleaved(kBlockFrames * 2);

        while (running_) {
            std::fill(mix0.begin(), mix0.end(), 0.f);
            std::fill(mix1.begin(), mix1.end(), 0.f);
            uint64_t t = aoo_osctime_get();
            int src_ch = 0;
            {
                std::lock_guard<std::mutex> lk(aoo_mu_);
                std::lock_guard<std::mutex> plk(peer_mu_);
                for (auto &p : peers_) {
                    if (!p.sink || p.muted || master_mute_) continue;
                    std::fill(t0.begin(), t0.end(), 0.f);
                    std::fill(t1.begin(), t1.end(), 0.f);
                    p.sink->process(tplanes, kBlockFrames, t);
                    const float g = p.gain * master_gain_;
                    for (int i = 0; i < kBlockFrames; ++i) {
                        mix0[i] += t0[i] * g;
                        mix1[i] += t1[i] * g;
                    }
                    aoo_format_storage fs{};
                    if (p.ep && p.sink->get_source_format(p.ep, 0, fs) > 0)
                        src_ch = std::max(src_ch, fs.header.nchannels);
                }
            }
            if (src_ch > 0) src_channels_.store(src_ch);

            float peak_l = 0.f, peak_r = 0.f;
            for (int i = 0; i < kBlockFrames; ++i) {
                peak_l = std::max(peak_l, std::fabs(mix0[i]));
                peak_r = std::max(peak_r, std::fabs(mix1[i]));
            }
            const bool mono_src = src_ch == 1 || (peak_l > 1e-5f && peak_r < 1e-6f);
            const bool mono_r = src_ch != 2 && peak_r > 1e-5f && peak_l < 1e-6f;
            if (mono_src) {
                mix1 = mix0;
                peak_r = peak_l;
            } else if (mono_r) {
                mix0 = mix1;
                peak_l = peak_r;
            }
            float vol = volume_.load();
            int out_ch = 2;
            {
                std::lock_guard<std::mutex> lk(alsa_mu_);
                out_ch = alsa_.ok() ? alsa_.channels() : 2;
            }
            float max_l = 0.f, max_r = 0.f;
            double sum_l = 0.0, sum_r = 0.0;
            if (out_ch == 1) {
                interleaved.resize(kBlockFrames);
                for (int i = 0; i < kBlockFrames; ++i) {
                    float m = (mix0[i] + mix1[i]) * 0.5f * vol;
                    m = std::clamp(m, -1.f, 1.f);
                    float a = std::fabs(m);
                    max_l = std::max(max_l, a);
                    max_r = max_l;
                    sum_l += static_cast<double>(m) * m;
                    interleaved[i] = static_cast<int16_t>(m * 32767.f);
                }
                sum_r = sum_l;
            } else {
                interleaved.resize(kBlockFrames * 2);
                for (int i = 0; i < kBlockFrames; ++i) {
                    float l = std::clamp(mix0[i] * vol, -1.f, 1.f);
                    float r = std::clamp(mix1[i] * vol, -1.f, 1.f);
                    float al = std::fabs(l), ar = std::fabs(r);
                    max_l = std::max(max_l, al);
                    max_r = std::max(max_r, ar);
                    sum_l += static_cast<double>(l) * l;
                    sum_r += static_cast<double>(r) * r;
                    interleaved[i * 2] = static_cast<int16_t>(l * 32767.f);
                    interleaved[i * 2 + 1] = static_cast<int16_t>(r * 32767.f);
                }
            }
            note_meter(max_l, max_r,
                       static_cast<float>(sum_l / kBlockFrames),
                       static_cast<float>(sum_r / kBlockFrames));
            if ((++meter_blocks_ % 8) == 0) write_levels();
            {
                std::lock_guard<std::mutex> lk(alsa_mu_);
                if (alsa_.ok()) {
                    alsa_.write_s16(interleaved.data(), kBlockFrames);
                } else {
                    usleep(static_cast<useconds_t>(kBlockFrames * 1000000 / kSampleRate));
                }
            }
        }
    }

    void drain_events_locked() {
        if (client_ && client_->events_available())
            client_->handle_events(&Daemon::client_events_thunk, this);
        std::vector<aoo::isink *> sinks;
        {
            std::lock_guard<std::mutex> plk(peer_mu_);
            for (auto &p : peers_) {
                if (p.sink) sinks.push_back(p.sink.get());
            }
        }
        for (auto *s : sinks) {
            if (s && s->events_available())
                s->handle_events(&Daemon::sink_events_thunk, this);
        }
    }

    static int32_t client_events_thunk(void *user, const aoo_event **ev, int32_t n) {
        return static_cast<Daemon *>(user)->on_client_events(ev, n);
    }
    static int32_t sink_events_thunk(void *user, const aoo_event **ev, int32_t n) {
        return static_cast<Daemon *>(user)->on_sink_events(ev, n);
    }

    int32_t on_client_events(const aoo_event **events, int32_t n) {
        for (int32_t i = 0; i < n; ++i) {
            switch (events[i]->type) {
            case AOONET_CLIENT_CONNECT_EVENT: {
                auto *e = reinterpret_cast<const aoonet_client_event *>(events[i]);
                if (e->result > 0) {
                    state_ = "connected";
                    last_error_.clear();
                    if (client_) client_->group_join(group_.c_str(), group_pwd_.c_str(), false);
                } else {
                    state_ = "error";
                    last_error_ = e->errormsg ? e->errormsg : "connect failed";
                }
                break;
            }
            case AOONET_CLIENT_DISCONNECT_EVENT: {
                auto *e = reinterpret_cast<const aoonet_client_event *>(events[i]);
                state_ = "idle";
                if (e->result == 0 && e->errormsg) last_error_ = e->errormsg;
                std::lock_guard<std::mutex> lk(peer_mu_);
                for (auto &p : peers_) {
                    if (p.sink) p.sink->uninvite_all();
                    p.sink.reset();
                }
                peers_.clear();
                break;
            }
            case AOONET_CLIENT_GROUP_JOIN_EVENT: {
                auto *e = reinterpret_cast<const aoonet_client_group_event *>(events[i]);
                if (e->result > 0) {
                    state_ = "in_group";
                    last_error_.clear();
                } else {
                    state_ = "error";
                    last_error_ = e->errormsg ? e->errormsg : "group join failed";
                }
                break;
            }
            case AOONET_CLIENT_PEER_JOIN_EVENT: {
                auto *e = reinterpret_cast<const aoonet_client_peer_event *>(events[i]);
                if (e->result > 0 && e->address) {
                    auto *sa = static_cast<const sockaddr *>(e->address);
                    Endpoint *ep = endpoint_for(sa, static_cast<socklen_t>(e->length));
                    Peer p;
                    p.group = e->group ? e->group : "";
                    p.user = e->user ? e->user : "";
                    p.address = sockaddr_key(sa, static_cast<socklen_t>(e->length));
                    p.ep = ep;
                    p.sink.reset(aoo::isink::create(next_sink_id_++));
                    if (p.sink) {
                        p.sink->setup(kSampleRate, kBlockFrames, kChannels);
                        p.sink->set_buffersize(kSinkBufMs);
                        invite_sources(p);
                    }
                    {
                        std::lock_guard<std::mutex> lk(peer_mu_);
                        peers_.erase(std::remove_if(peers_.begin(), peers_.end(),
                                                    [&](Peer &x) { return x.user == p.user; }),
                                     peers_.end());
                        peers_.push_back(std::move(p));
                        auto &np = peers_.back();
                        for (auto it = pending_mix_.begin(); it != pending_mix_.end(); ) {
                            if (users_equal(it->first, np.user)) {
                                apply_mix_to_peer(np, it->second.first, it->second.second);
                                it = pending_mix_.erase(it);
                            } else {
                                ++it;
                            }
                        }
                    }
                    send_cv_.notify_one();
                }
                break;
            }
            case AOONET_CLIENT_PEER_LEAVE_EVENT: {
                auto *e = reinterpret_cast<const aoonet_client_peer_event *>(events[i]);
                std::string user = e->user ? e->user : "";
                std::lock_guard<std::mutex> lk(peer_mu_);
                for (auto &p : peers_) {
                    if (p.user == user && p.sink) p.sink->uninvite_all();
                }
                peers_.erase(std::remove_if(peers_.begin(), peers_.end(),
                                            [&](const Peer &x) { return x.user == user; }),
                             peers_.end());
                break;
            }
            case AOONET_CLIENT_ERROR_EVENT: {
                auto *e = reinterpret_cast<const aoonet_client_event *>(events[i]);
                if (e->errormsg) last_error_ = e->errormsg;
                break;
            }
            default:
                break;
            }
        }
        write_status();
        return 1;
    }

    int32_t on_sink_events(const aoo_event **events, int32_t n) {
        for (int32_t i = 0; i < n; ++i) {
            if (events[i]->type == AOO_SOURCE_ADD_EVENT) {
                if (state_ == "in_group") state_ = "receiving";
            } else if (events[i]->type == AOO_SOURCE_FORMAT_EVENT) {
                auto *e = reinterpret_cast<const aoo_source_event *>(events[i]);
                std::lock_guard<std::mutex> lk(peer_mu_);
                for (auto &p : peers_) {
                    if (!p.sink || !e) continue;
                    aoo_format_storage fs{};
                    if (p.sink->get_source_format(e->endpoint, e->id, fs) > 0) {
                        if (fs.header.nchannels > src_channels_.load())
                            src_channels_.store(fs.header.nchannels);
                    }
                }
            }
        }
        write_status();
        return 1;
    }

    std::string levels_json_object() const {
        char buf[160];
        std::snprintf(buf, sizeof(buf),
                      "{\"peak_l\":%.4f,\"peak_r\":%.4f,\"rms_l\":%.4f,\"rms_r\":%.4f,\"clip\":%s}",
                      peak_l_.load(), peak_r_.load(), rms_l_.load(), rms_r_.load(),
                      clip_.load() ? "true" : "false");
        return buf;
    }

    void reset_meter() {
        peak_l_.store(0.f);
        peak_r_.store(0.f);
        rms_l_.store(0.f);
        rms_r_.store(0.f);
        clip_.store(false);
        meter_blocks_ = 0;
    }

    void note_meter(float peak_l, float peak_r, float ms_l, float ms_r) {
        constexpr float decay = 0.91f;
        constexpr float rms_a = 0.18f;
        peak_l_.store(std::max(peak_l, peak_l_.load() * decay));
        peak_r_.store(std::max(peak_r, peak_r_.load() * decay));
        rms_l_.store(rms_l_.load() * (1.f - rms_a) + std::sqrt(std::max(0.f, ms_l)) * rms_a);
        rms_r_.store(rms_r_.load() * (1.f - rms_a) + std::sqrt(std::max(0.f, ms_r)) * rms_a);
        if (peak_l >= 0.99f || peak_r >= 0.99f) clip_.store(true);
        else if (peak_l_.load() < 0.7f && peak_r_.load() < 0.7f) clip_.store(false);
    }

    // --- remote mix target ------------------------------------------------

    void osc_send(Endpoint *ep, const char *data, int n) {
        if (!ep) return;
        endpoint_reply(ep, data, n);
    }

    void send_rm_ack(Endpoint *ep, int seq) {
        char buf[AOO_MAXPACKETSIZE];
        osc::OutboundPacketStream msg(buf, sizeof(buf));
        try {
            msg << osc::BeginMessage("/sb/rmack")
                << kRmProtoVer << seq << username_.c_str()
                << osc::EndMessage;
            osc_send(ep, msg.Data(), msg.Size());
        } catch (const osc::Exception &) {}
    }

    void send_rm_nack(Endpoint *ep, int seq, int reason) {
        char buf[AOO_MAXPACKETSIZE];
        osc::OutboundPacketStream msg(buf, sizeof(buf));
        try {
            msg << osc::BeginMessage("/sb/rmnack")
                << kRmProtoVer << seq << username_.c_str() << reason
                << osc::EndMessage;
            osc_send(ep, msg.Data(), msg.Size());
        } catch (const osc::Exception &) {}
    }

    std::string mix_snapshot_json() {
        std::lock_guard<std::mutex> lk(peer_mu_);
        std::ostringstream o;
        o << "{\"sources\":[";
        o << "{\"id\":\"peer:" << json_escape(username_)
          << "\",\"name\":\"" << json_escape(username_)
          << "\",\"kind\":\"peer\",\"gain\":" << master_gain_
          << ",\"mute\":" << (master_mute_ ? "true" : "false") << "}";
        for (size_t i = 0; i < peers_.size(); ++i) {
            o << ",";
            o << "{\"id\":\"peer:" << json_escape(peers_[i].user)
              << "\",\"name\":\"" << json_escape(peers_[i].user)
              << "\",\"kind\":\"peer\",\"gain\":" << peers_[i].gain
              << ",\"mute\":" << (peers_[i].muted ? "true" : "false") << "}";
        }
        o << "]}";
        return o.str();
    }

    void queue_rm_state(Endpoint *ep) {
        if (!ep) return;
        const std::string json = mix_snapshot_json();
        const int max_chunk = std::max(256, AOO_MAXPACKETSIZE - 400);
        const int nchunks = std::max(1, (static_cast<int>(json.size()) + max_chunk - 1) / max_chunk);
        const int seq = next_rm_seq_++;
        for (int c = 0; c < nchunks; ++c) {
            const int start = c * max_chunk;
            const int len = std::min(max_chunk, static_cast<int>(json.size()) - start);
            std::string part = json.substr(static_cast<size_t>(start), static_cast<size_t>(len));
            char buf[AOO_MAXPACKETSIZE];
            osc::OutboundPacketStream msg(buf, sizeof(buf));
            try {
                msg << osc::BeginMessage("/sb/rmstate")
                    << kRmProtoVer << seq << username_.c_str()
                    << c << nchunks << part.c_str()
                    << osc::EndMessage;
                osc_send(ep, msg.Data(), msg.Size());
                PendingRm item;
                item.seq = seq;
                item.packet.assign(msg.Data(), msg.Data() + msg.Size());
                item.addr = ep->addr;
                item.addrlen = ep->addrlen;
                item.next_retry_ms = now_ms() + 40.0;
                item.tries = 0;
                item.retry_delay_ms = 40;
                std::lock_guard<std::mutex> lk(rm_mu_);
                pending_.push_back(std::move(item));
            } catch (const osc::Exception &) {}
        }
    }

    void process_rm_retries() {
        const double t = now_ms();
        std::lock_guard<std::mutex> lk(rm_mu_);
        for (auto it = pending_.begin(); it != pending_.end();) {
            if (t < it->next_retry_ms) {
                ++it;
                continue;
            }
            if (it->tries >= kRmMaxRetries) {
                it = pending_.erase(it);
                continue;
            }
            if (udp_fd_ >= 0) {
                sendto(udp_fd_, it->packet.data(), static_cast<int>(it->packet.size()), 0,
                       reinterpret_cast<sockaddr *>(&it->addr), it->addrlen);
            }
            ++it->tries;
            it->retry_delay_ms = std::min(320, it->retry_delay_ms * 2);
            it->next_retry_ms = t + it->retry_delay_ms;
            ++it;
        }
    }

    bool apply_named_or_remember(const std::string &user, float gain, bool muted) {
        for (auto &p : peers_) {
            if (!users_equal(p.user, user)) continue;
            apply_mix_to_peer(p, gain, muted);
            pending_mix_.erase(user);
            return true;
        }
        pending_mix_[user] = {gain, muted};
        return true;
    }

    bool apply_remote_mix(const std::string &source_id, const std::string &from,
                          float gain, bool muted) {
        std::lock_guard<std::mutex> lk(peer_mu_);
        if (source_id.rfind("local:", 0) == 0
            || source_id == "file"
            || source_id == "soundboard") {
            if (from.empty()) return false;
            return apply_named_or_remember(from, gain, muted);
        }
        if (source_id.rfind("peer:", 0) != 0) return false;
        const std::string user = source_id.substr(5);
        if (user.empty()) return false;
        if (users_equal(user, username_)) {
            apply_master_mix(gain, muted);
            return true;
        }
        return apply_named_or_remember(user, gain, muted);
    }

    void note_controller(const std::string &from) {
        std::lock_guard<std::mutex> lk(rm_mu_);
        controller_ = from;
        controller_until_ms_ = now_ms() + kRmControllerHoldMs;
    }

    void handle_sb(const char *data, int n, Endpoint *ep) {
        try {
            osc::ReceivedPacket packet(data, n);
            if (!packet.IsMessage()) return;
            osc::ReceivedMessage message(packet);
            const char *addr = message.AddressPattern();
            auto it = message.ArgumentsBegin();

            if (!strcmp(addr, "/sb/rmset") || !strcmp(addr, "/sb/rmlive")) {
                const bool live = !strcmp(addr, "/sb/rmlive");
                const int ver = (it++)->AsInt32();
                const int seq = (it++)->AsInt32();
                const std::string from = (it++)->AsString();
                const std::string source_id = (it++)->AsString();
                const float gain = osc_as_float(it);
                const int flags = (it++)->AsInt32();
                const bool muted = (flags & kRmMuteFlag) != 0;

                if (ver != kRmProtoVer) {
                    if (!live) send_rm_nack(ep, seq, kRmNackBadVer);
                    return;
                }
                if (!allow_remote_mix_.load()) {
                    if (!live) send_rm_nack(ep, seq, kRmNackDenied);
                    return;
                }

                bool apply = true;
                {
                    std::lock_guard<std::mutex> lk(rm_mu_);
                    auto &last_map = live ? last_live_ : last_applied_;
                    const std::string key = from + "\n" + source_id;
                    auto found = last_map.find(key);
                    if (found != last_map.end() && seq <= found->second)
                        apply = false;
                    else
                        last_map[key] = seq;
                }

                if (apply) {
                    if (!apply_remote_mix(source_id, from, gain, muted)) {
                        syslog(LOG_WARNING, "netlayd: unknown mix source %s from %s",
                               source_id.c_str(), from.c_str());
                        if (!live) send_rm_nack(ep, seq, kRmNackUnknown);
                        return;
                    }
                    syslog(LOG_INFO, "netlayd: mix %s gain=%.3f mute=%d from %s",
                           source_id.c_str(), gain, muted ? 1 : 0, from.c_str());
                    note_controller(from);
                }
                if (!live) send_rm_ack(ep, seq);
            } else if (!strcmp(addr, "/sb/rmquery")) {
                const int ver = (it++)->AsInt32();
                const int seq = (it++)->AsInt32();
                (void)(it++)->AsString();
                if (ver != kRmProtoVer) {
                    send_rm_nack(ep, seq, kRmNackBadVer);
                    return;
                }
                send_rm_ack(ep, seq);
                queue_rm_state(ep);
            } else if (!strcmp(addr, "/sb/rmack")) {
                const int ver = (it++)->AsInt32();
                const int seq = (it++)->AsInt32();
                (void)ver;
                (void)(it++)->AsString();
                std::lock_guard<std::mutex> lk(rm_mu_);
                pending_.erase(std::remove_if(pending_.begin(), pending_.end(),
                                              [&](const PendingRm &p) { return p.seq == seq; }),
                               pending_.end());
            }
        } catch (const osc::Exception &ex) {
            syslog(LOG_WARNING, "netlayd: /sb parse error: %s", ex.what());
        }
    }

    int udp_port_ = kDefaultUdpPort;
    int udp_fd_ = -1;
    int server_port_ = kDefaultServerPort;
    int next_sink_id_ = 1;
    int next_rm_seq_ = 1;
    std::string server_ = kDefaultServer;
    std::string username_ = "Slate7";
    std::string user_pwd_;
    std::string group_;
    std::string group_pwd_;
    std::string device_ = "auto";
    std::string state_ = "idle";
    std::string last_error_;

    aoo::net::iclient::pointer client_;

    std::atomic<bool> running_{false};
    std::atomic<bool> allow_remote_mix_{true};
    std::atomic<float> volume_{0.8f};
    std::atomic<float> peak_l_{0.f};
    std::atomic<float> peak_r_{0.f};
    std::atomic<float> rms_l_{0.f};
    std::atomic<float> rms_r_{0.f};
    std::atomic<bool> clip_{false};
    std::atomic<int> src_channels_{0};
    unsigned meter_blocks_{0};

    std::mutex aoo_mu_;
    std::mutex ep_mu_;
    std::mutex peer_mu_;
    std::mutex alsa_mu_;
    std::mutex net_mu_;
    std::mutex send_mu_;
    std::mutex rm_mu_;
    std::condition_variable send_cv_;

    std::map<std::string, std::unique_ptr<Endpoint>> endpoints_;
    std::vector<Peer> peers_;
    std::vector<PendingRm> pending_;
    std::map<std::string, int> last_applied_;
    std::map<std::string, int> last_live_;
    std::map<std::string, std::pair<float, bool>> pending_mix_;
    std::string controller_;
    double controller_until_ms_ = 0;
    float master_gain_ = 1.f;
    bool master_mute_ = false;
    AlsaOut alsa_;

    std::thread client_thread_;
    std::thread recv_thread_;
    std::thread send_thread_;
    std::thread audio_thread_;
};

std::atomic<bool> g_quit{false};
void on_sig(int) { g_quit = true; }

int ctl_client(const std::string &path, const std::string &cmd) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "socket: " << strerror(errno) << "\n";
        return 1;
    }
    sockaddr_un un{};
    un.sun_family = AF_UNIX;
    strncpy(un.sun_path, path.c_str(), sizeof(un.sun_path) - 1);
    if (connect(fd, reinterpret_cast<sockaddr *>(&un), sizeof(un)) < 0) {
        std::cout << "{\"ok\":false,\"state\":\"stopped\",\"error\":\"netlayd not running\",\"detail\":\""
                  << json_escape(strerror(errno)) << "\"}\n";
        close(fd);
        return 2;
    }
    std::string msg = cmd;
    if (msg.empty() || msg.back() != '\n') msg += '\n';
    if (write(fd, msg.data(), msg.size()) < 0) {
        close(fd);
        return 1;
    }
    shutdown(fd, SHUT_WR);
    char buf[8192];
    std::string resp;
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) resp.append(buf, buf + n);
    close(fd);
    std::cout << resp;
    if (!resp.empty() && resp.back() != '\n') std::cout << "\n";
    return 0;
}

std::string handle_cmd(Daemon &d, const std::string &line) {
    auto cmd = json_get(line, "cmd");
    if (cmd.empty()) {
        if (line.find("status") != std::string::npos) cmd = "status";
        else if (line.find("disconnect") != std::string::npos) cmd = "disconnect";
        else if (line.find("devices") != std::string::npos) cmd = "devices";
    }
    if (cmd == "status" || cmd.empty()) return d.status_json();
    if (cmd == "levels") return d.levels_json();
    if (cmd == "devices") {
        auto devices = list_alsa_devices();
        std::ostringstream o;
        o << "{\"ok\":true,\"devices\":[";
        for (size_t i = 0; i < devices.size(); ++i) {
            if (i) o << ",";
            o << "{\"id\":\"" << json_escape(devices[i].id)
              << "\",\"name\":\"" << json_escape(devices[i].name) << "\"}";
        }
        o << "]}";
        return o.str();
    }
    if (cmd == "volume") {
        auto v = json_get(line, "value");
        if (!v.empty()) d.set_volume(std::strtof(v.c_str(), nullptr));
        d.write_status();
        return d.status_json();
    }
    if (cmd == "allow_remote_mix") {
        auto v = json_get(line, "value");
        if (v.empty()) v = json_get(line, "allow_remote_mix");
        if (!v.empty()) d.set_allow_remote_mix(json_truthy(v));
        d.write_status();
        return d.status_json();
    }
    if (cmd == "disconnect") {
        d.disconnect();
        return d.status_json();
    }
    if (cmd == "connect") {
        auto server = json_get(line, "server");
        auto port_s = json_get(line, "port");
        auto user = json_get(line, "username");
        auto upwd = json_get(line, "user_password");
        auto group = json_get(line, "group");
        auto gpwd = json_get(line, "password");
        auto device = json_get(line, "device");
        auto vol = json_get(line, "volume");
        auto allow = json_get(line, "allow_remote_mix");
        if (!vol.empty()) d.set_volume(std::strtof(vol.c_str(), nullptr));
        if (!allow.empty()) d.set_allow_remote_mix(json_truthy(allow));
        std::string err;
        d.set_device(device.empty() ? "auto" : device, err);
        int port = port_s.empty() ? kDefaultServerPort : std::atoi(port_s.c_str());
        err = d.connect(server, port, user, upwd, group, gpwd);
        if (!err.empty())
            return std::string("{\"ok\":false,\"error\":\"") + json_escape(err) + "\"}";
        return d.status_json();
    }
    return "{\"ok\":false,\"error\":\"unknown command\"}";
}

int run_server(int udp_port, const std::string &ctl_path, const std::string &device) {
    signal(SIGINT, on_sig);
    signal(SIGTERM, on_sig);
    signal(SIGPIPE, SIG_IGN);
    openlog("netlayd", LOG_PID, LOG_DAEMON);

    mkdir("/var/run", 0755);
    unlink(ctl_path.c_str());

    Daemon d(udp_port);
    std::string err;
    if (!device.empty() && device != "auto" && device != "default")
        d.set_device(device, err);
    d.write_status();
    d.write_levels();

    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (lfd < 0) {
        std::cerr << "ctl socket: " << strerror(errno) << "\n";
        return 1;
    }
    sockaddr_un un{};
    un.sun_family = AF_UNIX;
    strncpy(un.sun_path, ctl_path.c_str(), sizeof(un.sun_path) - 1);
    if (bind(lfd, reinterpret_cast<sockaddr *>(&un), sizeof(un)) < 0) {
        std::cerr << "ctl bind: " << strerror(errno) << "\n";
        return 1;
    }
    chmod(ctl_path.c_str(), 0666);
    listen(lfd, 8);

    std::cerr << "netlayd listening on " << ctl_path
              << " udp=" << udp_port
              << " device=" << (err.empty() ? device : ("unopened:" + err))
              << "\n";

    while (!g_quit) {
        pollfd pfd{lfd, POLLIN, 0};
        int pr = poll(&pfd, 1, 250);
        if (pr <= 0) continue;
        int cfd = accept(lfd, nullptr, nullptr);
        if (cfd < 0) continue;
        std::string req;
        char buf[2048];
        while (true) {
            ssize_t n = read(cfd, buf, sizeof(buf));
            if (n <= 0) break;
            req.append(buf, buf + n);
            if (req.find('\n') != std::string::npos) break;
            if (req.size() > 8000) break;
        }
        auto resp = handle_cmd(d, req);
        resp += "\n";
        write(cfd, resp.data(), resp.size());
        close(cfd);
        d.write_status();
    }

    d.stop();
    close(lfd);
    unlink(ctl_path.c_str());
    unlink(kStatusPath);
    unlink(kLevelsPath);
    return 0;
}

void usage() {
    std::cerr
        << "netlayd — Netlay AOO listen client (USB out + remote mix target)\n"
        << "  netlayd [--udp-port N] [--socket PATH] [--device DEV]\n"
        << "  netlayd --ctl '{\"cmd\":\"status\"}'\n"
        << "  netlayd --ctl '{\"cmd\":\"connect\",\"group\":\"G\",\"username\":\"U\"}'\n";
}

} // namespace

int main(int argc, char **argv) {
    int udp = kDefaultUdpPort;
    std::string sock = kCtlPath;
    std::string device = "auto";
    std::string ctl;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto need = [&](int) {
            if (i + 1 >= argc) {
                usage();
                std::exit(1);
            }
            return std::string(argv[++i]);
        };
        if (a == "-h" || a == "--help") {
            usage();
            return 0;
        } else if (a == "--udp-port") {
            udp = std::atoi(need(i).c_str());
        } else if (a == "--socket") {
            sock = need(i);
        } else if (a == "--device") {
            device = need(i);
        } else if (a == "--ctl") {
            ctl = need(i);
        } else if (a.rfind("--ctl=", 0) == 0) {
            ctl = a.substr(6);
        } else {
            usage();
            return 1;
        }
    }

    if (!ctl.empty()) return ctl_client(sock, ctl);
    return run_server(udp, sock, device);
}
