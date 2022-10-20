#
# Copyright (c) 2013 The Linux Foundation. All rights reserved.
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=hvl_plm_ether
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/hvl_plm_ether
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE Kernel Modules
  TITLE:=LM101A NIDS driver
  DEPENDS:=+kmod-usb-core +kmod-usb-serial +kmod-usb-serial-option +kmod-usb-net
  FILES:=$(PKG_BUILD_DIR)/hvl_plm_ether.ko
#  AUTOLOAD:=$(call AutoLoad,62,hvl_plm_ether)
  KCONFIG:=
endef

define KernelPackage/hvl_plm_ether/description
  Kernel module to NDIS LM101A
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

define KernelPackage/hvl_plm_ether/install
	echo xxx
endef

$(eval $(call KernelPackage,hvl_plm_ether))
