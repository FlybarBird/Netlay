#!/bin/sh
# Install / verify USB-audio support packages on a GL-BE3600 (Slate 7).
# kmods are NOT IPK Depends — wrong QSDK ABI can brick the unit.
# Run on the router. Safe to re-run.

set -u

FEED_KMOD="https://fw.gl-inet.com/releases/qsdk_v12.5/kmod-4.7/be3600-ipq53xx"
FEED_PKGS="https://fw.gl-inet.com/releases/qsdk_v12.5/packages-4.x/ipq53xx/be9300/packages"
KMOD_SOUND_URL="$FEED_KMOD/kmod-sound-core_5.4.213-1_aarch64_cortex-a53_neon-vfpv4.ipk"
KMOD_USB_URL="$FEED_KMOD/kmod-usb-audio_5.4.213-1_aarch64_cortex-a53_neon-vfpv4.ipk"

PACKAGES="kmod-sound-core kmod-usb-audio alsa-lib alsa-utils"

STATUS_ONLY=0
JSON=0
for arg in "$@"; do
    case "$arg" in
        --status) STATUS_ONLY=1 ;;
        --json) JSON=1 ;;
        -h|--help)
            echo "Usage: netlay-install-support [--status] [--json]"
            echo "  default  install missing packages, load modules, check ALSA"
            echo "  --status report only (no opkg install)"
            echo "  --json   machine-readable summary"
            exit 0
            ;;
    esac
done

json_esc() {
    printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g; s/	/\\t/g' | tr '\n' ' '
}

pkg_installed() {
    opkg list-installed 2>/dev/null | grep -q "^$1 "
}

in_feed() {
    opkg list 2>/dev/null | grep -q "^$1 "
}

bool() {
    [ "$1" = "1" ] && echo true || echo false
}

log() {
    [ "$JSON" = "1" ] && return
    echo "$@"
}

if ! command -v opkg >/dev/null 2>&1; then
    if [ "$JSON" = "1" ]; then
        echo '{"ok":false,"error":"opkg not found; run on the router"}'
    else
        echo "FAIL: opkg not found. Run this script on the router, not the build machine."
    fi
    exit 2
fi

ensure_feed() {
    name="$1"
    url="$2"
    conf="/etc/opkg/customfeeds.conf"
    mkdir -p /etc/opkg
    touch "$conf"
    if grep -q "$url" "$conf" 2>/dev/null; then
        log "OK  feed $name already present"
        return
    fi
    echo "src/gz $name $url" >> "$conf"
    log "OK  added feed $name"
}

if [ "$STATUS_ONLY" != "1" ]; then
    log "== Netlay USB audio packages =="
    log "Kernel: $(uname -r 2>/dev/null || echo unknown)"
    if [ -r /etc/openwrt_release ]; then
        # shellcheck disable=SC1091
        . /etc/openwrt_release
        log "Dist:   ${DISTRIB_DESCRIPTION:-unknown}"
    fi
    log
    log "-- Feeds --"
    ensure_feed glinet_be3600_kmod "$FEED_KMOD"
    ensure_feed glinet_be9300_pkgs "$FEED_PKGS"
    if grep -q '^option check_signature' /etc/opkg.conf 2>/dev/null; then
        sed -i 's/^option check_signature/# option check_signature/' /etc/opkg.conf || true
        log "OK  disabled opkg signature check for this unsigned kmod feed"
    fi
    log "-- Updating package lists --"
    opkg update >/dev/null 2>&1 || log "WARN: opkg update failed; continuing"
fi

install_url() {
    pkg="$1"
    url="$2"
    tmp="/tmp/${pkg}.ipk"
    log "TRY download $url"
    if wget -q -O "$tmp" "$url" 2>/dev/null || curl -fsSL -o "$tmp" "$url" 2>/dev/null; then
        if opkg install "$tmp" >/dev/null 2>&1; then
            log "OK  installed $pkg from URL"
            rm -f "$tmp"
            return 0
        fi
        log "WARN: opkg install $pkg failed (kernel ABI mismatch is common on 4.8/4.9)"
        rm -f "$tmp"
        return 1
    fi
    log "FAIL: could not download $url"
    return 1
}

