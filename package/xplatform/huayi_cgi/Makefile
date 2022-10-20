#
# Copyright (c) 2013 The Linux Foundation. All rights reserved.
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=huayi_cgi
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=Xplatform
  CATEGORY:=Xplatform
  TITLE:=HUAYI FAST CGI
ifeq ($(CONFIG_PACKAGE_huayi_nginx),y)
  DEPENDS:= +kmod-huayi-universal +libuci +huayi_uci +huayi_core +fcgi +spawn-fcgi +libcjson +zlib +libm +libopenssl\
  			+PACKAGE_huayi_mysql:libmysqlclient +PACKAGE_huayi_mysql:huayi_mysql
else
  DEPENDS:= +kmod-huayi-universal +libuci +huayi_uci +huayi_core +libcjson +libm\
  			+PACKAGE_huayi_sqlite3:libsqlite3 +PACKAGE_huayi_sqlite3:huayi_sqlite3
endif
endef

define Package/$(PKG_NAME)/description
  HUAYI Fast CGI 
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	if [ -z $$(CONFIG_HUAYI_CGI_INSTALL_PATH) ]; then \
		echo "Warning:please check your config about HUAYI cgi install path" >&2; \
	fi
	$(INSTALL_DIR) $(1)$(CONFIG_HUAYI_CGI_INSTALL_PATH)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.cgi $(1)$(CONFIG_HUAYI_CGI_INSTALL_PATH)
ifeq ($(CONFIG_PACKAGE_huayi_nginx),y)
	$(INSTALL_DIR) $(1)/etc/init.d/ 
	$(INSTALL_BIN) ./files/huayi_cgi.init $(1)/etc/init.d/huayi_cgi	
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))