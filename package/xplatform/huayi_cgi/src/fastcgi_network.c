#include "fastcgi_common.h"

int get_multiwan_static_config_by_index(cJSON *wan_obj,char *section)
{
	char	ipaddr[IPV4_STR_LEN] = {0};
	char	netmask[IPV4_STR_LEN] = {0};
	char	gateway[IPV4_STR_LEN] = {0};
	char	temp_dns[OPTION_STR_LEN] = {0};
	char	dns[IPV4_STR_LEN] = {0};
	cJSON	*dns_array = NULL;
	int		i = 0;

	huayi_uci_get_str(PKG_NETWORK,section,"ipaddr",ipaddr);
	huayi_uci_get_str(PKG_NETWORK,section,"netmask",netmask);
	huayi_uci_get_str(PKG_NETWORK,section,"gateway",gateway);

	cJSON_AddStringToObject(wan_obj,"ip_addr",ipaddr);
	cJSON_AddStringToObject(wan_obj,"subnet_mask",netmask);
	cJSON_AddStringToObject(wan_obj,"gateway",gateway);

	huayi_uci_get_str(PKG_NETWORK,section,"dns",temp_dns);

	dns_array = cJSON_CreateArray();

	while(huayi_get_nth_value(i++, temp_dns, WEB_SPACE_DELIMIT, dns, sizeof(dns)))
	{
		cJSON_AddItemToArray(dns_array,cJSON_CreateString(dns));
	}

	cJSON_AddItemToObject(wan_obj,"dns_arr",dns_array);

	return FCGI_TRUE;
}

int set_multiwan_static_config_by_index(cJSON *wan_obj,char *section)
{
	char	ipaddr[IPV4_STR_LEN] = {0};
	char 	netmask[IPV4_STR_LEN] = {0};
	char	gateway[IPV4_STR_LEN] = {0};
	cJSON	*dns_array = NULL;
	cJSON	*dns_obj = NULL;
	int		i = 0;
	int		dns_count = 0;

	if(!huayi_json_get_str(wan_obj,"ip_addr",ipaddr,IPV4_STR_LEN)||
		!huayi_json_get_str(wan_obj,"subnet_mask",netmask,IPV4_STR_LEN)||
		!huayi_json_get_str(wan_obj,"gateway",gateway,IPV4_STR_LEN))
	{
		return FCGI_FALSE;
	}

	huayi_uci_set_str(PKG_NETWORK,section,"ipaddr",ipaddr);
	huayi_uci_set_str(PKG_NETWORK,section,"netmask",netmask);
	huayi_uci_set_str(PKG_NETWORK,section,"gateway",gateway);

	dns_array = cJSON_GetObjectItem(wan_obj, "dns_arr");

	if(dns_array == NULL)
	{
		return FCGI_FALSE;
	}

	dns_count = cJSON_GetArraySize(dns_array);
	huayi_uci_delete_option(PKG_NETWORK,section,"dns");
	for(i=0;i<dns_count;i++)
	{
		dns_obj = cJSON_GetArrayItem(dns_array,i);
		huayi_uci_add_list(PKG_NETWORK,section,"dns",dns_obj->valuestring);
	}

	return FCGI_TRUE;
}

int	get_multiwan_pppoe_config_by_index(cJSON *wan_obj,char *section)
{
	char 	username[PASSWD_LONG_STR_LEN] = {0};
	char	password[PASSWD_LONG_STR_LEN] = {0};
	char	ac[OPTION_STR_LEN] = {0};
	char	service[OPTION_STR_LEN] = {0};

	huayi_uci_get_str(PKG_NETWORK,section,"username",username);
	huayi_uci_get_str(PKG_NETWORK,section,"password",password);
	huayi_uci_get_str(PKG_NETWORK,section,"ac",ac);
	huayi_uci_get_str(PKG_NETWORK,section,"service",service);

	cJSON_AddStringToObject(wan_obj,"usr",username);
	cJSON_AddStringToObject(wan_obj,"pwd",password);
	cJSON_AddStringToObject(wan_obj,"ac",ac);
	cJSON_AddStringToObject(wan_obj,"service",service);

	return FCGI_TRUE;
}

int	set_multiwan_pppoe_config_by_index(cJSON *wan_obj,char *section)
{
	char	username[PASSWD_LONG_STR_LEN] = {0};
	char	password[PASSWD_LONG_STR_LEN] = {0};
	char	ac[OPTION_STR_LEN] = {0};
	char	service[OPTION_STR_LEN] = {0};

	if(!huayi_json_get_str(wan_obj,"usr",username,PASSWD_LONG_STR_LEN)||!huayi_json_get_str(wan_obj,"pwd",password,PASSWD_LONG_STR_LEN))
	{
			return FCGI_FALSE;
	}
	huayi_json_get_str(wan_obj,"ac",ac,OPTION_STR_LEN);
	huayi_json_get_str(wan_obj,"service",service,OPTION_STR_LEN);

	huayi_uci_set_str(PKG_NETWORK,section,"username",username);
	huayi_uci_set_str(PKG_NETWORK,section,"password",password);
	huayi_uci_set_str(PKG_NETWORK,section,"ac",ac);
	huayi_uci_set_str(PKG_NETWORK,section,"service",service);
	huayi_uci_delete_option(PKG_NETWORK,section,"dns");

	return FCGI_TRUE;

}

int	get_multiwan_dhcp_config_by_index(cJSON *wan_obj,char *section)
{
	char	hostname[OPTION_STR_LEN] = {0};

	huayi_uci_get_str(PKG_NETWORK,section,"hostname",hostname);

	if(strlen(hostname) <= 0)
	{
		huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
	}

	cJSON_AddStringToObject(wan_obj,"host_name",hostname);

	return FCGI_TRUE;
}

int	set_multiwan_dhcp_config_by_index(cJSON *wan_obj,char *section)
{
	char	hostname[OPTION_STR_LEN] = {0};

	huayi_json_get_str(wan_obj,"host_name",hostname,OPTION_STR_LEN);
	if(strlen(hostname)>0)
	{
		huayi_uci_set_str(PKG_NETWORK,section,"hostname",hostname);
	}

	huayi_uci_delete_option(PKG_NETWORK,section,"dns");

	return FCGI_TRUE;
}

int quick_set_lan_config(cJSON *request_root, cJSON * response_root)
{
	char	ipaddr[IPV4_STR_LEN] = {0};
	char    netmask[IPV4_STR_LEN] = {0};
	char 	IPADDR[IPV4_STR_LEN] = {0};
	char 	NETMASK[IPV4_STR_LEN] = {0};
	int		start 	= 0;
	int		limit	= 0;
	int 	ip	= 0;
	int		net = 0;
	int 	min	= 0;
	int		max = 0;
	int 	ip_range = 0;

	if(!huayi_json_get_str(request_root,"lan_ipaddr",ipaddr,IPV4_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"lan_netmask",netmask,IPV4_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",IPADDR))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","netmask",NETMASK))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(strcmp(IPADDR,ipaddr) || strcmp(NETMASK,netmask))
	{
		/*ip change need reload*/
		huayi_uci_set_str(PKG_NETWORK,"lan","ipaddr",ipaddr);
		huayi_uci_set_str(PKG_NETWORK,"lan","netmask",netmask);

		huayi_uci_get_int(PKG_DHCP,"lan","start",&start);

		huayi_uci_get_int(PKG_DHCP,"lan","limit",&limit);

		inet_pton(AF_INET,ipaddr, &ip);
		inet_pton(AF_INET,netmask, &net);
		ip = htonl(ip);
		net = htonl(net);
		min = (ip & net) + 1;
		max = (ip | ~net) - 1;
		ip_range = max - min + 1;

		if((ip_range < limit )||(ip_range < start ) || strcmp(NETMASK,netmask))
		{
			start	= HUAYI_DEFAULT_DHCP_START;
			limit	= ip_range;
			huayi_uci_set_int(PKG_DHCP,"lan","start",start);
			huayi_uci_set_int(PKG_DHCP,"lan","limit",limit);
			huayi_uci_commit(PKG_DHCP);
		}

		huayi_uci_commit(PKG_NETWORK);
	}

	cJSON_AddStringToObject(response_root,"reconnection_addr",ipaddr);
	cJSON_AddTrueToObject(response_root,"reconnection");

	return FCGI_RESP_CODE_SUCCESS;
}



void arplookup(char *ip, char *arp)
{
    char buf[CMD_STR_LEN];
	char ip_entry[RESULT_STR_LEN], hw_type[SMALL_STR_LEN],flags[SMALL_STR_LEN], hw_address[RESULT_STR_LEN];


    FILE *fp = fopen("/proc/net/arp", "r");

	strcpy(arp, "00:00:00:00:00:00");

	if(!fp)
	{
        return;
    }

    while(fgets(buf, CMD_STR_LEN, fp))
	{
        sscanf(buf, "%s %s %s %s", ip_entry, hw_type, flags, hw_address);
        if(!strcmp(ip, ip_entry))
		{
            strcpy(arp, hw_address);
            break;
        }
    }

    fclose(fp);

    return;
}

int get_current_user_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*result_root = NULL;
	char	*remote_ip =NULL;
	char	user_mac[MAC_STR_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	remote_ip = getenv("REMOTE_ADDR");

	arplookup(remote_ip,user_mac);

	cJSON_AddStringToObject(result_root,"usr_ip",remote_ip);

	cJSON_AddStringToObject(result_root,"usr_mac",user_mac);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int get_arp_list(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	FILE *fp = NULL;
	cJSON *result_root = NULL;
	cJSON *arp_array = NULL;
	cJSON *arp_obj = NULL;
	char buff[LIST_STR_LEN] = {0};
	char flags[SMALL_STR_LEN] = {0};
	char uperMacbuf[TEMP_STR_LEN] = {0},uperIpbuf[TEMP_STR_LEN] = {0},dev_name[TEMP_STR_LEN] = {0};
	int i = 0;
	int ret = 0;

	char arp_w_mac[LONG_LIST_BUFF_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	get_w_arp(arp_w_mac,"radio0_0");
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	get_w_arp(arp_w_mac,"radio1_0");
#endif

	fp = fopen("/proc/net/arp", "r");
	if (NULL == fp)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	result_root = cJSON_CreateObject();

	arp_array = cJSON_CreateArray();

	while(fgets(buff, sizeof(buff), fp) != NULL)
	{
		if(strstr(buff, "IP"))
		{
			continue;
		}

		sscanf(buff, "%s\t%*s\t%s\t%s\t%*s\t%s", uperIpbuf, flags, uperMacbuf, dev_name);
		if ((strlen(uperMacbuf) != 17) || (strcmp(uperMacbuf, "00:00:00:00:00:00") == 0))
		{
			continue;
		}

		if(strcmp(dev_name,"br-lan"))
		{
			continue;
		}

		if(strcmp(flags, "0x0") == 0)
		{
			if(strstr(str2upper(arp_w_mac),str2upper(uperMacbuf)) == NULL)
			{
				continue;
			}

			if(!check_ip_in_dhcplease(uperIpbuf))
			{
				continue;
			}

		}

		arp_obj = cJSON_CreateObject();

		cJSON_AddNumberToObject(arp_obj, "index", i);
		cJSON_AddStringToObject(arp_obj, "ip_addr", uperIpbuf);
		cJSON_AddStringToObject(arp_obj, "mac_addr", uperMacbuf);
		cJSON_AddItemToArray(arp_array,arp_obj);
		i++;
	}

	fclose(fp);
	system("huayi_cmd set system arp_flush 1");

	cJSON_AddItemToObject(result_root,"arp_arr",arp_array);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int get_network_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	cJSON *wan_obj = NULL;
	cJSON *lan_obj = NULL;
	cJSON *dns_array = NULL;
	int  wan_link = 0;
	int  wan_up = 0;
	int  wan_mtu = 0;
	char mac_address[MAC_STR_LEN] = {0};
	char ip_address[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char gateway[IPV4_STR_LEN] = {0};
	char temp_dns[TEMP_STR_LEN] = {0};
	char dns[IPV4_STR_LEN] = {0};
	int i = 0;
	int rx_bps = 0;
	int tx_bps = 0;
	unsigned long long rx_bbbps = 0;
	unsigned long long tx_bbbps = 0;
	unsigned long long wan_rx_bytes =0;
	unsigned long long wan_tx_bytes =0;
	int client_num = 0;
	char ifname[IPV4_STR_LEN] = {0};
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	char wan_type[OPTION_STR_LEN] = {0};
	int sim_state = 0;
	char str_sim_state[OPTION_STR_LEN] = {0};
	char lte_value[OPTION_STR_LEN] = {0};
#endif


	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	wan_obj = cJSON_CreateObject();

	lan_obj = cJSON_CreateObject();

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","mac_address",mac_address);
	cJSON_AddStringToObject(wan_obj,"mac_addr",mac_address);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&wan_link);
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up);

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","sim_ready", str_sim_state);
	sim_state = atoi(str_sim_state);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","iccid",lte_value);
	/*atcmd_get_iccid(lte_value);*/
	if(strlen(lte_value) < 10)
	{
		strcpy(lte_value, "");
	}
	if (!strcmp(wan_type,WAN_TYPE_4G_MOBILE))
	{
		huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &sim_state);
		if(sim_state == 0)
		{
			wan_up = 0;
		}
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_WIRED_PRIOROTY))
	{
		if (wan_link == 0)
		{
			huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &sim_state);
			if(sim_state == 0)
			{
				wan_up = 0;
			}
		}
	}
#elif defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_type, WAN_TYPE_REP4G_PRIOROTY))
	{
		wan_up = 1;
		huayi_uci_get_int(PKG_STATUS_BLEND, "wisp", "link", &wan_link);
		if (wan_link == 0)
		{
			huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &sim_state);
			//if (sim_state != MODEM_SIN_READY)
			if(sim_state == 0)
			{
				wan_up = 0;
			}
		}
	}
#endif
	else
