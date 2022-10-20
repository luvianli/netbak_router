#
# Copyright (C) 2015 TOTOLINK
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_dial4g
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=

include $(INCLUDE_DIR)/package.mk

define Package/huayi_dial4g
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE
  TITLE:=huayi_dial4g
  DEPENDS:=+libpthread +kmod-huayi-universal +libsqlite3 +sqlite3-cli +libuci +huayi_uci +libcjson +libm +libpthread
endef

define Package/huayi_dial4g/description
  huayi_dial4g deamon.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/huayi_dial4g/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_dial4g $(1)/usr/bin/huayi_dial4g
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_dial4g_init $(1)/usr/bin/huayi_dial4g_init
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-CM/quectel-CM $(1)/usr/bin/quectel-CM
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/MeiG-CM/MeiG-CM $(1)/usr/bin/MeiG-CM
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/huayi_dial4g.init $(1)/etc/init.d/huayi_dial4g
	$(INSTALL_BIN) ./files/huayi_dial4gcontrol.init $(1)/etc/init.d/huayi_dial4gcontrol
	$(INSTALL_BIN) ./files/huayi_dial4g_init.init $(1)/etc/init.d/huayi_dial4_init
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/huayi_modem.config $(1)/etc/config/huayi_modem
#	$(INSTALL_DATA) ./files/huayi_sms.config $(1)/etc/config/huayi_sms
endef


$(eval $(call BuildPackage,huayi_dial4g))

