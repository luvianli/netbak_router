
#include "huayi_handle_events.h"

void update_ports_led(unsigned char port_id,unsigned char status)
{
	/*maybe hct need to use this function in the future*/	
	return;
}

void update_netstate(unsigned char port_id,unsigned char status,unsigned char old_status)
{
	int sys_mode = 0;
	char wan_type[OPTION_STR_LEN] = {0};
	char proto[SHORT_STR_LEN] = {0};
	char wan_ifname[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};
	
	huayi_uci_get_str(PKG_NETWORK,"lan","proto",proto);
	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	if (strlen(wan_type) != 0 && strcmp(wan_type,"mobile") == 0)
	{
		if(port_id == CONFIG_HUAYI_WAN_PORT_NUM)
	 	{
	 		if(status)
	 		{
	 			system("huayi_cmd set network wan_link 1");
	 		}
			else
	 		{
	 			system("huayi_cmd set network wan_link 0");
	 		}
	 	}
		return;
	}

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);

	if (sys_mode == HUAYI_AP_MODE)
	{
		if(port_id == CONFIG_HUAYI_WAN_PORT_NUM)
		{
			if(status == 0 && old_status == 1)
			{
				if(!strcmp(proto,"dhcp"))
				{
					system("huayi_cmd set network lan_ipv4 0.0.0.0");
					system("huayi_cmd set network lan_gateway 0.0.0.0");
					system("killall -SIGUSR1 udhcpc");
				}
				huayi_core_delete_first_task_by_id(HUAYI_NETWORK_UDHCPD_CHECK_TASK);
				huayi_core_add_task(HUAYI_UDHCPD_DELAY_TIME,HUAYI_NETWORK_UDHCPD_CHECK_TASK,huayi_core_timer_udhcpd,NULL);
			}
		}
	}
	else if (sys_mode == HUAYI_ROUTE_MODE)
	{
		if(port_id == CONFIG_HUAYI_WAN_PORT_NUM)
	 	{
	 		if(status)
	 		{
			#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
				huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
				if(0 != strcmp(proto, "pppoe"))
				{
					huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);
					huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
					huayi_uci_commit(PKG_NETWORK);
					snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend wired_device %s",wan_ifname);
					system(cmd);
				}
				system("huayi_cmd set blend wired_link 1");
			#endif
	 			system("huayi_cmd set network wan_link 1");
				huayi_core_delete_first_task_by_id(HUAYI_IFUP_WAN_TASK);
				huayi_core_add_task(HUAYI_IFUP_WAN_INTERVAL,HUAYI_IFUP_WAN_TASK,huayi_core_timer_ifup_wan,NULL);
	 		}
			else
	 		{
				#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
				system("huayi_cmd set blend wired_link 0");
				#endif
	 			system("huayi_cmd set network wan_link 0");
				//system("ifdown wan");
	 		}
	 	}
	}
	
	return;
}

