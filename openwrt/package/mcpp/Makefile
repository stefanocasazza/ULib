include $(TOPDIR)/rules.mk

PKG_NAME:=mcpp
PKG_VERSION:=2.7.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_MD5SUM:=512de48c87ab023a69250edc7a0c7b05

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/mcpp
 TITLE:=cpp enhanced
 URL:=http://strace.sourceforge.net/
endef

define Package/mcpp/Description
  mcpp is a preprocessor with the highest conformance which implements C90, C99. mcpp has plentiful diagnostics and many pragmas.
  It is useful to check portability of your program, and also useful to debug complicated macro. This is a man-page for mcpp of compiler-independent-build.
endef

define Build/Configure
	$(call Build/Configure/Default, \
		--prefix=$(PKG_INSTALL_DIR)/usr)
endef

define Build/Compile
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar r" \
		RANLIB="$(TARGET_CROSS)ranlib"
endef

define Package/mcpp/install
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) DESTDIR="$(PKG_INSTALL_DIR)" install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mcpp $(1)/usr/bin
endef

$(eval $(call BuildPackage,mcpp))
