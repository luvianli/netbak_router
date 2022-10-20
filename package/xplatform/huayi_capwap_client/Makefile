#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_capwap_client
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI CAPWAP CLIENT
  DEPENDS:=+kmod-huayi-universal +kmod-huayi-events +kmod-huayi-alarm +huayi_udp +libpthread +libuci +huayi_uci +libcjson +libm +zlib
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin/
	$(INSTALL_BIN) ./files/huayi_capwap_client.init $(1)/etc/init.d/huayi_capwap_client
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/check-brlan-ip-change.hotplug $(1)/etc/hotplug.d/iface/31-check-brlan-ip-change
endef
	
$(eval $(call BuildPackage,$(PKG_NAME)))
