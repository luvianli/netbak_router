# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt7620
#PKG_VERSION:=gerrit
#PKG_SOURCE:=
PKG_VERSION:=MT7620_LinuxAP_V2.7.2.0
PKG_SOURCE:=MT7620_LinuxAP_V2.7.2.0.tar.bz2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt7620
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7620 wifi driver
  DEPENDS:=+mt7620-scripts +wifi-l1profile +mtk-base-files
  MENU:=1
endef

define KernelPackage/mt7620-in
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7620 BUILD_IN
  DEPENDS:=+mt7620-scripts
  FILES:=
  AUTOLOAD:=
  KCONFIG:= \
    CONFIG_WIFI_MT7620=y \
    CONFIG_MT7620_RALINK_RT6352=y \
    CONFIG_MT7620_RT2860V2_AP=y
endef

define KernelPackage/mt7620-ko
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7620 BUILD_KO
  DEPENDS:=+mt7620-scripts
  FILES:=$(LINUX_DIR)/drivers/net/wireless/mtk/mt7620/mt7620_wifi_ap/mt7620.ko
  AUTOLOAD:=$(call AutoProbe,mt7620)
  KCONFIG:= \
    CONFIG_WIFI_MT7620=y \
    CONFIG_MT7620_RALINK_RT6352=y \
    CONFIG_MT7620_RT2860V2_AP=m
endef


define KernelPackage/mt7620/config
	if PACKAGE_kmod-mt7620
	source "$(SOURCE)/config.in"
	endif
endef

define Build/Prepare
	@echo "this package is built with kernel"
endef

define Build/Compile
	@echo "this package is built with kernel"
endef

define Package/mt7620-scripts
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=mt7620 scripts
  DEPENDS:=+mtk-base-files
  HIDDEN:=1
endef

define Package/mt7620-scripts/install
	$(INSTALL_DIR) $(1)/lib/wifi/
	-if [ "$$(CONFIG_WIFI_SCRIPT_LUA)" = "y" ]; then \
	$(INSTALL_BIN) ./files/mt7620.lua $(1)/lib/wifi/; \
	elif [ "$$(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)" = "y" ]; then \
	$(INSTALL_BIN) ./files/huayi_mt7620.sh $(1)/lib/wifi/mt7620.sh; \
	else \
	$(INSTALL_BIN) ./files/mt7620.sh $(1)/lib/wifi/; \
	fi
	$(INSTALL_DIR) $(1)/etc/wireless/mt7620/
	-if [ "$$(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)" = "y" ]; then \
	$(INSTALL_BIN) ./files/huayi_mt7620.dat $(1)/etc/wireless/mt7620/mt7620.dat; \
	else \
	$(INSTALL_BIN) ./files/mt7620.dat $(1)/etc/wireless/mt7620/; \
	fi
	$(INSTALL_BIN) ./files/SingleSKU.dat $(1)/etc/wireless/mt7620/
	-if [ "$$(CONFIG_INTERNAL_PA_INTERNAL_LNA)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7620.eeprom.ipa.ilna.bin $(1)/etc/wireless/mt7620/mt7620.eeprom.bin; \
	elif [ "$$(CONFIG_INTERNAL_PA_EXTERNAL_LNA)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7620.eeprom.ipa.elna.bin $(1)/etc/wireless/mt7620/mt7620.eeprom.bin; \
	elif [ "$$(CONFIG_EXTERNAL_PA_EXTERNAL_LNA)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7620.eeprom.epa.elna.bin $(1)/etc/wireless/mt7620/mt7620.eeprom.bin; \
	else \
		$(INSTALL_BIN) ./files/mt7620.eeprom.ipa.elna.bin $(1)/etc/wireless/mt7620/mt7620.eeprom.bin; \
	fi
	echo $(PKG_VERSION) > $(1)/etc/wireless/mt7620/version
endef

define Package/mt7620-scripts/conffiles
/etc/wireless/mt7620/
endef

$(eval $(call KernelPackage,mt7620))
$(eval $(call KernelPackage,mt7620-in))
$(eval $(call KernelPackage,mt7620-ko))
$(eval $(call BuildPackage,mt7620-scripts))
