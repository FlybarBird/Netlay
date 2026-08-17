#!/bin/sh
# Cross-compile netlayd for GL-BE3600 from macOS using Zig (aarch64-linux-musl).
# Produces a statically linked binary and an OpenWrt IPK. No Docker required.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/.." && pwd)"
TARGET="aarch64-linux-musl"
WORK="${NETLAY_BUILD_DIR:-/tmp/netlay-cross}"
PREFIX="$WORK/sysroot"
DEPS="$WORK/deps"
TOOL="$WORK/toolchain"
STAGE="$WORK/stage"
OBJ="$WORK/obj"
OUT="$ROOT/output"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
OPUS_VER="1.5.2"
ALSA_VER="1.2.13"
PKG_VER="1.0.1-1"
PKG_ARCH="aarch64_cortex-a53_neon-vfpv4"
AOO="$REPO/deps/aoo"

ZIGFLAGS="-target ${TARGET} -mcpu=cortex_a53 -O2 -fno-sanitize=undefined"
export COPYFILE_DISABLE=1

need() { command -v "$1" >/dev/null || { echo "missing $1"; exit 1; }; }
need zig
need curl
need gtar
need python3

if [ ! -d "$AOO" ]; then
  echo "AOO sources not found at $AOO"
  exit 1
fi

mkdir -p "$WORK" "$PREFIX" "$DEPS" "$TOOL" "$STAGE" "$OUT" "$OBJ"

# Reuse a previous SonoBus/Netlay sysroot if present to skip Opus/ALSA rebuilds.
if [ ! -f "$PREFIX/lib/libopus.a" ] && [ -f /tmp/sonobus-cross/sysroot/lib/libopus.a ]; then
  echo "==> reuse /tmp/sonobus-cross sysroot"
  mkdir -p "$PREFIX"
  cp -a /tmp/sonobus-cross/sysroot/. "$PREFIX/"
fi

export CC="$TOOL/cc"
export CXX="$TOOL/c++"
export AR="$TOOL/ar"
export RANLIB="$TOOL/ranlib"
export CFLAGS="-Os -fPIC"
export CXXFLAGS="-Os -fPIC -std=c++17"
unset LDFLAGS

cat > "$TOOL/cc" <<EOF
#!/bin/sh
exec zig cc ${ZIGFLAGS} "\$@"
EOF
cat > "$TOOL/c++" <<EOF
#!/bin/sh
exec zig c++ ${ZIGFLAGS} -std=c++17 -Wno-nullability-completeness "\$@"
EOF
cat > "$TOOL/ar" <<EOF
#!/bin/sh
exec zig ar "\$@"
EOF
cat > "$TOOL/ranlib" <<EOF
#!/bin/sh
exec zig ranlib "\$@"
EOF
chmod +x "$TOOL/cc" "$TOOL/c++" "$TOOL/ar" "$TOOL/ranlib"

fetch() {
  url="$1"
  dest="$2"
  if [ ! -f "$dest" ]; then
    echo "==> download $url"
    curl -fsSL "$url" -o "$dest"
  fi
}

echo "==> Opus ${OPUS_VER}"
fetch "https://downloads.xiph.org/releases/opus/opus-${OPUS_VER}.tar.gz" "$DEPS/opus-${OPUS_VER}.tar.gz"
if [ ! -f "$PREFIX/lib/libopus.a" ]; then
  rm -rf "$DEPS/opus-${OPUS_VER}"
  tar -xzf "$DEPS/opus-${OPUS_VER}.tar.gz" -C "$DEPS"
  (
    cd "$DEPS/opus-${OPUS_VER}"
    ./configure --host=aarch64-linux-musl --prefix="$PREFIX" \
      --disable-shared --enable-static --disable-doc --disable-extra-programs \
      ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes
    make -j"$JOBS"
    make install
  )
fi

echo "==> alsa-lib ${ALSA_VER}"
fetch "https://www.alsa-project.org/files/pub/lib/alsa-lib-${ALSA_VER}.tar.bz2" "$DEPS/alsa-lib-${ALSA_VER}.tar.bz2"
if [ ! -f "$PREFIX/lib/libasound.a" ]; then
  rm -rf "$DEPS/alsa-lib-${ALSA_VER}"
  tar -xjf "$DEPS/alsa-lib-${ALSA_VER}.tar.bz2" -C "$DEPS"
  (
    cd "$DEPS/alsa-lib-${ALSA_VER}"
    ./configure --host=aarch64-linux-musl --prefix="$PREFIX" \
      --disable-shared --enable-static --disable-python --disable-alisp \
      --disable-old-symbols --without-debug \
      ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes
    make -j"$JOBS"
    make install
  )
fi

echo "==> netlayd"
rm -rf "$OBJ"
mkdir -p "$OBJ"

compile_c() {
  src="$1"
  out="$2"
  echo "  CC  $(basename "$src")"
  "$CC" -c "$src" -o "$out" \
    -I"$AOO/deps" -I"$PREFIX/include"
}

compile_cxx() {
  src="$1"
  out="$2"
  echo "  CXX $(basename "$src")"
  "$CXX" -c "$src" -o "$out" \
    -DUSE_CODEC_OPUS=1 -DAOO_STATIC -DAOO_TIMEFILTER_CHECK=0 \
    -DOSC_HOST_LITTLE_ENDIAN=1 -DLOGLEVEL=1 \
    -I"$AOO/lib" -I"$AOO/deps" -I"$AOO/deps/oscpack" \
    -I"$PREFIX/include"
}

