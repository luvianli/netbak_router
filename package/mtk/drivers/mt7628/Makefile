# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt7628
PKG_VERSION:=4.1.0.0
PKG_SOURCE:=MT7628_LinuxAP_V4.1.0.0_DPA_20160310-3600.tar.bz2

include $(INCLUDE_DIR)/package.mk
PKG_AUTOLOAD= $(PKG_NAME)

define KernelPackage/mt7628
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7628 wifi driver
  DEPENDS:=+mt7628-scripts +wifi-l1profile +mtk-base-files
  MENU:=1
  AUTOLOAD:=$(call AutoLoad,53,$(PKG_AUTOLOAD))
endef

define KernelPackage/mt7628-in
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7628 BUILD_IN
  DEPENDS:=+mt7628-scripts
  FILES:=
  AUTOLOAD:=
  KCONFIG:= \
    CONFIG_WIFI_MT7628=y \
    CONFIG_MT7628_AP_SUPPORT=y
endef

define KernelPackage/mt7628-ko
  HIDDEN:=1
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7628 BUILD_KO
  DEPENDS:=+mt7628-scripts
  FILES:=$(LINUX_DIR)/drivers/net/wireless/mtk/mt7628/mt7628_wifi_ap/mt7628.ko
  AUTOLOAD:=$(call AutoProbe,mt7628)
  KCONFIG:= \
    CONFIG_WIFI_MT7628=y \
    CONFIG_MT7628_AP_SUPPORT=m
endef


define KernelPackage/mt7628/config
	if PACKAGE_kmod-mt7628
	source "$(SOURCE)/config.in"
	endif
endef

define Build/Prepare
	@echo "this package is built with kernel"
endef

define Build/Compile
	@echo "this package is built with kernel"
endef

define Package/mt7628-scripts
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MT7628 scripts
  DEPENDS:=+mtk-base-files
  HIDDEN:=1
endef

define Package/mt7628-scripts/install
	$(INSTALL_DIR) $(1)/lib/wifi/
	-if [ "$$(CONFIG_WIFI_SCRIPT_LUA)" = "y" ]; then \
	$(INSTALL_BIN) ./files/mt7628.lua $(1)/lib/wifi/; \
	elif [ "$$(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)" = "y" ]; then \
	$(INSTALL_BIN) ./files/huayi_mt7628.sh $(1)/lib/wifi/mt7628.sh; \
	else \
	$(INSTALL_BIN) ./files/mt7628.sh $(1)/lib/wifi/; \
	fi
	
	$(INSTALL_DIR) $(1)/etc/wireless/mt7628/
	
	-if [ "$$(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)" = "y" ]; then \
	$(INSTALL_BIN) ./files/huayi_mt7628.dat $(1)/etc/wireless/mt7628/mt7628.dat; \
	else \
	$(INSTALL_BIN) ./files/mt7628.dat $(1)/etc/wireless/mt7628/mt7628.dat; \
	fi
	-if [ "$$(CONFIG_INTERNAL_PA_INTERNAL_LNA)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7628.eeprom.ipa.ilna.bin $(1)/etc/wireless/mt7628/mt7628.eeprom.bin; \
	elif [ "$$(CONFIG_INTERNAL_PA_EXTERNAL_LNA)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7628.eeprom.ipa.elna.bin $(1)/etc/wireless/mt7628/mt7628.eeprom.bin; \
	elif [ "$$(CONFIG_EXTERNAL_PA_EXTERNAL_LNA)" = "y" ]; then \
		$(INSTALL_BIN) ./files/mt7628.eeprom.epa.elna.bin $(1)/etc/wireless/mt7628/mt7628.eeprom.bin; \
	else \
		$(INSTALL_BIN) ./files/mt7628.eeprom.ipa.elna.bin $(1)/etc/wireless/mt7628/mt7628.eeprom.bin; \
	fi
	echo $(PKG_VERSION) > $(1)/etc/wireless/mt7628/version
endef

define Package/mt7628-scripts/conffiles
/etc/wireless/mt7628/
endef

$(eval $(call KernelPackage,mt7628))
$(eval $(call KernelPackage,mt7628-in))
$(eval $(call KernelPackage,mt7628-ko))
$(eval $(call BuildPackage,mt7628-scripts))
