#include "fastcgi_common.h"


/*FCGI_BOOL huayi_get_vpn_info(cJSON *result_root)
{
	int	total_vpn_user = 0;
	int online_vpn_user = 0;
	char tmp_str[SMALL_STR_LEN] = {0};
	int maxid = 0;

	if(NULL==result_root)
	{
		return FCGI_FALSE;
	}
	get_cmd_result("cat /tmp/vpn_status | wc -l",tmp_str,SMALL_STR_LEN);

	online_vpn_user = atoi(tmp_str);

	
	
	huayi_uci_get_int(PKG_VPN_USER,"@login[-1]","id",&maxid);
	if(online_vpn_user > maxid)
	{
		maxid = online_vpn_user;
	}
	
	cJSON_AddNumberToObject(result_root,"total_vpn_user",maxid);

	cJSON_AddNumberToObject(result_root,"online_vpn_user",online_vpn_user);

	return FCGI_TRUE;

}*/

int get_pptpd_server_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int pptpd_enable = 0;
	char pptpd_startip[IPV4_STR_LEN] = {0};
	char pptpd_endip[IPV4_STR_LEN] = {0};
	char xl2tpd_startip[IPV4_STR_LEN] = {0};
	char xl2tpd_endip[IPV4_STR_LEN] = {0};
	char server_ip[IPV4_STR_LEN] = {0};
	int mppe = 0;
	int dns_enable = 0;
	char primary_dns[IPV4_STR_LEN] = {0};
	char secondary_dns[IPV4_STR_LEN] = {0};
	int mtu = 0;
	int mru = 0;
	char proto[OPTION_STR_LEN] = {0};

	cJSON * result_root = NULL;
	
	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_uci_get_int(PKG_PPTPD,"pptpd","enable",&pptpd_enable);
	huayi_uci_get_str(PKG_PPTPD,"pptpd","server_ip",server_ip);
	huayi_uci_get_str(PKG_PPTPD,"pptpd","startip",pptpd_startip);
	huayi_uci_get_str(PKG_PPTPD,"pptpd","endip",pptpd_endip);
	huayi_uci_get_int(PKG_PPTPD,"pptpd","mppe",&mppe);
	huayi_uci_get_int(PKG_PPTPD,"pptpd","dns_enable",&dns_enable);
	huayi_uci_get_int(PKG_PPTPD,"pptpd","mtu",&mtu);
	huayi_uci_get_int(PKG_PPTPD,"pptpd","mru",&mru);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","startip",xl2tpd_startip);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","endip",xl2tpd_endip);
	huayi_uci_get_str(PKG_NETWORK,"wan1","proto",proto);
	huayi_uci_get_str(PKG_PPTPD,"pptpd","primary_dns",primary_dns);
	huayi_uci_get_str(PKG_PPTPD,"pptpd","secondary_dns",secondary_dns);

	
	result_root = cJSON_CreateObject();

	huayi_add_switch_to_json(result_root,"pptpd_enable",pptpd_enable);
	cJSON_AddStringToObject(result_root,"pptpd_start_addr",pptpd_startip);
	cJSON_AddStringToObject(result_root,"pptpd_end_addr",pptpd_endip);
	cJSON_AddStringToObject(result_root,"server_addr",server_ip);
	cJSON_AddNumberToObject(result_root,"mppe",mppe);
	huayi_add_switch_to_json(result_root,"dns_enable",dns_enable);
	cJSON_AddStringToObject(result_root,"primary_dns",primary_dns);
	cJSON_AddStringToObject(result_root,"secondary_dns",secondary_dns);
	cJSON_AddNumberToObject(result_root,"mtu",mtu);
	cJSON_AddNumberToObject(result_root,"mru",mru);
	cJSON_AddStringToObject(result_root,"xl2tpd_start_addr",xl2tpd_startip);
	cJSON_AddStringToObject(result_root,"xl2tpd_end_addr",xl2tpd_endip);
	cJSON_AddStringToObject(result_root,"proto",proto);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;

}



int set_pptpd_server_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{

	int pptpd_enable = 0;
	char startip[IPV4_STR_LEN] = {0};
	char endip[IPV4_STR_LEN] = {0};
	int mppe = 0;
	char server_ip[IPV4_STR_LEN] = {0};
	int dns_enable = 0;
	char primary_dns[IPV4_STR_LEN] = {0};
	char secondary_dns[IPV4_STR_LEN] = {0};
	int mtu = 0;
	int mru = 0;

	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_json_get_int(request_root,"pptpd_enable",&pptpd_enable);
	huayi_json_get_str(request_root,"pptpd_start_addr",startip,IPV4_STR_LEN);
	huayi_json_get_str(request_root,"pptpd_end_addr",endip,IPV4_STR_LEN);
	huayi_json_get_str(request_root,"server_addr",server_ip,IPV4_STR_LEN);
	huayi_json_get_int(request_root,"mppe",&mppe);
	huayi_json_get_int(request_root,"dns_enable",&dns_enable);
	huayi_json_get_str(request_root,"primary_dns",primary_dns,IPV4_STR_LEN);
	huayi_json_get_str(request_root,"secondary_dns",secondary_dns,IPV4_STR_LEN);
	huayi_json_get_int(request_root,"mtu",&mtu);
	huayi_json_get_int(request_root,"mru",&mru);	

	huayi_uci_set_int(PKG_PPTPD,"pptpd","enable",pptpd_enable);
	huayi_uci_set_str(PKG_PPTPD,"pptpd","startip",startip);
	huayi_uci_set_str(PKG_PPTPD,"pptpd","endip",endip);
	huayi_uci_set_str(PKG_PPTPD,"pptpd","server_ip",server_ip);
	huayi_uci_set_int(PKG_PPTPD,"pptpd","mppe",mppe);
	huayi_uci_set_int(PKG_PPTPD,"pptpd","dns_enable",dns_enable);
	huayi_uci_set_str(PKG_PPTPD,"pptpd","primary_dns",primary_dns);
	huayi_uci_set_str(PKG_PPTPD,"pptpd","secondary_dns",secondary_dns);
	huayi_uci_set_int(PKG_PPTPD,"pptpd","mtu",mtu);
	huayi_uci_set_int(PKG_PPTPD,"pptpd","mru",mru);

	huayi_uci_commit(PKG_PPTPD);

	*lock_time = HUAYI_RELOAD_VPN_SERVER_TIME;

	system("huayi-reset pptpd");
	
	return FCGI_RESP_CODE_SUCCESS;
	
}


