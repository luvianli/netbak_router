#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_cmd
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI IOCTL CMD TOOLS
  DEPENDS:=+kmod-huayi-universal +kmod-huayi-control +libuci +huayi_uci +libcjson +libm +huayi_core
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_cmd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_mac $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_sn $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_msg $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/huayi
	$(INSTALL_DATA) ./files/huayi_control.conf $(1)/etc/huayi
endef
	
$(eval $(call BuildPackage,$(PKG_NAME)))