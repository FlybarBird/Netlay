#!/bin/sh
# Pack a staged directory into an OpenWrt/opkg IPK.
# GL.iNet and OpenWrt IPKs are gzip-compressed tar archives (not Debian ar).
set -eu
STAGED="${1:?usage: pack-ipk.sh <staged-root> <outfile.ipk>}"
OUT="${2:?usage: pack-ipk.sh <staged-root> <outfile.ipk>}"
VERSION="${PKG_VERSION:-1.0.1-1}"
ARCH="${PKG_ARCH:-aarch64_cortex-a53_neon-vfpv4}"
PKG_NAME="${PKG_NAME:-netlay-glbe3600}"

if command -v gtar >/dev/null 2>&1; then
  TAR=gtar
else
  TAR=tar
fi

export COPYFILE_DISABLE=1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

gnu_tar_gz() {
  dir="$1"
  dest="$2"
  shift 2
  "$TAR" --format=gnu --numeric-owner --owner=0 --group=0 \
    --mode='u+rwX,go+rX,go-w' --mtime='@0' \
    --exclude='.DS_Store' --exclude='._*' \
    -C "$dir" -cf - "$@" | gzip -n -9 > "$dest"
}

INSTALLED_SIZE=$(python3 -c "
from pathlib import Path
p = Path(r'''$STAGED''')
print(sum(f.stat().st_size for f in p.rglob('*') if f.is_file()))
")

mkdir -p "$TMP/control" "$TMP/outer"
{
  echo "Package: $PKG_NAME"
  echo "Version: $VERSION"
  echo "Depends: libc"
  echo "Section: sound"
  echo "Priority: optional"
  echo "Maintainer: Netlay <netlay@localhost>"
  echo "Architecture: $ARCH"
  echo "Installed-Size: $INSTALLED_SIZE"
  echo "Description: Netlay USB audio client for GL-BE3600"
  echo " Join Netlay groups and play audio to a USB sound device."
  echo " Accepts Group Control from desktop Netlay."
} > "$TMP/control/control"
echo "/etc/config/netlay" > "$TMP/control/conffiles"
cat > "$TMP/control/postinst" <<'EOF'
#!/bin/sh
[ -n "$IPKG_INSTROOT" ] && exit 0
chmod 0755 /etc/init.d/netlay /usr/bin/netlayd /usr/libexec/netlay-install-support 2>/dev/null || true
if [ -x /usr/libexec/netlay-install-support ]; then
  /usr/libexec/netlay-install-support >/tmp/netlay-support.log 2>&1 || true
fi
[ -x /etc/init.d/netlay ] && /etc/init.d/netlay enable >/dev/null 2>&1 || true
[ -x /etc/init.d/netlay ] && /etc/init.d/netlay restart >/dev/null 2>&1 || true
[ -x /etc/init.d/nginx ] && /etc/init.d/nginx restart >/dev/null 2>&1 || true
if ! [ -S /var/run/netlay.ctl ]; then
  mkdir -p /var/run
  /usr/bin/netlayd --udp-port 28780 --socket /var/run/netlay.ctl --device auto >/tmp/netlayd.log 2>&1 &
fi
exit 0
EOF
cat > "$TMP/control/prerm" <<'EOF'
#!/bin/sh
[ -n "$IPKG_INSTROOT" ] && exit 0
[ -x /etc/init.d/netlay ] && /etc/init.d/netlay stop >/dev/null 2>&1 || true
[ -x /etc/init.d/netlay ] && /etc/init.d/netlay disable >/dev/null 2>&1 || true
killall netlayd >/dev/null 2>&1 || true
exit 0
EOF
chmod 0755 "$TMP/control/postinst" "$TMP/control/prerm"
printf '2.0\n' > "$TMP/outer/debian-binary"

gnu_tar_gz "$TMP/control" "$TMP/outer/control.tar.gz" .
gnu_tar_gz "$STAGED" "$TMP/outer/data.tar.gz" .
gnu_tar_gz "$TMP/outer" "$OUT" ./debian-binary ./data.tar.gz ./control.tar.gz

echo "Wrote $OUT"
ls -lh "$OUT"
file "$OUT"
