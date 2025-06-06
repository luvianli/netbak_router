
#include "fastcgi_common.h"

int (*FastCgiHandleFunction[MAX_HUAYI_HYID])(cJSON *request_root, cJSON *response_root,int *lock_time);

FCGI_BOOL FastCgiHandleFunctionInit(void)
{	
	memset(&FastCgiHandleFunction,0,sizeof(MAX_HUAYI_HYID*sizeof(int)));
	
	FastCgiHandleFunction[HYID_GET_PRE_LOGIN_CONFIG] = get_pre_login_config;

	FastCgiHandleFunction[HYID_SET_PRE_LOGIN_CONFIG] = set_pre_login_config;

	FastCgiHandleFunction[HYID_GET_LANGUAGE_CONFIG] = get_language_config;
	
	FastCgiHandleFunction[HYID_SET_LANGUAGE_CONFIG] = set_language_config;
	
	FastCgiHandleFunction[HYID_DO_LOGIN_AUTH] = do_login_auth;

	FastCgiHandleFunction[HYID_DO_LOGOUT_CLEAN] = do_logout_clean;
	
	FastCgiHandleFunction[HYID_CHANGE_ADMIN_PASSWORD] = change_admin_password;
	
	FastCgiHandleFunction[HYID_GET_QUICK_SETTING_CONFIG] = get_quick_setting_config;

	FastCgiHandleFunction[HYID_SET_QUICK_SETTING_CONFIG] = set_quick_setting_config;
	
	FastCgiHandleFunction[HYID_SET_SYSTEM_REBOOT] = set_system_reboot;
	
	FastCgiHandleFunction[HYID_SET_SYSTEM_FACTORY] = set_system_factory;

	FastCgiHandleFunction[HYID_GET_UPGRADE_INFO] = get_upgrade_info;
	
	FastCgiHandleFunction[HYID_GET_CURRETN_USER_INFO] = get_current_user_info;
	
	FastCgiHandleFunction[HYID_GET_LAN_ARP_LIST] = get_arp_list;
	
	FastCgiHandleFunction[HYID_GET_NETWORK_INFO] = get_network_info;
	
	FastCgiHandleFunction[HYID_GET_DHCPS_SERVER_CONFIG] = get_dhcps_server_config;
	
	FastCgiHandleFunction[HYID_SET_DHCPS_SERVER_CONFIG] = set_dhcps_server_config;

	FastCgiHandleFunction[HYID_GET_LAN_CONFIG] = get_lan_config;
	
	FastCgiHandleFunction[HYID_SET_LAN_CONFIG] = set_lan_config;
	
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
	FastCgiHandleFunction[HYID_GET_WAN_CONFIG] = get_wan_config;
	
	FastCgiHandleFunction[HYID_SET_ETH_WAN_CONFIG] = set_wan_config;

	FastCgiHandleFunction[HYID_GET_STATIC_ROUTER_CONFIG] = get_static_router_config;

	FastCgiHandleFunction[HYID_SET_STATIC_ROUTER_CONFIG] = set_static_router_config;

	FastCgiHandleFunction[HYID_GET_REMOTE_MANAGE_CONFIG] = get_remote_manage_config;

	FastCgiHandleFunction[HYID_SET_REMOTE_MANAGE_CONFIG] = set_remote_manage_config;

	FastCgiHandleFunction[HYID_GET_MAC_FILTER_CONFIG] = get_mac_filter_config;

	FastCgiHandleFunction[HYID_SET_MAC_FILTER_CONFIG] = set_mac_filter_config;

	FastCgiHandleFunction[HYID_GET_URL_FILTER_CONFIG] = get_url_filter_config;

	FastCgiHandleFunction[HYID_SET_URL_FILTER_CONFIG] = set_url_filter_config;

	FastCgiHandleFunction[HYID_GET_IP_PORT_FILTER_CONFIG] = get_ip_port_filter_config;

	FastCgiHandleFunction[HYID_SET_IP_PORT_FILTER_CONFIG] = set_ip_port_filter_config;

	FastCgiHandleFunction[HYID_GET_DMZ_CONFIG] = get_dmz_config;
	
	FastCgiHandleFunction[HYID_SET_DMZ_CONFIG] = set_dmz_config;

	FastCgiHandleFunction[HYID_GET_PORT_FORWARD_CONFIG] = get_port_forward_config;
	
	FastCgiHandleFunction[HYID_SET_PORT_FORWARD_CONFIG] = set_port_forward_config;

	FastCgiHandleFunction[HYID_GET_UPNPD_CONFIG] = get_upnpd_config;
	
	FastCgiHandleFunction[HYID_SET_UPNPD_CONFIG] = set_upnpd_config;

	FastCgiHandleFunction[HYID_GET_WAN_PING_CONFIG] = get_wan_ping_config;
	
	FastCgiHandleFunction[HYID_SET_WAN_PING_CONFIG] = set_wan_ping_config;

	FastCgiHandleFunction[HYID_GET_ACL_LIST] = get_acl_list;	
	
	FastCgiHandleFunction[HYID_ADD_ACL_LIST] = add_acl_list;	
	
	FastCgiHandleFunction[HYID_DEL_ACL_LIST] = delete_acl_list;

	FastCgiHandleFunction[HYID_GET_QOS_IPLIMIT_CONFIG] = get_qos_iplimit_config;

	FastCgiHandleFunction[HYID_SET_QOS_IPLIMIT_CONFIG] = set_qos_iplimit_config;
#endif

	FastCgiHandleFunction[HYID_GET_OVERVIEW_INFO] = get_overview_info;

	FastCgiHandleFunction[HYID_GET_PASSWORD_STATE] = get_passwd_state;
	
#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
	FastCgiHandleFunction[HYID_GET_DDNS_CONFIG] = get_ddns_config;

	FastCgiHandleFunction[HYID_SET_DDNS_CONFIG] = set_ddns_config;
#endif

	FastCgiHandleFunction[HYID_GET_PING_DETECT_RESULT] = get_ping_detect_result;

	FastCgiHandleFunction[HYID_DO_PING_DETECT] = do_ping_detect;

	FastCgiHandleFunction[HYID_STOP_PING_DETECT] = stop_ping_detect;

	FastCgiHandleFunction[HYID_GET_TRACERT_DETECT_RESULT] = get_tracert_detect_result;

	FastCgiHandleFunction[HYID_DO_TRACERT_DETECT] = do_tracert_detect;

	FastCgiHandleFunction[HYID_STOP_TRACERT_DETECT] = stop_tracert_detect;
	
#if defined(CONFIG_PACKAGE_HUAYI_DHCP_DETECT)
	FastCgiHandleFunction[HYID_GET_DHCP_DETECT_RESULT] = get_dhcp_detect_result;

	FastCgiHandleFunction[HYID_DO_DHCP_DETECT] = do_dhcp_detect;

	FastCgiHandleFunction[HYID_STOP_DHCP_DETECT] = stop_dhcp_detect;
#endif
	
	FastCgiHandleFunction[HYID_GET_WIFI_BASIC_CONFIG] = get_wifi_basic_config;

	FastCgiHandleFunction[HYID_SET_WIFI_BASIC_CONFIG] = set_wifi_basic_config;

	FastCgiHandleFunction[HYID_GET_WIFI_ADVANCE] = get_wifi_advance;

	FastCgiHandleFunction[HYID_SET_WIFI_ADVANCE] = set_wifi_advance;

	FastCgiHandleFunction[HYID_GET_WIFI_MACFILTERLIST] = get_wifi_macfilterlist;

	FastCgiHandleFunction[HYID_SET_WIFI_MACFILTERLIST] = set_wifi_macfilterlist;

	FastCgiHandleFunction[HYID_GET_WIFI_STAMACLIST] = get_wifi_stamaclist;

	FastCgiHandleFunction[HYID_GET_WFII_MBSSID] = get_wifi_mbssid_config;

	FastCgiHandleFunction[HYID_SET_WIFI_MBSSID] = set_wifi_mbssid_config;
	
#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7620) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7628) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
	FastCgiHandleFunction[HYID_GET_WIFI_VAP_VLAN] = get_wifi_vlan_config;
	
	FastCgiHandleFunction[HYID_SET_WIFI_VAP_VLAN] = set_wifi_vlan_config;
#endif
	
	FastCgiHandleFunction[HYID_GET_SYSTEM_OPMODE] = get_system_opmode;

	FastCgiHandleFunction[HYID_SET_SYSTEM_OPMODE] = set_system_opmode;

	FastCgiHandleFunction[HYID_GET_GATEWAY_MODE_CONFIG] = get_gateway_mode_config;

	FastCgiHandleFunction[HYID_DO_WAN_PROTO_DETECT] = do_wan_proto_detect;
	
	FastCgiHandleFunction[HYID_SET_WIFI_SCAN] = set_wifi_scan_start;

	FastCgiHandleFunction[HYID_GET_WIFI_SCAN_STATUS] = get_wifi_scanstatus;

	FastCgiHandleFunction[HYID_GET_WIFI_SCAN_LIST] = get_wifi_scanlist;

	FastCgiHandleFunction[HYID_GET_NTP_CONFIG] = get_ntp_config;	

	FastCgiHandleFunction[HYID_SET_NTP_CONFIG] = set_ntp_config;	

	FastCgiHandleFunction[HYID_USE_BROWSER_TIME] = use_browser_time;	

	FastCgiHandleFunction[HYID_GET_SYSLOG_CONFIG] = get_syslog_config;	

	FastCgiHandleFunction[HYID_SET_SYSLOG_CONFIG] = set_syslog_config;	

	FastCgiHandleFunction[HYID_DISPLAY_SYSLOG] = display_syslog;	

	FastCgiHandleFunction[HYID_CLEAN_SYSLOG] = clean_syslog;	

	FastCgiHandleFunction[HYID_GET_SCHE_REBOOT_CONFIG] = get_sche_reboot_config;	

	FastCgiHandleFunction[HYID_SET_SCHE_REBOOT_CONFIG] = set_sche_reboot_config;

	FastCgiHandleFunction[HYID_GET_SYSTEM_INFO] = get_system_info;

	FastCgiHandleFunction[HYID_GET_HWNAT_CONFIG] = get_hwnat_config;

	FastCgiHandleFunction[HYID_SET_HWNAT_CONFIG] = set_hwnat_config;

	FastCgiHandleFunction[HYID_GET_ALLSTALIST] = get_allstalist;
	
	FastCgiHandleFunction[HYID_GET_NETWORK_CONFIG] = get_network_config;

	FastCgiHandleFunction[HYID_SET_NETWORK_CONFIG] = set_network_config;

	FastCgiHandleFunction[HYID_GET_SYSMODE_PAGE_INFO] = get_opmode_info;

	FastCgiHandleFunction[HYID_GET_BASIC_PAGE_ALL_INFO] = get_basic_page_all_info;

	FastCgiHandleFunction[HYID_GET_BASIC_PAGE_STATUS_INFO] = get_basic_page_status_info;

	FastCgiHandleFunction[HYID_SET_BASIC_PAGE_WIFI_CONFIG] = set_basic_page_wifi_config;

	FastCgiHandleFunction[HYID_GET_WIFI_EASY_CONFIG] = get_wifi_easyconf;

	FastCgiHandleFunction[HYID_SET_WIFI_EASY_CONFIG] = set_wifi_easyconf;

	FastCgiHandleFunction[HYID_GET_PAGE_SYSTEM_STATUS] = get_all_system_info;

	FastCgiHandleFunction[HYID_GET_PAGE_NETWORK_STATUS] = get_all_network_info;

	FastCgiHandleFunction[HYID_GET_WIFI_SCHE_CONFIG] = get_wifi_sche_config;

	FastCgiHandleFunction[HYID_SET_WIFI_SCHE_CONFIG] = set_wifi_sche_config;

#if defined(CONFIG_PACKAGE_HUAYI_CAPWAP_CLIENT)
	FastCgiHandleFunction[HYID_GET_THINAP_CONFIG] = get_thinap_config;
	
	FastCgiHandleFunction[HYID_SET_THINAP_CONFIG] = set_thinap_config;
#endif

#if defined(CONFIG_HUAYI_CGI_WIZARD_SUPPORT)
	FastCgiHandleFunction[HYID_GET_ALL_QUICK_SETTING_CONFIG] = get_all_quick_setting_config;

	FastCgiHandleFunction[HYID_SET_ALL_QUICK_SETTING_CONFIG] = set_all_quick_setting_config;

	FastCgiHandleFunction[HYID_SET_SKIP_QUICK_SETTING] = set_skip_quick_setting;
#endif

	FastCgiHandleFunction[HYID_GET_WIRLESS_GUEST_CONFIG] = get_guest_network;

	FastCgiHandleFunction[HYID_SET_WIRLESS_GUEST_CONFIG] = set_guest_network;

	FastCgiHandleFunction[HYID_GET_INTERNET_STATUS] = get_internet_status;

	FastCgiHandleFunction[HYID_GET_DEVICE_INFO] = get_device_info;
	
	FastCgiHandleFunction[HYID_SET_OPTIMIZE] = set_optimize;
	
	FastCgiHandleFunction[HYID_GET_HOE_DEVICES] = get_home_devices;
	
	FastCgiHandleFunction[HYID_SET_DEVICE_NAME] = set_device_name;
	
	FastCgiHandleFunction[HYID_SET_WIRELESS_CLOCK] = set_wireless_clock;
	
	FastCgiHandleFunction[HYID_SET_GUEST_CLOCK] = set_guest_clock;

	FastCgiHandleFunction[HYID_GET_WIFI_WPS_CONFIG] = get_wifi_wps;
	
	FastCgiHandleFunction[HYID_SET_WIFI_WPS_CONFIG] = set_wifi_wps;
	
	FastCgiHandleFunction[HYID_SET_WIFI_WPS_PBC] = set_wifi_wps_pbc;
	
	FastCgiHandleFunction[HYID_SET_WIFI_WPS_PIN] = set_wifi_wpsstapin;
	
	FastCgiHandleFunction[HYID_GET_WIFI_WPS_LOCAL_PIN] = get_wifi_wpspin;
	
	FastCgiHandleFunction[HYID_SET_WIFI_WPS_FIRST_PIN] = set_wifi_wpsfirstpin;
	
	FastCgiHandleFunction[HYID_GET_WIFI_WPS_STATUS] = get_wifi_wpsstatus;
	
	FastCgiHandleFunction[HYID_SET_WIFI_WPS_PIN_CONFIG] = set_wifi_wps_ownpin;
	
	FastCgiHandleFunction[HYID_SET_WIFI_WPS_RANDOM_PIN] = set_wifi_wpsrandompin;
	
	FastCgiHandleFunction[HYID_GET_WIRELESS_STATUS] = get_wireless_status;

	FastCgiHandleFunction[HYID_SET_WIRELESS_STATUS] = set_wireless_status;

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	FastCgiHandleFunction[HYID_GET_PPTPD_CONFIG] = get_pptpd_server_config;
		
	FastCgiHandleFunction[HYID_SET_PPTPD_CONFIG] = set_pptpd_server_config;
		
	FastCgiHandleFunction[HYID_GET_XL2TPD_CONFIG] = get_xl2tpd_server_config;
	
	FastCgiHandleFunction[HYID_SET_XL2TPD_CONFIG] = set_xl2tpd_server_config;

	FastCgiHandleFunction[HYID_GET_PPTP_CONFIG] = get_pptp_client_config;
	
	FastCgiHandleFunction[HYID_SET_PPTP_CONFIG] = set_pptp_client_config;

	FastCgiHandleFunction[HYID_GET_L2TP_CONFIG] = get_l2tp_client_config;
	
	FastCgiHandleFunction[HYID_SET_L2TP_CONFIG] = set_l2tp_client_config;
	
	FastCgiHandleFunction[HYID_GET_VPN_USER_CONFIG] = get_vpn_user_config;

	FastCgiHandleFunction[HYID_SET_VPN_USER_CONFIG] = set_vpn_user_config;

	FastCgiHandleFunction[HYID_GET_VPN_USER_STATUS] = get_vpn_user_status;
	
	FastCgiHandleFunction[HYID_DO_KICK_VPN_USER] = do_kick_vpn_user;
	
	FastCgiHandleFunction[HYID_GET_OPENVPN_CONFIG] = get_openvpn_config;
	
	FastCgiHandleFunction[HYID_SET_OPENVPN_CONFIG] = set_openvpn_config;
#endif

#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
	FastCgiHandleFunction[HYID_GET_IPV6_CONFIG] = get_ipv6_config;

	FastCgiHandleFunction[HYID_SET_IPV6_CONFIG] = set_ipv6_config;
#endif

	FastCgiHandleFunction[HYID_GET_MULTICAST_CONFIG] = get_multicast_config;

	FastCgiHandleFunction[HYID_SET_MULTICAST_CONFIG] = set_multicast_config;

	FastCgiHandleFunction[HYID_GET_FTP_CONFIG] = get_ftp_config;

	FastCgiHandleFunction[HYID_SET_FTP_CONFIG] = set_ftp_config;

	FastCgiHandleFunction[HYID_GET_PRINTER_CONFIG] = get_printer_config;

	FastCgiHandleFunction[HYID_SET_PRINTER_CONFIG] = set_printer_config;

#ifdef CONFIG_PACKAGE_EASYCWMP
	FastCgiHandleFunction[HYID_GET_TR069_CONFIG] = get_tr069_config;	

	FastCgiHandleFunction[HYID_SET_TR069_CONFIG] = set_tr069_config;
#endif

#if defined(CONFIG_HUAYI_SUPPORT_REPEATER_DHCPD)
	FastCgiHandleFunction[HYID_GET_FORCE_FIX_DHCP] = get_force_fix_dhcp;

	FastCgiHandleFunction[HYID_SET_FORCE_FIX_DHCP] = set_force_fix_dhcp;
#endif
	FastCgiHandleFunction[HYID_GET_WANMODE_INFO] = get_wanmode_info;
    FastCgiHandleFunction[HYID_GET_HBP_CONFIG] = get_hbp_config;
    FastCgiHandleFunction[HYID_SET_HBP_CONFIG] = set_hbp_config;
    FastCgiHandleFunction[HYID_GET_EXTERNAL_CARRD_CONFIG] = get_external_card_config;
    FastCgiHandleFunction[HYID_SET_EXTERNAL_CARRD_CONFIG] = set_external_card_config;

	return FCGI_TRUE;
	
}

