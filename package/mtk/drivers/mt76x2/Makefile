# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt76x2
PKG_VERSION:=MT76x2E_MT7620_LinuxAP_V3.0.5.0
PKG_SOURCE:=MT76x2E_MT7620_LinuxAP_V3.0.5.0_DPA_20160121.tar.bz2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt76x2
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT76x2 wifi driver
  DEPENDS:=+mt76x2-scripts +wifi-l1profile +mtk-base-files +wifi_services
  MENU:=1
endef

define KernelPackage/mt7612-in
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7612e BUILD_IN
  DEPENDS:=+mt7612-scripts
  FILES:=
  AUTOLOAD:=
  KCONFIG:= \
    CONFIG_WIFI_MT7612E=y \
    CONFIG_MT7612E_CHIP_MT7612E=y \
    CONFIG_MT7612E_MT_WIFI=y \
    CONFIG_MT7612E_MT_AP_SUPPORT=y \
    CONFIG_MT7612E_WIFI_MODE_AP=y
endef

define KernelPackage/mt7612-ko
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7612e BUILD_KO
  DEPENDS:=+mt7612-scripts
  FILES:=$(LINUX_DIR)/drivers/net/wireless/mtk/mt76x2/mt76x2_wifi_ap/mt76x2.ko
  AUTOLOAD:=$(call AutoProbe,mt7612)
  KCONFIG:= \
    CONFIG_WIFI_MT7612E=y \
    CONFIG_MT7612E_CHIP_MT7612E=y \
    CONFIG_MT7612E_MT_WIFI=m \
    CONFIG_MT7612E_MT_AP_SUPPORT=m \
    CONFIG_MT7612E_WIFI_MODE_AP=m
endef

define KernelPackage/mt76x2/config
	if PACKAGE_kmod-mt76x2
	source "$(SOURCE)/config-mt7612.in"
	endif
endef

define Package/mt7612-scripts
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=mt7612 scripts
  DEPENDS:=+mtk-base-files
  HIDDEN:=1
endef

define Build/Prepare
	@echo "this package is built with kernel"
endef

define Build/Compile
	@echo "this package is built with kernel"
endef

define Package/mt7612-scripts/install
	$(INSTALL_DIR) $(1)/lib/wifi/
	$(INSTALL_DIR) $(1)/etc/wireless/mt7612/
	$(INSTALL_BIN) ./files/mt7612.dat $(1)/etc/wireless/mt7612/
	echo $(PKG_VERSION) > $(1)/etc/wireless/mt7612/version
	-if [ "$$(CONFIG_WIFI_SCRIPT_LUA)" = "y" ]; then \
	$(INSTALL_BIN) ./files/mt7612.lua $(1)/lib/wifi/; \
	elif [ "$$(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)" = "y" ]; then \
	$(INSTALL_BIN) ./files/mt7612.sh $(1)/lib/wifi/mt7612.sh; \
	else \
	$(INSTALL_BIN) ./files/mt7612.sh $(1)/lib/wifi/; \
	fi

	if [ "$$(CONFIG_KCONFIG_FIRST_IF_MT7612E)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7612*.bin $(1)/etc/wireless/mt7612/; \
	fi
	if [ "$$(CONFIG_FIRST_MT7612E_NORMAL)" = "y" ]; then \
		if [ "$$(CONFIG_KCONFIG_SECOND_IF_MT7603E)" = "y" ]; then \
			if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
			$(INSTALL_BIN) ./files/mt7612.map.dat $(1)/etc/wireless/mt7612/mt7612.dat; \
			else \
			$(INSTALL_BIN) ./files/mt7612.dat $(1)/etc/wireless/mt7612/; \
			fi \
		else \
			if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
			$(INSTALL_BIN) ./files/mt7612.dat $(1)/etc/wireless/mt7612/mt7612.dat; \
			else \
			$(INSTALL_BIN) ./files/mt7612.dat $(1)/etc/wireless/mt7612/; \
			fi \
		fi \
	fi
	if [ "$$(CONFIG_FIRST_MT7612E_DBDC)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7612.1.2G.map.dat $(1)/etc/wireless/mt7612/mt7612.1.2G.dat; \
		$(INSTALL_BIN) ./files/mt7612.1.5G.map.dat $(1)/etc/wireless/mt7612/mt7612.1.5G.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7612.1.2G.dat $(1)/etc/wireless/mt7612/; \
		$(INSTALL_BIN) ./files/mt7612.1.5G.dat $(1)/etc/wireless/mt7612/; \
		fi \
	fi

	if [ "$$(CONFIG_KCONFIG_SECOND_IF_MT7612E)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7612*.bin $(1)/etc/wireless/mt7612/; \
	fi
	if [ "$$(CONFIG_SECOND_MT7612E_NORMAL)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
			if [ "$$(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)" = "y" ]; then \
				$(INSTALL_BIN) ./files/huayi_mt7612.dat $(1)/etc/wireless/mt7612/mt7612.dat; \
			else \
				$(INSTALL_BIN) ./files/mt7612.map.dat $(1)/etc/wireless/mt7612/mt7612.dat; \
			fi \
		else \
		$(INSTALL_BIN) ./files/mt7612.dat $(1)/etc/wireless/mt7612/; \
		fi \
	fi
	if [ "$$(CONFIG_SECOND_MT7612E_DBDC)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7612.2G.map.dat $(1)/etc/wireless/mt7612/mt7612.2G.dat ; \
		$(INSTALL_BIN) ./files/mt7612.5G.map.dat $(1)/etc/wireless/mt7612/mt7612.5G.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7612.2G.dat $(1)/etc/wireless/mt7612/; \
		$(INSTALL_BIN) ./files/mt7612.5G.dat $(1)/etc/wireless/mt7612/; \
		fi \
	fi

	if [ "$$(CONFIG_KCONFIG_THIRD_IF_MT7612E)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7612*.bin $(1)/etc/wireless/mt7612/; \
	fi
	if [ "$$(CONFIG_THIRD_MT7612E_NORMAL)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7612.3.map.dat $(1)/etc/wireless/mt7612/mt7612.3.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7612.3.dat $(1)/etc/wireless/mt7612/; \
		fi \
	fi
	if [ "$$(CONFIG_THIRD_MT7612E_DBDC)" = "y" ]; then \
		if [ "$$(CONFIG_PACKAGE_libmapd)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7612.3.2G.map.dat $(1)/etc/wireless/mt7612/mt7612.3.2G.dat; \
		$(INSTALL_BIN) ./files/mt7612.3.5G.map.dat $(1)/etc/wireless/mt7612/mt7612.3.5G.dat; \
		else \
		$(INSTALL_BIN) ./files/mt7612.3.2G.dat $(1)/etc/wireless/mt7612/; \
		$(INSTALL_BIN) ./files/mt7612.3.5G.dat $(1)/etc/wireless/mt7612/; \
		fi \
	fi
endef

define Package/mt7612-scripts/conffiles
/etc/wireless/mt7612/
endef

$(eval $(call KernelPackage,mt76x2))
$(eval $(call KernelPackage,mt7612-ko))
$(eval $(call KernelPackage,mt7612-in))
$(eval $(call BuildPackage,mt7612-scripts))
