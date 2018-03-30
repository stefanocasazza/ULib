include $(TOPDIR)/rules.mk

PKG_NAME:=userver
PKG_VERSION:=1.4.2
PKG_RELEASE:=13

PKG_SOURCE:=ULib-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/stefanocasazza/ULib/archive/v1.4.2.tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/ULib-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

STAMP_CONFIGURED:=$(STAMP_CONFIGURED)_$(call confvar, CONFIG_USERVER_DEBUG)

define Package/userver
   TITLE:=application server by ULib
   URL:=http://stefanocasazza.github.io/ULib/
   DEPENDS:= +libstdcpp +libopenssl +zlib +libuuid +libpcre
endef

define Package/userver/Description
application server by ULib
endef

define Package/userver/config
   source "$(SOURCE)/Config.in"
endef

define Package/userver/conffiles
/etc/init.d/userver
/srv/userver/etc/firenze_ssl.cfg
/srv/userver/etc/firenze_tcp.cfg
/srv/userver/etc/uclient-firenze.cfg
/srv/userver/bin/reset_policy.sh
/srv/userver/bin/get_users_info.sh
/srv/userver/bin/card_activation.sh
/srv/userver/bin/send_req_to_portal.sh
/srv/userver/bin/check_for_assertion.sh
/srv/userver/bin/get_users_info_for_ap.sh
endef

USERVER_OPTIONS:= --with-ssl --with-libz --with-libuuid --with-pcre \
					   --without-expat --without-magic \
					   --disable-static --disable-new-ldflags --disable-zip --disable-LFS --disable-gcc-optimized --disable-HCRS --disable-HPRS --disable-HIS \
				      --enable-log --enable-static-server-plugin="proxy ssi http" \
					   $(if $(CONFIG_USERVER_DEBUG),--enable-debug)

define Build/Configure
	$(call Build/Configure/Default, \
		--prefix=$(PKG_INSTALL_DIR)/usr \
		$(USERVER_OPTIONS))
endef

define Build/Compile
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar r" \
		RANLIB="$(TARGET_CROSS)ranlib"
endef

define Package/userver/install
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) DESTDIR="$(PKG_INSTALL_DIR)" install
	$(INSTALL_DIR) $(1)/etc $(1)/usr/sbin $(1)/usr/lib
	$(CP)     $(PKG_INSTALL_DIR)/usr/bin/rdbgen \
				 $(PKG_INSTALL_DIR)/usr/bin/uclient \
				 $(PKG_INSTALL_DIR)/usr/bin/userver_tcp \
				 $(PKG_INSTALL_DIR)/usr/bin/userver_ssl $(1)/usr/sbin/
	$(CP) -a  $(PKG_INSTALL_DIR)/$(PKG_INSTALL_DIR)/usr/lib/libulib* \
				 $(PKG_INSTALL_DIR)/$(PKG_INSTALL_DIR)/usr/lib/usp/wi_auth.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,userver))
