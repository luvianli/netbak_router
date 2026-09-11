#
# Copyright (C) 2015
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=wisp_rep
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=

include $(INCLUDE_DIR)/package.mk

define Package/wisp_rep
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE
  TITLE:=wisp_rep
  DEPENDS:=+libpthread +huayi-universal +libuci +huayi_uci +libcjson +libpthread
endef

define Package/wisp_rep/description
  wisp_rep deamon: save up to 5 uplink(wisp/apcli) wifi records,
  auto site-survey and reconnect recorded APs one by one when uplink lost.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/wisp_rep/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wisp_rep $(1)/usr/bin/wisp_rep
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wisp_rep.init $(1)/etc/init.d/wisp_rep
endef


$(eval $(call BuildPackage,wisp_rep))
