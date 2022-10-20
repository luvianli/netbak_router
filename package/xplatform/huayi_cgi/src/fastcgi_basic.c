
#include "fastcgi_common.h"


int get_basic_page_status_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	char product_name[OPTION_STR_LEN] = {0};
	char company_website[OPTION_STR_LEN] = {0};
	char software_version[OPTION_STR_LEN]={0};
	char buildtime[OPTION_STR_LEN] = {0};
	char time_of_day[OPTION_STR_LEN]={0};
	int sys_mode = 0;
	int rep_status = 0, rep_24g_status = 0, rep_58g_status = 0;
	unsigned long sta_num = 0, sta_24g_num = 0, sta_58g_num = 0;
	int uptime = 0;
	int cpu_rt_usage = 0;
	int mem_rt_usage = 0;
	int flash_usage = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();
	
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_name",product_name);
	cJSON_AddStringToObject(result_root,"pro_name",product_name);

	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version",software_version);
	cJSON_AddStringToObject(result_root,"fw_version",software_version);

	huayi_uci_get_str(PKG_ABOUT,"about_software","buildtime",buildtime);
	cJSON_AddStringToObject(result_root,"release_time",buildtime);
	
	huayi_uci_get_str(PKG_ABOUT,"about_system","company_website",company_website);
	cJSON_AddStringToObject(result_root,"company_domain",company_website);

	get_current_time(time_of_day);
	cJSON_AddStringToObject(result_root,"time_day",time_of_day);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);
	cJSON_AddNumberToObject(result_root,"uptime",uptime);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","cpu_rt_usage",&cpu_rt_usage);
	cJSON_AddNumberToObject(result_root,"cpu_usage",cpu_rt_usage);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","mem_rt_usage",&mem_rt_usage);	
	cJSON_AddNumberToObject(result_root,"mem_usage",mem_rt_usage);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","flash_usage",&flash_usage);	
	cJSON_AddNumberToObject(result_root,"flash_usage",flash_usage);

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);
	cJSON_AddNumberToObject(result_root,"sys_mode",sys_mode);

	if (sys_mode == HUAYI_REP_MODE || sys_mode == HUAYI_WISP_MODE)
	{
		huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_24g_status",&rep_24g_status);	
		
		huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_58g_status",&rep_58g_status);	
	}
	rep_status = rep_24g_status|rep_58g_status;
	cJSON_AddNumberToObject(result_root,"rep_status",rep_status);

	get_wifi_stamacnum("ra0",0,&sta_24g_num);
	get_wifi_stamacnum("rai0",1,&sta_58g_num);
	sta_num = sta_24g_num + sta_58g_num;
	cJSON_AddNumberToObject(result_root,"sta_num",sta_num);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int get_basic_page_all_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	cJSON *all_obj = NULL;
	int sys_mode = 0;
	int rep_status = 0, rep_24g_status = 0, rep_58g_status = 0;
	int linkport = 0,apmode = 0;
	unsigned long sta_num = 0, sta_24g_num = 0,sta_58g_num = 0;
	//char run_cmd[CMD_STR_LEN]= {0};
	char mac_address[MAC_STR_LEN] = {0};
	char ip_address[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char gateway[IPV4_STR_LEN] = {0};
	char ssid_24g[OPTION_STR_LEN] = {0},ssid_58g[OPTION_STR_LEN] = {0};
	char encryption_24g[OPTION_STR_LEN] = {0},encryption_58g[OPTION_STR_LEN] = {0};
	char key_24g[TEMP_STR_LEN] = {0},key_58g[TEMP_STR_LEN] = {0};
	char section[OPTION_STR_LEN] = {0},proto[OPTION_STR_LEN] = {0};
	char ap_lanip[SHORT_STR_LEN] = {0};
	char product_name[OPTION_STR_LEN] = {0};
	char fw_version[OPTION_STR_LEN] = {0};
	char run_cmd[CMD_STR_LEN]= {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();
	
	all_obj = cJSON_CreateObject();

	huayi_uci_get_str(PKG_ABOUT,"about_system","product_name",product_name);
	cJSON_AddStringToObject(all_obj,"pro_name",product_name);
	
	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version",fw_version);
	cJSON_AddStringToObject(all_obj,"fw_version",fw_version);
	
	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);
	cJSON_AddNumberToObject(all_obj,"sys_mode",sys_mode);

	if (sys_mode == HUAYI_REP_MODE || sys_mode == HUAYI_WISP_MODE)
	{
		huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_24g_status",&rep_24g_status);	
		
		huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_58g_status",&rep_58g_status);	
	}
	rep_status = rep_24g_status|rep_58g_status;
	cJSON_AddNumberToObject(all_obj,"rep_status",rep_status);

	if (*lock_time >= FASTCGI_UPDATE_INFO_INTERVAL)
	{
		get_wifi_stamacnum("ra0",0,&sta_24g_num);
		snprintf(run_cmd,CMD_STR_LEN,"huayi_cmd set system wifi_24g_num %d",sta_24g_num);	
		system(run_cmd);

		get_wifi_stamacnum("rai0",1,&sta_58g_num);
		snprintf(run_cmd,CMD_STR_LEN,"huayi_cmd set system wifi_58g_num %d",sta_58g_num);	
		system(run_cmd);

		*lock_time = FASTCGI_UPDATE_INFO_FLAG;
	}
	else
	{
		huayi_uci_get_int(PKG_STATUS_SYSTEM, "sys", "wifi_24g_num",&sta_24g_num);
		huayi_uci_get_int(PKG_STATUS_SYSTEM, "sys", "wifi_58g_num",&sta_58g_num);
	}

	//get_wifi_stamacnum("ra0",0,&sta_24g_num);
	//get_wifi_stamacnum("rai0",1,&sta_58g_num);
	sta_num = sta_24g_num + sta_58g_num;
	cJSON_AddNumberToObject(all_obj,"sta_num",sta_num);

	cJSON_AddItemToObject(result_root,"status_info",all_obj);

	
	all_obj = cJSON_CreateObject();
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","mac_address",mac_address);
	cJSON_AddStringToObject(all_obj,"mac_addr",mac_address);
	
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",ip_address);
	cJSON_AddStringToObject(all_obj,"ip_addr",ip_address);

	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","netmask",netmask);
	cJSON_AddStringToObject(all_obj,"subnet_mask",netmask);

	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","gateway",gateway);
	cJSON_AddStringToObject(all_obj,"gateway",gateway);
	cJSON_AddItemToObject(result_root,"lan_info",all_obj);


	all_obj = cJSON_CreateObject();
	huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_24g);
	huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"encryption",encryption_24g);
	huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"key",key_24g);
	
	huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_58g);
	huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"encryption",encryption_58g);
	huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"key",key_58g);
	
	cJSON_AddStringToObject(all_obj,"ssid_24g",ssid_24g);
	cJSON_AddStringToObject(all_obj,"key_24g",key_24g);

	if (strncmp(encryption_24g,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(all_obj,"encryption_24g",0);
	}
	else
	{
		cJSON_AddNumberToObject(all_obj,"encryption_24g",1);
	}
	
	cJSON_AddStringToObject(all_obj,"ssid_58g",ssid_58g);
	cJSON_AddStringToObject(all_obj,"key_58g",key_58g);

	if (strncmp(encryption_58g,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(all_obj,"encryption_58g",0);
	}
	else
	{
		cJSON_AddNumberToObject(all_obj,"encryption_58g",1);
	}
	cJSON_AddItemToObject(result_root,"wifi_info",all_obj);

	all_obj = cJSON_CreateObject();
	if (sys_mode == HUAYI_REP_MODE)
	{
		snprintf(section,sizeof(section),"wwan");
	}
	else 
	{
		snprintf(section,sizeof(section),"lan");
	}
	huayi_uci_get_str(PKG_NETWORK,section,"proto",proto);
	cJSON_AddStringToObject(all_obj,"net_mode",proto);
	
	huayi_uci_get_int(PKG_STATUS_NETWORK,"lan","linkport",&linkport);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",ap_lanip);
	if(0 != linkport && strlen(ap_lanip)>=7 && strcmp("0.0.0.0", ap_lanip) != 0)
	{
		cJSON_AddNumberToObject(all_obj,"net_link",1);
	}
	else
	{
		cJSON_AddNumberToObject(all_obj,"net_link",0);
	}
	
	huayi_uci_get_int(PKG_THINAP,"global","enable",&apmode);
	cJSON_AddNumberToObject(all_obj,"ap_mode",apmode);
	
	cJSON_AddItemToObject(result_root,"quick_info",all_obj);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int set_basic_page_wifi_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	
	char	ssid_24g[OPTION_STR_LEN] = {0},key_24g[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	char 	authmode_24g[OPTION_STR_LEN] = {0};
	char 	cipher_24g[OPTION_STR_LEN] = {0};
	char    encryption_24g[OPTION_STR_LEN] = {0};
#else
	int 	encryption_24g = 0;
#endif

	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int		bandsteering = 0;
	char	ssid_58g[OPTION_STR_LEN] = {0},key_58g[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	char 	authmode_58g[OPTION_STR_LEN] = {0};
	char	cipher_58g[OPTION_STR_LEN] = {0};
	char    encryption_58g[OPTION_STR_LEN] = {0};
#else
	int		encryption_58g = 0;
#endif
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_str(request_root,"ssid_24g",ssid_24g,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	if(!huayi_json_get_str(request_root,"authmode_24g",authmode_24g,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"cipher_24g",cipher_24g,OPTION_STR_LEN))
	{
		//return FCGI_RESP_CODE_DATA_MISS;
	}
#else
	if(!huayi_json_get_int(request_root,"encryption_24g",&encryption_24g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
#endif	

	if(!huayi_json_get_str(request_root,"key_24g",key_24g,TEMP_STR_LEN))
	{
		;
	}
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_str(request_root,"ssid_58g",ssid_58g,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	if(!huayi_json_get_str(request_root,"authmode_58g",authmode_58g,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"cipher_58g",cipher_58g,OPTION_STR_LEN))
	{
		//return FCGI_RESP_CODE_DATA_MISS;
	}
#else
	if(!huayi_json_get_int(request_root,"encryption_58g",&encryption_58g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}	
#endif


	if(!huayi_json_get_str(request_root,"key_58g",key_58g,TEMP_STR_LEN))
	{
		;
	}
#endif

	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_24g);
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	if(strcmp(authmode_24g,"disable") == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W24G_IFACE,"key");
	}else
	{
		if(strcmp(authmode_24g,"WPAPSK") == 0)
		{
			strcpy(encryption_24g,"psk");
		}else if(strcmp(authmode_24g,"WPA2PSK") == 0)
		{
			strcpy(encryption_24g,"psk2");
		}else
		{
			strcpy(encryption_24g,"psk-mixed");
		}

		if(strcmp(cipher_24g,"AES") == 0)
		{
			strcat(encryption_24g,"+ccmp");
		}else if(strcmp(cipher_24g,"TKIP") == 0)
		{
			strcat(encryption_24g,"+tkip");
		}else
		{
			strcat(encryption_24g,"+ccmp+tkip");
		}

		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption",encryption_24g);
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"key",key_24g);
	}
#else
	if (encryption_24g == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W24G_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","mixed-psk+tkip+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"key",key_24g);
	}
#endif
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_58g);

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	if(strcmp(authmode_58g,"disable") == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W58G_IFACE,"key");
	}else
	{
		if(strcmp(authmode_58g,"WPAPSK") == 0)
		{
			strcpy(encryption_58g,"psk");
		}else if(strcmp(authmode_58g,"WPA2PSK") == 0)
		{
			strcpy(encryption_58g,"psk2");
		}else
		{
			strcpy(encryption_58g,"psk-mixed");
		}

		if(strcmp(cipher_58g,"AES") == 0)
		{
			strcat(encryption_58g,"+ccmp");
		}else if(strcmp(cipher_58g,"TKIP") == 0)
		{
			strcat(encryption_58g,"+tkip");
		}else if(strcmp(cipher_58g,"TKIPAES") == 0)
		{
			strcat(encryption_58g,"+ccmp+tkip");
		} else 
		{
			/*2.4G 5.8G same.*/
			strcpy(encryption_58g,encryption_24g);
		}

		/*if (strcmp(ssid_24g, ssid_58g) == 0 && strcmp(key_24g, key_58g) == 0)
		{
			memset(encryption_58g,0,sizeof(encryption_58g));
			strcpy(encryption_58g,encryption_24g);
		}*/

		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption",encryption_58g);
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"key",key_58g);
	}

	if (strcmp(encryption_24g, encryption_58g) == 0)
	{
		if (strcmp(ssid_24g, ssid_58g) == 0 && strcmp(key_24g, key_58g) == 0 &&
			strcmp(authmode_58g, authmode_24g) == 0 && strcmp(cipher_58g, cipher_24g) == 0)
		{
			bandsteering = 1;
		}
	}
#else
	if (encryption_58g == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W58G_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","mixed-psk+tkip+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"key",key_58g);
	}

	if (encryption_24g == encryption_58g)
	{
		if (strcmp(ssid_24g, ssid_58g) == 0 && (encryption_24g == 0 || strcmp(key_24g, key_58g) == 0))
		{
			bandsteering = 1;
		}
	}
#endif
	
	huayi_uci_set_int(PKG_WIRELESS,"radio0","bandsteering",bandsteering);
	huayi_uci_set_int(PKG_WIRELESS,"radio1","bandsteering",bandsteering);
#endif
	
	huayi_uci_commit(PKG_WIRELESS);
		
	system("huayi-reset wireless");
	
	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");
	
	return FCGI_RESP_CODE_SUCCESS;
}