#endif
	{
		if (wan_link == 0)
			wan_up = 0;
	}

	cJSON_AddNumberToObject(wan_obj,"link",wan_up);

	if (wan_up == 1)
	{
		huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","if_mtu",&wan_mtu);
		cJSON_AddNumberToObject(wan_obj,"mtu_num",wan_mtu);

		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",ip_address);
		cJSON_AddStringToObject(wan_obj,"ip_addr",ip_address);

		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","netmask",netmask);
		cJSON_AddStringToObject(wan_obj,"subnet_mask",netmask);

		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",gateway);
		cJSON_AddStringToObject(wan_obj,"gateway",gateway);

		memset(temp_dns,0,OPTION_STR_LEN);
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","dns_server",temp_dns);
		dns_array = cJSON_CreateArray();
		memset(dns,0,IPV4_STR_LEN);
		while(huayi_get_nth_value(i++, temp_dns, WEB_SPACE_DELIMIT, dns, sizeof(dns)))
		{
			cJSON_AddItemToArray(dns_array,cJSON_CreateString(dns));
		}
		cJSON_AddItemToObject(wan_obj,"dns_server_addr",dns_array);

		huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","tx_bps",&tx_bps);
		tx_bbbps = ((unsigned long long)tx_bps * 8);
		cJSON_AddNumberToObject(wan_obj,"tx_bps",tx_bbbps);

		huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","rx_bps",&rx_bps);
		rx_bbbps = ((unsigned long long)rx_bps * 8);
		cJSON_AddNumberToObject(wan_obj,"rx_bps",rx_bbbps);

		huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","rx_bytes",&wan_rx_bytes);
		cJSON_AddNumberToObject(wan_obj,"rx_bytes",wan_rx_bytes);

		huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","tx_bytes",&wan_tx_bytes);
		cJSON_AddNumberToObject(wan_obj,"tx_bytes",wan_tx_bytes);
	}
	else
	{
		cJSON_AddNumberToObject(wan_obj,"mtu_num",0);
		cJSON_AddStringToObject(wan_obj,"ip_addr","0.0.0.0");
		cJSON_AddStringToObject(wan_obj,"subnet_mask","0.0.0.0");
		cJSON_AddStringToObject(wan_obj,"gateway","0.0.0.0");

		dns_array = cJSON_CreateArray();
		cJSON_AddItemToArray(dns_array,cJSON_CreateString("0.0.0.0"));
		cJSON_AddItemToArray(dns_array,cJSON_CreateString("0.0.0.0"));
		cJSON_AddItemToObject(wan_obj,"dns_server_addr",dns_array);
		cJSON_AddNumberToObject(wan_obj,"tx_bps",0);
		cJSON_AddNumberToObject(wan_obj,"rx_bps",0);
		cJSON_AddNumberToObject(wan_obj,"rx_bytes",0);
		cJSON_AddNumberToObject(wan_obj,"tx_bytes",0);
	}
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	cJSON_AddStringToObject(wan_obj,"iccid", lte_value);
#endif
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ifname", ifname);
	if(!strcmp(ifname, "apcli0") || !strcmp(ifname, "apclii0"))
	{
		cJSON_AddStringToObject(wan_obj,"net_type", "WISP");
	}
	else if(!strcmp(ifname, "eth1") || !strcmp(ifname, "wwan0"))
	{
		cJSON_AddStringToObject(wan_obj,"net_type", "4G");
	}
	else if(!strcmp(ifname, "eth0.2"))
	{
		cJSON_AddStringToObject(wan_obj,"net_type", "Wired");
	}
	else
	{
		cJSON_AddStringToObject(wan_obj,"net_type", "4G");
	}
	cJSON_AddItemToObject(result_root,"wan_infos",wan_obj);


	memset(mac_address,0,MAC_STR_LEN);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","mac_address",mac_address);
	cJSON_AddStringToObject(lan_obj,"mac_addr",mac_address);

	memset(ip_address,0,IPV4_STR_LEN);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",ip_address);
	cJSON_AddStringToObject(lan_obj,"ip_addr",ip_address);

	memset(netmask,0,IPV4_STR_LEN);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","netmask",netmask);
	cJSON_AddStringToObject(lan_obj,"subnet_mask",netmask);

	get_active_arp(1,NULL,&client_num);
	cJSON_AddNumberToObject(lan_obj,"client_sum",client_num);

	cJSON_AddItemToObject(result_root,"lan_infos",lan_obj);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}


#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
int get_wan_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*result_root =NULL;
	char 	proto[OPTION_STR_LEN] = {0};
	char 	ipaddr[IPV4_STR_LEN] = {0};
	char 	netmask[IPV4_STR_LEN] = {0};
	char	gateway[IPV4_STR_LEN] = {0};
	char 	temp_dns[OPTION_STR_LEN] = {0};
	char 	dns[IPV4_STR_LEN] = {0};
	cJSON 	*dns_array = NULL;
	int 	i = 0;
	char 	username[PASSWD_LONG_STR_LEN] = {0};
	char 	password[PASSWD_LONG_STR_LEN] = {0};
	char 	ac[OPTION_STR_LEN] = {0};
	char 	service[OPTION_STR_LEN] = {0};
	char 	hostname[OPTION_STR_LEN] = {0};
	char 	macaddr[MAC_STR_LEN] = {0};
	char 	def_macaddr[MAC_STR_LEN] = {0};
	int  	ppp_mtu = 0;
	int  	cc_mtu = 0;
	int 	link=0;
#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	char 	hct_proto[OPTION_STR_LEN] = {0};
	char 	vpn_username[PASSWD_LONG_STR_LEN] = {0};
	char 	vpn_password[PASSWD_LONG_STR_LEN] = {0};
	char 	vpn_service[IPV4_STR_LEN] = {0};
	int 	xl2pd_enable = 0;
#endif
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	huayi_uci_get_str(PKG_NETWORK,"wan","ipaddr",ipaddr);
	cJSON_AddStringToObject(result_root,"ip_addr",ipaddr);

	huayi_uci_get_str(PKG_NETWORK,"wan","netmask",netmask);
	cJSON_AddStringToObject(result_root,"subnet_mask",netmask);

	huayi_uci_get_str(PKG_NETWORK,"wan","gateway",gateway);
	cJSON_AddStringToObject(result_root,"gateway",gateway);

	huayi_uci_get_str(PKG_NETWORK,"wan","dns",temp_dns);

	dns_array = cJSON_CreateArray();
	while(huayi_get_nth_value(i++, temp_dns, WEB_SPACE_DELIMIT, dns, sizeof(dns)))
	{
		cJSON_AddItemToArray(dns_array,cJSON_CreateString(dns));
	}
	cJSON_AddItemToObject(result_root,"dns_arr",dns_array);

	huayi_uci_get_str(PKG_NETWORK,"wan","username",username);
	cJSON_AddStringToObject(result_root,"usr",username);

	huayi_uci_get_str(PKG_NETWORK,"wan","password",password);
	cJSON_AddStringToObject(result_root,"pwd",password);

	huayi_uci_get_str(PKG_NETWORK,"wan","ac",ac);
	cJSON_AddStringToObject(result_root,"ac",ac);

	huayi_uci_get_str(PKG_NETWORK,"wan","service",service);
	cJSON_AddStringToObject(result_root,"service",service);

	huayi_uci_get_str(PKG_NETWORK,"wan","hostname",hostname);
	if(strlen(hostname) <= 0)
	{
		huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
	}
	cJSON_AddStringToObject(result_root,"host_name",hostname);

	huayi_uci_get_str(PKG_NETWORK,"wan","macaddr",macaddr);
	cJSON_AddStringToObject(result_root,"mac_addr",macaddr);

	huayi_uci_get_str(PKG_NETWORK,"wan","def_macaddr",def_macaddr);
	cJSON_AddStringToObject(result_root,"def_mac_addr",def_macaddr);

	huayi_uci_get_int(PKG_NETWORK,"wan","ppp_mtu",&ppp_mtu);
	cJSON_AddNumberToObject(result_root,"ppp_mtu",ppp_mtu);

	huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);
	cJSON_AddNumberToObject(result_root,"wan_mtu",cc_mtu);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&link);

	cJSON_AddNumberToObject(result_root,"link",link);

	huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
	//cJSON_AddStringToObject(result_root,"proto",proto);

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	huayi_uci_get_str(PKG_NETWORK,"wan","hct_proto",hct_proto);
	if(!strncmp(hct_proto,"l2tp",strlen("l2tp")) || !strncmp(hct_proto,"pptp",strlen("pptp")))
	{
		cJSON_AddStringToObject(result_root,"proto",hct_proto);
		cJSON_AddStringToObject(result_root,"hct_proto",proto);
	}
	else
	{
		cJSON_AddStringToObject(result_root,"proto",proto);
		cJSON_AddStringToObject(result_root,"hct_proto","dhcp");
	}

	if(!strncmp(hct_proto,"l2tp",strlen("l2tp")))
	{
		huayi_uci_get_str(PKG_PPL2TP,"xl2tp","server_ip",vpn_service);
		huayi_uci_get_str(PKG_PPL2TP,"xl2tp","username",vpn_username);
		huayi_uci_get_str(PKG_PPL2TP,"xl2tp","password",vpn_password);
		huayi_uci_get_int(PKG_XL2TPD,"xl2tpd","enable",&xl2pd_enable);
		if(xl2pd_enable == 1)
		{
			cJSON_AddTrueToObject(result_root,"xl2tpd_enable");
		}
		else
		{
			cJSON_AddFalseToObject(result_root,"xl2tpd_enable");
		}
	}else if(!strncmp(hct_proto,"pptp",strlen("pptp")))
	{
		huayi_uci_get_str(PKG_PPL2TP,"pptp","server_ip",vpn_service);
		huayi_uci_get_str(PKG_PPL2TP,"pptp","username",vpn_username);
		huayi_uci_get_str(PKG_PPL2TP,"pptp","password",vpn_password);
	}

	cJSON_AddStringToObject(result_root,"vpnservice",vpn_service);
	cJSON_AddStringToObject(result_root,"vpnusr",vpn_username);
	cJSON_AddStringToObject(result_root,"vpnpwd",vpn_password);
#else
	cJSON_AddStringToObject(result_root,"proto",proto);
#endif
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int set_wan_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char proto[OPTION_STR_LEN] = {0};
	char macaddr[MAC_STR_LEN] = {0};
	int cc_mtu = 0;
	int ppp_mtu =0;
#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	char hct_proto[OPTION_STR_LEN] = {0};
	char vpn_username[PASSWD_LONG_STR_LEN] = {0};
	char vpn_password[PASSWD_LONG_STR_LEN] = {0};
	char vpn_service[IPV4_STR_LEN] = {0};
	int vpn_enable = 0;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
	char tmpStr[OPTION_STR_LEN]={0};
