# Netlay

**Netlay** is a slim, standalone network-audio app for playing and mixing together over the internet or a local network. Pick a group name, join, and you’re connected.

It is a focused fork of [SonoBus](https://github.com/sonosaurus/sonobus): same peer-to-peer Opus/PCM audio path, rebuilt around a simpler session UI and **remote mix control**.

Current version: **1.1.0** (macOS standalone).

## What’s different

- **One window, five pages** — Network, Peers, Group Control, Soundboard, Settings
- **Group Control** — move another person’s listen mix from your machine (true remote control, not just your own monitoring)
- **Mixing Station–style strips** — vertical faders, meters, mute, for remote levels and for your local input mixer (under Settings)
- **Standalone only** — no VST/AU/LV2 plugin targets in this fork
- **Trimmed feature set** — chat, FX, reverb, recording, metronome, and several other SonoBus surfaces are off by default

## Group Control

Group Control is for running someone else’s mix while you play.

1. Join the same group as the people you want to control.
2. Open **Group Control**.
3. Use **Main Mix** (defaults to everyone connected) or create a named control group and pick members / sources.
4. Move a fader — that source’s receive level (or mute) changes on the target’s machine, and their UI follows.

Transport stays on the existing `/sb` OSC path over UDP, with sequence numbers, ACK/NACK, coalesced retries, and periodic snapshot reconcile so a dropped packet can’t leave a fader stuck.

Targets can turn control off in **Settings → Allow others to control my mix** (on by default). While someone is driving your mix, the header shows who.

Local input grouping (channel groups, send mono/stereo/multichannel) lives on **Settings**, under Options.

## Tips

- Use headphones if you have a live mic — there is no echo cancellation.
- Wired ethernet beats Wi‑Fi for lowest latency.
- Audio is peer-to-peer. The connection server is only used so people in a group can find each other. Data is not encrypted.

## Building (macOS)

You need [CMake](https://cmake.org) ≥ 3.15 and Xcode / Command Line Tools. Dependencies (JUCE, AOO, Opus, etc.) are vendored in this repo.

```bash
export PATH="/opt/homebrew/bin:$PATH"   # if needed on Apple Silicon
cmake -S . -B build
cmake --build build --config Release
```

The app lands at:

```text
build/SonoBus_artefacts/Release/Standalone/Netlay.app
```

Optional helper scripts from upstream still work if you prefer them: `./setupcmake.sh` then `./buildcmake.sh`.

## Repository layout

| Path | Role |
|------|------|
| `Source/` | App UI and processor (including remote mix) |
| `Source/SonobusFeatures.h` | Compile-time feature switches for this slim fork |
| `images/` | Netlay icons and wordmark |
| `deps/` | Vendored JUCE, AOO, meters, Opus |
| `openwrt/` | GL.iNet Slate 7 plugin (`netlayd` + Applications page). See [openwrt/README.md](openwrt/README.md). |

Active development is on the `simplify` branch.

## License

Netlay inherits SonoBus’s license: **GPLv3** (see `LICENSE`), with the App Store exception in `LICENSE_EXCEPTION` where applicable. Dependencies keep their own licenses.

SonoBus was written by Jesse Chappell. Netlay is based on that work and remains open source under the same terms.

Upstream project: [github.com/sonosaurus/sonobus](https://github.com/sonosaurus/sonobus)

## Credits

- **SonoBus** — Jesse Chappell
- **AOO** — Christof Ressi
- **Soundboard** — Sten Wessel, Hannah Schellekens
- Built with JUCE and the Opus codec