int discon_24g_count = 0, discon_58g_count = 0;
void update_repeater_info(struct VcoreAgent *Agent,struct huayi_port_event *event)
{
	char cmd[CMD_STR_LEN] = {0};
	char proto[SHORT_STR_LEN] = {0};
	int scan_status = 0;
	char scan_status_str[SMALL_STR_LEN] = {0};
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	int rep_24g_status = 0, rep_58g_status = 0;
#endif
#if defined(CONFIG_HUAYI_SUPPORT_REPEATER_DHCPD)
	int fix_enable = 0;
#endif
	huayi_uci_get_str(PKG_NETWORK,"lan","proto",proto);
	switch(event->port_id) 
	{
		case WLAN_24G_REPEATER_PORT_NUM:
			if (Agent->repeater_status_24g == 1 && event->status == 0)
			{
				if(strcmp(proto,"dhcp") == 0)
				{
					system("huayi_cmd set network lan_ipv4 0.0.0.0");
					system("huayi_cmd set network lan_gateway 0.0.0.0");
					system("killall -SIGUSR1 udhcpc");
				}
				huayi_core_delete_first_task_by_id(HUAYI_NETWORK_UDHCPD_CHECK_TASK);
				huayi_core_add_task(HUAYI_UDHCPD_DELAY_TIME,HUAYI_NETWORK_UDHCPD_CHECK_TASK,huayi_core_timer_udhcpd,NULL);
			}
#if defined(CONFIG_HUAYI_SUPPORT_REPEATER_DHCPD)
			huayi_uci_get_int(PKG_HUAYI_UDHCPD,"dhcp","fix_enable",&fix_enable);
			if(fix_enable == 0)
#endif
			{
				if (event->status == 1 && strcmp(proto,"static") == 0)
				{
					system("/etc/init.d/huayi_udhcpd stop");
				}
			}
			#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
			CON_LOG("### %s, %d\n", __FUNCTION__, __LINE__);
				if(Agent->repeater_status_24g == 0 && event->status == 1)  //disconnect ---> connect
				{
					if (access(WISP4G_TMPFILE, F_OK) != 0)
					{
						snprintf(cmd,CMD_STR_LEN,"echo 'wisp' > %s", WISP4G_TMPFILE);	
						system(cmd);
						system("ifup wan");
					}
				}
			#endif
			Agent->repeater_status_24g = event->status;
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set system rep_24g_status %d",event->status);	
			system(cmd);
			if(Agent->repeater_status_24g == 1 && event->status == 0)   //connect---> disconnect
			{
				get_cmd_result("huayi_cmd get system scan_state",scan_status_str,SMALL_STR_LEN);
				scan_status = atoi(scan_status_str);
				if(scan_status != 3)		/*后台扫描处理断线重连时，这里不再计数*/
					discon_24g_count += 1;
			}
			break;
		case WLAN_58G_REPEATER_PORT_NUM:
			if (Agent->repeater_status_58g == 1 && event->status == 0)
			{
				if(strcmp(proto,"dhcp") == 0)
				{
					system("huayi_cmd set network lan_ipv4 0.0.0.0");
					system("huayi_cmd set network lan_gateway 0.0.0.0");
					system("killall -SIGUSR1 udhcpc");
				}
				huayi_core_delete_first_task_by_id(HUAYI_NETWORK_UDHCPD_CHECK_TASK);
				huayi_core_add_task(HUAYI_UDHCPD_DELAY_TIME,HUAYI_NETWORK_UDHCPD_CHECK_TASK,huayi_core_timer_udhcpd,NULL);
			}
#if defined(CONFIG_HUAYI_SUPPORT_REPEATER_DHCPD)
			huayi_uci_get_int(PKG_HUAYI_UDHCPD,"dhcp","fix_enable",&fix_enable);
			if(fix_enable == 0)	
#endif
			{
				if (event->status == 1 && strcmp(proto,"static") == 0)
				{
					system("/etc/init.d/huayi_udhcpd stop");
				}
			}
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
			CON_LOG("### %s, %d\n", __FUNCTION__, __LINE__);
			if(Agent->repeater_status_58g == 0 && event->status == 1)  //disconnect ---> connect
			{
				if (access(WISP4G_TMPFILE, F_OK) != 0)
				{
					snprintf(cmd,CMD_STR_LEN,"echo 'wisp' > %s", WISP4G_TMPFILE);	
					system(cmd);
					system("ifup wan");
				}
			}
#endif

			Agent->repeater_status_58g = event->status;
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set system rep_58g_status %d",event->status);	
			system(cmd);
			if(Agent->repeater_status_58g == 1 && event->status == 0)   //connect---> disconnect
			{
				get_cmd_result("huayi_cmd get system scan_state",scan_status_str,SMALL_STR_LEN);
				scan_status = atoi(scan_status_str);
				if(scan_status != 3)    /*后台扫描处理断线重连时，这里不再计数*/
					discon_58g_count += 1;
			}
			break;
		case WLAN_24G_WPS_PORT_NUM:
			Agent->wps_connect_status_24g = WPS_SUCCESS;
			Agent->encryption = event->encryption;
			memcpy(Agent->wpa_key,event->key,WLAN_WPA_KEY_BUFF_LEN);
			break;
		case WLAN_58G_WPS_PORT_NUM:
			Agent->wps_connect_status_58g = WPS_SUCCESS;
			Agent->encryption = event->encryption;
			memcpy(Agent->wpa_key,event->key,WLAN_WPA_KEY_BUFF_LEN);
			break;
		default:
			break;
	}
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	//huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_24g_status",&rep_24g_status);	
	//huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_58g_status",&rep_58g_status);
	if((Agent->repeater_status_24g ==1 && Agent->repeater_status_58g == 0) || (Agent->repeater_status_24g == 0 && Agent->repeater_status_58g == 1))
	{
		CON_LOG("### %s, %d, rep_58g_status = %d, rep_24g_status=%d\n", __FUNCTION__, __LINE__,Agent->repeater_status_58g, Agent->repeater_status_24g);
		system("huayi_cmd set blend wisp_link 1");
		system("huayi_cmd set network wan_link 1"); 	
	}

	else
	{
		
		CON_LOG("### %s, %d, rep_58g_status = %d, rep_24g_status=%d\n", __FUNCTION__, __LINE__,Agent->repeater_status_58g, Agent->repeater_status_24g);
		system("huayi_cmd set blend wisp_link 0");
		system("huayi_cmd set network wan_link 0");		
	}
#endif

}

void huayi_port_event_process(struct VcoreAgent *Agent,struct huayi_port_event *event)
{
	char cmd_str[CMD_STR_LEN]	= {0};
	int linkport = 0;
	unsigned char old_wan_link = 0;
	
	if (event->port_id >= WLAN_24G_REPEATER_PORT_NUM)
	{
		update_repeater_info(Agent,event);
		return;
	}

	if (Agent->link_port&(1<<CONFIG_HUAYI_WAN_PORT_NUM))
		old_wan_link = 1;
	
	linkport = Agent->link_port;

	if(0 == event->status)
	{
		linkport = (~(1 << event->port_id)) & linkport;
	}
	else if(1 == event->status)
	{
		linkport = (1 << event->port_id) | linkport;
	}

	Agent->link_port = linkport;

	snprintf(cmd_str,sizeof(cmd_str),"huayi_cmd set network linkport %d",linkport);
	system(cmd_str);

	update_netstate(event->port_id,event->status,old_wan_link);
	update_ports_led(event->port_id,event->status);
	return;
}

#if defined(CONFIG_HUAYI_SUPPORT_GPIO_EVENT)
void do_button_reset(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	Agent->reset_button_state = BUTTON_INIT_STATE;
	
	huayi_sys_reset(Agent,NULL);
	
	return;
}

