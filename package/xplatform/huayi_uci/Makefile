#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_OWN:=HUAYI

PKG_NAME:=huayi_uci
PKG_VERSION:=1.0
PKG_RELEASE:=0

PKG_SOURCE_VERSION:=$(PKG_VERSION)

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:=HUAYI UCI API
  DEPENDS:=+libuci +kmod-huayi-universal +libpthread +libcjson
endef

define Package/$(PKG_NAME)/description
  HUAYI UCI API
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so  $(STAGING_DIR)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so  $(1)/usr/lib/
endef


$(eval $(call BuildPackage,$(PKG_NAME)))
