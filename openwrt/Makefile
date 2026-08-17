include $(TOPDIR)/rules.mk

PKG_NAME:=netlay-glbe3600
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

PKG_MAINTAINER:=Netlay
PKG_LICENSE:=GPL-3.0-or-later
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Release

define Package/netlay-glbe3600
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=Netlay USB audio client for GL.iNet Slate 7
  URL:=https://github.com/FlybarBird/Netlay
  DEPENDS:=+libstdcpp +libopus +alsa-lib
endef

define Package/netlay-glbe3600/description
  Join Netlay / SonoBus groups from a GL-BE3600 (Slate 7) and play
  received audio to a USB Audio Class device. Accepts Group Control
  from desktop Netlay. Adds an Applications → Netlay page to the
  GL.iNet 4.x admin UI.
endef

define Package/netlay-glbe3600/conffiles
/etc/config/netlay
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./CMakeLists.txt $(PKG_BUILD_DIR)/
	$(CP) ./src $(PKG_BUILD_DIR)/
	$(CP) ../deps/aoo $(PKG_BUILD_DIR)/aoo
endef

define Package/netlay-glbe3600/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/netlayd $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/netlay $(1)/etc/config/netlay

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/netlay $(1)/etc/init.d/netlay

	$(INSTALL_DIR) $(1)/etc/hotplug.d/usb
	$(INSTALL_BIN) ./files/etc/hotplug.d/usb/20-netlay-audio $(1)/etc/hotplug.d/usb/20-netlay-audio

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/99-netlay-firewall $(1)/etc/uci-defaults/99-netlay-firewall

	$(INSTALL_DIR) $(1)/usr/lib/oui-httpd/rpc
	$(INSTALL_DATA) ./files/usr/lib/oui-httpd/rpc/netlay $(1)/usr/lib/oui-httpd/rpc/netlay

	$(INSTALL_DIR) $(1)/usr/share/oui/menu.d
	$(INSTALL_DATA) ./files/usr/share/oui/menu.d/zz-netlay.json $(1)/usr/share/oui/menu.d/zz-netlay.json

	$(INSTALL_DIR) $(1)/usr/share/gl-validator.d
	$(INSTALL_DATA) ./files/usr/share/gl-validator.d/netlay.lua $(1)/usr/share/gl-validator.d/netlay.lua

	$(INSTALL_DIR) $(1)/www/views
	gzip -c ./files/gl-sdk4-ui-netlay.common.js > $(1)/www/views/gl-sdk4-ui-netlay.common.js.gz
	chmod 644 $(1)/www/views/gl-sdk4-ui-netlay.common.js.gz

	$(INSTALL_DIR) $(1)/www
	$(INSTALL_DATA) ./files/www/netlay-wordmark.png $(1)/www/netlay-wordmark.png

	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) ./scripts/install-support.sh $(1)/usr/libexec/netlay-install-support
endef

define Package/netlay-glbe3600/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] && exit 0
rm -rf /tmp/luci-modulecache /tmp/luci-indexcache 2>/dev/null
chmod 0755 /etc/init.d/netlay /usr/bin/netlayd /usr/libexec/netlay-install-support 2>/dev/null
if [ -x /usr/libexec/netlay-install-support ]; then
	/usr/libexec/netlay-install-support >/tmp/netlay-support.log 2>&1 || true
fi
[ -x /etc/init.d/netlay ] && /etc/init.d/netlay enable
[ -x /etc/init.d/netlay ] && /etc/init.d/netlay restart
echo "Netlay plugin installed. Open Applications → Netlay."
exit 0
endef

define Package/netlay-glbe3600/prerm
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] && exit 0
[ -x /etc/init.d/netlay ] && /etc/init.d/netlay stop
[ -x /etc/init.d/netlay ] && /etc/init.d/netlay disable
exit 0
endef

$(eval $(call BuildPackage,netlay-glbe3600))