#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
void do_wps_led_blinks_fast(void)
{
	char cmd[CMD_STR_LEN] = {0};
	
	if(CONFIG_HUAYI_WPS_LED_GPIO_NUM == 5) {
		/*for 7628DA GPIO5*/
		strcpy(cmd, "devmem 0x10000060 32 0x4110441");	
		system(cmd);
	}
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 2 2 4000 0 4000", CONFIG_HUAYI_WPS_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void do_wps_led_blinks_slowly(void)
{
	char cmd[CMD_STR_LEN] = {0};
	
	if(CONFIG_HUAYI_WPS_LED_GPIO_NUM == 5) {
		/*for 7628DA GPIO5*/
		strcpy(cmd, "devmem 0x10000060 32 0x4110441");	
		system(cmd);
	}
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 8 8 4000 0 4000", CONFIG_HUAYI_WPS_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void do_wps_led_on(void)
{
	char cmd[CMD_STR_LEN] = {0};
	
	if(CONFIG_HUAYI_WPS_LED_GPIO_NUM == 5) {
		/*for 7628DA GPIO5*/
		strcpy(cmd, "devmem 0x10000060 32 0x4110441");	
		system(cmd);
	}
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_WPS_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void butto_do_wps_led_off(void)
{
	char cmd[CMD_STR_LEN] = {0};
	
	if(CONFIG_HUAYI_WPS_LED_GPIO_NUM == 5) {
		/*for 7628DA GPIO5*/
		strcpy(cmd, "devmem 0x10000060 32 0x4110441");	
		system(cmd);
	}
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_WPS_LED_GPIO_NUM);

	system(cmd);
	
	return;
}
#endif

#if defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
void do_white_led_off(void)
{
	char cmd[CMD_STR_LEN] = {0};
	
#ifdef CONFIG_HUAYI_WHITE_LED_GPIO_LOW_LEVEL
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_WHITE_LED_GPIO_NUM);
#else
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_WHITE_LED_GPIO_NUM);
#endif

	system(cmd);
	
	return;
}

void do_orange_led_off(void)
{
	char cmd[CMD_STR_LEN] = {0};

#ifdef CONFIG_HUAYI_ORANGE_LED_GPIO_LOW_LEVEL
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_ORANGE_LED_GPIO_NUM);
#else
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_ORANGE_LED_GPIO_NUM);
#endif	
	
	system(cmd);
	
	return;
}

