#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_sim_change
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI SIM CARD CHANGE
  DEPENDS:=+kmod-huayi-universal +libpthread +libuci +huayi_uci +libcjson +libm
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/$(PKG_NAME)
	$(CP) $(PKG_BUILD_DIR)/include/* $(STAGING_DIR)/usr/include/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hy_sc $(1)/usr/bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hy_sc $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/hy_sim_change.init $(1)/etc/init.d/hy_sc
endef
	
$(eval $(call BuildPackage,$(PKG_NAME)))