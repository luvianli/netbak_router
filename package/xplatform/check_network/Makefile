#
# Copyright (C) 2015
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=check_network
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=

include $(INCLUDE_DIR)/package.mk

define Package/check_network
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE
  TITLE:=check_network
  DEPENDS:=+libpthread +huayi-universal +libuci +huayi_uci +libcjson +libpthread
endef

define Package/$(PKG_NAME)/description
  $(PKG_NAME) deamon.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/check_network $(1)/usr/bin/check_network
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/check_network.init $(1)/etc/init.d/check_network
endef


$(eval $(call BuildPackage,check_network))