int get_xl2tpd_server_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int xl2tpd_enable = 0;
	int xl2tp_enable = 0;
	char xl2tpd_startip[IPV4_STR_LEN] = {0};
	char xl2tpd_endip[IPV4_STR_LEN] = {0};
	char server_ip[IPV4_STR_LEN] = {0};
	int ipsec_enable = 0;
	char ipsec_key[OPTION_STR_LEN] = {0};
	int dns_enable = 0;
	char primary_dns[IPV4_STR_LEN] = {0};
	char secondary_dns[IPV4_STR_LEN] = {0};
	int mtu = 0;
	int mru = 0;
	char pptpd_startip[IPV4_STR_LEN] = {0};
	char pptpd_endip[IPV4_STR_LEN] = {0};
	char proto[OPTION_STR_LEN] = {0};

	cJSON * result_root = NULL;

	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	huayi_uci_get_int(PKG_XL2TPD,"xl2tpd","enable",&xl2tpd_enable);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","startip",xl2tpd_startip);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","endip",xl2tpd_endip);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","server_ip",server_ip);
	huayi_uci_get_int(PKG_XL2TPD,"xl2tpd","ipsec_enable",&ipsec_enable);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","ipsec_key",ipsec_key);
	huayi_uci_get_int(PKG_XL2TPD,"xl2tpd","dns_enable",&dns_enable);
	huayi_uci_get_int(PKG_XL2TPD,"xl2tpd","mtu",&mtu);
	huayi_uci_get_int(PKG_XL2TPD,"xl2tpd","mru",&mru);
	huayi_uci_get_str(PKG_PPTPD,"pptpd","startip",pptpd_startip);
	huayi_uci_get_str(PKG_PPTPD,"pptpd","endip",pptpd_endip);
	huayi_uci_get_str(PKG_NETWORK,"wan1","proto",proto);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","primary_dns",primary_dns);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","secondary_dns",secondary_dns);
	huayi_uci_get_int(PKG_PPL2TP,"xl2tp","enable",&xl2tp_enable);


	result_root = cJSON_CreateObject();

	huayi_add_switch_to_json(result_root,"xl2tpd_enable",xl2tpd_enable);
	huayi_add_switch_to_json(result_root,"xl2tp_enable",xl2tp_enable);
	cJSON_AddStringToObject(result_root,"xl2tpd_start_addr",xl2tpd_startip);
	cJSON_AddStringToObject(result_root,"xl2tpd_end_addr",xl2tpd_endip);
	cJSON_AddStringToObject(result_root,"server_addr",server_ip);
	huayi_add_switch_to_json(result_root,"ipsec_enable",ipsec_enable);
	cJSON_AddStringToObject(result_root,"ipsec_key",ipsec_key);
	huayi_add_switch_to_json(result_root,"dns_enable",dns_enable);
	cJSON_AddStringToObject(result_root,"primary_dns",primary_dns);
	cJSON_AddStringToObject(result_root,"secondary_dns",secondary_dns);
	cJSON_AddNumberToObject(result_root,"mtu",mtu);
	cJSON_AddNumberToObject(result_root,"mru",mru);
	cJSON_AddStringToObject(result_root,"proto",proto);
	cJSON_AddStringToObject(result_root,"pptpd_start_addr",pptpd_startip);
	cJSON_AddStringToObject(result_root,"pptpd_end_addr",pptpd_endip);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	 
	return FCGI_RESP_CODE_SUCCESS;
	
}

int set_xl2tpd_server_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int xl2tpd_enable = 0;
	char xl2tpd_startip[IPV4_STR_LEN] = {0};
	char xl2tpd_endip[IPV4_STR_LEN] = {0};
	char server_ip[IPV4_STR_LEN] = {0};
	int ipsec_enable = 0;
	char ipsec_key[OPTION_STR_LEN] = {0};
	int dns_enable = 0;
	char primary_dns[IPV4_STR_LEN] = {0};
	char secondary_dns[IPV4_STR_LEN] = {0};
	int mtu = 0;
	int mru = 0;
	
	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_json_get_int(request_root,"xl2tpd_enable",&xl2tpd_enable);
	huayi_json_get_str(request_root,"xl2tpd_start_addr",xl2tpd_startip,IPV4_STR_LEN);
	huayi_json_get_str(request_root,"xl2tpd_end_addr",xl2tpd_endip,IPV4_STR_LEN);
	huayi_json_get_str(request_root,"server_addr",server_ip,IPV4_STR_LEN);
	huayi_json_get_int(request_root,"ipsec_enable",&ipsec_enable);
	huayi_json_get_str(request_root,"ipsec_key",ipsec_key,OPTION_STR_LEN);
	huayi_json_get_int(request_root,"dns_enable",&dns_enable);
	huayi_json_get_str(request_root,"primary_dns",primary_dns,OPTION_STR_LEN);
	huayi_json_get_str(request_root,"secondary_dns",secondary_dns,IPV4_STR_LEN);
	huayi_json_get_int(request_root,"mtu",&mtu);
	huayi_json_get_int(request_root,"mru",&mru);

	huayi_uci_set_int(PKG_XL2TPD,"xl2tpd","enable",xl2tpd_enable);
	huayi_uci_set_str(PKG_XL2TPD,"xl2tpd","startip",xl2tpd_startip);
	huayi_uci_set_str(PKG_XL2TPD,"xl2tpd","endip",xl2tpd_endip);
	huayi_uci_set_str(PKG_XL2TPD,"xl2tpd","server_ip",server_ip);
	huayi_uci_set_int(PKG_XL2TPD,"xl2tpd","ipsec_enable",ipsec_enable);
	huayi_uci_set_str(PKG_XL2TPD,"xl2tpd","ipsec_key",ipsec_key);
	huayi_uci_set_int(PKG_XL2TPD,"xl2tpd","dns_enable",dns_enable);
	huayi_uci_set_str(PKG_XL2TPD,"xl2tpd","primary_dns",primary_dns);
	huayi_uci_set_str(PKG_XL2TPD,"xl2tpd","secondary_dns",secondary_dns);
	huayi_uci_set_int(PKG_XL2TPD,"xl2tpd","mtu",mtu);
	huayi_uci_set_int(PKG_XL2TPD,"xl2tpd","mru",mru);

	huayi_uci_commit(PKG_XL2TPD);

	*lock_time = HUAYI_RELOAD_VPN_SERVER_TIME;
	
	system("huayi-reset xl2tpd");
	
	return FCGI_RESP_CODE_SUCCESS;
	
}