void do_white_led_blinks_fast(void)
{
	char cmd[CMD_STR_LEN] = {0};

	do_orange_led_off();
	do_white_led_off();

	snprintf(cmd, CMD_STR_LEN, "gpio l %d 2 2 4000 0 4000", CONFIG_HUAYI_WHITE_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void do_white_led_blinks_slowly(void)
{
	char cmd[CMD_STR_LEN] = {0};

	do_orange_led_off();
	do_white_led_off();

	snprintf(cmd, CMD_STR_LEN, "gpio l %d 8 8 4000 0 4000", CONFIG_HUAYI_WHITE_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void do_white_led_on(void)
{
	char cmd[CMD_STR_LEN] = {0};

	do_orange_led_off();
	do_white_led_off();

#ifdef CONFIG_HUAYI_WHITE_LED_GPIO_LOW_LEVEL	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_WHITE_LED_GPIO_NUM);
#else	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_WHITE_LED_GPIO_NUM);
#endif

	system(cmd);
	
	return;
}

void do_orange_led_blinks_fast(void)
{
	char cmd[CMD_STR_LEN] = {0};

	do_orange_led_off();
	do_white_led_off();

	snprintf(cmd, CMD_STR_LEN, "gpio l %d 2 2 4000 0 4000", CONFIG_HUAYI_ORANGE_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void do_orange_led_blinks_slowly(void)
{
	char cmd[CMD_STR_LEN] = {0};

	do_orange_led_off();
	do_white_led_off();

	snprintf(cmd, CMD_STR_LEN, "gpio l %d 8 8 4000 0 4000", CONFIG_HUAYI_ORANGE_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void two_color_blink_slowly()
{
	char cmd[CMD_STR_LEN] = {0};
	int andOr=0;
#if (defined(CONFIG_HUAYI_ORANGE_LED_GPIO_LOW_LEVEL) && defined(CONFIG_HUAYI_WHITE_LED_GPIO_LOW_LEVEL)) || \
	(!defined(CONFIG_HUAYI_ORANGE_LED_GPIO_LOW_LEVEL) && !defined(CONFIG_HUAYI_WHITE_LED_GPIO_LOW_LEVEL))
	andOr = 1; // two LED same logic
#endif	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d%c%d 8 8 4000 0 4000", CONFIG_HUAYI_ORANGE_LED_GPIO_NUM,andOr==1?'+':'-',CONFIG_HUAYI_WHITE_LED_GPIO_NUM);
	system(cmd);
	
}

void do_orange_led_on(void)
{
	char cmd[CMD_STR_LEN] = {0};

	do_orange_led_off();
	do_white_led_off();

#ifdef CONFIG_HUAYI_ORANGE_LED_GPIO_LOW_LEVEL	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_ORANGE_LED_GPIO_NUM);
#else	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_ORANGE_LED_GPIO_NUM);
#endif

	system(cmd);
	
	return;
}

#endif

void do_wps_end(void* arg)
{
	int wlan_2g_disabled = 0;
	int wlan_5g_disabled = 0;
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	huayi_uci_get_int(PKG_WIRELESS,"radio0_cli0","disabled",&wlan_2g_disabled);
	huayi_uci_get_int(PKG_WIRELESS,"radio1_cli0","disabled",&wlan_5g_disabled);

	Agent->wpstimeout = VT_TRUE;

#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
	if((HUAYI_AP_MODE == Agent->opmode)||(HUAYI_ROUTE_MODE == Agent->opmode))
	{
		if (CONFIG_HUAYI_WPS_LED_DEFAULT_STATUS == 1)
		{
			do_wps_led_on();
		}
		else
		{
			butto_do_wps_led_off();
		}
	}
#elif defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
	Agent->special_led_status = LED_RESET;	
	Agent->wps_downlink_status = WPS_INIT;
#endif


	Agent->wps_freq = AP_WPS_INIT;
	Agent->old_reset_button_state = BUTTON_INIT_STATE;
	Agent->wps_downlink_status = WPS_INIT;

	if (wlan_2g_disabled == 1)
	{
		system("ifconfig apcli0 down");
	}
	
	if (wlan_5g_disabled == 1)
	{
		system("ifconfig apclii0 down");
	}
	
	return;
}

void do_ap_wps_5g(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	system("iwpriv rai0 set WscConfMode=7");
	system("iwpriv rai0 set WscConfStatus=2");
	system("iwpriv rai0 set WscMode=2");
	system("iwpriv rai0 set WscGetConf=1");

	Agent->wps_freq = AP_WPS_5G_DONE;
	Agent->reset_button_state = BUTTON_WPS_5G_DONE;
	Agent->wps_downlink_status = WPS_DOING;

#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
	do_wps_led_blinks_fast();	
#elif defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
	do_white_led_blinks_fast();
#endif

	huayi_core_delete_first_task_by_id(HUAYI_WPS_CONNECT_TASK);
	Agent->wps_timer_count = 0;
	strcpy(Agent->wps_interface, "rai0");
	huayi_wps_connect(arg);
	
	huayi_core_add_task(HUAYI_WPS_END_TIME,HUAYI_WPS_END_TASK,do_wps_end, Agent);

	return;
}

void do_ap_wps_2g(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	system("iwpriv ra0 set WscConfMode=7");
	system("iwpriv ra0 set WscConfStatus=2");
	system("iwpriv ra0 set WscMode=2");
	system("iwpriv ra0 set WscGetConf=1");

	Agent->wps_freq = AP_WPS_2G_DONE;
	Agent->reset_button_state = BUTTON_WPS_2G_DONE;
	Agent->wps_downlink_status = WPS_DOING;

#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
	do_wps_led_blinks_fast();
#elif defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
	do_white_led_blinks_fast();
#endif

	huayi_core_delete_first_task_by_id(HUAYI_WPS_CONNECT_TASK);
	Agent->wps_timer_count = 0;
	strcpy(Agent->wps_interface, "ra0");
	huayi_wps_connect(arg);


	huayi_core_add_task(HUAYI_WPS_END_TIME,HUAYI_WPS_END_TASK,do_wps_end, Agent);

	return;
}

void check_wps_complete(void* arg)
{
	int channel = 0;
	char wifi_device[SHORT_STR_LEN] = {0}, wifi_iface[SHORT_STR_LEN] = {0};
	char other_wifi_iface[SHORT_STR_LEN] = {0}, wifi_vap[SHORT_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0}, cmdresult[TEMP_STR_LEN] = {0};
	char ssid[OPTION_STR_LEN] = {0};
	char authmode[RESULT_STR_LEN] = {0}, encryptype[RESULT_STR_LEN] = {0};
	char wan_lan_ifname[OPTION_STR_LEN] = {0};
	char ssid_def[OPTION_STR_LEN] = {0};
	char bssid[RESULT_STR_LEN] = {0};
	char *p = NULL;
	struct VcoreAgent *Agent = NULL;
	
	Agent = (struct VcoreAgent*)arg;
	
	if(WPS_SUCCESS == Agent->wps_connect_status_24g)
	{
		strcpy(wifi_device,W24G_DEV);
		strcpy(wifi_iface,W24G_REP_IFACE);
		strcpy(wifi_vap,W24G_REP_IFNAME);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)		
		strcpy(other_wifi_iface,W58G_REP_IFACE);
#endif
	}
	else if(WPS_SUCCESS == Agent->wps_connect_status_58g)
	{
		strcpy(wifi_device,W58G_DEV);
		strcpy(wifi_iface,W58G_REP_IFACE);
		strcpy(wifi_vap,W58G_REP_IFNAME);
#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
		strcpy(other_wifi_iface,W24G_REP_IFACE);
#endif
	}
	
	if((WPS_SUCCESS == Agent->wps_connect_status_24g)||
		(WPS_SUCCESS == Agent->wps_connect_status_58g))
	{
		memset(cmd, 0, sizeof(cmd));
		memset(cmdresult, 0, sizeof(cmdresult));
		snprintf(cmd, sizeof(cmd), "iwconfig  %s | grep ESSID", wifi_vap);
		get_cmd_result(cmd, cmdresult, sizeof(cmdresult));
		
		if ((p = strstr(cmdresult,"\"")) != NULL)
		{
			p = p + strlen("\"");
			if (strlen(cmdresult) > 3)
			{
				cmdresult[strlen(cmdresult)-3] = '\0';
				strncpy(ssid, p, sizeof(ssid));
			}
		}
		
		if (strlen(ssid) == 0)
		{
			huayi_core_add_task(HUAYI_WPS_CHECK_TIME,HUAYI_WIFI_WPS_CHECK_TASK,check_wps_complete, Agent);
			return;
		}
		
		memset(cmd, 0, sizeof(cmd));
		memset(cmdresult, 0, sizeof(cmdresult));
		snprintf(cmd, sizeof(cmd), "iwconfig  %s | grep Channel", wifi_vap);
		get_cmd_result(cmd, cmdresult, sizeof(cmdresult));
		p = strstr(cmdresult,"Channel=");
		if(NULL != p)
		{
			p = p + strlen("Channel=");
			channel = atoi(p);
		}

		memset(cmd, 0, sizeof(cmd));
		memset(cmdresult, 0, sizeof(cmdresult));
		snprintf(cmd, sizeof(cmd), "iwconfig  %s | grep \"Access Point: \"", wifi_vap);
		get_cmd_result(cmd, cmdresult, sizeof(cmdresult));
		p = strstr(cmdresult,"Access Point: ");
		if(NULL != p)
		{
			p = p + strlen("Access Point: ");
			strncpy(bssid,p,17);
		}

		str2lower(bssid);
		
		huayi_uci_set_int(PKG_WIRELESS,wifi_device,"channel",channel);
		huayi_uci_set_int(PKG_WIRELESS,wifi_iface,"disabled",0);
		huayi_uci_set_int(PKG_WIRELESS,wifi_iface,"apcli_enable",1);
		huayi_uci_set_str(PKG_WIRELESS,wifi_iface,"apcli_ssid",ssid);
		huayi_uci_set_str(PKG_WIRELESS,wifi_iface,"apcli_bssid",bssid);
		
		if(Agent->encryption > 0)
		{
			if (Agent->encryption&WLAN_AUTHTYPE_WPAPSK)
			{
				strcpy(authmode,"WPAPSK");
			}
			else if(Agent->encryption&WLAN_AUTHTYPE_WPA2PSK)
			{
				strcpy(authmode,"WPA2PSK");
			}
			else if(Agent->encryption&WLAN_AUTHTYPE_WPAPSKWPA2PSK)
			{
				strcpy(authmode,"WPAPSKWPA2PSK");
			}

			if (Agent->encryption&WLAN_ENCRTYPE_TKIP)
			{
				strcpy(encryptype,"TKIP");
			}
			else if(Agent->encryption&WLAN_ENCRTYPE_AES)
			{
				strcpy(encryptype,"AES");
			}

			huayi_uci_set_str(PKG_WIRELESS,wifi_iface,"apcli_wpapsk",Agent->wpa_key);
			//huayi_uci_delete_option(PKG_WIRELESS,wifi_iface,"apcli_bssid");
			huayi_uci_set_str(PKG_WIRELESS,wifi_iface,"apcli_authmode",authmode);
			huayi_uci_set_str(PKG_WIRELESS,wifi_iface,"apcli_encryptype",encryptype);		
		}
		else
		{
			huayi_uci_delete_option(PKG_WIRELESS,wifi_iface,"key");
			//huayi_uci_delete_option(PKG_WIRELESS,wifi_iface,"apcli_bssid");
			huayi_uci_set_str(PKG_WIRELESS,wifi_iface,"apcli_authmode","OPEN");
			huayi_uci_set_str(PKG_WIRELESS,wifi_iface,"apcli_encryptype","NONE");
		}
		
		if(WPS_SUCCESS == Agent->wps_connect_status_58g && strlen(other_wifi_iface) > 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,other_wifi_iface,"apcli_ssid",DEF_REP_W24G_SSID);
		}
		
		else if(WPS_SUCCESS == Agent->wps_connect_status_24g && strlen(other_wifi_iface) > 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,other_wifi_iface,"apcli_ssid",DEF_REP_W58G_SSID);
		}

		if (strlen(other_wifi_iface) > 0)
		{
			huayi_uci_set_int(PKG_WIRELESS,other_wifi_iface,"disabled",1);
			huayi_uci_set_str(PKG_WIRELESS,other_wifi_iface,"apcli_wpapsk",RANDOM_KEY);
			huayi_uci_set_str(PKG_WIRELESS,other_wifi_iface,"apcli_authmode","WPA2PSK");
			huayi_uci_set_str(PKG_WIRELESS,other_wifi_iface,"apcli_encryptype","AES");
		}
	#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
		huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"def_ssid",ssid_def);
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_def);
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","none");
	#endif
	#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"def_ssid",ssid_def);
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_def);
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","none");
	#endif
		huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_lan_ifname",wan_lan_ifname);
		strcat(wan_lan_ifname, " ");
		strcat(wan_lan_ifname, wifi_vap);
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",wan_lan_ifname);

		huayi_uci_commit(PKG_NETWORK);		
		huayi_uci_commit(PKG_WIRELESS);
		
		system("huayi-reset network");
		
		Agent->wps_connect_status_24g = WPS_INIT;
		Agent->wps_connect_status_58g = WPS_INIT;
	}
	else
	{
		if(VT_FALSE == Agent->wpstimeout)
		{			
			huayi_core_add_task(HUAYI_WPS_CHECK_TIME,HUAYI_WIFI_WPS_CHECK_TASK,check_wps_complete, Agent);
		}
		else
		{
			Agent->wpstimeout = VT_FALSE;
			Agent->wps_connect_status_24g = WPS_FALSE;
			Agent->wps_connect_status_58g = WPS_FALSE;
						
			//WPS failed, keep disconnected
		#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
			system("iwpriv apcli0 set ApCliEnable=1");
		#endif
		#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			system("iwpriv apclii0 set ApCliEnable=1");
		#endif
		#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
			do_wps_led_blinks_slowly();
		#elif defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
			Agent->special_led_status = LED_RESET;
		#endif
		}
	}
	
	return;
}

void wps_to_up(void* arg)
{
	int wlan_2g_disabled = 0;
	int wlan_5g_disabled = 0;
	int wifi_2g_disabled = 0;
	int wifi_5g_disabled = 0;
	
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	
#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
	do_wps_led_blinks_fast();
#elif defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
	do_white_led_blinks_fast();
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	huayi_uci_get_int(PKG_WIRELESS,"radio0_cli0","disabled",&wlan_2g_disabled);
	if (wlan_2g_disabled == 1)
	{
		system("ifconfig apcli0 up");
	}

	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&wifi_2g_disabled);
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_int(PKG_WIRELESS,"radio1_cli0","disabled",&wlan_5g_disabled);
	if (wlan_5g_disabled == 1)
	{
		system("ifconfig apclii0 up");
	}

	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&wifi_5g_disabled);
#endif
	
	if (wlan_2g_disabled == 1 || wlan_5g_disabled == 1)
		sleep(2);

	if(wifi_2g_disabled == 0)
	{
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
		system("iwpriv apcli0 set WscStop");
		system("iwpriv apcli0 set ApCliBssid=");
		system("iwpriv apcli0 set ApCliEnable=0");
		system("iwpriv apcli0 set WscConfMode=1");
		system("iwpriv apcli0 set WscMode=2");
		system("iwpriv apcli0 set WscGetConf=1");
#endif
	}else if(wifi_2g_disabled == 1 && wifi_5g_disabled == 0)
	{
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		system("iwpriv apclii0 set WscStop");
		system("iwpriv apclii0 set ApCliBssid=");
		system("iwpriv apclii0 set ApCliEnable=0");
		system("iwpriv apclii0 set WscConfMode=1");
		system("iwpriv apclii0 set WscMode=2");
		system("iwpriv apclii0 set WscGetConf=1");
#endif
	}

	Agent->wps_freq = AP_WPS_TO_UP_DONE;
	Agent->reset_button_state = BUTTON_WPS_TO_UP_DONE;
	Agent->wpstimeout = VT_FALSE;
	
	Agent->wps_connect_status_24g = WPS_DOING;
	Agent->wps_connect_status_58g = WPS_DOING;
	
	Agent->sta_connect_status_old = 0;
	
	huayi_core_add_task(HUAYI_WPS_CHECK_TIME,HUAYI_WIFI_WPS_CHECK_TASK,check_wps_complete, Agent);
	//check_wps_complete(Agent);
	huayi_core_add_task(HUAYI_WPS_END_TIME,HUAYI_WPS_END_TASK,do_wps_end, Agent);

	return;
}

