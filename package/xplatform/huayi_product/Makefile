#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=huayi_product
PKG_RELEASE:=1.1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
#HUAYI_VERSION:=$(shell git tag | grep $(CONFIG_HUAYI_PRODUCT_MODEL) | awk 'END {print}' | awk -F "-" '{print $$NF}' )
HUAYI_VERSION:=$(shell git for-each-ref --sort=taggerdate | grep $(CONFIG_HUAYI_PRODUCT_MODEL)- | awk 'END {print}' | awk -F "-" '{print $$NF}' )
#HUAYI_VERSION:=$(shell cat $(TOPDIR)/package/base-files/files/etc/openwrt_version )

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:=HUAYI PRODUCT customized information include web
  DEPENDS:=+base-files
endef

define Package/$(PKG_NAME)/description
 huayi product default config customized information
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
ifeq ($(CONFIG_NEWUI_MTK3.6),y)
ifeq ($(CONFIG_HUAYI_WEB_RELEASE),y)
	@echo huayi product release web code clone from version tag
	cd $(PKG_BUILD_DIR)/ && git clone -b $(CONFIG_HUAYI_PRODUCT_MODEL)-$(HUAYI_VERSION)  git@192.168.99.188:rex/newui_mtk3.6.git \
	&& cd newui_mtk3.6/ && yarn && gulp config --pro $(CONFIG_HUAYI_PRODUCT_MODEL) \
	&& gulp build
else
ifeq ($(CONFIG_HUAYI_WEB_DEVELOPMENT),y)
	@echo huayi product release web code clone from master branch
	cd $(PKG_BUILD_DIR)/ && git clone git@192.168.99.188:rex/newui_mtk3.6.git \
	&& cd newui_mtk3.6/ && yarn && gulp config --pro $(CONFIG_HUAYI_PRODUCT_MODEL) \
	&& gulp build
else
	@echo huayi product use local web files instead of get from git
	mkdir -p $(PKG_BUILD_DIR)/newui_mtk3.6/build
	$(CP) ./local_web/$(CONFIG_HUAYI_PRODUCT_MODEL)/* $(PKG_BUILD_DIR)/newui_mtk3.6/build
endif
endif
else
ifeq ($(CONFIG_HUAYI_WEB_RELEASE),y)
	@echo huayi product release web code clone from version tag
	cd $(PKG_BUILD_DIR)/ && git clone -b $(CONFIG_HUAYI_PRODUCT_MODEL)-$(HUAYI_VERSION)  git@192.168.99.188:rex/huayi_web_mtk3.6.git \
	&& cd huayi_web_mtk3.6/ && yarn && gulp config --pro $(CONFIG_HUAYI_PRODUCT_MODEL) \
	&& gulp build
else
ifeq ($(CONFIG_HUAYI_WEB_DEVELOPMENT),y)
	@echo huayi product release web code clone from master branch
	cd $(PKG_BUILD_DIR)/ && git clone git@192.168.99.188:rex/huayi_web_mtk3.6.git \
	&& cd huayi_web_mtk3.6/ && yarn && gulp config --pro $(CONFIG_HUAYI_PRODUCT_MODEL) \
	&& gulp build
else
	@echo huayi product use local web files instead of get from git
	mkdir -p $(PKG_BUILD_DIR)/huayi_web_mtk3.6/build
	$(CP) ./local_web/$(CONFIG_HUAYI_PRODUCT_MODEL)/* $(PKG_BUILD_DIR)/huayi_web_mtk3.6/build
endif
endif
endif
endef

define Build/Compile/Default
endef

Build/Compile = $(Build/Compile/Default)

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/www/
ifeq ($(CONFIG_NEWUI_MTK3.6),y)
	$(CP) $(PKG_BUILD_DIR)/newui_mtk3.6/build/* $(1)/www/
else
	$(CP) $(PKG_BUILD_DIR)/huayi_web_mtk3.6/build/* $(1)/www/
endif
	$(INSTALL_DIR) $(1)/
	$(CP) ./product_info/$(CONFIG_HUAYI_PRODUCT_MODEL)/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
