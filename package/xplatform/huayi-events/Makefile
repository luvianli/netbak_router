#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_OWN:=HUAYI

PKG_NAME:=huayi-events

PKG_REV:=1
PKG_VERSION:=r$(PKG_REV)
PKG_RELEASE:=0

PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=./src
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)


LIBNAME := libhuayievents

###### build the sample program ######
BUILD_SAMPLE:=n

include $(INCLUDE_DIR)/package.mk

EVENTS_FILES:=$(PKG_BUILD_DIR)/$(PKG_NAME).ko 
EVENTS_AUTOLOAD:=$(PKG_NAME)

define KernelPackage/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=HUAYI Kernel Modules
  TITLE:=HUAYI UNIVERSAL UTILITIES
  MAINTAINER:= gerry_li
  DEPENDS:= +kmod-huayi-universal
  FILES:=$(EVENTS_FILES)
  AUTOLOAD:=$(call AutoLoad,56,$(EVENTS_AUTOLOAD))
endef

define KernelPackage/$(PKG_NAME)/description
 This package contains a message queue for Altai.
endef

MAKE_ARGS:= \
	PATH="$(TARGET_PATH)" \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	KERNELPATH="$(LINUX_DIR)" \
	LDOPTS="--no-warn-mismatch " 
	
HUAYI_INCS := -I$(STAGING_DIR)/usr/include

MAKE_ARGS += HUAYI_INCS="$(HUAYI_INCS)"
MAKE_ARGS += HUAYI_COPTS="$(EXTRA_CFLAGS)"
MAKE_ARGS += CFLAGS="$(TARGET_CFLAGS)"

$(STAMP_BUILT): $(shell find $(PKG_SOURCE_SUBDIR) -name '*.[c|h]')

define Build/Prepare
	cp -rf $(PKG_SOURCE_SUBDIR)/* $(PKG_BUILD_DIR)/
endef


define Build/Compile
	$(call Build/Prepare)
	$(MAKE) -C $(PKG_BUILD_DIR)/ $(MAKE_ARGS) mods
	$(MAKE) -C $(PKG_BUILD_DIR)/ $(CONFIGURE_VARS) ALT_COPTS=$(ALT_COPTS) $(LIBNAME) BUILD_USERSPACE=y
  ifeq ($(BUILD_SAMPLE),y)	
	$(MAKE) -C $(PKG_BUILD_DIR)/ $(CONFIGURE_VARS) ALT_COPTS=$(ALT_COPTS) testevent
  endif	
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib
	$(CP) -f $(PKG_BUILD_DIR)/$(LIBNAME).so $(STAGING_DIR)/usr/lib/
endef

define KernelPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(LIBNAME).so  $(1)/usr/lib/
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
