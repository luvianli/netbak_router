#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_mysql
PKG_VERSION:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += $(FPIC)

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:=HUAYI MYSQL STATIC LIB
  DEPENDS:=+libmysqlclient
  MENU:=1
endef

define Package/$(PKG_NAME)/description
	huayi vendor mysql static lib 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/huayi_mysql.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.a $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
