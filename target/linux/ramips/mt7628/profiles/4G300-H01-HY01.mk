#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/4G300-H01-HY01
	NAME:=4G300-H01-HY01
	PACKAGES:= -swconfig -rt2x00 \
		ated regs gpio btnd switch ethstt uci2dat mii_mgr watchdog 8021xd \
		wireless-tools xl2tpd block-mount fstools kmod-scsi-generic \
		kmod-usb-core kmod-usb2 kmod-usb-storage \
		kmod-fs-vfat kmod-fs-ntfs kmod-nls-base kmod-nls-utf8 kmod-nls-cp936 \
        kmod-nls-cp437 kmod-nls-cp850 kmod-nls-iso8859-1 kmod-nls-iso8859-15 kmod-nls-cp950
endef

define Profile/4G300-H01-HY01/Description
	4G300-H01-HY01 package set compatible with most boards.
endef
$(eval $(call Profile,4G300-H01-HY01))



