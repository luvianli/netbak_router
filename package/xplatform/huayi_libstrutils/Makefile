#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_libstrutils

PKG_REV:=0001
PKG_VERSION:=r$(PKG_REV)
PKG_RELEASE:=6

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:=LIB STR COMMON API
  DEPENDS:=+libc +libgcc
endef


define Package/$(PKG_NAME)/description
  str common lib
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
	$(CP) $(PKG_BUILD_DIR)/include/libutils.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libstrutils.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libstrutils.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
