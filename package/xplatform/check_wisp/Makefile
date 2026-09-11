#
# Copyright (C) 2015
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=check_wisp
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=

include $(INCLUDE_DIR)/package.mk

define Package/check_wisp
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE
  TITLE:=check_wisp
  DEPENDS:=+libpthread +huayi-universal +libuci +huayi_uci +libcjson +libpthread
endef

define Package/check_wisp/description
  check_wisp deamon.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/check_wisp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/check_wisp $(1)/usr/bin/check_wisp
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/check_wisp.init $(1)/etc/init.d/check_wisp
endef


$(eval $(call BuildPackage,check_wisp))

