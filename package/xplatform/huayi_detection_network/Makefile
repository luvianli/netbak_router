#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_detect_network
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI DETECT NETWORK
  DEPENDS:=+kmod-huayi-universal +libpthread +libuci +huayi_uci +libcjson +libm
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_detect_network $(1)/usr/sbin/
#	$(INSTALL_DIR) $(1)/etc/init.d
#	$(INSTALL_BIN) ./files/huayi_detect_network.init $(1)/etc/init.d/huayi_detect_network
endef
	
$(eval $(call BuildPackage,$(PKG_NAME)))