#
# Copyright (C) 2020 HUAYI
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_ddns
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI DDNS MODULE
  DEPENDS:=+libpthread +kmod-huayi-universal +libuci +huayi_uci +libcjson +libm
endef

define Package/$(PKG_NAME)/description
Dynamic domain name
support noip-2.1.9 yddns orayddns
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/noip-2.1.9/noip2 $(1)/usr/bin/noip2
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/yddns/yddns $(1)/usr/bin/yddns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/orayddns/orayddns $(1)/usr/bin/orayddns
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/noip2.init $(1)/etc/init.d/noip2
	$(INSTALL_BIN) ./files/orayddns.init $(1)/etc/init.d/orayddns
	$(INSTALL_BIN) ./files/yddns.init $(1)/etc/init.d/yddns
	
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/ddns.config $(1)/etc/config/ddns	
endef

	
$(eval $(call BuildPackage,$(PKG_NAME)))