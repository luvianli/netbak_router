#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_OWN:=HUAYI

PKG_NAME:=huayi-filter
PKG_REV:=1
PKG_VERSION:=$(PKG_REV)

PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:= ./src/
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)

PKG_FILES:= $(PKG_BUILD_DIR)/huayi_qos.ko $(PKG_BUILD_DIR)/huayi_filter_hook.ko


PKG_AUTOLOAD:= huayi_qos huayi_filter_hook


include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=HUAYI Kernel Modules
  TITLE:=HUAYI RUNSTATE MODULE
  MAINTAINER:= <"gerry_li">
  DEPENDS:=+kmod-huayi-universal +kmod-huayi-control
  FILES:=$(PKG_FILES)
  AUTOLOAD:=$(call AutoLoad,3,$(PKG_AUTOLOAD))
endef


define KernelPackage/$(PKG_NAME)/description
	This package contains huayi filter module for HUAYI.
endef

MAKE_ARGS:= \
	PATH="$(TARGET_PATH)" \
	ARCH="$(LINUX_KARCH)" \
	ARCH-y="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	KERNELPATH="$(LINUX_DIR)" \
	LDOPTS="--no-warn-mismatch " \
	$(TARGET_CONFIGURE_OPTS)

HUAYI_INCS := -I$(STAGING_DIR)/usr/include



MAKE_ARGS += HUAYI_INCS=$(HUAYI_INCS)
MAKE_ARGS += HUAYI_COPTS="$(EXTRA_CFLAGS)"
MAKE_ARGS += CFLAGS="$(TARGET_CFLAGS)"


$(STAMP_BUILT): $(shell find $(PKG_SOURCE_SUBDIR) -name '*.[c|h]')

Hooks/Compile/Pre+=HUAYI/Compile/Pre

define HUAYI/Compile/Pre
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -rf $(PKG_SOURCE_SUBDIR)* $(PKG_BUILD_DIR)/.
endef

update:
	@echo "==============================================================================="
	@echo " Override default 'compile' rule"
	@echo " to copy ($(PKG_SOURCE_SUBDIR))'s modified codes to build_dir to re-compile"
	@echo "==============================================================================="
	$(call HUAYI/Compile/Pre)
	$(call Build/Compile)

define Build/Prepare
	$(call HUAYI/Compile/Pre)
endef

define Build/Compile/$(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR)/ $(MAKE_ARGS) all
endef

define Build/Compile
	$(call Build/Compile/$(PKG_NAME))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/$(PKG_NAME)
	$(CP) $(PKG_BUILD_DIR)/include/* $(STAGING_DIR)/usr/include/$(PKG_NAME)
endef

define KernelPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ipqos $(1)/etc/init.d
endef

$(eval $(call KernelPackage,$(PKG_NAME)))

