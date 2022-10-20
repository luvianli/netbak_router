#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libcjson

PKG_REV:=0001
PKG_VERSION:=r$(PKG_REV)
PKG_RELEASE:=6

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:=LIB Json C API
  MAINTAINER:= <huayi.luo@fixdlink.com>
  DEPENDS:=+libc +libgcc
endef


define Package/$(PKG_NAME)/description
  A cJSON lib
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections $(FPIC)
TARGET_LDFLAGS += -Wl,--gc-sections

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)
endef

TARGET_CFLAGS += $(FPIC)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		all
endef	

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/cJSON.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libcjson.so $(1)/usr/lib/
endef

define Package/libcjson/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libcjson.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libcjson))