#endif
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"mac_addr",macaddr,MAC_STR_LEN)||
		!huayi_json_get_str(request_root,"proto",proto,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	if(!strcmp(proto,HUAYI_NET_PRORO_PPTP) || !strcmp(proto,HUAYI_NET_PROPO_XL2TP))
	{
		if(!huayi_json_get_str(request_root,"vpnproto",hct_proto,OPTION_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		if(!huayi_json_get_str(request_root,"vpnservice",vpn_service,OPTION_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		if(!huayi_json_get_str(request_root,"vpnusr",vpn_username,OPTION_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		if(!huayi_json_get_str(request_root,"vpnpwd",vpn_password,OPTION_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}

#endif
	if(!strcmp(proto,HUAYI_NET_PROTO_STATIC))
	{
		if(!huayi_json_get_int(request_root,"wan_mtu",&cc_mtu))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		if(!set_interface_static_config(request_root,"wan"))
		{
			huayi_uci_revert(PKG_NETWORK);
			return FCGI_RESP_CODE_DATA_MISS;
		}

		huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
		huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
	}else if(!strcmp(proto,HUAYI_NET_PROTO_PPPOE))
	{
		if(!huayi_json_get_int(request_root,"ppp_mtu",&ppp_mtu))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		if(!set_interface_pppoe_config(request_root,"wan"))
		{
			huayi_uci_revert(PKG_NETWORK);
			return FCGI_RESP_CODE_DATA_MISS;
		}

		huayi_uci_set_int(PKG_NETWORK,"wan","ppp_mtu",ppp_mtu);
		huayi_uci_set_int(PKG_NETWORK,"wan","mtu",ppp_mtu);
#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
		if(huayi_uci_get_str(PKG_NETWORK,"wan6","proto",tmpStr)){
			huayi_uci_set_str(PKG_NETWORK,"wan6","proto","dhcpv6");
			huayi_uci_set_str(PKG_NETWORK,"wan6","ifname","pppoe-wan");
		}
#endif

	}else if(!strcmp(proto,HUAYI_NET_PROTO_DHCP))
	{
		if(!huayi_json_get_int(request_root,"wan_mtu",&cc_mtu))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		if(!set_interface_dhcp_config(request_root,"wan"))
		{
			huayi_uci_revert(PKG_NETWORK);
			return FCGI_RESP_CODE_DATA_MISS;
		}
		huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
		huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
		if(huayi_uci_get_str(PKG_NETWORK,"wan6","proto",tmpStr)){
			huayi_uci_set_str(PKG_NETWORK,"wan6","proto","dhcpv6");
			huayi_uci_set_str(PKG_NETWORK,"wan6","ifname","eth1");
		}
#endif
	}
#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	else if(!strcmp(proto,HUAYI_NET_PRORO_PPTP))
	{
		if(!strcmp(hct_proto,HUAYI_NET_PROTO_STATIC))
		{
			if(!huayi_json_get_int(request_root,"wan_mtu",&cc_mtu))
			{
				return FCGI_RESP_CODE_DATA_MISS;
			}

			if(!set_interface_static_config(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return FCGI_RESP_CODE_DATA_MISS;
			}

			huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
		}else if(!strcmp(hct_proto,HUAYI_NET_PROTO_DHCP))
		{
			if(!huayi_json_get_int(request_root,"wan_mtu",&cc_mtu))
			{
				return FCGI_RESP_CODE_DATA_MISS;
			}

			if(!set_interface_dhcp_config(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
		}
		huayi_uci_set_int(PKG_PPL2TP,"pptp","enable",1);
		huayi_uci_set_str(PKG_PPL2TP,"pptp","server_ip",vpn_service);
		huayi_uci_set_str(PKG_PPL2TP,"pptp","username",vpn_username);
		huayi_uci_set_str(PKG_PPL2TP,"pptp","password",vpn_password);
		huayi_uci_set_int(PKG_PPL2TP,"pptp","route_type",1);
		huayi_uci_set_int(PKG_PPL2TP,"pptp","mppe",1);
		vpn_enable = 1;

	}else if(!strcmp(proto,HUAYI_NET_PROPO_XL2TP))
	{
		if(!strcmp(hct_proto,HUAYI_NET_PROTO_STATIC))
		{
			if(!huayi_json_get_int(request_root,"wan_mtu",&cc_mtu))
			{
				return FCGI_RESP_CODE_DATA_MISS;
			}

			if(!set_interface_static_config(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return FCGI_RESP_CODE_DATA_MISS;
			}

			huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
		}else if(!strcmp(hct_proto,HUAYI_NET_PROTO_DHCP))
		{
			if(!huayi_json_get_int(request_root,"wan_mtu",&cc_mtu))
			{
				return FCGI_RESP_CODE_DATA_MISS;
			}

			if(!set_interface_dhcp_config(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
		}
		huayi_uci_set_int(PKG_PPL2TP,"xl2tp","enable",1);
		huayi_uci_set_str(PKG_PPL2TP,"xl2tp","server_ip",vpn_service);
		huayi_uci_set_str(PKG_PPL2TP,"xl2tp","username",vpn_username);
		huayi_uci_set_str(PKG_PPL2TP,"xl2tp","password",vpn_password);
		huayi_uci_set_int(PKG_PPL2TP,"xl2tp","route_type",1);
		vpn_enable = 1;
	}
	if(vpn_enable == 0)
	{
		huayi_uci_set_int(PKG_PPL2TP,"pptp","enable",0);
		huayi_uci_set_int(PKG_PPL2TP,"xl2tp","enable",0);
		huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
		huayi_uci_set_str(PKG_NETWORK,"wan","hct_proto","dhcp");
	}
	else
	{
		huayi_uci_set_str(PKG_NETWORK,"wan","proto",hct_proto);
		huayi_uci_set_str(PKG_NETWORK,"wan","hct_proto",proto);
	}
	huayi_uci_commit(PKG_PPL2TP);
#else
	huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
#endif



	huayi_uci_set_str(PKG_NETWORK,"wan","macaddr",macaddr);

	huayi_uci_commit(PKG_NETWORK);

	system("huayi-reset network");

	*lock_time = HUAYI_RELOAD_NETWORK_TIME;

	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");

	return FCGI_RESP_CODE_SUCCESS;
}


int get_static_router_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *static_router_array = NULL;
	cJSON *static_router_obj = NULL;
	char section[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char gateway[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};
	char desc[OPTION_STR_LEN] = {0};
	int metric = 0;
	int i = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	static_router_array = cJSON_CreateArray();

	while(1)
	{
		snprintf(section,sizeof(section),"@route[%d]",i);
		if(!huayi_uci_get_str(PKG_NETWORK,section,"target",target))
		{
			break;
		}
		huayi_uci_get_str(PKG_NETWORK,section,"interface",inter_face);

		huayi_uci_get_str(PKG_NETWORK,section,"gateway",gateway);

		huayi_uci_get_str(PKG_NETWORK,section,"netmask",netmask);

		huayi_uci_get_int(PKG_NETWORK,section,"metric",&metric);

		huayi_uci_get_str(PKG_NETWORK,section,"desc",desc);

		static_router_obj = cJSON_CreateObject();

		cJSON_AddStringToObject(static_router_obj,"interface",inter_face);

		cJSON_AddStringToObject(static_router_obj,"target",target);

		cJSON_AddStringToObject(static_router_obj,"gateway",gateway);

		cJSON_AddStringToObject(static_router_obj,"subnet_mask",netmask);

		cJSON_AddNumberToObject(static_router_obj,"metric",metric);

		cJSON_AddStringToObject(static_router_obj,"com",desc);

		cJSON_AddItemToArray(static_router_array,static_router_obj);

		i++;
	}

	cJSON_AddItemToObject(result_root,"static_router_arr",static_router_array);

	get_interface_list(result_root);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int set_static_router_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*static_router_array = NULL;
	cJSON 	*static_router_obj = NULL;
	int 	size = 0;
	int 	i = 0;
	int		k = 0;
	int		max_id=0;
	char section[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char gateway[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};
	char desc[OPTION_STR_LEN] = {0};
	int metric = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}


	if(!(static_router_array= cJSON_GetObjectItem(request_root,"static_router_arr")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	size = cJSON_GetArraySize(static_router_array);

	snprintf(section,sizeof(section),"@route[-1]");
	huayi_uci_get_int(PKG_NETWORK,section,"id",&max_id);

	if(max_id == 0)
	{
		;/*no section*/
	}else
	{
		snprintf(section,sizeof(section),"@route[0]");
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_NETWORK,section);
		}
	}


	for(k=0;k<size;k++)
	{
		static_router_obj = cJSON_GetArrayItem(static_router_array,k);
		huayi_json_get_str(static_router_obj,"target",target,IPV4_STR_LEN);
		huayi_json_get_str(static_router_obj,"gateway",gateway,IPV4_STR_LEN);
		huayi_json_get_str(static_router_obj,"subnet_mask",netmask,IPV4_STR_LEN);
		huayi_json_get_str(static_router_obj,"interface",inter_face,OPTION_STR_LEN);
		huayi_json_get_int(static_router_obj,"metric",&metric);
		huayi_json_get_str(static_router_obj,"com",desc,OPTION_STR_LEN);
		huayi_uci_add_section(PKG_NETWORK,"route");

		snprintf(section,sizeof(section),"@route[%d]",k);

		huayi_uci_set_str(PKG_NETWORK,section,"target",target);

		huayi_uci_set_str(PKG_NETWORK,section,"gateway",gateway);
		huayi_uci_set_str(PKG_NETWORK,section,"netmask",netmask);

		huayi_uci_set_str(PKG_NETWORK,section,"interface",inter_face);

		huayi_uci_set_int(PKG_NETWORK,section,"metric",metric);

		huayi_uci_set_str(PKG_NETWORK,section,"desc",desc);

		huayi_uci_set_int(PKG_NETWORK,section,"id",k+1);

	}

	huayi_uci_commit(PKG_NETWORK);

	system("huayi-reset network");

	*lock_time = HUAYI_RELOAD_NETWORK_TIME;

	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");

	return FCGI_RESP_CODE_SUCCESS;
}
#endif

int get_lan_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*result_root = NULL;
	char	ipaddr[IPV4_STR_LEN] = {0};
	char    netmask[IPV4_STR_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","netmask",netmask))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	result_root = cJSON_CreateObject();

	cJSON_AddStringToObject(result_root,"ip_addr",ipaddr);

	cJSON_AddStringToObject(result_root,"subnet_mask",netmask);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}


int set_lan_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char	ipaddr[IPV4_STR_LEN] = {0};
	char    netmask[IPV4_STR_LEN] = {0};
	char 	IPADDR[IPV4_STR_LEN] = {0};
	char 	NETMASK[IPV4_STR_LEN] = {0};
	int		start 	= 0;
	int		limit	= 0;
	int 	ip	= 0;
	int		net = 0;
	int 	min	= 0;
	int		max = 0;
	int 	ip_range = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"ip_addr",ipaddr,IPV4_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"subnet_mask",netmask,IPV4_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",IPADDR))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","netmask",NETMASK))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(strcmp(IPADDR,ipaddr) || strcmp(NETMASK,netmask))
	{
		/*ip change need reload*/
		huayi_uci_set_str(PKG_NETWORK,"lan","ipaddr",ipaddr);
		huayi_uci_set_str(PKG_NETWORK,"lan","netmask",netmask);
		if(!strcmp(IPADDR,ipaddr))
		{
			cJSON_AddFalseToObject(response_root,"reconnection");
			cJSON_AddStringToObject(response_root,"reconnection_addr","");
		}else
		{
			cJSON_AddStringToObject(response_root,"reconnection_addr",ipaddr);
			cJSON_AddTrueToObject(response_root,"reconnection");
		}

		huayi_uci_get_int(PKG_DHCP,"lan","start",&start);

		huayi_uci_get_int(PKG_DHCP,"lan","limit",&limit);

		inet_pton(AF_INET,ipaddr, &ip);
		inet_pton(AF_INET,netmask, &net);
		ip = htonl(ip);
		net = htonl(net);
		min = (ip & net) + 1;
		max = (ip | ~net) - 1;
		ip_range = max - min + 1;

		if((ip_range < limit )||(ip_range < start ) || strcmp(NETMASK,netmask))
		{
			start	= HUAYI_DEFAULT_DHCP_START;
			limit	= ip_range;
			huayi_uci_set_int(PKG_DHCP,"lan","start",start);
			huayi_uci_set_int(PKG_DHCP,"lan","limit",limit);
			huayi_uci_commit(PKG_DHCP);
		}

		huayi_uci_commit(PKG_NETWORK);

		system("huayi-reset network");

#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
		session_logout("all");
#endif

		*lock_time = HUAYI_RELOAD_NETWORK_TIME;
	}else
	{
		/*no changes*/
		*lock_time = 0;
	}

	return FCGI_RESP_CODE_SUCCESS;
}

/*this api for bridge mode network config*/
int get_network_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	char ipaddr[IPV4_STR_LEN] = {0}, netmask[IPV4_STR_LEN] = {0},gateway[IPV4_STR_LEN] = {0};
	char hostname[OPTION_STR_LEN] = {0};
	char proto[OPTION_STR_LEN] = {0};
	char 	temp_dns[OPTION_STR_LEN] = {0};
	char 	dns[IPV4_STR_LEN] = {0};
	cJSON 	*dns_array = NULL;
	int  i = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","netmask",netmask))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","proto",proto))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_uci_get_str(PKG_NETWORK,"lan","gateway",gateway);

	huayi_uci_get_str(PKG_NETWORK,"lan","hostname",hostname);
	if(strlen(hostname) <= 0)
	{
		huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
	}

	huayi_uci_get_str(PKG_NETWORK,"lan","dns",temp_dns);

	result_root = cJSON_CreateObject();
	dns_array = cJSON_CreateArray();
	while(huayi_get_nth_value(i++, temp_dns, WEB_SPACE_DELIMIT, dns, sizeof(dns)))
	{
		cJSON_AddItemToArray(dns_array,cJSON_CreateString(dns));
	}

	cJSON_AddStringToObject(result_root,"proto",proto);
	cJSON_AddStringToObject(result_root,"ip_addr",ipaddr);
	cJSON_AddStringToObject(result_root,"subnet_mask",netmask);
	cJSON_AddStringToObject(result_root,"gateway",gateway);
	cJSON_AddStringToObject(result_root,"host_name",hostname);
	cJSON_AddItemToObject(result_root,"dns_arr",dns_array);


	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int set_network_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char proto[OPTION_STR_LEN] = {0};
	char ipaddr[OPTION_STR_LEN] = {0};
	int opmode = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"proto",proto,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!strcmp(proto,HUAYI_NET_PROTO_STATIC))
	{

		if(!set_interface_static_config(request_root,"lan"))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}else
	{
		if(!set_interface_dhcp_config(request_root,"lan"))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}

	huayi_uci_set_str(PKG_NETWORK,"lan","proto",proto);

	huayi_uci_commit(PKG_NETWORK);

	system("huayi-reset network");

#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
	session_logout("all");
#endif

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode);
	if(opmode == HUAYI_ROUTE_MODE)
	{
		huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr);
	}else
	{
		huayi_uci_get_str(PKG_ABOUT,"about_system","domain_name",ipaddr);
	}

	cJSON_AddTrueToObject(response_root,"reconnection");

	cJSON_AddStringToObject(response_root,"reconnection_addr",ipaddr);

	*lock_time = HUAYI_RELOAD_NETWORK_TIME;

	return FCGI_RESP_CODE_SUCCESS;
}

int get_dhcps_server_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*result_root = NULL;
	cJSON	*host_array  = NULL;
	char	ipaddr[IPV4_STR_LEN] = {0};
	char    netmask[IPV4_STR_LEN] = {0};
	int 	ignore 	= 0;
	char    minip[IPV4_STR_LEN]  = {0};
	char    maxip[IPV4_STR_LEN]  = {0};
	int 	ip 		= 0;
	int 	net 	= 0;
	int 	min		= 0;
	int 	max 	= 0;
	int     start 	= 0;
	int 	limit	= 0;
	int 	leasetime = 0;
	char    startip[IPV4_STR_LEN]  = {0};
	char    endip[IPV4_STR_LEN]  = {0};
	int		i = 0;
	cJSON	*host_obj = NULL;
	char	section[OPTION_STR_LEN] = {0};
	i = 0;
	char host_ip[IPV4_STR_LEN] = {0};
	char host_mac[MAC_STR_LEN] = {0};
	char host_name[OPTION_STR_LEN] = {0};
	int  host_lease = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_get_int(PKG_DHCP,"lan","ignore",&ignore);

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","netmask",netmask))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_DHCP,"lan","start",&start))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_DHCP,"lan","limit",&limit))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_DHCP,"lan","leasetime",&leasetime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	inet_pton(AF_INET,ipaddr, &ip);
	inet_pton(AF_INET,netmask, &net);
	ip = htonl(ip);
	net = htonl(net);

	min = htonl((ip & net) +1);
	max = htonl((ip | ~net) -1);


	inet_ntop(AF_INET, &min, minip, IPV4_STR_LEN);
	inet_ntop(AF_INET, &max, maxip, IPV4_STR_LEN);

	start = htonl(ntohl(min)+start-1);
	limit = htonl(htonl(start)+limit - 1);

	inet_ntop(AF_INET, &start, startip, IPV4_STR_LEN);

	inet_ntop(AF_INET, &limit, endip, IPV4_STR_LEN);

	result_root = cJSON_CreateObject();

	huayi_add_switch_to_json(result_root,"dhcp_enable",!ignore); /*dhcp switch ignore 1 off: igore 0 on*/

	cJSON_AddStringToObject(result_root,"min_ip",minip);

	cJSON_AddStringToObject(result_root,"max_ip",maxip);

	cJSON_AddStringToObject(result_root,"start_ip",startip);

	cJSON_AddStringToObject(result_root,"end_ip",endip);

	cJSON_AddNumberToObject(result_root,"lease_time",leasetime);

	host_array = cJSON_CreateArray();

	while(1)
	{
		memset(section,0,sizeof(section));
		memset(host_ip,0,sizeof(host_ip));
		memset(host_mac,0,sizeof(host_mac));
		memset(host_name,0,sizeof(host_name));
		host_lease = 0;
		snprintf(section,sizeof(section),"@host[%d]",i);
		if(!huayi_uci_get_str(PKG_DHCP,section,"ip",host_ip))
		{
			break;
		}
		huayi_uci_get_str(PKG_DHCP,section,"mac",host_mac);
		huayi_uci_get_str(PKG_DHCP,section,"name",host_name);
		huayi_uci_get_int(PKG_DHCP,section,"leasetime",&host_lease);

		host_obj = cJSON_CreateObject();

		cJSON_AddStringToObject(host_obj,"ip_addr",host_ip);

		cJSON_AddStringToObject(host_obj,"mac_addr",host_mac);

		cJSON_AddStringToObject(host_obj,"name",host_name);

		cJSON_AddNumberToObject(host_obj,"lease_time",host_lease);

		cJSON_AddItemToArray(host_array,host_obj);

		i++;
	}

	cJSON_AddItemToObject(result_root,"static_dhcp_arr",host_array);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}


int set_dhcps_server_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char	startip[IPV4_STR_LEN] = {0};
	char    endip[IPV4_STR_LEN] = {0};
	char	ipaddr[IPV4_STR_LEN] = {0};
	char    netmask[IPV4_STR_LEN] = {0};
	int 	enable  = 0;
	int		leasetime = 0;
	int 	ip 		= 0;
	int 	net 	= 0;
	int 	min		= 0;
	int     start 	= 0;
	int		end		= 0;
	int 	limit	= 0;
	int 	config_start	= 0;
	cJSON 	*static_dhcp_array = NULL;
	cJSON	*host_obj = NULL;
	char	section[OPTION_STR_LEN] = {0};
	char 	host_ip[IPV4_STR_LEN] = {0};
	char	host_mac[MAC_STR_LEN] = {0};
	char 	host_name[OPTION_STR_LEN] = {0};
	int  	host_lease = 0;
	int 	size = 0;
	int 	i = 0;
	int 	max_id 	= 0;
	int 	k = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"start_ip",startip,IPV4_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"end_ip",endip,IPV4_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"dhcp_enable",&enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"lease_time",&leasetime))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr);

	huayi_uci_get_str(PKG_NETWORK,"lan","netmask",netmask);

	inet_pton(AF_INET,ipaddr, &ip);
	inet_pton(AF_INET,netmask, &net);
	inet_pton(AF_INET,startip, &start);
	inet_pton(AF_INET,endip, &end);

	ip = htonl(ip);
	net = htonl(net);
	start = htonl(start);
	end = htonl(end);

	min = htonl(ip & net);

	if((min != htonl(start & net)) || (min != htonl(end & net)))
	{
		return FCGI_RESP_CODE_DATA_FORMAT_ERROR;
	}

	config_start = (start-ntohl(min));

	limit = (end-start) + 1;

	huayi_uci_set_int(PKG_DHCP,"lan","start",config_start);

	huayi_uci_set_int(PKG_DHCP,"lan","limit",limit);

	if(enable)
	{
		/*on set ignore 0*/
		huayi_uci_set_int(PKG_DHCP,"lan","ignore",0);
	}
	else
	{
		/*off set ignore 1*/
		huayi_uci_set_int(PKG_DHCP,"lan","ignore",1);
	}

	huayi_uci_set_int(PKG_DHCP,"lan","leasetime",leasetime);

	static_dhcp_array = cJSON_GetObjectItem(request_root, "static_dhcp_arr");

	size = cJSON_GetArraySize(static_dhcp_array);

	snprintf(section,sizeof(section),"@host[-1]");
	huayi_uci_get_int(PKG_DHCP,section,"id",&max_id);

	if(max_id == 0)
	{
		;/*no section*/
	}else
	{
		snprintf(section,sizeof(section),"@host[0]");
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_DHCP,section);
		}
	}

	for(k=0;k<size;k++)
	{
		memset(host_ip,0,IPV4_STR_LEN);
		memset(host_mac,0,MAC_STR_LEN);
		memset(host_name,0,OPTION_STR_LEN);
		host_obj = cJSON_GetArrayItem(static_dhcp_array,k);
		huayi_json_get_str(host_obj,"ip_addr",host_ip,IPV4_STR_LEN);
		huayi_json_get_str(host_obj,"mac_addr",host_mac,MAC_STR_LEN);
		huayi_json_get_str(host_obj,"name",host_name,OPTION_STR_LEN);
		huayi_json_get_int(host_obj,"lease_time",&host_lease);
		huayi_uci_add_section(PKG_DHCP,"host");
		snprintf(section,sizeof(section),"@host[%d]",k);
		huayi_uci_set_str(PKG_DHCP,section,"ip",host_ip);
		huayi_uci_set_str(PKG_DHCP,section,"mac",host_mac);
		huayi_uci_set_str(PKG_DHCP,section,"name",host_name);
		huayi_uci_set_int(PKG_DHCP,section,"leasetime",host_lease);
		huayi_uci_set_int(PKG_DHCP,section,"id",k+1);
		huayi_uci_set_int(PKG_DHCP,section,"dns",1);
	}

	huayi_uci_commit(PKG_DHCP);

	system("huayi-reset dhcps");

	return FCGI_RESP_CODE_SUCCESS;
}


