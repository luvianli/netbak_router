#
# Copyright (C) 2015
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=check_sim
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=

include $(INCLUDE_DIR)/package.mk

define Package/check_sim
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE
  TITLE:=check_sim
  DEPENDS:=+libpthread +huayi-universal +libuci +huayi_uci +libcjson +libpthread
endef

define Package/check_sim/description
  check_sim deamon.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/check_sim/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/check_sim $(1)/usr/bin/check_sim
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/check_sim.init $(1)/etc/init.d/check_sim
endef


$(eval $(call BuildPackage,check_sim))