compile_cxx "$AOO/lib/src/client.cpp" "$OBJ/client.o"
compile_cxx "$AOO/lib/src/codec_opus.cpp" "$OBJ/codec_opus.o"
compile_cxx "$AOO/lib/src/codec_pcm.cpp" "$OBJ/codec_pcm.o"
compile_cxx "$AOO/lib/src/common.cpp" "$OBJ/common.o"
compile_cxx "$AOO/lib/src/net_utils.cpp" "$OBJ/net_utils.o"
compile_cxx "$AOO/lib/src/server.cpp" "$OBJ/server.o"
compile_cxx "$AOO/lib/src/sink.cpp" "$OBJ/sink.o"
compile_cxx "$AOO/lib/src/source.cpp" "$OBJ/source.o"
compile_cxx "$AOO/lib/src/sync.cpp" "$OBJ/sync.o"
compile_cxx "$AOO/lib/src/time.cpp" "$OBJ/time.o"
compile_c   "$AOO/deps/md5/md5.c" "$OBJ/md5.o"
compile_cxx "$AOO/deps/oscpack/osc/OscOutboundPacketStream.cpp" "$OBJ/OscOutboundPacketStream.o"
compile_cxx "$AOO/deps/oscpack/osc/OscPrintReceivedElements.cpp" "$OBJ/OscPrintReceivedElements.o"
compile_cxx "$AOO/deps/oscpack/osc/OscReceivedElements.cpp" "$OBJ/OscReceivedElements.o"
compile_cxx "$AOO/deps/oscpack/osc/OscTypes.cpp" "$OBJ/OscTypes.o"
compile_cxx "$ROOT/src/netlayd/main.cpp" "$OBJ/main.o"

echo "  LD  netlayd"
"$CXX" -static -o "$OBJ/netlayd" \
  "$OBJ"/main.o "$OBJ"/client.o "$OBJ"/codec_opus.o "$OBJ"/codec_pcm.o \
  "$OBJ"/common.o "$OBJ"/net_utils.o "$OBJ"/server.o "$OBJ"/sink.o \
  "$OBJ"/source.o "$OBJ"/sync.o "$OBJ"/time.o "$OBJ"/md5.o \
  "$OBJ"/OscOutboundPacketStream.o "$OBJ"/OscPrintReceivedElements.o \
  "$OBJ"/OscReceivedElements.o "$OBJ"/OscTypes.o \
  "$PREFIX/lib/libopus.a" "$PREFIX/lib/libasound.a" \
  -lm -lpthread
if zig objcopy -S "$OBJ/netlayd" "$OBJ/netlayd.stripped" 2>/dev/null; then
  :
elif [ -x /opt/homebrew/opt/llvm@21/bin/llvm-strip ]; then
  /opt/homebrew/opt/llvm@21/bin/llvm-strip "$OBJ/netlayd" -o "$OBJ/netlayd.stripped"
else
  cp "$OBJ/netlayd" "$OBJ/netlayd.stripped"
fi
file "$OBJ/netlayd.stripped"
ls -lh "$OBJ/netlayd.stripped"

echo "==> stage IPK tree"
rm -rf "$STAGE"
mkdir -p "$STAGE/usr/bin" \
  "$STAGE/etc/config" \
  "$STAGE/etc/init.d" \
  "$STAGE/etc/hotplug.d/usb" \
  "$STAGE/etc/uci-defaults" \
  "$STAGE/usr/lib/oui-httpd/rpc" \
  "$STAGE/usr/share/oui/menu.d" \
  "$STAGE/usr/share/gl-validator.d" \
  "$STAGE/www/views" \
  "$STAGE/usr/libexec"

install -m 0755 "$OBJ/netlayd.stripped" "$STAGE/usr/bin/netlayd"
install -m 0644 "$ROOT/files/etc/config/netlay" "$STAGE/etc/config/netlay"
install -m 0755 "$ROOT/files/etc/init.d/netlay" "$STAGE/etc/init.d/netlay"
install -m 0755 "$ROOT/files/etc/hotplug.d/usb/20-netlay-audio" "$STAGE/etc/hotplug.d/usb/20-netlay-audio"
install -m 0755 "$ROOT/files/etc/uci-defaults/99-netlay-firewall" "$STAGE/etc/uci-defaults/99-netlay-firewall"
install -m 0644 "$ROOT/files/usr/lib/oui-httpd/rpc/netlay" "$STAGE/usr/lib/oui-httpd/rpc/netlay"
install -m 0644 "$ROOT/files/usr/share/oui/menu.d/zz-netlay.json" "$STAGE/usr/share/oui/menu.d/zz-netlay.json"
install -m 0644 "$ROOT/files/usr/share/gl-validator.d/netlay.lua" "$STAGE/usr/share/gl-validator.d/netlay.lua"
gzip -c "$ROOT/files/gl-sdk4-ui-netlay.common.js" > "$STAGE/www/views/gl-sdk4-ui-netlay.common.js.gz"
chmod 0644 "$STAGE/www/views/gl-sdk4-ui-netlay.common.js.gz"
install -m 0644 "$ROOT/files/www/netlay-wordmark.png" "$STAGE/www/netlay-wordmark.png"
install -m 0755 "$ROOT/scripts/install-support.sh" "$STAGE/usr/libexec/netlay-install-support"

echo "==> pack IPK"
PKG_NAME=netlay-glbe3600 PKG_VERSION="$PKG_VER" PKG_ARCH="$PKG_ARCH" \
  sh "$ROOT/scripts/pack-ipk.sh" "$STAGE" "$OUT/netlay-glbe3600_${PKG_VER}_${PKG_ARCH}.ipk"

echo
echo "Built: $OUT/netlay-glbe3600_${PKG_VER}_${PKG_ARCH}.ipk"
ls -lh "$OUT"/netlay-glbe3600_*.ipk