int get_hwnat_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int hw_enabled = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_HWNAT,"global","enabled",&hw_enabled);

	huayi_add_switch_to_json(result_root,"nat_admin",hw_enabled);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int set_hwnat_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{

	int hw_enabled = 0, qos_enable = 0,pre_qos=0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"nat_admin",&hw_enabled))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_HWNAT,"global","enabled",hw_enabled);

	huayi_uci_commit(PKG_HWNAT);

	system("huayi-reset hwnat");

	if(1 == hw_enabled)
	{
		huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&qos_enable);
		huayi_uci_set_int(PKG_HWNAT,"global","pre_qos",qos_enable);
		huayi_uci_commit(PKG_HWNAT);
		if(1 == qos_enable)
		{
			huayi_uci_set_int(PKG_QOS,"qos","qos_mode",0);
			huayi_uci_commit(PKG_QOS);
			system("huayi-reset iplimit");
		}
	}else
	{
		huayi_uci_get_int(PKG_HWNAT,"global","pre_qos",&pre_qos);
		huayi_uci_set_int(PKG_QOS,"qos","qos_mode",pre_qos);
		huayi_uci_commit(PKG_QOS);
		system("huayi-reset iplimit");
	}

	return FCGI_RESP_CODE_SUCCESS;
}


int get_allstalist(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL, *stalist_array = NULL;
	int client_num = 0;

	result_root = cJSON_CreateObject();
	stalist_array = cJSON_CreateArray();

	get_active_arp(0,stalist_array,&client_num);

	cJSON_AddItemToObject(result_root,"staline_arr",stalist_array);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int get_system_opmode(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON	*result_root = NULL;
	int 	sys_mode = 0;
	char	ssid_24g[OPTION_STR_LEN] = {0},encryption_24g[OPTION_STR_LEN] = {0},key_24g[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	char authmode_24g[OPTION_STR_LEN] = {0};
	char cipher_24g[OPTION_STR_LEN] = {0};
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	char	ssid_58g[OPTION_STR_LEN] = {0},encryption_58g[OPTION_STR_LEN] = {0},key_58g[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	char authmode_58g[OPTION_STR_LEN] = {0};
	char cipher_58g[OPTION_STR_LEN] = {0};
#endif

#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	if(!huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	//if(!huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","repeater_state",&rep_status))
	//{
	//	return FCGI_RESP_CODE_SERVER_FAILURE;
	//}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_24g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"encryption",encryption_24g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"key",key_24g))
	{
		;
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_58g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"encryption",encryption_58g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"key",key_58g))
	{
		;
	}
#endif

	cJSON_AddNumberToObject(result_root,"sysmode",sys_mode);

	//cJSON_AddNumberToObject(result_root,"rep_status",rep_status);

	cJSON_AddStringToObject(result_root,"ssid_24g",ssid_24g);

	cJSON_AddStringToObject(result_root,"key_24g",key_24g);

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	if(strcmp(encryption_24g,"none") == 0)
	{
		strcpy(authmode_24g,"disable");
		strcpy(cipher_24g,"AES");
	}else
	{
		if(strstr(encryption_24g,"psk-mixed"))
		{
			strcpy(authmode_24g,"WPAPSKWPA2PSK");
		}else if(strstr(encryption_24g,"psk2"))
		{
			strcpy(authmode_24g,"WPA2PSK");
		}else
		{
			strcpy(authmode_24g,"WPAPSK");
		}

		if(strstr(encryption_24g,"ccmp") && strstr(encryption_24g,"tkip"))
		{
			strcpy(cipher_24g,"TKIPAES");
		}else if(strstr(encryption_24g,"tkip"))
		{
			strcpy(cipher_24g,"TKIP");
		}else if(strstr(encryption_24g,"ccmp"))
		{
			strcpy(cipher_24g,"AES");
		}else
		{
			strcpy(cipher_24g,"none");
		}
	}
	cJSON_AddStringToObject(result_root,"authmode_24g",authmode_24g);
	cJSON_AddStringToObject(result_root,"cipher_24g",cipher_24g);
#endif
	if (strncmp(encryption_24g,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(result_root,"encryption_24g",0);
	}
	else
	{
		cJSON_AddNumberToObject(result_root,"encryption_24g",1);
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	cJSON_AddStringToObject(result_root,"ssid_58g",ssid_58g);

	cJSON_AddStringToObject(result_root,"key_58g",key_58g);

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	if(strcmp(encryption_58g,"none") == 0)
	{
		strcpy(authmode_58g,"disable");
		strcpy(cipher_58g,"AES");
	}else
	{
		if(strstr(encryption_58g,"psk-mixed"))
		{
			strcpy(authmode_58g,"WPAPSKWPA2PSK");
		}else if(strstr(encryption_58g,"psk2"))
		{
			strcpy(authmode_58g,"WPA2PSK");
		}else
		{
			strcpy(authmode_58g,"WPAPSK");
		}

		if(strstr(encryption_58g,"ccmp") && strstr(encryption_58g,"tkip"))
		{
			strcpy(cipher_58g,"TKIPAES");
		}else if(strstr(encryption_58g,"tkip"))
		{
			strcpy(cipher_58g,"TKIP");
		}else if(strstr(encryption_58g,"ccmp"))
		{
			strcpy(cipher_58g,"AES");
		}else
		{
			strcpy(cipher_58g,"none");
		}
	}
	cJSON_AddStringToObject(result_root,"authmode_58g",authmode_58g);
	cJSON_AddStringToObject(result_root,"cipher_58g",cipher_58g);
#endif
	if (strncmp(encryption_58g,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(result_root,"encryption_58g",0);
	}
	else
	{
		cJSON_AddNumberToObject(result_root,"encryption_58g",1);
	}
#endif

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

FCGI_BOOL set_system_route_opmode(cJSON *request_root)
{
	char 	lan_ifname[OPTION_STR_LEN] = {0},wan_ifname[OPTION_STR_LEN] = {0};
	char	def_lan_ip[IPV4_STR_LEN] = {0};
	char	proto[OPTION_STR_LEN] = {0};
	int     cc_mtu = 0;
	int     ppp_mtu = 0;
	char	ssid_24g[OPTION_STR_LEN] = {0},key_24g[TEMP_STR_LEN] = {0};
	int 	encryption_24g = 0;
	int		first_boot = 1, old_sys_mode = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int		encryption_58g = 0;
	char	ssid_58g[OPTION_STR_LEN] = {0},key_58g[TEMP_STR_LEN] = {0};
	int		bandsteering = 0;
#endif

	if(!huayi_json_get_str(request_root,"ssid_24g",ssid_24g,OPTION_STR_LEN) || \
		!huayi_json_get_int(request_root,"encryption_24g",&encryption_24g))
	{
		first_boot = 0;
	}

	if(!huayi_json_get_str(request_root,"key_24g",key_24g,TEMP_STR_LEN))
	{
		if(1 == encryption_24g)
		{
			first_boot = 0;
		}
	}

	if (first_boot == 1)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_DEV,"channel","auto");
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_24g);

		if (encryption_24g == 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,W24G_IFACE,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","psk-mixed+ccmp");
			huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"key",key_24g);
		}
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_str(request_root,"ssid_58g",ssid_58g,OPTION_STR_LEN) || \
		!huayi_json_get_int(request_root,"encryption_58g",&encryption_58g))
	{
		first_boot = 0;
	}

	if(!huayi_json_get_str(request_root,"key_58g",key_58g,TEMP_STR_LEN))
	{
		if(1 == encryption_58g)
		{
			first_boot = 0;
		}
	}

	if (first_boot == 1)
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_DEV,"channel","auto");
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_58g);

		if (encryption_58g == 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,W58G_IFACE,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","psk-mixed+ccmp");
			huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"key",key_58g);
		}

		if (encryption_24g == encryption_58g)
		{
			if (strcmp(ssid_24g, ssid_58g) == 0 && (encryption_24g == 0 || strcmp(key_24g, key_58g) == 0))
			{
				bandsteering = 1;
			}
		}

		huayi_uci_set_int(PKG_WIRELESS,"radio0","bandsteering",bandsteering);
		huayi_uci_set_int(PKG_WIRELESS,"radio1","bandsteering",bandsteering);
	}
#endif

	if(!huayi_json_get_str(request_root,"proto",proto,OPTION_STR_LEN))
	{
		return FCGI_FALSE;
	}

	if(!strcmp(proto,HUAYI_NET_PROTO_STATIC))
	{
		if(!set_interface_static_config(request_root,"wan"))
		{
			huayi_uci_revert(PKG_NETWORK);
			return FCGI_FALSE;
		}
		huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);
		huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
	}
	else if(!strcmp(proto,HUAYI_NET_PROTO_PPPOE))
	{
		huayi_uci_delete_option(PKG_NETWORK,"wan","gateway");
		if(!set_interface_pppoe_config(request_root,"wan"))
		{
			huayi_uci_revert(PKG_NETWORK);
			return FCGI_FALSE;
		}
		huayi_uci_get_int(PKG_NETWORK,"wan","ppp_mtu",&ppp_mtu);
		huayi_uci_set_int(PKG_NETWORK,"wan","mtu",ppp_mtu);
	}
	else if(!strcmp(proto,HUAYI_NET_PROTO_DHCP))
	{
		huayi_uci_delete_option(PKG_NETWORK,"wan","gateway");
		if(!set_interface_dhcp_config(request_root,"wan"))
		{
			huayi_uci_revert(PKG_NETWORK);
			return FCGI_FALSE;
		}
		huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);
		huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
	}
	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&old_sys_mode);
	if(old_sys_mode != HUAYI_ROUTE_MODE)   //AP\REPEATER   ---> ROUTE MODE
	{
		huayi_uci_get_str(PKG_NETWORK,"lan","def_ip",def_lan_ip);
		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);


		huayi_uci_set_str(PKG_NETWORK,"lan","ipaddr",def_lan_ip);
		huayi_uci_set_str(PKG_NETWORK,"lan","netmask",HUAYI_DEFAULT_NET_MASK);
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"lan","proto","static");

	}
	huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);
	huayi_uci_set_str(PKG_DHCP,"lan","ignore","0");
	huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
	huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
	/*keep original ssid and passphrase*/
	//set_wireless_main_def();

	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"network","lan");
	huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","1");
	huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_enable","1");
	huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"network","lan");
	huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled","1");
	huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable","1");
	huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
#endif
#if defined(CONFIG_PACKAGE_HWNAT)
	huayi_uci_set_int(PKG_HWNAT,"global","enabled",1);
	huayi_uci_commit(PKG_HWNAT);
#endif
	return FCGI_TRUE;
}

FCGI_BOOL set_system_ap_opmode(cJSON *request_root)
{
	int 	orig_opmode = 0;
	char	wan_lan_ifname[OPTION_STR_LEN] = {0};
	char	ssid_24g[OPTION_STR_LEN] = {0},key_24g[TEMP_STR_LEN] = {0};
	int 	encryption_24g = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int		encryption_58g = 0;
	char	ssid_58g[OPTION_STR_LEN] = {0},key_58g[TEMP_STR_LEN] = {0};
	int		bandsteering = 0;
#endif

	if(!huayi_json_get_str(request_root,"ssid_24g",ssid_24g,OPTION_STR_LEN) || \
		!huayi_json_get_int(request_root,"encryption_24g",&encryption_24g))
	{
		return FCGI_FALSE;
	}

	if(!huayi_json_get_str(request_root,"key_24g",key_24g,TEMP_STR_LEN))
	{
		if(1 == encryption_24g)
		{
			return FCGI_FALSE;
		}
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_str(request_root,"ssid_58g",ssid_58g,OPTION_STR_LEN) || \
		!huayi_json_get_int(request_root,"encryption_58g",&encryption_58g))
	{
		return FCGI_FALSE;
	}

	if(!huayi_json_get_str(request_root,"key_58g",key_58g,TEMP_STR_LEN))
	{
		if(1 == encryption_58g)
		{
			return FCGI_FALSE;
		}
	}
#endif

	if(HUAYI_ROUTE_MODE == orig_opmode||HUAYI_WISP_MODE == orig_opmode)
	{
		huayi_uci_delete_option(PKG_NETWORK,"wan","ifname");
	}

	huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_lan_ifname",wan_lan_ifname);

	huayi_uci_set_str(PKG_NETWORK,"lan","proto","dhcp");

	huayi_uci_set_str(PKG_NETWORK,"lan","ifname",wan_lan_ifname);

	huayi_uci_set_str(PKG_DHCP,"lan","ignore","1");

	huayi_uci_set_str(PKG_WIRELESS,W24G_DEV,"channel","auto");
	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_24g);

	if (encryption_24g == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W24G_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","psk-mixed+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"key",key_24g);
	}

	huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","1");
	huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_enable","1");
	huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_set_str(PKG_WIRELESS,W58G_DEV,"channel","auto");
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_58g);

	if (encryption_58g == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W58G_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","psk-mixed+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"key",key_58g);
	}

	huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled","1");
	huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable","1");
	huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
	if (encryption_24g == encryption_58g)
	{
		if (strcmp(ssid_24g, ssid_58g) == 0 && (encryption_24g == 0 || strcmp(key_24g, key_58g) == 0))
		{
			bandsteering = 1;
		}
	}

	huayi_uci_set_int(PKG_WIRELESS,"radio0","bandsteering",bandsteering);
	huayi_uci_set_int(PKG_WIRELESS,"radio1","bandsteering",bandsteering);