void wps_freq_change(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	Agent->wps_freq = AP_WPS_5G;
	
	huayi_core_add_task((HUAYI_AP_WPS_5G_DELAY_TIME - HUAYI_AP_WPS_2G_DELAY_TIME),HUAYI_DO_AP_WPS_TASK,do_ap_wps_5g, Agent);
	
	return;
}

void button_wps_timeout(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	Agent->reset_button_state = BUTTON_READY_TO_RESET;

	huayi_core_add_task((HUAYI_BUTTON_DELAY_RESET_TIME - HUAYI_BUTTON_DELAY_WPS_TIMEOUT),HUAYI_BUTTON_MULT_TO_RESET_TASK,do_button_reset, Agent);
	
	return;
}

void run_gpio_function(struct VcoreAgent *Agent, unsigned char gpio_id,unsigned char status)
{
	char echo_str[128] = {0};
	int sys_mode = 0, uptime = 0;
	int reset_buttonid = -1, wps_buttonid = -1;
	int wlan_2g_disabled = 1, wlan_5g_disabled = 1;
	#if defined(CONFIG_HUAYI_BUTTON_WIFI_SUPPORT)
	int wifi_button = 0;
	int wifi_bottonid = -1;
	
	wifi_bottonid = CONFIG_HUAYI_BUTTON_WIFI_GPIO_NUM;
	#endif
	#if defined(CONFIG_HUAYI_BUTTON_RESET_SUPPORT)
	reset_buttonid = CONFIG_HUAYI_BUTTON_RESET_GPIO_NUM;
	#endif
	#if defined(CONFIG_HUAYI_BUTTON_WPS_SUPPORT)
	wps_buttonid = CONFIG_HUAYI_BUTTON_WPS_GPIO_NUM;
	#endif
	

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);

	#if defined(CONFIG_HUAYI_BUTTON_RESET_SUPPORT)
	if(reset_buttonid == gpio_id)
 	{
		if(reset_buttonid == wps_buttonid)// Reset and WPS key multiplexing
		{
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);
			if(BUTTON_PRESSED == status)
			{
				if (uptime - Agent->wps_button_time <= 1)
				{
					return;
				}
					
				Agent->reset_button_state = BUTTON_NEED_WPS;

				

				huayi_core_add_task(HUAYI_BUTTON_DELAY_WPS_TIMEOUT,HUAYI_BUTTON_MULT_WPS_TIMEOUT_TASK,button_wps_timeout, Agent);
			}
			else if(BUTTON_RELEASED == status)
			{
				Agent->wps_button_time = uptime;
				/*released*/
				switch(Agent->reset_button_state)
				{
					case BUTTON_NEED_WPS:
						huayi_core_delete_first_task_by_id(HUAYI_BUTTON_MULT_WPS_TIMEOUT_TASK);
						if (Agent->old_reset_button_state == BUTTON_NEED_WPS)
						{
							break;
						}
						
						Agent->old_reset_button_state = BUTTON_NEED_WPS;

						if(sys_mode != HUAYI_REP_MODE)
						{
							huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&wlan_2g_disabled);
							huayi_uci_get_int(PKG_WIRELESS,"radio1","disabled",&wlan_5g_disabled);

							if(0 == wlan_2g_disabled)
							{
							#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
								do_ap_wps_2g(Agent);
							#endif
							}					
							else if(0 == wlan_5g_disabled)
							{
							#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
								do_ap_wps_5g(Agent);
							#endif
							}
						}
						else
						{
							wps_to_up(Agent);
						}

						break;
						
					case BUTTON_READY_TO_RESET:
						Agent->reset_button_state = BUTTON_INIT_STATE;
						huayi_core_delete_first_task_by_id(HUAYI_BUTTON_MULT_TO_RESET_TASK);
						break;
					
					default:
						break;
				}
				/*for product test*/
				system("touch /tmp/reset_test");
			}
		}
		else//only reset
		{
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);
			if(BUTTON_PRESSED == status)
			{
				/*pressed*/
				if (uptime - Agent->wps_button_time <= 1)
				{
					return;
				}
				Agent->reset_button_state = BUTTON_READY_TO_RESET;
				huayi_core_add_task(HUAYI_BUTTON_DELAY_RESET_TIME,HUAYI_BUTTON_ONLY_RESET_TASK,do_button_reset, Agent);
			}
			else
			{
				Agent->wps_button_time = uptime;
				/*released*/
				switch(Agent->reset_button_state)
				{
					case BUTTON_READY_TO_RESET:
						Agent->reset_button_state = BUTTON_INIT_STATE;
						huayi_core_delete_first_task_by_id(HUAYI_BUTTON_ONLY_RESET_TASK);
						break;
						
					case BUTTON_NEED_RESET:
						Agent->reset_button_state = BUTTON_INIT_STATE;
					case BUTTON_INIT_STATE:	
					default:
						break;
				}
				/*for product test*/
				system("touch /tmp/reset_test");
			}
		}
 	}
	#endif
	#if defined(CONFIG_HUAYI_BUTTON_WPS_SUPPORT)
	else if(wps_buttonid == gpio_id)
	{
		if(sys_mode != HUAYI_REP_MODE)
		{
			if(BUTTON_PRESSED == status)
			{
				/*pressed*/
				if(Agent->wps_freq != AP_WPS_INIT)
				{
					return;
				}
				
				Agent->wps_freq = AP_WPS_2G;
				
				huayi_core_add_task(HUAYI_AP_WPS_5G_DELAY_TIME,HUAYI_AP_WPS_FREQ_CHANGE_TASK,do_ap_wps_5g, Agent);
			}else
			{
				/*released*/
				switch(Agent->wps_freq)
				{	
					case AP_WPS_2G:
						huayi_core_delete_first_task_by_id(HUAYI_AP_WPS_FREQ_CHANGE_TASK);
						do_ap_wps_2g(Agent);
						break;
					case AP_WPS_5G:
						//huayi_core_delete_first_task_by_id(HUAYI_DO_AP_WPS_TASK);
						//do_ap_wps_2g(Agent);
						break;
					case AP_WPS_INIT:	
					default:
						break;
				}
				/*for product test*/
				system("touch /tmp/wps_test");
			}
		}
		else
		{
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);
			if (BUTTON_PRESSED == status)
			{
				Agent->wps_button_time = uptime;
			}
			else
			{
				Agent->wps_button_time = uptime - Agent->wps_button_time;
				//if(Agent->wps_button_time > 1)
				{
					Agent->wps_button_time = 0;
					wps_to_up(Agent);
				}
				/*for product test*/
				system("touch /tmp/wps_test");
			}
		}
	}
	#endif
	#if defined(CONFIG_HUAYI_BUTTON_WIFI_SUPPORT)
	else if(wifi_bottonid == gpio_id)
	{
		if(BUTTON_PRESSED == status)
		{
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","wifi_button",&wifi_button);
			
			if(wifi_button == 0)
			{
				system(" huayi_cmd set system wifi_button 1");
			}
			else if(wifi_button == 1)
			{
				system(" huayi_cmd set system wifi_button 0");
			}
			
			system("huayi-reset wireless");
		}
	}
	#endif
	
	return;
}

