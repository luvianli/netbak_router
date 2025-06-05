
#include "huayi_core_queue.h"
#include "huayi_handle_reload.h"

extern VTBool gRunAgent;
extern VTBool gNeedReload;
pthread_mutex_t reload_mutex;

VTBool huayi_reload_functions_init(struct VcoreAgent *Agent)
{
	Agent->CoreReloadFunction[HUAYI_MSG_SYS_UPGRADE-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_sys_upgrade;
	Agent->CoreReloadFunction[HUAYI_MSG_SYS_RESET-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_sys_reset;
	Agent->CoreReloadFunction[HUAYI_MSG_SYS_REBOOT-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_sys_reboot;
	Agent->CoreReloadFunction[HUAYI_MSG_SCHE_REBOOT-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_sche_reboot;
	Agent->CoreReloadFunction[HUAYI_MSG_RESTORE_CONFIG-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_restore_system_config;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_NETWORK-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_network;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_DHCPS-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_dhcps;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_WIRELESS-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_wireless;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_NTPD-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_system_ntpd;
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_FIREWALL-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_firewall;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_WANUP-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_wanup;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_STATIC_ROUTE-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_static_route;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_FIREWALL_REMOTE_MANAGE-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_remote_manage;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_FIREWALL_MACFILTER-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_macfilter;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_FIREWALL_URLFILTER-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_urlfilter;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_FIREWALL_IPPORTFILTER-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_ipportfilter;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_FIREWALL_DMZ-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_dmz;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_FIREWALL_PORTFORWARD-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_port_forward;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_WANPING-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_wanping;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_UPNPD-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_upnpd;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_ACL-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_acl;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_WIFIGUESTFILTER-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_wifiguestfilter;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_QOS_IPLIMIT-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_qos_iplimit;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_DDNS-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_ddns;
#endif
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_WPSPBC-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_wps_pbc;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_WPSPIN-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_wps_pin;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_HWNAT-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_hwnat;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_WIRELESS_SCAN-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_wireless_scan;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_WIFI_SCHE-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_wifi_sche;
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_LTE_4G-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_lte4g;
#endif

#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_SAVE_DATABASE-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_save_database;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_SAVE_DATABASE-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_save_database;
#endif

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_PPTPD-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_pptpd;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_XL2TPD-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_xl2tpd;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_PPTP-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_pptp;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_XL2TP-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_xl2tp;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_VPN_USER-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_vpn_user;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_VPN_ROUTE-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_vpn_route;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_SERVER_VPN_ROUTE-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_vpn_server_route;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_OPENVPN-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_openvpn;
#endif
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_FTPD-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_ftpd;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_PRINTER-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_printer;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_MOUNT-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_mount;
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_UDHCPD-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_udhcpd;
#if defined(CONFIG_HUAYI_SUPPORT_CONTROL_TELNET_OPTION)
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_TELNET-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_telnet;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	Agent->CoreReloadFunction[HUAYI_MSG_RELOAD_SMALL_4GFUNC-HUAYI_MSG_PAYLOAD_OFFSET] = huayi_reload_small4g_func;
#endif

	Agent->CoreReloadFunction[HUAYI_MSG_SWITCH_CARD-HUAYI_MSG_PAYLOAD_OFFSET] =huayi_switch_card;
    Agent->CoreReloadFunction[HUAYI_MSG_CHECK_INTERNET_STATE-HUAYI_MSG_PAYLOAD_OFFSET] =huayi_check_internet_state;

    Agent->CoreReloadFunction[HUAYI_MEIGE_MODULE_DECRYPTION_STATE-HUAYI_MSG_PAYLOAD_OFFSET] =huayi_meige_module_decryption;
	Agent->CoreReloadFunction[HUAYI_MEIGE_MODULE_SET_BAND-HUAYI_MSG_PAYLOAD_OFFSET] =huayi_meige_set_band;
	Agent->CoreReloadFunction[HUAYI_MEIGE_MODULE_CLEAN_CARDINFO-HUAYI_MSG_PAYLOAD_OFFSET] =huayi_meige_clean_cardinfo;


	return VT_TRUE;
}

VTBool huayi_execute_need_reload(void)
{
	gNeedReload = VT_TRUE;
	pthread_mutex_unlock(&reload_mutex);
	return VT_TRUE;
}

void huayi_execute_reload(void * arg)
{
	struct VcoreAgent *Agent= (struct VcoreAgent *)arg;
	struct huayi_handle_msg node;

	pthread_mutex_init(&reload_mutex,NULL);

	pthread_mutex_lock(&reload_mutex);
	
	while(gRunAgent == VT_TRUE)
	{
		memset(&node,0,sizeof(node));
		pthread_mutex_lock(&reload_mutex); 		
		if(gNeedReload == VT_TRUE)
		{	
			gNeedReload = VT_FALSE;
			
queue_pop:
			if(huayi_msg_queue_pop(&node))
			{
				if(Agent->CoreReloadFunction[node.msgid-HUAYI_MSG_PAYLOAD_OFFSET])
				{	
					Agent->CoreReloadFunction[node.msgid-HUAYI_MSG_PAYLOAD_OFFSET]((void *)Agent,node.input_root);
					cJSON_Delete(node.input_root);
				}
				goto queue_pop;
			}
		}
	}
	
	pthread_exit(NULL);
	
	pthread_mutex_destroy(&reload_mutex);
	
	printf("Thread huayi_execute_reload: Exit!\n");
	
	return;

}