#endif

#if defined(CONFIG_PACKAGE_HWNAT)
	huayi_uci_set_int(PKG_HWNAT,"global","enabled",0);
	huayi_uci_commit(PKG_HWNAT);
#endif

	return FCGI_TRUE;
}


FCGI_BOOL set_system_repeater_opmode(cJSON *request_root)
{
	int 	orig_opmode = 0;
	int 	channel  = 0, ht_extcha = 1;
	char	authmode_uci[OPTION_STR_LEN] = {0},encryption_uci[OPTION_STR_LEN] = {0};
	char	wan_lan_ifname[OPTION_STR_LEN] = {0};
	char	bssid[OPTION_STR_LEN] = {0},ssid[OPTION_STR_LEN] = {0};
	char	authmode[OPTION_STR_LEN] = {0},encryption[OPTION_STR_LEN] = {0},authmode_encryptype[OPTION_STR_LEN] = {0},key[TEMP_STR_LEN] = {0};
	char 	ssid_24g_ext[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	char	ssid_58g_ext[TEMP_STR_LEN] = {0};
#endif
	char	apcli_ifname[OPTION_STR_LEN] = {0};
	char	resultbuf[OPTION_STR_LEN] = {0};
	char	get_cmd[OPTION_STR_LEN] = {0};


	if(!huayi_json_get_int(request_root,"channel",&channel) || \
		!huayi_json_get_str(request_root,"bssid",bssid,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"ssid",ssid,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"rep_ssid_24g",ssid_24g_ext,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"auth_mode",authmode,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"encryption",encryption,OPTION_STR_LEN) || \
		!huayi_json_get_int(request_root,"ht_extcha",&ht_extcha))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_str(request_root,"rep_ssid_58g",ssid_58g_ext,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
#endif
	huayi_json_get_str(request_root,"key",key,TEMP_STR_LEN);

	huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_lan_ifname",wan_lan_ifname);
	huayi_uci_set_str(PKG_DHCP,"lan","ignore","1");
	//huayi_uci_set_str(PKG_DHCP,"lan1","ignore","0");
	huayi_uci_set_str(PKG_NETWORK,"lan","proto","dhcp");

	if(HUAYI_ROUTE_MODE == orig_opmode||HUAYI_WISP_MODE == orig_opmode)
	{
		huayi_uci_delete_option(PKG_NETWORK,"wan","ifname");
	}

	if ((strncmp(authmode,"NONE",strlen("NONE ")) == 0) || (strncmp(authmode,"OPEN",strlen("OPEN ")) == 0))
	{
		strcpy(authmode_uci,"OPEN");
		strcpy(encryption_uci,"NONE");
		strcpy(authmode_encryptype,"none");
	}
	else
	{
		strcpy(authmode_uci,get_wireless_config_authmode(authmode));
		strcpy(encryption_uci,get_wireless_config_encryptype(encryption));
		strcpy(authmode_encryptype,"psk-mixed+ccmp");
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if (channel > 14)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled","0");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"network","lan");
		strcat(wan_lan_ifname, " "W58G_REP_IFNAME);

		huayi_uci_set_int(PKG_WIRELESS,W58G_DEV,"channel",channel);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_bssid",bssid);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",ssid);

		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_authmode",authmode_uci);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_encryptype",encryption_uci);

		if(strcmp("NONE",encryption_uci) == 0)
		{
			//huayi_uci_delete_option(PKG_WIRELESS,W58G_REP_IFACE,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_wpapsk",key);
		}
		huayi_uci_get_str(PKG_WIRELESS,W58G_REP_IFACE,"ifname",apcli_ifname);
	}
	else
#endif
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","0");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"network","lan");
		strcat(wan_lan_ifname, " "W24G_REP_IFNAME);

		huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"channel",channel);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_bssid",bssid);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_ssid",ssid);

		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_authmode",authmode_uci);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_encryptype",encryption_uci);

		if(strcmp("NONE",encryption_uci) == 0)
		{
			//huayi_uci_delete_option(PKG_WIRELESS,W24G_REP_IFACE,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_wpapsk",key);
		}
		huayi_uci_get_str(PKG_WIRELESS,W24G_REP_IFACE,"ifname",apcli_ifname);
	}

	huayi_uci_set_str(PKG_NETWORK,"lan","ifname",wan_lan_ifname);

	huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"ht_extcha",ht_extcha);
	//huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"network","x");
	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_24g_ext);
	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption", authmode_encryptype);
	if(strcmp("none",authmode_encryptype) == 0)
	{
		//huayi_uci_delete_option(PKG_WIRELESS,W24G_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"key",key);
	}

	sprintf(get_cmd,"cat /sys/class/net/%s/address | tr '[a-z]' '[A-Z]'",apcli_ifname);
	get_cmd_result(get_cmd,resultbuf,sizeof(resultbuf));
	if(strlen(resultbuf))
	{
		huayi_uci_set_str(PKG_NETWORK,"lan","macaddr",resultbuf);
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	//huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"network","x");
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_58g_ext);
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption", authmode_encryptype);
	if(strcmp("none",authmode_encryptype) == 0)
	{
		//huayi_uci_delete_option(PKG_WIRELESS,W58G_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"key",key);
	}
#endif

#if defined(CONFIG_PACKAGE_HWNAT)
	huayi_uci_set_int(PKG_HWNAT,"global","enabled",0);
	huayi_uci_commit(PKG_HWNAT);
#endif
	return FCGI_TRUE;
}

FCGI_BOOL set_system_wisp_opmode(cJSON *request_root)
{
	int 	channel  = 0, ht_extcha = 1,old_sys_mode = 0;
	char	authmode_uci[OPTION_STR_LEN] = {0},encryption_uci[OPTION_STR_LEN] = {0};
	char	wan_ifname[OPTION_STR_LEN] = {0};
	char	wan_lan_ifname[OPTION_STR_LEN] = {0};
	char	bssid[OPTION_STR_LEN] = {0},ssid[OPTION_STR_LEN] = {0};
	char	authmode[OPTION_STR_LEN] = {0},encryption[OPTION_STR_LEN] = {0},key[TEMP_STR_LEN] = {0};
	char	def_lan_ip[IPV4_STR_LEN] = {0};
	char 	apcli_ifname[OPTION_STR_LEN] = {0};
	char    resultbuf[OPTION_STR_LEN] = {0};
	char    get_cmd[OPTION_STR_LEN] = {0};


	if(!huayi_json_get_int(request_root,"channel",&channel) || \
		!huayi_json_get_str(request_root,"bssid",bssid,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"ssid",ssid,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"auth_mode",authmode,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"encryption",encryption,OPTION_STR_LEN) || \
		!huayi_json_get_int(request_root,"ht_extcha",&ht_extcha))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_json_get_str(request_root,"key",key,TEMP_STR_LEN);

	if ((strncmp(authmode,"NONE",strlen("NONE ")) == 0) || (strncmp(authmode,"OPEN",strlen("OPEN ")) == 0))
	{
		strcpy(authmode_uci,"OPEN");
		strcpy(encryption_uci,"NONE");
	}
	else
	{
		strcpy(authmode_uci,get_wireless_config_authmode(authmode));
		strcpy(encryption_uci,get_wireless_config_encryptype(encryption));
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if (channel > 14)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFNAME,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFNAME,"disabled","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFNAME,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled","0");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"network","wan");
		strcpy(wan_ifname, W58G_REP_IFNAME);

		huayi_uci_set_int(PKG_WIRELESS,W58G_DEV,"channel",channel);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_bssid",bssid);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",ssid);

		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_authmode",authmode_uci);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_encryptype",encryption_uci);

		if(strcmp("NONE",encryption_uci) == 0)
		{
			//huayi_uci_delete_option(PKG_WIRELESS,W58G_REP_IFACE,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_wpapsk",key);
		}
		huayi_uci_get_str(PKG_WIRELESS,W58G_REP_IFACE,"ifname",apcli_ifname);
	}
	else
#endif
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","0");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"network","wan");
		strcpy(wan_ifname, W24G_REP_IFNAME);

		huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"channel",channel);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_bssid",bssid);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_ssid",ssid);

		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_authmode",authmode_uci);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_encryptype",encryption_uci);

		if(strcmp("NONE",encryption_uci) == 0)
		{
			//huayi_uci_delete_option(PKG_WIRELESS,W24G_REP_IFACE,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_wpapsk",key);
		}
		huayi_uci_get_str(PKG_WIRELESS,W24G_REP_IFACE,"ifname",apcli_ifname);
	}

	huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"ht_extcha",ht_extcha);
	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&old_sys_mode);
	if(old_sys_mode != HUAYI_ROUTE_MODE)   //AP\REPEATER   ---> wisp MODE
	{
		huayi_uci_get_str(PKG_NETWORK,"lan","def_ip",def_lan_ip);
		huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_lan_ifname",wan_lan_ifname);

		huayi_uci_set_str(PKG_NETWORK,"lan","ipaddr",def_lan_ip);
		huayi_uci_set_str(PKG_NETWORK,"lan","netmask",HUAYI_DEFAULT_NET_MASK);
		huayi_uci_set_str(PKG_NETWORK,"lan","proto","static");
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",wan_lan_ifname);
	}

	huayi_uci_set_str(PKG_DHCP,"lan","ignore","0");

	huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);

	huayi_uci_set_str(PKG_NETWORK,"wan","proto","dhcp");
	huayi_uci_delete_option(PKG_NETWORK,"wan","gateway");
	huayi_uci_delete_option(PKG_NETWORK,"wan","dns");

	/*keep original ssid and passphrase*/
	//huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"def_ssid",ssid_def);
	//huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_def);
	//huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","none");
	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"network","lan");

	sprintf(get_cmd,"cat /sys/class/net/%s/address | tr '[a-z]' '[A-Z]'",apcli_ifname);
	get_cmd_result(get_cmd,resultbuf,sizeof(resultbuf));
	if(strlen(resultbuf))
	{
		huayi_uci_set_str(PKG_NETWORK,"wan","macaddr",resultbuf);
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	//huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"def_ssid",ssid_def);

	//huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_def);
	//huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","none");
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"network","lan");
#endif
#if defined(CONFIG_PACKAGE_HWNAT)
	huayi_uci_set_int(PKG_HWNAT,"global","enabled",1);
	huayi_uci_commit(PKG_HWNAT);
#endif

	return FCGI_TRUE;
}


int set_system_opmode(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int		opmode = 0, old_sys_mode = 0;
	char	reconnection_addr[OPTION_STR_LEN] = {0};


	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"sysmode",&opmode))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&old_sys_mode);

	if(HUAYI_ROUTE_MODE == opmode)
	{
		if(!set_system_route_opmode(request_root))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		if(old_sys_mode != HUAYI_ROUTE_MODE)   //AP\REPEATER   ---> ROUTE MODE
		{
			huayi_uci_get_str(PKG_NETWORK,"lan","def_ip",reconnection_addr);
		}
		else
		{
			huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",reconnection_addr);
		}
		*lock_time = HUAYI_SYSTEM_ROUTE_MODE_TIME;
	}
	else if(HUAYI_AP_MODE == opmode)
	{
		if(!set_system_ap_opmode(request_root))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		huayi_uci_get_str(PKG_ABOUT,"about_system","domain_name",reconnection_addr);
		*lock_time = HUAYI_SYSTEM_CHANGE_MODE_TIME;
	}
	else if(HUAYI_REP_MODE == opmode)
	{
		if(!set_system_repeater_opmode(request_root))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		huayi_uci_get_str(PKG_ABOUT,"about_system","domain_name",reconnection_addr);
		*lock_time = HUAYI_SYSTEM_REP_MODE_TIME;
	}
	else if(HUAYI_WISP_MODE == opmode)
	{
		if(!set_system_wisp_opmode(request_root))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		huayi_uci_get_str(PKG_NETWORK,"lan","def_ip",reconnection_addr);
		*lock_time = HUAYI_SYSTEM_CHANGE_MODE_TIME;
	}

	huayi_uci_set_int(PKG_SYSTEM,"mode","opmode",opmode);
	huayi_uci_set_int(PKG_SYSTEM,"global","first_boot",0);
	huayi_uci_commit(PKG_SYSTEM);
	huayi_uci_commit(PKG_NETWORK);
	huayi_uci_commit(PKG_DHCP);
	huayi_uci_commit(PKG_WIRELESS);

	system("huayi-reset reboot");

	cJSON_AddTrueToObject(response_root,"reconnection");

	cJSON_AddStringToObject(response_root,"reconnection_addr",reconnection_addr);

	return FCGI_RESP_CODE_SUCCESS;
}


int get_gateway_mode_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int link = 0;
	int wan_proto = 0;
	char 	proto[OPTION_STR_LEN] = {0};
	cJSON *ssid_obj = NULL;
	char hostname[OPTION_STR_LEN] = {0};
	char ssid_24g[OPTION_STR_LEN] = {0},encryption_24g[OPTION_STR_LEN] = {0},key_24g[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	char ssid_58g[OPTION_STR_LEN] = {0},encryption_58g[OPTION_STR_LEN] = {0},key_58g[TEMP_STR_LEN] = {0};
#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	ssid_obj=cJSON_CreateObject();

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_24g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"encryption",encryption_24g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"key",key_24g))
	{
		;
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_58g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"encryption",encryption_58g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"key",key_58g))
	{
		;
	}