int get_pptp_client_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int pptp_enable = 0;
	char server_ip[LIST_STR_LEN ] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	int mppe = 0;
	int route_type = 0;
	char pptp_ip[IPV4_STR_LEN] = {0};
	int pptp_status = 0;
	cJSON * result_root = NULL;

	cJSON *pptp_static_router_array = NULL;
	cJSON *pptp_static_router_obj = NULL;
	char section[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};

	int i = 0;

	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_get_int(PKG_PPL2TP,"pptp","enable",&pptp_enable);
	huayi_uci_get_str(PKG_PPL2TP,"pptp","server_ip",server_ip);
	huayi_uci_get_str(PKG_PPL2TP,"pptp","username",username);
	huayi_uci_get_str(PKG_PPL2TP,"pptp","password",password);
	huayi_uci_get_int(PKG_PPL2TP,"pptp","mppe",&mppe);
	huayi_uci_get_int(PKG_PPL2TP,"pptp","route_type",&route_type);
	huayi_uci_get_str(PKG_STATUS_SYSTEM,"sys","pptp_ip",pptp_ip);
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","pptp_link",&pptp_status);

	result_root = cJSON_CreateObject();

	huayi_add_switch_to_json(result_root,"pptp_enable",pptp_enable);
	cJSON_AddStringToObject(result_root,"usr",username);
	cJSON_AddStringToObject(result_root,"pwd",password);
	cJSON_AddStringToObject(result_root,"server_addr",server_ip);
	cJSON_AddNumberToObject(result_root,"mppe",mppe);
	cJSON_AddNumberToObject(result_root,"route_type",route_type);
	cJSON_AddNumberToObject(result_root,"pptp_status",pptp_status);
	cJSON_AddStringToObject(result_root,"pptp_addr",pptp_ip);

	pptp_static_router_array = cJSON_CreateArray();
	while(1)
	{
		snprintf(section,sizeof(section),"@route[%d]",i);
		if(!huayi_uci_get_str(PKG_PPL2TP,section,"interface",inter_face))
		{
			break;
		}
		if(strcmp(inter_face,"PPTP") == 0)
		{
			huayi_uci_get_str(PKG_PPL2TP,section,"target",target);
			huayi_uci_get_str(PKG_PPL2TP,section,"netmask",netmask);
	
			pptp_static_router_obj = cJSON_CreateObject();
			cJSON_AddStringToObject(pptp_static_router_obj,"target",target);
			cJSON_AddStringToObject(pptp_static_router_obj,"subnet_mask",netmask);
			cJSON_AddItemToArray(pptp_static_router_array,pptp_static_router_obj);
		}
		
		i++;
	}
	
	cJSON_AddItemToObject(result_root,"pptp_static_arr",pptp_static_router_array);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
	
}


