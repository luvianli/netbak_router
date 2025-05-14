include $(TOPDIR)/rules.mk

PKG_NAME:=meige_ota
PKG_VERSION:=1.0.0
PKG_RELEASE:=1


PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk


define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Meige OTA Update Tool
  DEPENDS:=+libpthread +libm
endef

define Package/$(PKG_NAME)/description
  Meige OTA update tool for firmware updates
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/meige_ota $(1)/usr/bin/meige_ota
endef

$(eval $(call BuildPackage,$(PKG_NAME)))