void huayi_gpio_event_process(struct VcoreAgent *Agent,struct huayi_gpio_event *event)
{
	run_gpio_function(Agent, event->gpio_id,event->status);
	return;
}
#endif

#if defined(CONFIG_HUAYI_POWER_LED_SUPPORT)
void do_power_led_off(void)
{
	char cmd[CMD_STR_LEN] = {0};
	char product_model[OPTION_STR_LEN] = {0};

	/*add by rex, 针对这个型号的产品特殊处理，后续考虑删除*/
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	if(!strcmp(product_model, "4G1200-H002-HY06"))
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", 37); //blue led  off
		system(cmd);
		//red led on
		//snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_INTERNET_LED_GPIO_NUM);
		//system(cmd);
		return;
	}
	/*end by rex*/
	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_POWER_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void do_power_led_blinks_fast(void)
{
	char cmd[CMD_STR_LEN] = {0};

	snprintf(cmd, CMD_STR_LEN, "gpio l %d 2 2 4000 0 4000", CONFIG_HUAYI_POWER_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

#endif

#if defined(CONFIG_HUAYI_REPEATER_LED_SUPPORT)
void do_repeater_led_all_blinks_slowly(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 8 8 4000 0 4000", CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 8 8 4000 0 4000", CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 8 8 4000 0 4000", CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}

void do_repeater_led_no_signal_or_off(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}

void do_repeater_led_weak_signal(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}

void do_repeater_led_medium_signal(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}

void do_repeater_led_high_signal_or_on(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_SIGNAL_1_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_SIGNAL_2_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_SIGNAL_3_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}
#endif

#if defined(CONFIG_HUAYI_INTERNET_LED_SUPPORT)
void do_internet_led_on(void)
{
	char cmd[CMD_STR_LEN] = {0};
	char product_model[OPTION_STR_LEN] = {0};

	/*add by rex, 针对这个型号的产品特殊处理，后续考虑删除*/
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	if(!strcmp(product_model, "4G1200-H002-HY06"))
	{
		//red led off
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_INTERNET_LED_GPIO_NUM);
		system(cmd);
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", 37); //blue led  on
		system(cmd);
		return;
	}
	/*end by rex*/
	if(CONFIG_HUAYI_INTERNET_LED_GPIO_NUM == 3) {
		/*for 7628DA GPIO3*/
		strcpy(cmd, "devmem 0x10000060 32 0x4110441");	
		system(cmd);
	}
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_INTERNET_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void do_internet_led_off(void)
{
	char cmd[CMD_STR_LEN] = {0};
	char product_model[OPTION_STR_LEN] = {0};

	/*add by rex, 针对这个型号的产品特殊处理，后续考虑删除*/
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	if(!strcmp(product_model, "4G1200-H002-HY06"))
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", 37); //blue led  off
		system(cmd);
		//red led on
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_INTERNET_LED_GPIO_NUM);
		system(cmd);
		return;
	}
	/*end by rex*/
	
	if(CONFIG_HUAYI_INTERNET_LED_GPIO_NUM == 3) {
		/*for 7628DA GPIO3*/
		strcpy(cmd, "devmem 0x10000060 32 0x4110441");	
		system(cmd);
	}
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_INTERNET_LED_GPIO_NUM);

	system(cmd);
	
	return;
}