int set_pptp_client_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int pptp_enable = 0;
	char server_ip[LIST_STR_LEN ] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	int mppe = 0;
	int route_type = 0;
	int old_route_type = 0;
	int old_enable = 0;

	cJSON *pptp_static_router_array = NULL;
	cJSON *pptp_static_router_obj = NULL;
	int size = 0;
	int i = 0;
	int	k = 0;
	int	max_id=0;
	char section[OPTION_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char max_route[SMALL_STR_LEN] = {0};

	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_uci_get_int(PKG_PPL2TP,"pptp","route_type",&old_route_type);
	huayi_uci_get_int(PKG_PPL2TP,"pptp","enable",&old_enable);

	huayi_json_get_int(request_root,"pptp_enable",&pptp_enable);
	huayi_json_get_str(request_root,"server_addr",server_ip,LIST_STR_LEN );
	huayi_json_get_str(request_root,"usr",username,OPTION_STR_LEN);
	huayi_json_get_str(request_root,"pwd",password,OPTION_STR_LEN);
	huayi_json_get_int(request_root,"mppe",&mppe);
	huayi_json_get_int(request_root,"route_type",&route_type);
	
	huayi_uci_set_int(PKG_PPL2TP,"pptp","enable",pptp_enable);
	huayi_uci_set_str(PKG_PPL2TP,"pptp","server_ip",server_ip);
	huayi_uci_set_str(PKG_PPL2TP,"pptp","username",username);
	huayi_uci_set_str(PKG_PPL2TP,"pptp","password",password);
	huayi_uci_set_int(PKG_PPL2TP,"pptp","mppe",mppe);
	huayi_uci_set_int(PKG_PPL2TP,"pptp","route_type",route_type);
	
	if(route_type == 0 && pptp_enable == 1)
	{
		if(!(pptp_static_router_array= cJSON_GetObjectItem(request_root,"pptp_static_arr")))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		size = cJSON_GetArraySize(pptp_static_router_array);
		get_cmd_result("uci show ppl2tp | grep interface | wc -l",max_route,SMALL_STR_LEN);
		max_id = atoi(max_route);
		if(max_id == 0)
		{
			;/*no section*/
		}else
		{
			for(i = max_id - 1 ;i >= 0 ;i--)
			{
				snprintf(section,sizeof(section),"@route[%d]",i);
				huayi_uci_get_str(PKG_PPL2TP,section,"interface",inter_face);
				if(strcmp(inter_face,"PPTP") == 0)
				{
					huayi_uci_del_section(PKG_PPL2TP,section);
				}
			}

			max_id = 0;
			get_cmd_result("uci show ppl2tp | grep interface | wc -l",max_route,SMALL_STR_LEN);
			max_id = atoi(max_route);
		}

		for(k=0;k<size;k++)
		{
			pptp_static_router_obj = cJSON_GetArrayItem(pptp_static_router_array,k);
			huayi_json_get_str(pptp_static_router_obj,"target",target,IPV4_STR_LEN);
			huayi_json_get_str(pptp_static_router_obj,"netmask",netmask,IPV4_STR_LEN);
			
			huayi_uci_add_section(PKG_PPL2TP,"route");
			snprintf(section,sizeof(section),"@route[%d]",k + max_id);
			
			huayi_uci_set_str(PKG_PPL2TP,section,"target",target);
			huayi_uci_set_str(PKG_PPL2TP,section,"subnet_mask",netmask);
			huayi_uci_set_str(PKG_PPL2TP,section,"interface","PPTP");
		}
	}

	huayi_uci_commit(PKG_PPL2TP);
	
	/*if((route_type == 0 && pptp_enable == 1) || (old_route_type == 0 && route_type == 1 && pptp_enable == 1))
	{
		system("huayi-reset network");
		*lock_time = HUAYI_RELOAD_NETWORK_TIME;
	}
	else
	{
		system("huayi-reset pptp");
	}*/
	if((old_route_type == 0 && route_type == 1 && pptp_enable == 1) || 
		(route_type == 0 && pptp_enable == 1 && old_enable == 1 && old_route_type == 0))
	{
		system("huayi-reset network");
		*lock_time = HUAYI_RELOAD_NETWORK_TIME;
	}
	else
	{
		system("huayi-reset pptp");
		*lock_time = 0;
	}
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_l2tp_client_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int xl2tp_enable = 0;
	int xl2tpd_enable = 0;
	char server_ip[LIST_STR_LEN ] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	int route_type = 0;
	int xl2tp_status = 0;
	char xl2tp_ip[IPV4_STR_LEN] = {0};

	cJSON * result_root = NULL;

	cJSON *xl2tp_static_router_array = NULL;
	cJSON *xl2tp_static_router_obj = NULL;
	char section[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};
	int ipsec_enable = 0;
	char ipsec_key[OPTION_STR_LEN] = {0};

	int i = 0;

	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_get_int(PKG_PPL2TP,"xl2tp","enable",&xl2tp_enable);
	huayi_uci_get_str(PKG_PPL2TP,"xl2tp","server_ip",server_ip);
	huayi_uci_get_str(PKG_PPL2TP,"xl2tp","username",username);
	huayi_uci_get_str(PKG_PPL2TP,"xl2tp","password",password);
	huayi_uci_get_int(PKG_PPL2TP,"xl2tp","route_type",&route_type);
	//huayi_uci_get_int(PKG_XL2TPD,"xl2tpd","enable",&xl2tpd_enable);	
	char dev [OPTION_STR_LEN],value [TEMP_STR_LEN],cmd [TEMP_STR_LEN];
	int status;
	huayi_uci_get_str(PKG_PPL2TP,"xl2tp","dev",dev);
	if(xl2tp_enable){
		memset(cmd, 0, TEMP_STR_LEN);
		memset(value, 0, sizeof(value));
		sprintf(cmd, "ifconfig %s 2>/dev/null | awk '/inet / {split($2, a, \":\"); print a[2]}'",dev);
		get_cmd_result(cmd, value, sizeof(value));
		CON_LOG("%s",value);
		if(!strlen(value))
			status = 0;
		else
			status = 1;
	}else{
		status = 0;
	}

	xl2tpd_enable = status;

	
	//huayi_uci_get_int(PKG_PPL2TP,"xl2tp","enable",&xl2tpd_enable);	
	//huayi_uci_get_int(PKG_PPL2TP,"xl2tp","ipsec_enable",&ipsec_enable);
	//if(ipsec_enable)
	huayi_uci_get_str(PKG_PPL2TP,"xl2tp","ipsec_key",ipsec_key);
	
	//huayi_uci_get_str(PKG_STATUS_SYSTEM,"sys","xl2tp_ip",xl2tp_ip);
	//huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","xl2tp_link",&xl2tp_status);
	//huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",xl2tp_ip);
	//huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&xl2tp_status);
	result_root = cJSON_CreateObject();

	huayi_add_switch_to_json(result_root,"xl2tp_enable",xl2tp_enable);
	huayi_add_switch_to_json(result_root,"xl2tpd_enable",xl2tpd_enable);
	cJSON_AddStringToObject(result_root,"usr",username);
	cJSON_AddStringToObject(result_root,"pwd",password);
	cJSON_AddStringToObject(result_root,"server_addr",server_ip);
	cJSON_AddNumberToObject(result_root,"route_type",route_type);
	cJSON_AddNumberToObject(result_root,"xl2tp_status",status);
	cJSON_AddStringToObject(result_root,"xl2tp_ip",value);
	//cJSON_AddNumberToObject(result_root,"ipsec_enable",ipsec_enable);
	cJSON_AddStringToObject(result_root,"ipsec_key",ipsec_key);
	

	xl2tp_static_router_array = cJSON_CreateArray();
	xl2tp_static_router_obj = cJSON_CreateObject();

	memset(cmd, 0, TEMP_STR_LEN);
	sprintf(cmd, "route -n | awk '$8 == \"%s\" {print $3}'",dev);
	get_cmd_result(cmd, netmask, sizeof(value));
	cJSON_AddStringToObject(xl2tp_static_router_obj,"subnet_mask",netmask);
	memset(cmd, 0, TEMP_STR_LEN);
	sprintf(cmd, "route -n | awk '$8 == \"%s\" {print $2}'",dev);
	get_cmd_result(cmd, target, sizeof(value));
	cJSON_AddStringToObject(xl2tp_static_router_obj,"target",target);
	cJSON_AddItemToArray(xl2tp_static_router_array,xl2tp_static_router_obj);
/*
	while(1)
	{
		snprintf(section,sizeof(section),"@route[%d]",i);
		if(!huayi_uci_get_str(PKG_PPL2TP,section,"interface",inter_face))
		{
			break;
		}
		if(strcmp(inter_face,"ppp0") == 0)
		{
			huayi_uci_get_str(PKG_PPL2TP,section,"target",target);
			huayi_uci_get_str(PKG_PPL2TP,section,"netmask",netmask);
	
			xl2tp_static_router_obj = cJSON_CreateObject();
			cJSON_AddStringToObject(xl2tp_static_router_obj,"target",target);
			cJSON_AddStringToObject(xl2tp_static_router_obj,"subnet_mask",netmask);
			cJSON_AddItemToArray(xl2tp_static_router_array,xl2tp_static_router_obj);
		}
		
		i++;
	}
	*/
	cJSON_AddItemToObject(result_root,"xl2tp_static_arr",xl2tp_static_router_array);
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
	
}

