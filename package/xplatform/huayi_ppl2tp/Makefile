#
# Copyright (c) 2013 The Linux Foundation. All rights reserved.
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_ppl2tp
PKG_VERSION:=1.1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:=VPN PPTP  L2TP Client
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/pptp.init $(1)/etc/init.d/pptp
	
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/ppl2tp.conf $(1)/etc/config/ppl2tp
	
	$(INSTALL_DIR) $(1)/etc/ppp
	$(INSTALL_DATA) ./files/options.pptp $(1)/etc/ppp/
	$(INSTALL_BIN) ./files/pptp-down $(1)/etc/ppp/pptp-down
	$(INSTALL_BIN) ./files/pptp-up $(1)/etc/ppp/pptp-up
	
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/xl2tp.init $(1)/etc/init.d/xl2tp
	
	$(INSTALL_DIR) $(1)/etc/ppp
	$(INSTALL_DATA) ./files/options.xl2tp $(1)/etc/ppp/
	$(INSTALL_BIN) ./files/xl2tp-down $(1)/etc/ppp/xl2tp-down
	$(INSTALL_BIN) ./files/xl2tp-up $(1)/etc/ppp/xl2tp-up

	$(INSTALL_DIR) $(1)/etc/xl2tp 
	$(INSTALL_DATA) ./files/xl2tp.conf $(1)/etc/xl2tp/xl2tp.conf
endef

$(eval $(call BuildPackage,$(PKG_NAME)))