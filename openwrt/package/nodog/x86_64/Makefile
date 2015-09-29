include $(TOPDIR)/rules.mk

PKG_NAME:=nodog
PKG_VERSION:=1.4.2
PKG_RELEASE:=13

PKG_SOURCE:=ULib-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/stefanocasazza/ULib/archive/v1.4.2.tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/ULib-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

STAMP_CONFIGURED:=$(STAMP_CONFIGURED)_$(call confvar, CONFIG_NODOG_DEBUG)

define Package/nodog
   TITLE:=nocat enhanced
   URL:=http://stefanocasazza.github.io/ULib/
   DEPENDS:= +libopenssl +libstdcpp          +zlib $(if $(CONFIG_NODOG_UUID),+libuuid) +libpthread +librt +tdb
## DEPENDS:=             +libstdcpp +libpcre +zlib $(if $(CONFIG_NODOG_UUID),+libuuid) \
##													 $(if $(CONFIG_NODOG_OPENSSL),+libopenssl) $(if $(CONFIG_NODOG_CYASSL),+libcyassl) \
##   													+iptables-mod-ipopt +iptables-mod-nat +iptables-mod-nat-extra
endef

define Package/nodog/Description
  NoDog is a web capture system. It is similar to wireless access systems used in some coffee shops and airports.
  In commercial environments, web capture is used to authorise and pay for network use. Within SEPS, it's used for authentication purposes
  When run on a gateway/router on a network, all web requests are redirected until the client either logs in or clicks "I Accept" to an AUP.
  The gateway daemon then changes the firewall rules on the gateway to pass traffic for that client (based on IP address and MAC address).
endef

define Package/nodog/config
   source "$(SOURCE)/Config.in"
endef

define Package/nodog/conffiles
/etc/nodog.key
/etc/nodog.portal
/etc/uclient.conf
/etc/init.d/nodog
/usr/lib/nodog/firewall/nodog.fw
/usr/lib/nodog/firewall/tcrules.awk
endef

NODOG_OPTIONS:= --disable-examples \
					 --with-ssl --with-libz \
					 --without-expat --without-magic --without-pcre \
					 --disable-static --disable-new-ldflags --disable-zip --disable-LFS \
					 --enable-log --enable-captive-portal --enable-static-server-plugin="nocat http" \
					 --disable-stdcpp --disable-gcc-optimized --disable-thread --disable-HCRS --disable-HPRS \
					 $(if $(CONFIG_NODOG_DEBUG),--enable-debug) \
					 $(if $(CONFIG_NODOG_UUID),--with-libuuid,--without-libuuid) \
					 $(if $(CONFIG_NODOG_MEMPOOL),--enable-memory-pool,--disable-memory-pool)

define Build/Configure
	$(call Build/Configure/Default, \
		--prefix=$(PKG_INSTALL_DIR)/usr \
		$(NODOG_OPTIONS))
endef

## LIBS=-luClibc++
define Build/Compile
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar r" \
		RANLIB="$(TARGET_CROSS)ranlib"
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/examples/userver \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar r" \
		RANLIB="$(TARGET_CROSS)ranlib"
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/examples/uclient \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar r" \
		RANLIB="$(TARGET_CROSS)ranlib"
endef

define Package/nodog/install
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) 						DESTDIR="$(PKG_INSTALL_DIR)" install
	$(MAKE) -C $(PKG_BUILD_DIR)/examples/userver DESTDIR="$(PKG_INSTALL_DIR)" install
	$(MAKE) -C $(PKG_BUILD_DIR)/examples/uclient DESTDIR="$(PKG_INSTALL_DIR)" install
	$(INSTALL_DIR) $(1)/etc $(1)/usr/sbin $(1)/usr/lib $(1)/usr/lib/nodog
##	$(CP) -a  $(PKG_INSTALL_DIR)/usr/lib/server_plugin_proxy* $(1)/usr/lib
	$(CP) -a  $(PKG_INSTALL_DIR)/$(PKG_INSTALL_DIR)/usr/lib/libulib* $(1)/usr/lib/
	$(CP)     $(PKG_INSTALL_DIR)/usr/bin/uclient $(PKG_INSTALL_DIR)/usr/bin/userver_tcp $(1)/usr/sbin/
	$(CP) -r  $(PKG_BUILD_DIR)/tests/examples/nocat/*    $(1)/usr/lib/nodog
##	$(CP) -r	 $(PKG_BUILD_DIR)/tests/examples/nocat/.ht* $(1)/usr/lib/nodog
	$(CP) -r  $(1)/usr/lib/nodog/etc  						  $(1)/
	$(RM) -rf $(1)/usr/lib/nodog/etc
endef

$(eval $(call BuildPackage,nodog))
