##############################
##      README file         ##
##############################

+ How to build
	Please select one of HOST_TYPE(EMBEDDED or LINUX_PC) in Makefile.
	You need to modify COMPILER_PATH and COMPILER_PREFIX appropriately
	And run below to compile "cli_cm_atc".
	# make clean
	# make

+ How to install
	vlan_setup.sh file need to be placed with cli_cm_atc.
	When you copy cli_cm_atc to host device, you also need to copy this file.
	And you need to setup hotplug detection script if you use USB interface between GDM724x and host device.
	Please see below section, "USB hotplug detection" for more details.

+ USB hotplug detection
1. "/proc/sys/kernel/hotplug" should contain below and this can be run in /etc/init.d/rcS

	#echo "/sbin/hotplug" > /proc/sys/kernel/hotplug

2. /sbin/hotplug script will run automatically when USB hotplug event occurred.
	"hotplug" script should contain below because application will check the $ACTION in /tmp/usb_status
	You can refer hotplug file in source code folder.
	
	#vi /sbin/hotplug
		#!/bin/sh
		echo "hotplug detected : "$ACTION > /var/log/usb_status


+ How to add new command list.
1. Open cmd.c file
2. Add FUNCTION_INDEX
3. Add new command in the cmd_list[].
4. Define command function accordingly. 
5. Add command help detailed description in the function "cmd_help_detail()" if needed.


+ How to change current log level.
1. Open ltetype.h file
2. Change below line in ltetype.h.
	#define CURRENT_LOG_LEVEL	LOG_LEVEL_INFO


+ How to setup VLAN mode device firmware (IMS & Internet)
	ucfg set config wan lte vendor 1
	ucfg set config wan lte autocm manual 1
	ucfg set config wan lte atcm use_autocm_req 1
	ucfg set config lan vlan ENABLE 1
	ucfg set config lan vlan lte0pdn0 ENABLE 1
	ucfg set config lan vlan lte0pdn1 ENABLE 0
	ucfg set config lan vlan lte0pdn2 ENABLE 0
	ucfg set config lan vlan lte0pdn3 ENABLE 0
	ucfg set config lan usbeth mode 2
	ucfg set config serial ttygs0 1
	ucfg set config serial ttygs1 1
	ucfg set config serial ttydm0 1
	ucfg set config fota ENABLE 1
	ucfg set ltenv info operator 0000

	ucfg set config wan lte apntable apn1 ENABLE 1
	ucfg set config wan lte apntable apn1 apn_name {IMS APN Name, ex)ims.lguplus.co.kr}
	ucfg set config wan lte apntable apn3 ENABLE 1

	ucfg set config wan lte ims ENABLE 1
	ucfg set config wan lte ims engine 0
	ucfg set config wan lte ims conf 1 
	ucfg set config wan lte ims manual 0
	ucfg set config wan lte ims audio 3
	ucfg set config wan lte ims ims_test_mode 0
	ucfg set config wan lte ims pcscf_read_mode 1

	optional:
	ucfg set config lan br ip 192.168.30.1
	ucfg set config lan br gateway 192.168.30.1



+ How to setup linux kernel with VLAN

	CONFIG_USB_ACM=y
	CONFIG_USB_USBNET=y
	CONFIG_USB_NET_CDCETHER=y
	CONFIG_USB_NET_CDC_EEM=y
	CONFIG_VLAN_8021Q=y
	CONFIG_NETFILTER_NETLINK=y 
	CONFIG_NETFILTER_NETLINK_QUEUE=y 
	CONFIG_NF_CT_NETLINK=y 
	CONFIG_IP_ADVANCED_ROUTER=y 
	CONFIG_NET_SCH_INGRESS=y 
	CONFIG_NET_SCHED=y 
	CONFIG_IP_MULTIPLE_TABLES=y 
	CONFIG_NETFILTER_XT_TARGET_MARK=y