int set_l2tp_client_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int enable = 0;
	char server_ip[LIST_STR_LEN ] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	int route_type = 0;
	int old_route_type = 0;
	int old_enable = 0;

	cJSON *xl2tp_static_router_array = NULL;
	cJSON *xl2tp_static_router_obj = NULL;
	int size = 0;
	int i = 0;
	int	k = 0;
	int	max_id=0;
	char section[OPTION_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char max_route[SMALL_STR_LEN] = {0};
	//int ipsec_enable = 0;
	char ipsec_key[OPTION_STR_LEN] = {0};
	

	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"xl2tp_enable",&enable) ||
		!huayi_json_get_str(request_root,"server_addr",server_ip,LIST_STR_LEN ) ||
		!huayi_json_get_str(request_root,"usr",username,OPTION_STR_LEN) ||
		!huayi_json_get_str(request_root,"pwd",password,OPTION_STR_LEN) ||
		!huayi_json_get_int(request_root,"route_type",&route_type) ||
		//!huayi_json_get_int(request_root,"ipsec_enable",&ipsec_enable) ||
		!huayi_json_get_str(request_root,"ipsec_key",ipsec_key,OPTION_STR_LEN))
		
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_get_int(PKG_PPL2TP,"xl2tp","route_type",&old_route_type);
	huayi_uci_get_int(PKG_PPL2TP,"xl2tp","enable",&old_enable);
	
	huayi_uci_set_int(PKG_PPL2TP,"xl2tp","enable",enable);
	huayi_uci_set_str(PKG_PPL2TP,"xl2tp","server_ip",server_ip);
	huayi_uci_set_str(PKG_PPL2TP,"xl2tp","username",username);
	huayi_uci_set_str(PKG_PPL2TP,"xl2tp","password",password);
	huayi_uci_set_int(PKG_PPL2TP,"xl2tp","route_type",route_type);

	
	//huayi_uci_set_int(PKG_PPL2TP,"xl2tp","ipsec_enable",ipsec_enable);
	//if(ipsec_enable)
	huayi_uci_set_str(PKG_PPL2TP,"xl2tp","ipsec_key",ipsec_key);
/*	
	if(route_type == 0 && enable == 1)
	{
		if(!(xl2tp_static_router_array= cJSON_GetObjectItem(request_root,"xl2tp_static_arr")))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		size = cJSON_GetArraySize(xl2tp_static_router_array);
		get_cmd_result("uci show ppl2tp | grep interface | wc -l",max_route,SMALL_STR_LEN);
		max_id = atoi(max_route);
		if(max_id == 0)
		{
			;
		}else
		{
			for(i = max_id - 1 ;i >= 0 ;i--)
			{
				snprintf(section,sizeof(section),"@route[%d]",i);
				huayi_uci_get_str(PKG_PPL2TP,section,"interface",inter_face);
				if(strcmp(inter_face,"XL2TP") == 0)
				{
					huayi_uci_del_section(PKG_PPL2TP,section);
				}
			}

			max_id = 0;
			get_cmd_result("uci show ppl2tp | grep interface | wc -l",max_route,SMALL_STR_LEN);
			max_id = atoi(max_route);
		}

		for(k=0;k<size;k++)
		{
			xl2tp_static_router_obj = cJSON_GetArrayItem(xl2tp_static_router_array,k);
			huayi_json_get_str(xl2tp_static_router_obj,"target",target,IPV4_STR_LEN);
			huayi_json_get_str(xl2tp_static_router_obj,"subnet_mask",netmask,IPV4_STR_LEN);
			
			huayi_uci_add_section(PKG_PPL2TP,"route");
			snprintf(section,sizeof(section),"@route[%d]",k + max_id);
			
			huayi_uci_set_str(PKG_PPL2TP,section,"target",target);
			huayi_uci_set_str(PKG_PPL2TP,section,"netmask",netmask);
			huayi_uci_set_str(PKG_PPL2TP,section,"interface","XL2TP");
		}
	}
*/
	huayi_uci_commit(PKG_PPL2TP);
	if(enable){
		system("huayi-reset xl2tpd_ipsec");
		*lock_time = HUAYI_RELOAD_WIRELESS_TIME;


	}

	/*			

	if((route_type == 0 && enable == 1 && old_enable == 1 && old_route_type == 0) ||
		(route_type == 1 && old_route_type == 0 && enable == 1))
	{
		system("huayi-reset network");
		system("huayi-reset ipsec_up");
		*lock_time = HUAYI_RELOAD_NETWORK_TIME;
	}
	else
	{
		system("huayi-reset xl2tp");
		*lock_time = 0;
	}
	*/
	return FCGI_RESP_CODE_SUCCESS;
}

