#
# Copyright (C) 2015 CaryStudio
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_vpn_user
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI VPN USER MODULE
  DEPENDS:=
endef

define Build/Compile/Default
endef
	
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/vpn_user.init $(1)/etc/init.d/vpn_user
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/vpn_user.config $(1)/etc/config/vpn_user	
endef

	
$(eval $(call BuildPackage,$(PKG_NAME)))