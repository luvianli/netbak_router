#include "huayi_handle_reload.h"
#include "huayi_utimer_functions.h"

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
VTBool huayi_power_off_lte4g(void)
{
	int lteMode = 0;
	char pmodel[RESULT_STR_LEN]={0};
	
	huayi_uci_get_int(PKG_STATUS_MODEM,"modem","lteMode",&lteMode);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);	
	
	if (lteMode == 1)
	{
		if(strncmp("EC200T",pmodel, 6) == 0 || strncmp("EC200A",pmodel, 6) == 0)
		{
			system("huayi_AT_cli AT+QPOWD=1");
			sleep(5);
		}
		else if(strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
		{
			system("huayi_AT_cli AT+QPOWD=1");
			sleep(5);
		}
		else if(strncmp("N720V5_V1",pmodel, 9) == 0)
		{
			system("huayi_AT_cli AT+CFUN=1,1");
			sleep(5);
		}
		else
		{
			printf("Unidentified modem module!!!\n");
		}
		
	}
	
	return VT_TRUE;
}
#endif

VTBool huayi_sys_upgrade(void * arg,cJSON *input_root)
{
	int keep_setting = 1;
	char cmd[CMD_STR_LEN] = {0};
	
	if(!huayi_file_exists(UPGRADE_FILE))
	{
		return VT_FALSE;
	}

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_power_off_lte4g();
#endif
	
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","upgrade_keep_setting",&keep_setting);
	
	if(keep_setting == 0)  // 0 - upgrade and reset
	{
		snprintf(cmd,sizeof(cmd),"sysupgrade -n %s",UPGRADE_FILE);
		
		system(cmd);
	}else if(keep_setting == 1)	// 1 - keep all setting
	{
		snprintf(cmd,sizeof(cmd),"sysupgrade %s",UPGRADE_FILE);
		
		system(cmd);
	}else if(keep_setting == 2) // 2 -keep network setting
	{
		snprintf(cmd,sizeof(cmd),"sysupgrade -N %s",UPGRADE_FILE);
		
		system(cmd);
	}
	
	return VT_TRUE;
}


VTBool huayi_sys_reset(void * arg,cJSON *input_root)
{
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_power_off_lte4g();
#endif

#if defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
	do_orange_led_blinks_fast();
#endif

#if defined(CONFIG_HUAYI_POWER_LED_SUPPORT)
	do_power_led_blinks_fast();
#elif defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
	do_wps_led_blinks_slowly();
#endif

	/*maybe need to fixed led here*/
	system("rm -rf /overlay/*");	
	sleep(3);
	system("mkdir -p /overlay/etc/config");
	system("cp /etc/config/wireless /overlay/etc/config/");
	system("cp /etc/config/dhcp /overlay/etc/config/");
	system("/sbin/reboot");
	return VT_TRUE;
}

VTBool huayi_sys_reboot(void * arg,cJSON *input_root)
{
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_power_off_lte4g();
#endif
	system("echo '1' >/etc/opkg/keys/uptime");
	system("/sbin/reboot"); 
	return VT_TRUE;
}

VTBool huayi_reload_sche_reboot(void * arg,cJSON *input_root)
{
	huayi_core_delete_first_task_by_id(HUAYI_SCHE_REBOOT_TASK);
	huayi_core_schedule_reboot(arg);
	return VT_TRUE;
}

VTBool huayi_restore_system_config(void * arg,cJSON *input_root)
{
	char cmd[CMD_STR_LEN] = {0};

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_power_off_lte4g();
#endif
	
	snprintf(cmd,sizeof(cmd),"sysupgrade -r %s",REAL_BACKUP_FILE);

	system(cmd);

	system("reboot -f");
	
	return VT_TRUE;
}

VTBool huayi_reload_system_ntpd(void * arg,cJSON *input_root)
{
	system("/etc/init.d/sysntpd restart");
	
	return VT_TRUE;
}