#endif

	cJSON_AddStringToObject(ssid_obj,"ssid_24g",ssid_24g);

	cJSON_AddStringToObject(ssid_obj,"key_24g",key_24g);

	if (strncmp(encryption_24g,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(ssid_obj,"encryption_24g",0);
	}
	else
	{
		cJSON_AddNumberToObject(ssid_obj,"encryption_24g",1);
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	cJSON_AddStringToObject(ssid_obj,"ssid_58g",ssid_58g);

	cJSON_AddStringToObject(ssid_obj,"key_58g",key_58g);

	if (strncmp(encryption_58g,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(ssid_obj,"encryption_58g",0);
	}
	else
	{
		cJSON_AddNumberToObject(ssid_obj,"encryption_58g",1);
	}
#endif

	cJSON_AddItemToObject(result_root,"ssid_info",ssid_obj);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&link);

	cJSON_AddNumberToObject(result_root,"link",link);

	//huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","wan_proto",&wan_proto);
	/*not use proto_detect, from network config get*/
	huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
	if(strcmp(proto, "dhcp") == 0)
	{
		wan_proto = 2;
	}
	else if(strcmp(proto, "pppoe") == 0)
	{
		wan_proto = 1;
	}
	else
	{
		wan_proto = 0;
	}
	cJSON_AddNumberToObject(result_root,"wan_proto",wan_proto);

	huayi_uci_get_str(PKG_NETWORK,"wan","hostname",hostname);
	if(strlen(hostname) <= 0)
	{
		huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
	}
	cJSON_AddStringToObject(result_root,"host_name",hostname);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int do_wan_proto_detect(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int wan_proto = 0;
	int link = 0;
	int speed = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	system("killall huayi_proto_detect");

	system("/usr/bin/huayi_proto_detect > /dev/null");

	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","wan_proto",&wan_proto);

	cJSON_AddNumberToObject(result_root,"wan_proto_mode",wan_proto);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&link);

	cJSON_AddNumberToObject(result_root,"link_status",link);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","speed",&speed);

	cJSON_AddNumberToObject(result_root,"speed",speed);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}


int do_get_quick_setting_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int link = 0;
	int speed = 0;
	int wan_proto = 0;
	int first_boot = 0;
	char proto[OPTION_STR_LEN] = {0};
	int sys_mode = 0;
	char detectip[OPTION_STR_LEN] = {0};
	int detect_time = 0;
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	char wan_type[OPTION_STR_LEN] = {0};
	char auth_type[OPTION_STR_LEN] = {0};
	char apn_name[OPTION_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	int  use_apn = 0;
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G) || defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	cJSON *ssid_obj = NULL;
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	char ssid_2g[OPTION_STR_LEN] = {0};
	char encryption_2g[OPTION_STR_LEN] = {0};
	char passphrase_2g[TEMP_STR_LEN] = {0};
	int encry_2g = 0;
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	char ssid_5g[OPTION_STR_LEN] = {0};
	char encryption_5g[OPTION_STR_LEN] = {0};
	char passphrase_5g[TEMP_STR_LEN] = {0};
	int encry_5g = 0;
#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	cJSON_AddStringToObject(result_root,"wan_type",wan_type);

	if(!strcmp(wan_type,"mobile"))
	{
		huayi_uci_get_str(PKG_NETWORK,"wan4g","proto",proto);
		huayi_uci_get_int(PKG_MODEM, "status","mode", &use_apn);
		if(1 == use_apn)
		{
			huayi_uci_get_str(PKG_MODEM,"status","validation",auth_type);
			huayi_uci_get_str(PKG_MODEM,"status","apn",apn_name);
			huayi_uci_get_str(PKG_MODEM,"status","user4g",username);
			huayi_uci_get_str(PKG_MODEM,"status","pass4g",password);

			cJSON_AddStringToObject(result_root,"auth_type",auth_type);
			cJSON_AddStringToObject(result_root,"apn_name",apn_name);
			cJSON_AddStringToObject(result_root,"apn_usr",username);
			cJSON_AddStringToObject(result_root,"apn_pwd",password);
		}
		cJSON_AddNumberToObject(result_root,"use_apn",use_apn);
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_WIRED_PRIOROTY))
	{
		huayi_uci_get_str(PKG_NETWORK,"wan4g","proto",proto);
		cJSON_AddStringToObject(result_root,"proto_4g",proto);

		huayi_uci_get_int(PKG_MODEM, "status","mode", &use_apn);
		if(1 == use_apn)
		{
			huayi_uci_get_str(PKG_MODEM,"status","validation",auth_type);
			huayi_uci_get_str(PKG_MODEM,"status","apn",apn_name);
			huayi_uci_get_str(PKG_MODEM,"status","user4g",username);
			huayi_uci_get_str(PKG_MODEM,"status","pass4g",password);

			cJSON_AddStringToObject(result_root,"auth_type",auth_type);
			cJSON_AddStringToObject(result_root,"apn_name",apn_name);
			cJSON_AddStringToObject(result_root,"apn_usr",username);
			cJSON_AddStringToObject(result_root,"apn_pwd",password);
		}
		cJSON_AddNumberToObject(result_root,"use_apn",use_apn);

		memset(proto, 0, sizeof(proto));
		huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
		huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
		cJSON_AddStringToObject(result_root,"detect_ip",detectip);
	}
#elif defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_type, WAN_TYPE_REP4G_PRIOROTY))
	{
		huayi_uci_get_str(PKG_NETWORK,"wan4g","proto",proto);
		cJSON_AddStringToObject(result_root,"proto_4g",proto);

		huayi_uci_get_int(PKG_MODEM, "status","mode", &use_apn);
		if(1 == use_apn)
		{
			huayi_uci_get_str(PKG_MODEM,"status","validation",auth_type);
			huayi_uci_get_str(PKG_MODEM,"status","apn",apn_name);
			huayi_uci_get_str(PKG_MODEM,"status","user4g",username);
			huayi_uci_get_str(PKG_MODEM,"status","pass4g",password);

			cJSON_AddStringToObject(result_root,"auth_type",auth_type);
			cJSON_AddStringToObject(result_root,"apn_name",apn_name);
			cJSON_AddStringToObject(result_root,"apn_usr",username);
			cJSON_AddStringToObject(result_root,"apn_pwd",password);
		}
		cJSON_AddNumberToObject(result_root,"use_apn",use_apn);

		memset(proto, 0, sizeof(proto));
		huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
		huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
		cJSON_AddStringToObject(result_root,"detect_ip",detectip);
		huayi_uci_get_int(PKG_SYSTEM,"mode","detect_time", &detect_time);
		cJSON_AddNumberToObject(result_root,"detect_time",detect_time);
	}
#endif
	else
#endif
	{
		huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
	}

	cJSON_AddStringToObject(result_root,"proto",proto);

	/*Wireless  configuration*/
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G) || defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	ssid_obj=cJSON_CreateObject();

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	if(!huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid",ssid_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	if(!huayi_uci_get_str(PKG_WIRELESS,"radio0_0","encryption",encryption_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key",passphrase_2g);

	if (strcmp(encryption_2g,"none") == 0)
	{
		encry_2g = 0;
	}
	else
	{
		encry_2g = 1;
	}
	cJSON_AddStringToObject(ssid_obj,"ssid_2g",ssid_2g);
	cJSON_AddNumberToObject(ssid_obj,"encry_2g",encry_2g);
	cJSON_AddStringToObject(ssid_obj,"passphrase_2g",passphrase_2g);

#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_uci_get_str(PKG_WIRELESS,"radio1_0","ssid",ssid_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	if(!huayi_uci_get_str(PKG_WIRELESS,"radio1_0","encryption",encryption_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_uci_get_str(PKG_WIRELESS,"radio1_0","key",passphrase_5g);

	if (strcmp(encryption_5g,"none") == 0)
	{
		encry_5g = 0;
	}
	else
	{
		encry_5g = 1;
	}
	cJSON_AddStringToObject(ssid_obj,"ssid_5g",ssid_5g);
	cJSON_AddNumberToObject(ssid_obj,"encry_5g",encry_5g);
	cJSON_AddStringToObject(ssid_obj,"passphrase_5g",passphrase_5g);
#endif
	cJSON_AddItemToObject(result_root,"ssid_config",ssid_obj);
#endif

	/*Wire network configuration*/
	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);

	cJSON_AddNumberToObject(result_root,"sysmode",sys_mode);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&link);

	cJSON_AddNumberToObject(result_root,"link",link);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","speed",&speed);

	cJSON_AddNumberToObject(result_root,"speed",speed);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","wan_proto",&wan_proto);

	cJSON_AddNumberToObject(result_root,"wan_proto",wan_proto);

	get_multiwan_static_config_by_index(result_root,"wan");

	get_multiwan_pppoe_config_by_index(result_root,"wan");

	get_multiwan_dhcp_config_by_index(result_root,"wan");

	huayi_uci_get_int(PKG_SYSTEM,"global","first_boot",&first_boot);

	cJSON_AddNumberToObject(result_root,"first_boot",first_boot);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int do_set_quick_setting_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char 	proto[OPTION_STR_LEN] = {0};
	int 	ppp_mtu = 0;
	int		cc_mtu = 0;
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	char wan_ifname[OPTION_STR_LEN] = {0};
	char lan_ifname[OPTION_STR_LEN] = {0};
	char wan_type[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};
	int  use_apn = 0;
	int  auth_type = 0;
	char apn_name[OPTION_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	char ssid_2g[OPTION_STR_LEN] = {0};
	char passphrase_2g[TEMP_STR_LEN] = {0};
	int  encry_2g = 0;
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	char ssid_5g[OPTION_STR_LEN] = {0};
	char passphrase_5g[TEMP_STR_LEN] = {0};
	int  encry_5g = 0;
#endif
	char detectip[OPTION_STR_LEN] = {0};
	int ret = 0;
	char pmodel[16]={0};
	int  detect_time = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	/*set wireless network config*/
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	if(!huayi_json_get_str(request_root,"ssid_2g",ssid_2g,OPTION_STR_LEN)||
		!huayi_json_get_int(request_root,"encry_2g",&encry_2g)||
		!huayi_json_get_str(request_root,"passphrase_2g",passphrase_2g,TEMP_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_str(PKG_WIRELESS,"radio0_0","ssid",ssid_2g);

	if(encry_2g == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio0_0","encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,"radio0_0","key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio0_0","encryption","psk-mixed+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,"radio0_0","key",passphrase_2g);
	}

	huayi_uci_commit(PKG_WIRELESS);
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_str(request_root,"ssid_5g",ssid_5g,OPTION_STR_LEN)||
		!huayi_json_get_int(request_root,"encry_5g",&encry_5g)||
		!huayi_json_get_str(request_root,"passphrase_5g",passphrase_5g,TEMP_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_str(PKG_WIRELESS,"radio1_0","ssid",ssid_5g);

	if(encry_5g == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio1_0","encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,"radio1_0","key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio1_0","encryption","psk-mixed+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,"radio1_0","key",passphrase_5g);
	}

	huayi_uci_commit(PKG_WIRELESS);
#endif
#if !defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	if(!huayi_json_get_str(request_root,"proto",proto,OPTION_STR_LEN))
	{
		//return FCGI_RESP_CODE_DATA_MISS;
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	if(!huayi_json_get_str(request_root,"wan_type",wan_type,OPTION_STR_LEN))
	{
		//return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_uci_set_str(PKG_SYSTEM,"mode","wan_type",wan_type);

	if (strcmp(wan_type ,WAN_TYPE_4G_MOBILE) == 0)
	{
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
		if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
				&& strncmp("EC200",pmodel, 5) != 0)
		{
			huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s","wwan0");
		}
		else
		{
			huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s","eth1");
		}
		huayi_uci_delete_option(PKG_NETWORK,"wan","ifname");
		huayi_uci_set_str(PKG_NETWORK,"wan4g","proto", "dhcp");
		system(cmd);

		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_4g_ifname",lan_ifname);
		if(strlen(lan_ifname)>0)
		{
			huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
		}

		if(!huayi_json_get_int(request_root,"use_apn",&use_apn))
		{
			//return FCGI_RESP_CODE_DATA_MISS;
		}

		if(1 == use_apn)
		{
			huayi_json_get_int(request_root,"auth_type",&auth_type);
			huayi_json_get_str(request_root,"apn_name",apn_name,OPTION_STR_LEN);
			huayi_json_get_str(request_root,"apn_usr",username,OPTION_STR_LEN);
			huayi_json_get_str(request_root,"apn_pwd",password,OPTION_STR_LEN);

			huayi_uci_set_int(PKG_MODEM,"status","validation",auth_type);
			huayi_uci_set_str(PKG_MODEM,"status","apn",apn_name);
			huayi_uci_set_str(PKG_MODEM,"status","user4g",username);
			huayi_uci_set_str(PKG_MODEM,"status","pass4g",password);
		}
		huayi_uci_set_int(PKG_MODEM,"status","mode",use_apn);
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(strcmp(wan_type ,WAN_TYPE_WIRED_PRIOROTY) == 0)
	{
		{
			memset(cmd,0,sizeof(cmd));
			huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
			if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
				&& strncmp("EC200",pmodel, 5) != 0)
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device wwan0");
			}
			else
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device eth1");
			}
			system(cmd);

			if(!huayi_json_get_int(request_root,"use_apn",&use_apn))
			{
				return FCGI_RESP_CODE_DATA_MISS;
			}
			if(1 == use_apn)
			{
				huayi_json_get_int(request_root,"auth_type",&auth_type);
				huayi_json_get_str(request_root,"apn_name",apn_name,OPTION_STR_LEN);
				huayi_json_get_str(request_root,"apn_usr",username,OPTION_STR_LEN);
				huayi_json_get_str(request_root,"apn_pwd",password,OPTION_STR_LEN);

				huayi_uci_set_int(PKG_MODEM,"status","validation",auth_type);
				huayi_uci_set_str(PKG_MODEM,"status","apn",apn_name);
				huayi_uci_set_str(PKG_MODEM,"status","user4g",username);
				huayi_uci_set_str(PKG_MODEM,"status","pass4g",password);
			}
			huayi_uci_set_int(PKG_MODEM,"status","mode",use_apn);
		}

		{
			huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);

			huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);

			//snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
			//system(cmd);
			memset(cmd,0,sizeof(cmd));
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend wired_device %s",wan_ifname);
			system(cmd);

			huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
			huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);

			huayi_uci_get_int(PKG_NETWORK,"wan","ppp_mtu",&ppp_mtu);

			huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);

			if(!strcmp(proto,HUAYI_NET_PROTO_STATIC))
			{
				if(!set_multiwan_static_config_by_index(request_root,"wan"))
				{
					huayi_uci_revert(PKG_NETWORK);
					return FCGI_RESP_CODE_DATA_MISS;
				}
				huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
			}else if(!strcmp(proto,HUAYI_NET_PROTO_PPPOE))
			{
				if(!set_multiwan_pppoe_config_by_index(request_root,"wan"))
				{
					huayi_uci_revert(PKG_NETWORK);
					return FCGI_RESP_CODE_DATA_MISS;
				}
				huayi_uci_set_int(PKG_NETWORK,"wan","mtu",ppp_mtu);
			}else if(!strcmp(proto,HUAYI_NET_PROTO_DHCP))
			{
				if(!set_multiwan_dhcp_config_by_index(request_root,"wan"))
				{
					huayi_uci_revert(PKG_NETWORK);
					return FCGI_RESP_CODE_DATA_MISS;
				}
				huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
			}

			huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);

			huayi_json_get_str(request_root,"detect_ip", detectip, OPTION_STR_LEN);
			if(strlen(detectip)>0)
			{
				huayi_uci_set_str(PKG_SYSTEM, "mode", "detect_ip", detectip);
			}
		}
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(strcmp(wan_type ,WAN_TYPE_REP4G_PRIOROTY) == 0)
	{
		memset(cmd,0,sizeof(cmd));
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
		if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
				&& strncmp("EC200",pmodel, 5) != 0)
		{
			huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device wwan0");
		}
		else
		{
			huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device eth1");
		}
		system(cmd);


		if(!huayi_json_get_int(request_root,"use_apn",&use_apn))
		{
			//return FCGI_RESP_CODE_DATA_MISS;
		}
		if(1 == use_apn)
		{
			huayi_json_get_int(request_root,"auth_type",&auth_type);
			huayi_json_get_str(request_root,"apn_name",apn_name,OPTION_STR_LEN);
			huayi_json_get_str(request_root,"apn_usr",username,OPTION_STR_LEN);
			huayi_json_get_str(request_root,"apn_pwd",password,OPTION_STR_LEN);

			huayi_uci_set_int(PKG_MODEM,"status","validation",auth_type);
			huayi_uci_set_str(PKG_MODEM,"status","apn",apn_name);
			huayi_uci_set_str(PKG_MODEM,"status","user4g",username);
			huayi_uci_set_str(PKG_MODEM,"status","pass4g",password);
		}
		huayi_uci_set_int(PKG_MODEM,"status","mode",use_apn);

		huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);

		huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);

		//snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
		//system(cmd);
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend wisp_device %s",wan_ifname);
		system(cmd);

		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);

		ret = set_system_wisp_opmode(request_root);
		if(FCGI_TRUE != ret)
			return ret;

		huayi_json_get_str(request_root,"detect_ip", detectip, OPTION_STR_LEN);
		if(strlen(detectip)>0)
		{
			huayi_uci_set_str(PKG_SYSTEM, "mode", "detect_ip", detectip);
		}
		huayi_json_get_int(request_root,"detect_time", &detect_time);
		huayi_uci_set_int(PKG_SYSTEM, "mode", "detect_time", detect_time);
		huayi_uci_set_str(PKG_NETWORK,"wan","proto", "dhcp");
		huayi_uci_set_str(PKG_NETWORK,"wan4g","proto", "dhcp");
	}