int get_vpn_user_config(cJSON *request_root, cJSON *response_root, int *lock_time)
{
	int  i = 0;
	char ppp_username[OPTION_STR_LEN] = {0};
	char ppp_password[OPTION_STR_LEN] = {0};
	char authenticate[OPTION_STR_LEN] = {0};
	char remoteip[OPTION_STR_LEN] = {0};
	char desc[OPTION_STR_LEN] = {0};
	char section[OPTION_STR_LEN] = {0};
	char pptpd_startip[IPV4_STR_LEN] = {0};
	char pptpd_endip[IPV4_STR_LEN] = {0};
	char xl2tpd_startip[IPV4_STR_LEN] = {0};
	char xl2tpd_endip[IPV4_STR_LEN] = {0};
	cJSON * result_root = NULL;
	cJSON * ppp_user_array = NULL;
	cJSON * ppp_user_obj = NULL;
	cJSON * router_array = NULL;
	cJSON * router_obj = NULL;
	char vpn_username[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	int j = 0;
	
	if((NULL == request_root) || (NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	result_root = cJSON_CreateObject();

	ppp_user_array = cJSON_CreateArray();
	
	while(1)
	{
		memset(section,0,OPTION_STR_LEN);
		snprintf(section,sizeof(section),"@login[%d]",i);
		
		if(!huayi_uci_get_str(PKG_VPN_USER,section,"username",ppp_username))
		{
			break;
		}
		memset(desc,0,sizeof(desc));
		memset(remoteip,0,sizeof(remoteip));
		huayi_uci_get_str(PKG_VPN_USER,section,"password",ppp_password);
		huayi_uci_get_str(PKG_VPN_USER,section,"authenticate",authenticate);
		huayi_uci_get_str(PKG_VPN_USER,section,"remoteip",remoteip);
		huayi_uci_get_str(PKG_VPN_USER,section,"desc",desc);
		
		ppp_user_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(ppp_user_obj,"usr",ppp_username);
		cJSON_AddStringToObject(ppp_user_obj,"pwd",ppp_password);
		if(!strcmp(authenticate, "pptp-server"))
		{
			cJSON_AddStringToObject(ppp_user_obj,"authenticate","pptp");
		}else if(!strcmp(authenticate, "xl2tp-server"))
		{
			cJSON_AddStringToObject(ppp_user_obj,"authenticate","l2tp");
		}else if(!strcmp(authenticate, "pppoe-server"))
		{
			cJSON_AddStringToObject(ppp_user_obj,"authenticate","pppoe");
		}else
		{
			cJSON_AddStringToObject(ppp_user_obj,"authenticate","all");
		}
	
		cJSON_AddStringToObject(ppp_user_obj,"remote_addr",remoteip);
		cJSON_AddStringToObject(ppp_user_obj,"com",desc);

		router_array = cJSON_CreateArray();
		j = 0;
		while(1)
		{
			memset(section,0,OPTION_STR_LEN);
			snprintf(section,sizeof(section),"@route[%d]",j++);
			if(!huayi_uci_get_str(PKG_VPN_USER,section,"vpn_username",vpn_username))
			{
				break;
			}
			if(strcmp(vpn_username,ppp_username))
			{
				continue;
			}

			huayi_uci_get_str(PKG_VPN_USER,section,"target",target);
			huayi_uci_get_str(PKG_VPN_USER,section,"netmask",netmask);
			router_obj = cJSON_CreateObject();
			cJSON_AddStringToObject(router_obj,"target",target);
			cJSON_AddStringToObject(router_obj,"subnet_mask",netmask);
			cJSON_AddItemToArray(router_array,router_obj);
			
		}
		cJSON_AddItemToObject(ppp_user_obj,"route_arr",router_array);
		cJSON_AddItemToArray(ppp_user_array,ppp_user_obj);
		i++;

	}
		
	
	huayi_uci_get_str(PKG_PPTPD,"pptpd","startip",pptpd_startip);
	huayi_uci_get_str(PKG_PPTPD,"pptpd","endip",pptpd_endip);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","startip",xl2tpd_startip);
	huayi_uci_get_str(PKG_XL2TPD,"xl2tpd","endip",xl2tpd_endip);
	
	cJSON_AddItemToObject(result_root,"vpnusr_arr",ppp_user_array);
	if(!get_vpn_type_list(result_root))
	{
		cJSON_Delete(result_root);
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	cJSON_AddStringToObject(result_root,"pptpd_start_addr",pptpd_startip);
	cJSON_AddStringToObject(result_root,"pptpd_end_addr",pptpd_endip);
	cJSON_AddStringToObject(result_root,"xl2tpd_start_addr",xl2tpd_startip);
	cJSON_AddStringToObject(result_root,"xl2tpd_end_addr",xl2tpd_endip);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}



/*
	HYID:134
*/
int set_vpn_user_config(cJSON *request_root, cJSON *response_root, int *lock_time)
{
	int  i = 0;
	char ppp_username[OPTION_STR_LEN] = {0};
	char ppp_password[OPTION_STR_LEN] = {0};
	char authenticate[OPTION_STR_LEN] = {0};
	char remoteip[OPTION_STR_LEN] = {0};
	char desc[OPTION_STR_LEN] = {0};
	char section[OPTION_STR_LEN] = {0};
	cJSON * ppp_user_array = NULL;
	cJSON * ppp_user_obj = NULL;
	cJSON * route_array = NULL;
	cJSON * route_obj = NULL;
	int  size = 0;
	int  maxid = 0;
	int  k = 0;
	int  route_size = 0;
	int  max_id = 0;
	int  j = 0;
	int	 route_num = 0;
	char target[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char max_route[SMALL_STR_LEN] = {0};
	
	if((NULL == request_root) || (NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}
	
	ppp_user_array = cJSON_GetObjectItem(request_root,"vpnusr_arr");

	size = cJSON_GetArraySize(ppp_user_array);
	
	snprintf(section,sizeof(section),"@login[-1]");

	huayi_uci_get_int(PKG_VPN_USER,section,"id",&maxid);

	if(maxid == 0)
	{
		;/*no section*/
	}
	else
	{
		snprintf(section,sizeof(section),"@login[0]");
		
		for(i=1;i<=maxid;i++)
		{
			huayi_uci_del_section(PKG_VPN_USER,section);
		}
	}

	get_cmd_result("uci show vpn_user | grep vpn_username | wc -l",max_route,SMALL_STR_LEN);
	max_id = atoi(max_route);

	if(max_id == 0)
	{
		;
	}else
	{
		snprintf(section,sizeof(section),"@route[0]");
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_VPN_USER,section);
		}
	}

	for(k=0;k<size;k++)
	{	
		ppp_user_obj = cJSON_GetArrayItem(ppp_user_array,k);
		huayi_json_get_str(ppp_user_obj,"usr",ppp_username,OPTION_STR_LEN);
		huayi_json_get_str(ppp_user_obj,"pwd",ppp_password,OPTION_STR_LEN);
		huayi_json_get_str(ppp_user_obj,"authenticate",authenticate,OPTION_STR_LEN);
		huayi_json_get_str(ppp_user_obj,"remote_addr",remoteip,OPTION_STR_LEN);
		huayi_json_get_str(ppp_user_obj,"com",desc,OPTION_STR_LEN);
		
		huayi_uci_add_section(PKG_VPN_USER,"login");
		snprintf(section,sizeof(section),"@login[%d]",k);
		
		huayi_uci_set_str(PKG_VPN_USER,section,"username",ppp_username);
		huayi_uci_set_str(PKG_VPN_USER,section,"password",ppp_password);
		if(!strcmp(authenticate, "l2tp"))
		{
			huayi_uci_set_str(PKG_VPN_USER,section,"authenticate","xl2tp-server");
		}else if(!strcmp(authenticate, "pppoe"))
		{
			huayi_uci_set_str(PKG_VPN_USER,section,"authenticate","pppoe-server");
		}else if(!strcmp(authenticate, "pptp"))
		{
			huayi_uci_set_str(PKG_VPN_USER,section,"authenticate","pptp-server");
		}else
		{
			huayi_uci_set_str(PKG_VPN_USER,section,"authenticate","*");
		}
		
		huayi_uci_set_str(PKG_VPN_USER,section,"remoteip",remoteip);

		huayi_uci_set_str(PKG_VPN_USER,section,"desc",desc);
		
		huayi_uci_set_int(PKG_VPN_USER,section,"id",k+1);
		
		route_array = cJSON_GetObjectItem(ppp_user_obj,"route_arr");
		route_size	= cJSON_GetArraySize(route_array);

		for(j=0;j<route_size;j++)
		{
			route_obj = cJSON_GetArrayItem(route_array,j);
			memset(target,0,IPV4_STR_LEN);
			memset(netmask,0,IPV4_STR_LEN);
			huayi_json_get_str(route_obj,"target",target,IPV4_STR_LEN);
			huayi_json_get_str(route_obj,"subnet_mask",netmask,IPV4_STR_LEN);
			
			huayi_uci_add_section(PKG_VPN_USER,"route");
			snprintf(section,sizeof(section),"@route[%d]",j + route_num);
			
			huayi_uci_set_str(PKG_VPN_USER,section,"target",target);
			huayi_uci_set_str(PKG_VPN_USER,section,"netmask",netmask);
			huayi_uci_set_str(PKG_VPN_USER,section,"vpn_username",ppp_username);
		}
		route_num += route_size;
		
	}
	
	huayi_uci_commit(PKG_VPN_USER);
	
	system("huayi-reset vpn_user");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_vpn_user_status(cJSON *request_root, cJSON *response_root, int *lock_time)
{
	char account_type[OPTION_STR_LEN] = {0};
	int pid = 0;
	char ip[IPV4_STR_LEN] = {0};
	char device_ip[IPV4_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	int uptime = 0;
	char buff[TEMP_STR_LEN] = {0};
	FILE * fp = NULL;
	cJSON * result_root = NULL;
	cJSON * user_list = NULL;
	cJSON * user_obj = NULL;
	unsigned long time_of_day = 0;
	unsigned long join_time;
	int cur_uptime = 0;
	time_t time_tmp;
	struct tm *p;
	FCGI_BOOL file_exist = FCGI_FALSE;
	
	if((NULL == request_root) || (NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	fp = fopen("/tmp/vpn_status", "r");
	
	if (NULL == fp)
	{
		file_exist = FCGI_FALSE;
	}else
	{
		file_exist = FCGI_TRUE;
	}
	
	result_root = cJSON_CreateObject();

	user_list = cJSON_CreateArray();
	
	if(FCGI_TRUE == file_exist)
	{
		huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&cur_uptime);
		
		huayi_uci_get_long(PKG_STATUS_SYSTEM,"sys","time_of_day",&time_of_day);
		
		while(fgets(buff, sizeof(buff), fp) != NULL) 
		{
		
			sscanf(buff,"%s %d %*s %s %s %s %d",account_type,&pid,device_ip,ip,username,&uptime);
			user_obj = cJSON_CreateObject();
			cJSON_AddStringToObject(user_obj,"account",username);
			cJSON_AddStringToObject(user_obj,"account_type",str2upper(account_type));
			cJSON_AddNumberToObject(user_obj,"pid",pid);
			cJSON_AddStringToObject(user_obj,"ip_addr",ip);
			cJSON_AddStringToObject(user_obj,"device_addr",device_ip);
			time_tmp = (time_t)(time_of_day-(cur_uptime -uptime));	
			p = localtime(&time_tmp);
			join_time = time_tmp + p->tm_gmtoff - 28800;  

			cJSON_AddNumberToObject(user_obj,"jointime",join_time);
			cJSON_AddItemToArray(user_list,user_obj);
		}
		fclose(fp);
		fp = NULL;
	}

	cJSON_AddItemToObject(result_root,"usr_arr",user_list);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
	
}

int do_kick_vpn_user(cJSON *request_root, cJSON *response_root, int *lock_time)
{	
	int pid = 0;
	char cmd[CMD_STR_LEN] = {0};
	
	if(NULL == request_root || NULL == response_root || NULL == lock_time)
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"pid",&pid))
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	snprintf(cmd,sizeof(cmd),"kill %d",pid);

	system(cmd);

	return FCGI_RESP_CODE_SUCCESS;
	
}



int get_openvpn_config(cJSON *request_root, cJSON *response_root, int *lock_time){


	if((NULL == request_root) || (NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}
	int enable,port,status = 0,option;
	char host_name[CMD_STR_LEN],user_name[CMD_STR_LEN],pwd[CMD_STR_LEN],dev[CMD_STR_LEN];
	cJSON * result_root = NULL;
	char cmd[TEMP_STR_LEN] = {0};
	char value[OPTION_STR_LEN] = { 0 };
	huayi_uci_get_int(PKG_PPL2TP,"openvpn","option",&option);
	huayi_uci_get_int(PKG_PPL2TP,"openvpn","enable",&enable);
	result_root = cJSON_CreateObject();

	if(!option)
	{

		
		huayi_uci_get_int(PKG_PPL2TP,"openvpn","port",&port);
		huayi_uci_get_str(PKG_PPL2TP,"openvpn","host_name",host_name);
		huayi_uci_get_str(PKG_PPL2TP,"openvpn","username",user_name);
		huayi_uci_get_str(PKG_PPL2TP,"openvpn","pwd",pwd);
		CON_LOG ("%s %s %s %d %d %d %d",host_name,user_name,pwd,enable,port,option,status);



		cJSON_AddStringToObject(result_root,"host_name",host_name);
		cJSON_AddStringToObject(result_root,"usr",user_name);
		cJSON_AddStringToObject(result_root,"pwd",pwd);
		cJSON_AddNumberToObject(result_root,"port",port);
	}
	


		huayi_uci_get_str(PKG_PPL2TP,"openvpn","dev",dev);
		if(enable){
		memset(cmd, 0, TEMP_STR_LEN);
		memset(value, 0, sizeof(value));
		sprintf(cmd, "ifconfig | grep %s | awk '{print $1}'",dev);
		get_cmd_result(cmd, value, sizeof(value));
		CON_LOG("%s",value);
		if(strlen(value)<strlen(dev)){
			status = 0;CON_LOG ("000");
		}else{
			status = 1;CON_LOG ("111");
		}
		 }else{
				status = 2;
				CON_LOG ("222");
		 	}
	cJSON_AddNumberToObject(result_root,"enable",enable);
	cJSON_AddNumberToObject(result_root,"status",status);
	cJSON_AddNumberToObject(result_root,"option",option);
	CON_LOG ("%s %s %s %d %d %d %d",host_name,user_name,pwd,enable,port,option,status);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;

}




int set_openvpn_config(cJSON *request_root, cJSON *response_root, int *lock_time){

	if((NULL == request_root) || (NULL == response_root) || (NULL == lock_time))
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	int enable, port,status,option;
	char host_name[CMD_STR_LEN],user_name[CMD_STR_LEN],pwd[CMD_STR_LEN];



	if(!huayi_json_get_str(request_root,"host_name",host_name,CMD_STR_LEN)||
	!huayi_json_get_str(request_root,"usr",user_name,CMD_STR_LEN)||
	!huayi_json_get_str(request_root,"pwd",pwd,CMD_STR_LEN)||
	!huayi_json_get_int(request_root,"enable",&enable)||
	!huayi_json_get_int(request_root,"port",&port)||
	!huayi_json_get_int(request_root,"option",&option)){
	

			return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!option)
	{
		huayi_uci_set_int(PKG_PPL2TP,"openvpn","option",option);
		huayi_uci_set_int(PKG_PPL2TP,"openvpn","enable",enable);
		huayi_uci_set_int(PKG_PPL2TP,"openvpn","port",port);
		huayi_uci_set_str(PKG_PPL2TP,"openvpn","host_name",host_name);
		huayi_uci_set_str(PKG_PPL2TP,"openvpn","username",user_name);
		huayi_uci_set_str(PKG_PPL2TP,"openvpn","pwd",pwd);
		huayi_uci_commit(PKG_PPL2TP);
	}

	

	system("huayi-reset openvpn");
	*lock_time = HUAYI_RELOAD_VPN_SERVER_TIME;
	return FCGI_RESP_CODE_SUCCESS;

}




