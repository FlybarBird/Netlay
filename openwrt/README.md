# Netlay plugin for GL.iNet Slate 7 (GL-BE3600)

Join real [Netlay](https://github.com/FlybarBird/Netlay) / [SonoBus](https://github.com/sonosaurus/sonobus) groups from a Slate 7 and play the mix out a **USB Audio Class** device. Desktop Netlay can mix this box from **Group Control**.

A Netlay page is added under **Applications** in the GL.iNet 4.x admin UI.

This is **not** the native Netlay JUCE app. `netlayd` is a listen-only AOO client built from the AOO sources Netlay vendors, plus the `/sb` remote-mix target protocol.

v1 is playback only (USB speakers/headphones). There is no microphone send. Remote mix is receive-only: others can move this device’s listen levels; this page does not mix other people.

## Hardware

| | |
|---|---|
| Router | GL.iNet Slate 7 (**GL-BE3600**) |
| Firmware | GL.iNet 4.8 / 4.9 (OpenWrt 23.05 QSDK) |
| Arch | `aarch64_cortex-a53_neon-vfpv4` |
| Audio | USB Audio Class dongle on the USB 3.0 port |

## Support packages

On install, the IPK runs `/usr/libexec/netlay-install-support`, which pulls:

- `kmod-sound-core`
- `kmod-usb-audio`
- `alsa-lib`
- `alsa-utils`

from the GL BE3600 feeds. Kernel modules are **not** listed in `Depends:` — a mismatched QSDK kmod ABI has bricked BE3600 units.

If USB audio is still missing, open **Applications → Netlay** and click **Install audio packages**, or SSH:

```sh
/usr/libexec/netlay-install-support
```

Use a class-compliant USB dongle. Bus-powered speakers may need a powered hub. Power the Slate 7 from USB-C PD.

## Build

On macOS. Zig cross-compiles a static `aarch64-linux-musl` binary. No Docker.

```sh
brew install zig gnu-tar
./openwrt/scripts/build-mac-cross.sh
# openwrt/output/netlay-glbe3600_1.0.1-1_aarch64_cortex-a53_neon-vfpv4.ipk
```

## Install

1. Copy `openwrt/output/netlay-glbe3600_*.ipk` (or the [release IPK](https://github.com/FlybarBird/Netlay/releases/tag/v1.1.0)) to a computer on the Slate’s LAN.
2. Open [192.168.8.1](http://192.168.8.1) → **SYSTEM → Advanced Settings** → **LuCI**.
3. In LuCI: **System → Software → Upload Package…**, choose the IPK, then **Install**.
4. Return to the GL.iNet UI and open **Applications → Netlay**.

If LuCI reports a malformed package, install from SSH instead:

```sh
scp -O openwrt/output/netlay-glbe3600_*.ipk root@192.168.8.1:/tmp/
ssh root@192.168.8.1 'opkg install /tmp/netlay-glbe3600_*.ipk'
```

Enter the same group name (and password) as the other Netlay clients, pick the USB device, click **Connect**. Leave **Allow others to control my mix** on if a desktop Netlay should drive this box from Group Control.

## How it works

```
GL.iNet Applications page
        │  JSON-RPC (netlay)
        ▼
   netlayd  ──UDP──  aoo.sonobus.net (group discovery only)
        │
        └── P2P AOO/Opus ──► peers
        └── /sb remote mix ──► desktop Group Control
        └── ALSA ──► USB audio
```

- Default connection server: `aoo.sonobus.net:10998`
- Local UDP port `28780` is opened on WAN for hole punching
- Status/control socket: `/var/run/netlay.ctl`
- UCI: `/etc/config/netlay`

CLI:

```sh
netlayd --ctl '{"cmd":"status"}'
netlayd --ctl '{"cmd":"connect","group":"mygroup","username":"Slate7"}'
netlayd --ctl '{"cmd":"disconnect"}'
```

## Limits and risks

- **No encryption** (upstream SonoBus / Netlay behaviour).
- Travel/CGNAT/hotel networks may block P2P UDP.
- Keep groups to a couple of Opus streams; this is a 1.1 GHz travel router.
- Do not `opkg upgrade` random kmods on GL firmware.

## License

Plugin code is GPL-3.0-or-later, same as Netlay / SonoBus. Vendored AOO is BSD. Opus and ALSA have their own licenses.