void do_internet_led_slowly(void)
{
	char cmd[CMD_STR_LEN] = {0};
	char product_model[OPTION_STR_LEN] = {0};

	/*add by rex, 针对这个型号的产品特殊处理，后续考虑删除*/
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	if(!strcmp(product_model, "4G1200-H002-HY06"))
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", 37); //blue led  off
		system(cmd);
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 8 8 4000 0 4000", 1); //red led
		system(cmd);
		return;
	}
	/*end by rex*/
	
	if(CONFIG_HUAYI_INTERNET_LED_GPIO_NUM == 3) {
		/*for 7628DA GPIO3*/
		strcpy(cmd, "devmem 0x10000060 32 0x4110441");	
		system(cmd);
	}

	snprintf(cmd, CMD_STR_LEN, "gpio l %d 8 8 4000 0 4000", CONFIG_HUAYI_INTERNET_LED_GPIO_NUM);

	system(cmd);
	
	return;
}
#endif

#if defined(CONFIG_HUAYI_4GONLINE_LED_SUPPORT)
void do_4G_online_led_off(void)
{
	char cmd[CMD_STR_LEN] = {0};
	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_4GONLINE_LED_GPIO_NUM);

	system(cmd);

	return;
}

void do_4G_online_led_on(void)
{
	char cmd[CMD_STR_LEN] = {0};
	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_4GONLINE_LED_GPIO_NUM);

	system(cmd);

	return;
}
#endif