int main (int argc, char *argv[])
{
	
	char  *rest_data		= NULL;
	char  *content_type		= NULL;
	FCGI_RESP_CODE   rest_code;
	int   content_len 		= 0;
	char  *query_data 		= NULL;
	cJSON *request_root 	= NULL;
	cJSON *response_root 	= NULL;
	int lock_time			= 0;
	int  HYID = 0;
	int now_time = 0;
	int old_time = 0;
	
	if(!FastCgiHandleFunctionInit())
	{
		return FCGI_FALSE;
	}

#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)	
	web_cookie_init();
#endif

	//FCGX_Init();
	
	//while (FCGI_Accept() >= 0) 
	{
		rest_data		= NULL;
		content_type	= NULL;
		rest_code		= FCGI_RESP_CODE_SUCCESS;
		content_len		= 0;
		query_data 		= NULL;
		request_root 	= NULL;
		response_root	= NULL;
		lock_time		= 0;
		HYID			= 0;

		response_root = cJSON_CreateObject();
		
		rest_data = getenv("QUERY_STRING");
		
		if(rest_data && (strlen(rest_data) > 0))
		{
			rest_code = FCGI_RESP_CODE_DATA_FORMAT_ERROR;
			
			goto v_out;
		}
		
		content_type =getenv("CONTENT_TYPE");

		if(strstr(content_type,"application/json") == NULL)
		{
			rest_code = FCGI_RESP_CODE_DATA_FORMAT_ERROR;
			
			goto v_out;
		}
		
		content_len = strtol(getenv("CONTENT_LENGTH"), NULL, 10) + 1;
		
		if(content_len <= 1)
		{
			rest_code = FCGI_RESP_CODE_DATA_FORMAT_ERROR;

			goto v_out;
		}
		
	 	query_data = malloc(content_len);

		if(!query_data)
		{
			rest_code = FCGI_RESP_CODE_SERVER_FAILURE;

			goto v_out;
		}

		memset(query_data,0,content_len);
		
		fread(query_data, 1, content_len, stdin);
		
		request_root = cJSON_Parse(query_data);
		
		if (!request_root) 
		{
			rest_code = FCGI_RESP_CODE_DATA_FORMAT_ERROR;
			
			goto v_out;
		}
		
		if(!huayi_json_get_int(request_root,"HYID",&HYID))
		{
			rest_code = FCGI_RESP_CODE_HYID_MISS;
			
			goto v_out;
		}
		
		if(HYID < HYID_GET_PRE_LOGIN_CONFIG || HYID >= MAX_HUAYI_HYID)
		{
			rest_code = FCGI_RESP_CODE_ILLEGAL_HYID;
			FASTCGI_LOG(" HYID = %d\n", HYID);
			goto v_out;
		}
		
#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
		if(HYID > HYID_DO_LOGIN_AUTH && HYID != HYID_SET_WIFI_SCAN && HYID != HYID_GET_WIFI_SCAN_STATUS && 
			HYID != HYID_GET_WIFI_SCAN_LIST)
#else
		if(HYID > HYID_DO_LOGIN_AUTH)
#endif
		{
			if(FCGI_RESP_CODE_UNAUTHORIZED == check_landing_status())
			{
				rest_code = FCGI_RESP_CODE_UNAUTHORIZED;
				goto v_out;
			}
		}
#else
		char sys_password[OPTION_STR_LEN]={0};
		char cookie_pass[OPTION_STR_LEN]={0};
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
		if(HYID > HYID_DO_LOGIN_AUTH && HYID != HYID_SET_WIFI_SCAN && HYID != HYID_GET_WIFI_SCAN_STATUS && 
					HYID != HYID_GET_WIFI_SCAN_LIST)
#else
		if(HYID > HYID_DO_LOGIN_AUTH)
#endif
		{
			if(huayi_uci_get_str(PKG_SYSTEM,"login","http_pass",sys_password) && strlen(sys_password) > 0)
			{
				char *fullP = NULL;
				char *sess_id = NULL;
				char *tmpP = NULL;
				fullP = getenv("HTTP_COOKIE");
				if(NULL != fullP)
				{
					if((sess_id = strstr(fullP, "PSW=")) != NULL)
					{
						sess_id += strlen("PSW=");
						strncpy(cookie_pass, sess_id, sizeof(cookie_pass));
						if((tmpP = strstr(cookie_pass, ";")) != NULL)
						{
							*tmpP = '\0';
						}
						if(strcmp(cookie_pass,sys_password)!=0){
							rest_code = FCGI_RESP_CODE_UNAUTHORIZED;
							goto v_out;
						}
					}
					else
					{
						rest_code = FCGI_RESP_CODE_UNAUTHORIZED;
						goto v_out;
					}		
				}
			}
		}

#endif
		huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&now_time);
		lock_time = now_time - old_time;

		if(FastCgiHandleFunction[HYID] != NULL)
		{
			rest_code = FastCgiHandleFunction[HYID](request_root,response_root,&lock_time);
		}else
		{
			FASTCGI_LOG(" HYID = %d\n", HYID);
			rest_code = FCGI_RESP_CODE_ILLEGAL_HYID;
		}

		if (lock_time == FASTCGI_UPDATE_INFO_FLAG || lock_time < 0)
			old_time = now_time;
v_out:
		fast_cgi_response_web(response_root,rest_code,lock_time);

		cJSON_Delete(request_root);
		
		cJSON_Delete(response_root);
		
		if(query_data)
		{
			free(query_data);
		}
		//continue;
	}
	return FCGI_TRUE;
}