missing=""
pkg_json=""
for pkg in $PACKAGES; do
    inst=0
    if pkg_installed "$pkg"; then
        inst=1
        log "OK  installed: $pkg"
    elif [ "$STATUS_ONLY" != "1" ]; then
        if in_feed "$pkg"; then
            log "TRY install:  $pkg"
            if opkg install "$pkg" >/dev/null 2>&1; then
                inst=1
                log "OK  installed: $pkg"
            else
                log "FAIL: opkg install $pkg"
            fi
        else
            log "WARN: $pkg is not in configured feeds"
        fi
        if [ "$inst" != "1" ]; then
            case "$pkg" in
                kmod-sound-core) install_url "$pkg" "$KMOD_SOUND_URL" && inst=1 || missing="$missing $pkg" ;;
                kmod-usb-audio)  install_url "$pkg" "$KMOD_USB_URL" && inst=1 || missing="$missing $pkg" ;;
                *) missing="$missing $pkg" ;;
            esac
        fi
        pkg_installed "$pkg" && inst=1
    else
        missing="$missing $pkg"
    fi
    [ -n "$pkg_json" ] && pkg_json="$pkg_json,"
    pkg_json="${pkg_json}{\"name\":\"$pkg\",\"installed\":$(bool $inst)}"
done

KVER="$(uname -r)"
MODDIR="/lib/modules/$KVER"

load_one() {
    name="$1"
    sysname="$(echo "$name" | tr '-' '_')"
    if [ -e "/sys/module/$sysname" ]; then
        log "OK  $name already loaded"
        return 0
    fi
    [ "$STATUS_ONLY" = "1" ] && return 1
    ko="$(find "$MODDIR" -name "${name}.ko*" 2>/dev/null | head -n 1 || true)"
    if [ -n "$ko" ] && insmod "$ko" 2>/dev/null; then
        log "OK  insmod $ko"
        return 0
    fi
    if modprobe "$name" 2>/dev/null; then
        log "OK  modprobe $name"
        return 0
    fi
    log "WARN: could not load $name"
    return 1
}

if [ "$STATUS_ONLY" != "1" ]; then
    log
    log "-- Load sound stack --"
fi
load_one snd || true
load_one snd-timer || true
load_one snd-hwdep || true
load_one snd-pcm || true
load_one snd-usbmidi-lib || true
load_one snd-usb-audio || true

snd_usb=0
[ -e /sys/module/snd_usb_audio ] && snd_usb=1
cards=0
[ -r /proc/asound/cards ] && cards=1
pcm="$(ls /dev/snd/pcmC*D*p 2>/dev/null || true)"

hint=""
ok=1
if [ ! -d /dev/snd ]; then
    ok=0
    hint="Sound core is not loaded. kmod-sound-core did not come up (kernel ABI mismatch is common on 4.8/4.9)."
elif [ -z "$pcm" ]; then
    ok=0
    hint="USB audio module loaded, but no playback PCM yet. Plug a class-compliant dongle into the USB-A port (SoundWire needs USB-C-to-A, not the PD port)."
fi
if [ -n "$missing" ] && [ "$ok" = "1" ]; then
    hint="Some packages missing:$missing"
fi

if [ "$JSON" = "1" ]; then
    printf '{"ok":%s,"packages":[%s],"snd_usb_audio":%s,"cards":%s,"pcm":%s,"hint":"%s"}\n' \
        "$(bool $ok)" "$pkg_json" "$(bool $snd_usb)" "$(bool $cards)" \
        "$([ -n "$pcm" ] && echo true || echo false)" "$(json_esc "$hint")"
    [ "$ok" = "1" ]
    exit $?
fi

log
if [ -r /proc/asound/cards ]; then
    log "OK  /proc/asound/cards"
    cat /proc/asound/cards
fi
if [ -d /dev/snd ]; then
    log "OK  /dev/snd:"
    ls -l /dev/snd || true
else
    log "FAIL: /dev/snd missing — sound core is not up"
    [ -n "$missing" ] && log "  Missing packages:$missing"
    exit 1
fi

if [ -z "$pcm" ]; then
    log "WARN: no playback PCM yet. Plug in USB audio and re-run."
    exit 3
fi

log "OK  playback PCM:"
log "$pcm"
log "USB audio gate passed."
exit 0
