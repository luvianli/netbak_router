#
# Copyright (c) 2013 The Linux Foundation. All rights reserved.
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=quectel_GobiNet
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/quectel_GobiNet
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE Kernel Modules
  TITLE:=Quectel EC25 Driver
  FILES:=$(PKG_BUILD_DIR)/quectel_GobiNet.ko
  DEPENDS:=+kmod-usb-core +kmod-usb-serial +kmod-usb-serial-option +kmod-usb-net
  #AUTOLOAD:=$(call AutoLoad,62,quectel_GobiNet)
  KCONFIG:=
endef

define KernelPackage/quectel_GobiNet/description
  Kernel module for Quectel EC25/EC20/EC2x ...
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

define KernelPackage/quectel_GobiNet/install
	echo quectel_GobiNet install...
#	[ -e ./files/$(BOARD) ] && \
#	  $(INSTALL_DIR) $(1)/etc/uci-defaults && \
#	  $(INSTALL_BIN) ./files/$(BOARD) $(1)/etc/uci-defaults/quectel_GobiNet || \
#	  true
endef

$(eval $(call KernelPackage,quectel_GobiNet))

