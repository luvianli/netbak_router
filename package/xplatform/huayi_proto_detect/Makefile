#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_proto_detect

PKG_REV:=0001
PKG_VERSION:=r$(PKG_REV)
PKG_RELEASE:=6

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  DEPENDS=+libpcap +libuci +huayi_uci
  TITLE:=aplication wan proto type detect
endef

define Package/$(PKG_NAME)/description
 wan link proto type detect
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections -lpcap

MAKE_FLAGS := \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \

MAKEOPTS:= -C $(PKG_BUILD_DIR) \
              $(MAKE_FLAGS)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LIBS="$(TARGET_LDFLAGS)  " \
		all
endef
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/huayi_proto_detect.init $(1)/etc/init.d/huayi_proto_detect
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