#if defined(CONFIG_HUAYI_SIM_LED_SUPPORT)
void do_sim_led_off(void)
{
	char cmd[CMD_STR_LEN] = {0};
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_SIM_LED_GPIO_NUM);
	system(cmd);
	return;
}

void do_sim_led_on(void)
{
	char cmd[CMD_STR_LEN] = {0};
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_SIM_LED_GPIO_NUM);
	system(cmd);
	return;
}

#endif

#if defined(CONFIG_HUAYI_SIGNAL_LED_SUPPORT)
void do_signal_led_high(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_HIGH_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_HIGH_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_MIDDLE_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_MIDDLE_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_LOW_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_LOW_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}

void do_signal_led_low(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_HIGH_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_HIGH_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_MIDDLE_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_MIDDLE_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_LOW_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_LOW_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}

void do_signal_led_middle(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_HIGH_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_HIGH_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_MIDDLE_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_MIDDLE_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_LOW_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_LOW_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}

void do_signal_led_off(void)
{
	char cmd[CMD_STR_LEN] = {0};

	if (CONFIG_HUAYI_HIGH_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_HIGH_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_MIDDLE_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_MIDDLE_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}

	if (CONFIG_HUAYI_LOW_SIGNAL_LED_GPIO_NUM != 1000)
	{
		snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_LOW_SIGNAL_LED_GPIO_NUM);
		system(cmd);
	}
	
	return;
}

#endif

