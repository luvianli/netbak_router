#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_X3A_JH
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI X3A JTCARD
  DEPENDS:=+kmod-huayi-universal +libpthread +libuci +huayi_uci +libcjson +libm
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files//jtcard_sdk $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/hy_jtcard_sdk.init $(1)/etc/init.d/hy_jtcard_sdk
endef
	
$(eval $(call BuildPackage,$(PKG_NAME)))