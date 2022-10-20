#
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_mode_change
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:= HUAYI SYSTEM OPMODE change
  DEPENDS:=
endef

define Package/$(PKG_NAME)/config
	select BUSYBOX_CONFIG_UDHCPD
endef

define Build/Compile/Default
endef

Build/Compile = $(Build/Compile/Default)

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d/	
ifeq ($(CONFIG_HUAYI_SUPPORT_REPEATER_DHCPD),y)
	$(INSTALL_BIN) ./files/huayi_udhcpd_repeater.init $(1)/etc/init.d/huayi_udhcpd
else
	$(INSTALL_BIN) ./files/huayi_udhcpd.init $(1)/etc/init.d/huayi_udhcpd
endif
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
ifeq ($(CONFIG_HUAYI_SUPPORT_REPEATER_DHCPD),y)
	$(INSTALL_BIN) ./files/31-huayi-mode-change-repeater.hotplug $(1)/etc/hotplug.d/iface/31-huayi-mode-change
else
	$(INSTALL_BIN) ./files/31-huayi-mode-change.hotplug $(1)/etc/hotplug.d/iface/31-huayi-mode-change
endif
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_BIN) ./files/huayi_udhcpd.config $(1)/etc/config/huayi_udhcpd
endef
	
$(eval $(call BuildPackage,$(PKG_NAME)))