#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_init
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI INIT MODULE
  DEPENDS:=+kmod-huayi-universal +libuci +huayi_uci +libcjson +libm
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/huayi_init $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/config_system $(1)/etc/init.d/config_system
	$(INSTALL_BIN) ./files/huayi_init.init $(1)/etc/init.d/huayi_init
	$(INSTALL_DIR) $(1)/etc/hotplug.d/
	$(INSTALL_DIR) $(1)/lib/netifd/
ifeq ($(CONFIG_PACKAGE_huayi_wired_prioroty),y)
	$(INSTALL_BIN) ./files/huayi-proto-wired-prioroty.sh $(1)/lib/netifd/huayi-proto.sh
else 
	$(INSTALL_BIN) ./files/huayi-proto.sh $(1)/lib/netifd/
endif

ifeq ($(CONFIG_PACKAGE_huayi_wisp_4g),y)
	$(INSTALL_BIN) ./files/huayi-proto-rep4g-prioroty.sh $(1)/lib/netifd/huayi-proto.sh
endif
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
ifeq ($(CONFIG_PACKAGE_huayi_wired_prioroty),y)
	$(INSTALL_BIN) ./files/30-huayi-netstate-wired-prioroty.hotplug $(1)/etc/hotplug.d/iface/30-huayi-netstate
else 
	$(INSTALL_BIN) ./files/30-huayi-netstate.hotplug $(1)/etc/hotplug.d/iface/30-huayi-netstate
endif

ifeq ($(CONFIG_PACKAGE_huayi_wisp_4g),y)
	$(INSTALL_BIN) ./files/30-huayi-netstate-rep4g-prioroty.hotplug $(1)/etc/hotplug.d/iface/30-huayi-netstate
endif

endef
	
$(eval $(call BuildPackage,$(PKG_NAME)))