#endif
	else if(strcmp(wan_type ,WAN_TYPE_REP) == 0)
	{
		ret = set_system_wisp_opmode(request_root);
		if(FCGI_TRUE != ret)
			return ret;
	}
	else
#else
	if(strcmp(wan_type ,WAN_TYPE_REP) == 0)
	{
		ret = set_system_wisp_opmode(request_root);
		if(FCGI_TRUE != ret)
			return ret;
	}
	else
#endif
	{
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);

	huayi_uci_delete_option(PKG_NETWORK,"wan4g","ifname");
	huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);

	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
	system(cmd);

	huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
	huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
#endif

		/*set wire network config*/
		huayi_uci_get_int(PKG_NETWORK,"wan","ppp_mtu",&ppp_mtu);

		huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);

		if(!strcmp(proto,HUAYI_NET_PROTO_STATIC))
		{
			if(!set_multiwan_static_config_by_index(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				//return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
		}else if(!strcmp(proto,HUAYI_NET_PROTO_PPPOE))
		{
			if(!set_multiwan_pppoe_config_by_index(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				//return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",ppp_mtu);
		}else if(!strcmp(proto,HUAYI_NET_PROTO_DHCP))
		{
			if(!set_multiwan_dhcp_config_by_index(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				//return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
		}

		huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
	}

	huayi_uci_commit(PKG_NETWORK);

	huayi_uci_set_int(PKG_SYSTEM,"global","first_boot",0);

	huayi_uci_commit(PKG_SYSTEM);
	huayi_uci_commit(PKG_MODEM);
	huayi_uci_commit(PKG_WIRELESS);

	if (!strcmp(wan_type, WAN_TYPE_4G_MOBILE))
	{
		system("huayi-reset lte4g parameter 0");
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_type, WAN_TYPE_WIRED_PRIOROTY))
	{
		system("huayi-reset lte4g parameter 0");
	}
#elif defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_type, WAN_TYPE_REP4G_PRIOROTY))
	{
		system("huayi-reset lte4g parameter 0");
	}
#endif

	system("huayi-reset network");

	*lock_time = HUAYI_RELOAD_NETWORK_TIME;

	cJSON_AddFalseToObject(response_root,"reconnection");

	cJSON_AddStringToObject(response_root,"reconnection_addr","");

	return FCGI_RESP_CODE_SUCCESS;
}


#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
int get_ipv6_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *wan6_type=NULL, *lan_type=NULL;
	int  i = 0;
	char ip6addr[64]={0};
	char proto[16]={0};
	char ifName[8]={0};
	char *tmpP=NULL, tmpStr[128]={0};
	char assigned_type[16]={0};
	char lanDhcp[16]={0}, lanRa[16]={0};
	int raManage=0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	result_root = cJSON_CreateObject();
	wan6_type = cJSON_CreateObject();
	lan_type = cJSON_CreateObject();

//WAN IF
	if(!huayi_uci_get_str(PKG_NETWORK,"wan6","proto",proto)){
		cJSON_AddNumberToObject(result_root, "enable",0);
	}
	else{
		cJSON_AddNumberToObject(result_root, "enable",1);
		if(!strcmp(proto,"dhcpv6")){
			huayi_uci_get_str(PKG_NETWORK,"wan6","ifname",ifName);
			memset(proto,0x00,sizeof(proto));
			if(!strcmp(ifName,"eth1"))
				strcpy(proto,"dhcpv6");
			else if(strstr(ifName,"pppoe"))
				strcpy(proto,"pppoe");
		}
		cJSON_AddStringToObject(wan6_type, "proto", proto);

		memset(ip6addr,0x00,sizeof(ip6addr));
		huayi_uci_get_str(PKG_NETWORK,"wan6","ip6addr",ip6addr);
		tmpP = strchr(ip6addr,'/');
		if(tmpP){
			strcpy(tmpStr,tmpP+1);
			*tmpP = '\0';
		}
		ipv6_reduce(ip6addr);
		cJSON_AddStringToObject(wan6_type, "ip6addr", ip6addr);
		cJSON_AddStringToObject(wan6_type, "prefixlen", tmpStr);

		memset(ip6addr,0x00,sizeof(ip6addr));
		huayi_uci_get_str(PKG_NETWORK,"wan6","ip6gw",ip6addr);
		tmpP = strchr(ip6addr,'/');
		if(tmpP){
			*tmpP = '\0';
		}
		ipv6_reduce(ip6addr);
		cJSON_AddStringToObject(wan6_type, "ip6gw", ip6addr);
	}
//LAN IF
	huayi_uci_get_str(PKG_DHCP,"lan","dhcpv6",lanDhcp);
	huayi_uci_get_str(PKG_DHCP,"lan","ra",lanRa);
	if(huayi_uci_get_int(PKG_DHCP,"lan","ra_management",&raManage)){
		if(!strcmp(lanRa,"server")){
			if(!strcmp(lanDhcp,"server") && (raManage==2))
				strcpy(assigned_type,"dhcpv6");
			else if(!strcmp(lanDhcp,"server") && (raManage==1))
				strcpy(assigned_type,"dhcpv6+slaac");
			else if(!strcmp(lanDhcp,"disabled") && (raManage==0))
				strcpy(assigned_type,"slaac");
		}
		else if(!strcmp(lanRa,"relay") && !strcmp(lanDhcp,"relay")){
			strcpy(assigned_type,"relay");
		}
		else if(!strcmp(lanRa,"disabled") && !strcmp(lanDhcp,"disabled")){
			strcpy(assigned_type,"disabled");
		}

	}
	else if(!strcmp(lanRa,"relay")){
		strcpy(assigned_type,"relay");
	}
	else{
		strcpy(assigned_type,"disabled");
	}
	cJSON_AddStringToObject(lan_type, "assigned_type", assigned_type);

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","ip6addr",ip6addr))
	{
		get_ipv6addr(ip6addr,"br-lan",IPV6_ADDR_LINKLOCAL);
	}
	tmpP = strchr(ip6addr,'/');
	if(tmpP){
		*tmpP = '\0';
	}

	ipv6_reduce(ip6addr);
	cJSON_AddStringToObject(lan_type, "ip6addr", ip6addr);

	memset(tmpStr,0x00,sizeof(tmpStr));
	memset(ip6addr,0x00,sizeof(ip6addr));
	huayi_uci_get_str(PKG_DHCP,"lan","dns",tmpStr);
	tmpP = strtok(tmpStr," ");
	i = 1;
	while(tmpP){
		sprintf(ip6addr,"ip6dns%d",i);
		ipv6_reduce(tmpP);
		cJSON_AddStringToObject(lan_type, ip6addr, tmpP);
		tmpP = strtok(NULL,",");
		i++;
	}

	cJSON_AddItemToObject(result_root,"wan6_type",wan6_type);
	cJSON_AddItemToObject(result_root,"lan",lan_type);
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}
int set_ipv6_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *wan6_type=NULL, *lan_type=NULL;
	int  i = 0;
	int enable=0;
	char ip6addr[64]={0};
	char proto[16]={0};
	char tmpStr[128]={0};
	char assigned_type[16]={0};

	huayi_json_get_int(request_root, "enable", &enable);
	if(enable==0){
		disable_ip6();
		goto IPV6_SET_OK;
	}
	enable_ip6();
//WAN_IF
	wan6_type = cJSON_GetObjectItem(request_root, "wan6_type");
	if(wan6_type==NULL){
		return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_json_get_str(wan6_type,"proto",proto,16);
	if(!strcmp(proto,"static")){
		huayi_json_get_str(wan6_type,"ip6addr",ip6addr,OPTION_STR_LEN);
		huayi_json_get_str(wan6_type,"prefixlen",tmpStr,OPTION_STR_LEN);

		if(is_validipv6(ip6addr)){
			ipv6_reduce(ip6addr);
			if(strchr(ip6addr,'/')==NULL){
				strcat(ip6addr,"/");
				if(atoi(tmpStr)<=128 && atoi(tmpStr)>=0)
					strcat(ip6addr,tmpStr);
				else
					strcat(ip6addr,"64");
			}
			huayi_uci_set_str(PKG_NETWORK,"wan6","ip6addr",ip6addr);
		}
		memset(ip6addr,0x00,sizeof(ip6addr));
		huayi_json_get_str(wan6_type,"ip6gw",ip6addr,OPTION_STR_LEN);

		if(is_validipv6(ip6addr)){
			ipv6_reduce(ip6addr);
			huayi_uci_set_str(PKG_NETWORK,"wan6","ip6gw",ip6addr);
		}
		huayi_uci_set_str(PKG_NETWORK,"wan6","proto","static");
		huayi_uci_set_str(PKG_NETWORK,"wan6","ifname","eth1");
	}
	else if(!strcmp(proto,"dhcpv6")){
		huayi_uci_set_str(PKG_NETWORK,"wan6","proto","dhcpv6");
		huayi_uci_set_str(PKG_NETWORK,"wan6","ifname","eth1");
	}
	else if(!strcmp(proto,"pppoe")){
		huayi_uci_set_str(PKG_NETWORK,"wan6","proto","dhcpv6");
		huayi_uci_set_str(PKG_NETWORK,"wan6","ifname","pppoe-wan");
	}

//LAN_IF
	lan_type = cJSON_GetObjectItem(request_root, "lan");
	if(lan_type==NULL){
		return FCGI_RESP_CODE_DATA_MISS;
	}

	memset(ip6addr,0x00,sizeof(ip6addr));
	huayi_json_get_str(lan_type,"ip6addr",ip6addr,OPTION_STR_LEN);
	if(is_validipv6(ip6addr)){
		ipv6_reduce(ip6addr);
		if(strchr(ip6addr,'/')==NULL){
			strcat(ip6addr,"/64");
		}
		huayi_uci_set_str(PKG_NETWORK,"lan","ip6addr",ip6addr);
	}

	memset(tmpStr,0x00,sizeof(tmpStr));
	memset(proto,0x00,sizeof(proto));
	huayi_uci_set_str(PKG_DHCP, "lan","dns","");
	for(i=1;i<=2;i++){
		sprintf(proto,"ip6dns%d",i);
		memset(ip6addr,0x00,sizeof(ip6addr));
		huayi_json_get_str(lan_type,proto,ip6addr,OPTION_STR_LEN);
		if(is_validipv6(ip6addr)){
			huayi_uci_add_list(PKG_DHCP,"lan","dns",ip6addr);
		}
	}
	huayi_json_get_str(lan_type,"assigned_type",assigned_type,sizeof(assigned_type));
	//DHCPv6,SLAAC,DHCPv6+SLAAC,DISABLED
	if(!strcmp(assigned_type,"relay")){// relay
		huayi_uci_set_str(PKG_DHCP,"lan","ra","relay");
		huayi_uci_set_str(PKG_DHCP,"lan","dhcpv6","relay");
		huayi_uci_set_str(PKG_DHCP,"lan","ndp","relay");
		huayi_uci_set_str(PKG_DHCP,"wan","ra","relay");
		huayi_uci_set_str(PKG_DHCP,"wan","dhcpv6","relay");
		huayi_uci_set_str(PKG_DHCP,"wan","ndp","relay");
		huayi_uci_set_str(PKG_DHCP,"wan","master","1");
	}
	else{
		huayi_uci_set_str(PKG_DHCP,"wan","ra","");
		huayi_uci_set_str(PKG_DHCP,"wan","dhcpv6","");
		huayi_uci_set_str(PKG_DHCP,"lan","ndp","");
		huayi_uci_set_str(PKG_DHCP,"wan","ndp","");

		if(!strcmp(assigned_type,"dhcpv6")){// statefull
			huayi_uci_set_str(PKG_DHCP,"lan","ra","server");
			huayi_uci_set_str(PKG_DHCP,"lan","dhcpv6","server");
			huayi_uci_set_str(PKG_DHCP,"lan","ra_management","2");
		}
		else if(!strcmp(assigned_type,"slaac")){// stateless
			huayi_uci_set_str(PKG_DHCP,"lan","ra","server");
			huayi_uci_set_str(PKG_DHCP,"lan","dhcpv6","disabled");
			huayi_uci_set_str(PKG_DHCP,"lan","ra_management","0");
		}
		else if(!strcmp(assigned_type,"dhcpv6+slaac")){// statefull+stateless
			huayi_uci_set_str(PKG_DHCP,"lan","ra","server");
			huayi_uci_set_str(PKG_DHCP,"lan","dhcpv6","server");
			huayi_uci_set_str(PKG_DHCP,"lan","ra_management","1");
		}
		else if(!strcmp(assigned_type,"disabled")){// disabled
			huayi_uci_set_str(PKG_DHCP,"lan","ra","disabled");
			huayi_uci_set_str(PKG_DHCP,"lan","dhcpv6","disabled");
			huayi_uci_set_str(PKG_DHCP,"lan","ra_management","0");
		}
	}
IPV6_SET_OK:
	huayi_uci_commit(PKG_NETWORK);
	huayi_uci_commit(PKG_DHCP);
	*lock_time = HUAYI_RELOAD_NETWORK_TIME;
	system("huayi-reset network");
	return FCGI_RESP_CODE_SUCCESS;
}
#endif


int get_multicast_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int igmp_version = 3;
	int igmpsnoop = 0;
	int igmpforword = 0;
	int igmpproxy = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_int(PKG_NETWORK,"lan","igmp_version",&igmp_version))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_DEV,"igmpsnoop",&igmpsnoop))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_DEV,"igmpforword",&igmpforword))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_IGMP_PROXY,"@igmpproxy[0]","enable",&igmpproxy))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	result_root = cJSON_CreateObject();

	cJSON_AddNumberToObject(result_root,"igmp_proxy",igmpproxy);
	cJSON_AddNumberToObject(result_root,"igmp_snooping",igmpsnoop);
	cJSON_AddNumberToObject(result_root,"igmp_version",igmp_version);
	cJSON_AddNumberToObject(result_root,"wifi_multicast_forword",igmpforword);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int set_multicast_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int igmp_version = 3;
	int igmpsnoop = 0;
	int igmpforword = 0;
	int igmpproxy = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"igmp_proxy",&igmpproxy))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"igmp_snooping",&igmpsnoop))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"igmp_version",&igmp_version))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wifi_multicast_forword",&igmpforword))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_NETWORK,"lan","igmp_version",igmp_version);

	huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"igmpsnoop",igmpsnoop);
	huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"igmpforword",igmpforword);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_set_int(PKG_WIRELESS,W58G_DEV,"igmpsnoop",igmpsnoop);
	huayi_uci_set_int(PKG_WIRELESS,W58G_DEV,"igmpforword",igmpforword);
