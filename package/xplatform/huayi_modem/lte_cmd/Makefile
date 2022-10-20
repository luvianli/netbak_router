#
# Copyright (C) 2015 TOTOLINK
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=lte_cmd
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/lte_cmd
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE
  TITLE:=LTE Modem AT Server
  DEPENDS:=+libpthread +kmod-huayi-universal +libuci +huayi_uci +libcjson
endef

define Package/lte_cmd/description
  CLI CM based
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/lte_cmd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_atcmd_server $(1)/usr/bin/huayi_atcmd_server
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_AT_cli $(1)/usr/bin/huayi_AT_cli
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/huayi_atcmd.init $(1)/etc/init.d/huayi_atcmd
endef


$(eval $(call BuildPackage,lte_cmd))

