#
# Copyright (c) 2013 The Linux Foundation. All rights reserved.
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=yuga_GobiNet
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/yuga_GobiNet
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE Kernel Modules
  TITLE:=Yuga CLM920 Module Driver
  FILES:=$(PKG_BUILD_DIR)/yuga_GobiNet.ko
  DEPENDS:=+kmod-usb-core +kmod-usb-serial +kmod-usb-serial-option +kmod-usb-net
  #AUTOLOAD:=$(call AutoLoad,62,yuga_GobiNet)
  KCONFIG:=
endef

define KernelPackage/yuga_GobiNet/description
  Kernel module for YUGA CLM920 ...
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

define KernelPackage/yuga_GobiNet/install
	echo yuga_GobiNet install...
endef

$(eval $(call KernelPackage,yuga_GobiNet))