#endif

	huayi_uci_set_int(PKG_IGMP_PROXY,"@igmpproxy[0]","enable",igmpproxy);

	huayi_uci_commit(PKG_IGMP_PROXY);
	huayi_uci_commit(PKG_WIRELESS);
	huayi_uci_commit(PKG_NETWORK);

	system("huayi-reset network");

	*lock_time = HUAYI_RELOAD_NETWORK_TIME;

	return FCGI_RESP_CODE_SUCCESS;
}

int get_ftp_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *ftp_array = NULL;
	cJSON *ftp_obj = NULL;
	int ftp_enable = 0;
	int is_usb = 0;
	char drive[OPTION_STR_LEN] = {0};
	char path[OPTION_STR_LEN] = {0};
	char mnt_path[OPTION_STR_LEN] = {0};
	char lan_addr[IPV4_STR_LEN] = {0};
	char ftp_path[OPTION_STR_LEN] = {0};
	struct statfs s;
	long total = 0;
	long unused = 0;
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_int(PKG_USB_CONFIG,"ftp_service","enable",&ftp_enable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_USB_CONFIG,"ftp_service","drive_path",drive))
	{
		//return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",lan_addr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	if(strcmp(lan_addr,"0.0.0.0") == 0)
	{
		huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",lan_addr);
	}

	if(!huayi_uci_get_str(PKG_USB_CONFIG,"ftp_service","mnt_path",mnt_path))
	{
		//return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_USB_CONFIG,"ftp_service","is_usb",&is_usb))
	{
		//return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_USB_CONFIG,"ftp_service","ftp_path",ftp_path))
	{
		//return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	sprintf(path,"ftp://%s/%s",lan_addr,ftp_path);

	result_root = cJSON_CreateObject();

	if(ftp_enable == 1)
	{
		cJSON_AddTrueToObject(result_root,"ftp_enable");
	}
	else
	{
		cJSON_AddFalseToObject(result_root,"ftp_enable");
	}

	if(is_usb == 1)
	{
		if( 0 != statfs(mnt_path, &s) )
		{
			return FCGI_RESP_CODE_SERVER_FAILURE;
		}

		if(s.f_bsize >= 1024)
		{
			total = (s.f_bsize / 1024) * s.f_blocks;
			unused = (s.f_bsize / 1024) * s.f_bavail;
		}
		else
		{
			total = (s.f_blocks / 1024 ) * s.f_bsize;
			unused = (s.f_bavail / 1024 ) * s.f_bsize;
		}
		cJSON_AddTrueToObject(result_root,"is_usb");
	}
	else
	{
		cJSON_AddFalseToObject(result_root,"is_usb");
	}

	ftp_array = cJSON_CreateArray();

	ftp_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(ftp_obj,"drive",drive);
	cJSON_AddNumberToObject(ftp_obj,"capacity",total);
	cJSON_AddNumberToObject(ftp_obj,"unused",unused);
	cJSON_AddStringToObject(ftp_obj,"path",path);

	cJSON_AddItemToArray(ftp_array,ftp_obj);


	cJSON_AddItemToObject(result_root,"ftp_list",ftp_array);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int set_ftp_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int ftp_enable = 0;
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"ftp_enable",&ftp_enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_USB_CONFIG,"ftp_service","enable",ftp_enable);

	huayi_uci_commit(PKG_USB_CONFIG);

	system("huayi-reset ftpd");

	*lock_time = HUAYI_USB_ENABLE_TIME;

	return FCGI_RESP_CODE_SUCCESS;


}

int get_printer_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int enabled = 0;
	char section[OPTION_STR_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	snprintf(section,sizeof(section),"@p910nd[0]");

	if(!huayi_uci_get_int(PKG_PRINTER_CONFIG,section,"enabled",&enabled))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	result_root = cJSON_CreateObject();

	if(enabled == 1)
	{
		cJSON_AddTrueToObject(result_root,"printer_enable");
	}
	else
	{
		cJSON_AddFalseToObject(result_root,"printer_enable");
	}

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;

}

int set_printer_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int printer_enable = 0;
	char section[OPTION_STR_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"printer_enable",&printer_enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	//huayi_uci_set_int(PKG_USB_CONFIG,"printer_service","enable",printer_enable);
	snprintf(section,sizeof(section),"@p910nd[0]");

	huayi_uci_set_int(PKG_PRINTER_CONFIG,section,"enabled",printer_enable);


	huayi_uci_commit(PKG_PRINTER_CONFIG);

	system("huayi-reset printer");

	*lock_time = HUAYI_USB_ENABLE_TIME;

	return FCGI_RESP_CODE_SUCCESS;
}


#if defined(CONFIG_HUAYI_SUPPORT_REPEATER_DHCPD)
int get_force_fix_dhcp(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int fix_enable = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		FASTCGI_LOG("request_root is NULL");
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_HUAYI_UDHCPD,"dhcp","fix_enable",&fix_enable);
	if(fix_enable == 1)
	{
		cJSON_AddTrueToObject(result_root,"fix_dhcp_admin");
	}
	else
	{
		cJSON_AddFalseToObject(result_root,"fix_dhcp_admin");
	}

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	*lock_time = 5;
	return FCGI_RESP_CODE_SUCCESS;

}

int set_force_fix_dhcp(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int fix_enable = 0;
	int opmode = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode);

	if(HUAYI_REP_MODE == opmode)
	{
		if(!huayi_json_get_int(request_root,"fix_dhcp_admin",&fix_enable))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		huayi_uci_set_int(PKG_HUAYI_UDHCPD,"dhcp","fix_enable",fix_enable);

		system("huayi-reset udhcpd");

		huayi_uci_commit(PKG_HUAYI_UDHCPD);
	}

	*lock_time = HUAYI_USB_ENABLE_TIME;
	return FCGI_RESP_CODE_SUCCESS;
}


#endif

int get_wanmode_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char 	proto[OPTION_STR_LEN] = {0};
	char 	ipaddr[IPV4_STR_LEN] = {0};
	char 	netmask[IPV4_STR_LEN] = {0};
	char	gateway[IPV4_STR_LEN] = {0};
	char 	temp_dns[OPTION_STR_LEN] = {0};
	char 	dns[IPV4_STR_LEN] = {0};
	cJSON 	*dns_array = NULL;
	int 	i = 0;
	char 	username[PASSWD_LONG_STR_LEN] = {0};
	char 	password[PASSWD_LONG_STR_LEN] = {0};
	char 	ac[OPTION_STR_LEN] = {0};
	char 	service[OPTION_STR_LEN] = {0};
	char 	hostname[OPTION_STR_LEN] = {0};
	char 	macaddr[MAC_STR_LEN] = {0};
	char 	def_macaddr[MAC_STR_LEN] = {0};
	int  	ppp_mtu = 0;
	int  	cc_mtu = 0;
	cJSON	*result_root = NULL;
	int 	sys_mode = 0;
	char	ssid_24g[OPTION_STR_LEN] = {0},encryption_24g[OPTION_STR_LEN] = {0},key_24g[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	char authmode_24g[OPTION_STR_LEN] = {0};
	char cipher_24g[OPTION_STR_LEN] = {0};
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	char	ssid_58g[OPTION_STR_LEN] = {0},encryption_58g[OPTION_STR_LEN] = {0},key_58g[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	char authmode_58g[OPTION_STR_LEN] = {0};
	char cipher_58g[OPTION_STR_LEN] = {0};
#endif

#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	if(!huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	cJSON_AddNumberToObject(result_root,"sysmode",sys_mode);
	//if(sys_mode == HUAYI_ROUTE_MODE)
	{
		huayi_uci_get_str(PKG_NETWORK,"wan","ipaddr",ipaddr);
		cJSON_AddStringToObject(result_root,"ip_addr",ipaddr);

		huayi_uci_get_str(PKG_NETWORK,"wan","netmask",netmask);
		cJSON_AddStringToObject(result_root,"subnet_mask",netmask);

		huayi_uci_get_str(PKG_NETWORK,"wan","gateway",gateway);
		cJSON_AddStringToObject(result_root,"gateway",gateway);

		huayi_uci_get_str(PKG_NETWORK,"wan","dns",temp_dns);

		dns_array = cJSON_CreateArray();
		while(huayi_get_nth_value(i++, temp_dns, WEB_SPACE_DELIMIT, dns, sizeof(dns)))
		{
			cJSON_AddItemToArray(dns_array,cJSON_CreateString(dns));
		}
		cJSON_AddItemToObject(result_root,"dns_arr",dns_array);

		huayi_uci_get_str(PKG_NETWORK,"wan","username",username);
		cJSON_AddStringToObject(result_root,"usr",username);

		huayi_uci_get_str(PKG_NETWORK,"wan","password",password);
		cJSON_AddStringToObject(result_root,"pwd",password);

		huayi_uci_get_str(PKG_NETWORK,"wan","ac",ac);
		cJSON_AddStringToObject(result_root,"ac",ac);

		huayi_uci_get_str(PKG_NETWORK,"wan","service",service);
		cJSON_AddStringToObject(result_root,"service",service);

		huayi_uci_get_str(PKG_NETWORK,"wan","hostname",hostname);
		if(strlen(hostname) <= 0)
		{
			huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
		}
		cJSON_AddStringToObject(result_root,"host_name",hostname);

		huayi_uci_get_str(PKG_NETWORK,"wan","macaddr",macaddr);
		cJSON_AddStringToObject(result_root,"mac_addr",macaddr);

		huayi_uci_get_str(PKG_NETWORK,"wan","def_macaddr",def_macaddr);
		cJSON_AddStringToObject(result_root,"def_mac_addr",def_macaddr);

		huayi_uci_get_int(PKG_NETWORK,"wan","ppp_mtu",&ppp_mtu);
		cJSON_AddNumberToObject(result_root,"ppp_mtu",ppp_mtu);

		huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);
		cJSON_AddNumberToObject(result_root,"wan_mtu",cc_mtu);

		huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
		cJSON_AddStringToObject(result_root,"proto",proto);
	}
	//else if(sys_mode == HUAYI_AP_MODE)
	{
		if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_24g))
		{
			return FCGI_RESP_CODE_SERVER_FAILURE;
		}

		if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"encryption",encryption_24g))
		{
			return FCGI_RESP_CODE_SERVER_FAILURE;
		}

		if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"key",key_24g))
		{
			;
		}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_58g))
		{
			return FCGI_RESP_CODE_SERVER_FAILURE;
		}

		if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"encryption",encryption_58g))
		{
			return FCGI_RESP_CODE_SERVER_FAILURE;
		}

		if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"key",key_58g))
		{
			;
		}
#endif
		cJSON_AddStringToObject(result_root,"ssid_24g",ssid_24g);
		cJSON_AddStringToObject(result_root,"key_24g",key_24g);
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
		if(strcmp(encryption_24g,"none") == 0)
		{
			strcpy(authmode_24g,"disable");
			strcpy(cipher_24g,"AES");
		}
		else
		{
			if(strstr(encryption_24g,"psk-mixed"))
			{
				strcpy(authmode_24g,"WPAPSKWPA2PSK");
			}else if(strstr(encryption_24g,"psk2"))
			{
				strcpy(authmode_24g,"WPA2PSK");
			}else
			{
				strcpy(authmode_24g,"WPAPSK");
			}

			if(strstr(encryption_24g,"ccmp") && strstr(encryption_24g,"tkip"))
			{
				strcpy(cipher_24g,"TKIPAES");
			}else if(strstr(encryption_24g,"tkip"))
			{
				strcpy(cipher_24g,"TKIP");
			}else if(strstr(encryption_24g,"ccmp"))
			{
				strcpy(cipher_24g,"AES");
			}else
			{
				strcpy(cipher_24g,"none");
			}
		}
		cJSON_AddStringToObject(result_root,"authmode_24g",authmode_24g);
		cJSON_AddStringToObject(result_root,"cipher_24g",cipher_24g);
#endif

		if (strncmp(encryption_24g,"none",strlen("none")) == 0)
		{
			cJSON_AddNumberToObject(result_root,"encryption_24g",0);
		}
		else
		{
			cJSON_AddNumberToObject(result_root,"encryption_24g",1);
		}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		cJSON_AddStringToObject(result_root,"ssid_58g",ssid_58g);
		cJSON_AddStringToObject(result_root,"key_58g",key_58g);

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
		if(strcmp(encryption_58g,"none") == 0){
			strcpy(authmode_58g,"disable");
			strcpy(cipher_58g,"AES");
		} else {
			if(strstr(encryption_58g,"psk-mixed")) {
				strcpy(authmode_58g,"WPAPSKWPA2PSK");
			}else if(strstr(encryption_58g,"psk2")) {
				strcpy(authmode_58g,"WPA2PSK");
			} else {
				strcpy(authmode_58g,"WPAPSK");
			}

			if(strstr(encryption_58g,"ccmp") && strstr(encryption_58g,"tkip"))
			{
				strcpy(cipher_58g,"TKIPAES");
			}else if(strstr(encryption_58g,"tkip"))
			{
				strcpy(cipher_58g,"TKIP");
			}else if(strstr(encryption_58g,"ccmp"))
			{
				strcpy(cipher_58g,"AES");
			}else
			{
				strcpy(cipher_58g,"none");
			}
		}
		cJSON_AddStringToObject(result_root,"authmode_58g",authmode_58g);
		cJSON_AddStringToObject(result_root,"cipher_58g",cipher_58g);
#endif
		if (strncmp(encryption_58g,"none",strlen("none")) == 0)
		{
			cJSON_AddNumberToObject(result_root,"encryption_58g",0);
		}
		else
		{
			cJSON_AddNumberToObject(result_root,"encryption_58g",1);
		}
#endif
	}
	/*else if(sys_mode == HUAYI_REP_MODE)
	{
		;
	}
	else
	{
		;
	}*/

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}


