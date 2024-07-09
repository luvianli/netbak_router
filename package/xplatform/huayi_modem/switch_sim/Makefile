########################################################
#  Copyright (C) 2018-2099 Quectel Wireless Co.,Ltd. 
#  Quectel Wireless Proprietary and Confidential.
#
#  Compile helloworld.
#  Created by Nebula.Li 2019-07-24
#
########################################################

include $(TOPDIR)/rules.mk

PKG_NAME:=switch_sim
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  SUBMENU:=LTE
  TITLE:=Switch Sim Card
  DEPENDS:=+libpthread +huayi-universal +libuci +huayi_uci +libcjson +libpthread
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP)  ./src/* $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/switch_sim $(1)/usr/bin/switch_sim
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
