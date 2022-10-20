#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_OWN:=HUAYI

PKG_NAME:=huayi-khello
PKG_KNAME:=huayi_khello

PKG_FILES= \
	$(PKG_BUILD_DIR)/$(PKG_KNAME).ko

PKG_REV:=1
PKG_VERSION:=r$(PKG_REV)
PKG_SOURCE_VERSION:=$(PKG_REV)

PKG_SOURCE_SUBDIR:= ./src

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
#in order to support direction finding , need to insmod df.ko before stt.ko 

PKG_AUTOLOAD= $(PKG_KNAME)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=HUAYI Kernel Modules
  TITLE:=HUAYI HelloWorld MODULES
  MAINTAINER:=gerry
  DEPENDS:=+libc +libgcc
  FILES:=$(PKG_FILES)
  AUTOLOAD:=$(call AutoLoad,55,$(PKG_AUTOLOAD))
endef


define KernelPackage/$(PKG_NAME)/description
 This package implements the huayi hello module
endef

MAKE_ARGS:= \
	PATH="$(TARGET_PATH)" \
	ARCH="$(LINUX_KARCH)" \
	ARCH-y="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	$(TARGET_CONFIGURE_OPTS) \
	TOOLPREFIX="$(KERNEL_CROSS)" \
	TOOLPATH="$(KERNEL_CROSS)" \
	KERNELPATH="$(LINUX_DIR)" \
	LDOPTS="--no-warn-mismatch "


HUAYI_INCS := -I$(STAGING_DIR)/usr/include

MAKE_ARGS += HUAYI_INCS="$(HUAYI_INCS)"
MAKE_ARGS += HUAYI_COPTS="$(EXTRA_CFLAGS)"
MAKE_ARGS += CFLAGS="$(TARGET_CFLAGS)"


$(STAMP_BUILT): $(shell find $(PKG_SOURCE_SUBDIR) -name '*.[c|h]')

define Build/Prepare
endef

Hooks/Compile/Pre+=HUAYI/Compile/Pre
define HUAYI/Compile/Pre
	@echo "==============================================================================="
	@echo " -- HUAYI Hooks/Compile/Pre --"
	@echo " Copy HUAYI package source ($(PKG_NAME)) to build_dir to compile"
	@echo "==============================================================================="
	$(CP) -rf $(PKG_SOURCE_SUBDIR)/* $(PKG_BUILD_DIR)/.
endef

define Build/Compile/$(PKG_NAME)
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR)/ $(MAKE_ARGS) all 
endef

define Build/Compile
	$(call Build/Compile/$(PKG_NAME))
endef

define Build/InstallDev
endef

$(eval $(call KernelPackage,$(PKG_NAME)))

