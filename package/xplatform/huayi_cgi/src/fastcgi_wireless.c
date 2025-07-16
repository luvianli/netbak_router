#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT)
#include "fastcgi_common.h"
#include "oid.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

int get_wifi_stamacnum(char *vap_ifname, int radio_id, unsigned long *stanum)
{
	int socket_id = 0;
    struct iwreq iwr;
	RT_802_11_7603_MAC_TABLE table_24G = {0};
	RT_802_11_7663_MAC_TABLE table_58G = {0};
	int disabled = 0;
	
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_id < 0) 
	{
		return -1;
	}
	
	strncpy(iwr.ifr_name, vap_ifname, IFNAMSIZ);

	if (radio_id == 0)
	{
		iwr.u.data.pointer = (caddr_t) &table_24G;
		if(!huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&disabled))
		{
			return FCGI_FALSE;
		}
	}
	else
	{
		iwr.u.data.pointer = (caddr_t) &table_58G;
		if(!huayi_uci_get_int(PKG_WIRELESS,"radio1","disabled",&disabled))
		{
			return FCGI_FALSE;
		}
	}

	if(disabled == 1)
	{
		return FCGI_FALSE;
	}
	
	if (ioctl(socket_id, 0x8bef, &iwr) < 0) 
	{
		close(socket_id);
		return -1;
	}

	if (radio_id == 0)
	{
		*stanum = table_24G.Num;
	}
	else
	{
		*stanum = table_58G.Num;
	}
	
	close(socket_id);

	return 0;
}


int get_wifi_basic_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*result_root = NULL;
	char vap_section[OPTION_STR_LEN] = {0}, radio_section[OPTION_STR_LEN] = {0};	
	char ssid[OPTION_STR_LEN] = {0}, key[TEMP_STR_LEN] = {0};
	char encryption[OPTION_STR_LEN] = {0};
	int radio_id = 0, radio_enable = 0, hwmode = 0, encryp_type = 0;
	int band = 0, hidden = 0, channel = 0, txpower = 0;
	int disabled = 0, bandsteering = 0, noforward = 0, maxstanum = 0;
#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	char authmode[OPTION_STR_LEN] = {0};
	char cipher[OPTION_STR_LEN] = {0};
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
		
	if(!huayi_json_get_int(request_root,"wifi_radio",&radio_id))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	snprintf(radio_section,OPTION_STR_LEN,"radio%d",radio_id);
	snprintf(vap_section,OPTION_STR_LEN,"radio%d_0",radio_id);

	result_root = cJSON_CreateObject();
	
	if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"ssid",ssid))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"key",key))
	{
		;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,vap_section,"hidden",&hidden))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,vap_section,"noforward",&noforward))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,vap_section,"maxstanum",&maxstanum))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"encryption",encryption))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"channel",&channel))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"txpower",&txpower))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"wifimode",&hwmode))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"bw",&band))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,vap_section,"disabled",&disabled))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"bandsteering",&bandsteering))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	radio_enable = disabled?0:1;

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	if(strcmp(encryption,"none") == 0)
	{
		strcpy(authmode,"disable");
		strcpy(cipher,"AES");
	}else
	{
		if(strstr(encryption,"psk-mixed"))
		{
			strcpy(authmode,"WPAPSKWPA2PSK");
		}else if(strstr(encryption,"psk2"))
		{
			strcpy(authmode,"WPA2PSK");
		}else
		{
			strcpy(authmode,"WPAPSK");
		}

		if(strstr(encryption,"ccmp") && strstr(encryption,"tkip"))
		{
			strcpy(cipher,"TKIPAES");
		}else if(strstr(encryption,"tkip"))
		{
			strcpy(cipher,"TKIP");
		}else if(strstr(encryption,"ccmp"))
		{
			strcpy(cipher,"AES");
		}else
		{
			strcpy(cipher,"none");
		}
	}	
#endif
	if (strcmp(encryption,"none") == 0)
	{
		encryp_type = 0;
	}
	else
	{
		encryp_type = 1;
	}
	
	cJSON_AddNumberToObject(result_root,"wifi_admin",radio_enable);
	cJSON_AddNumberToObject(result_root,"band_steering",bandsteering);
	
	cJSON_AddNumberToObject(result_root,"hwmode",hwmode);
	cJSON_AddNumberToObject(result_root,"band",band);
	cJSON_AddNumberToObject(result_root,"channel",channel);
	cJSON_AddNumberToObject(result_root,"tx_power",txpower);
	cJSON_AddStringToObject(result_root,"ssid",ssid);
	cJSON_AddStringToObject(result_root,"key",key);
	
	cJSON_AddNumberToObject(result_root,"hidden",hidden);
	cJSON_AddNumberToObject(result_root,"noforward",noforward);
	cJSON_AddNumberToObject(result_root,"max_num",maxstanum);

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	cJSON_AddStringToObject(result_root,"auth_mode",authmode);
	cJSON_AddStringToObject(result_root,"cipher_mode",cipher);
#endif
	
	cJSON_AddNumberToObject(result_root,"encryp",encryp_type);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int set_wifi_basic_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char vap_section[OPTION_STR_LEN] = {0}, radio_section[OPTION_STR_LEN] = {0};	
	char ssid[OPTION_STR_LEN] = {0}, key[TEMP_STR_LEN] = {0};
	int radio_id = 0, radio_enable = 0, hwmode = 0;
	int band = 0, hidden = 0, channel = 0, txpower = 0;
	int disabled = 0, bandsteering = 0, noforward = 0, maxstanum = 0;

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	char authmode[OPTION_STR_LEN] = {0};
	char cipher[OPTION_STR_LEN] = {0};
	char encryption[OPTION_STR_LEN] = {0};
#else
	int  encryp_type = 0;
#endif
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"wifi_radio",&radio_id))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	
	
	snprintf(radio_section,OPTION_STR_LEN,"radio%d",radio_id);
	snprintf(vap_section,OPTION_STR_LEN,"radio%d_0",radio_id);

	if(!huayi_json_get_int(request_root,"hwmode",&hwmode))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"band",&band))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"hidden",&hidden))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"channel",&channel))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
		
	if(!huayi_json_get_str(request_root,"ssid",ssid,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
	if(!huayi_json_get_str(request_root,"auth_mode",authmode,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_str(request_root,"cipher_mode",cipher,OPTION_STR_LEN))
	{
		//return FCGI_RESP_CODE_DATA_MISS;
	}
#else
	if(!huayi_json_get_int(request_root,"encryp",&encryp_type))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
#endif
	
	if(!huayi_json_get_str(request_root,"key",key,TEMP_STR_LEN))
	{
		;
	}
	
	if(!huayi_json_get_int(request_root,"tx_power",&txpower))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"noforward",&noforward))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"max_num",&maxstanum))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wifi_admin",&radio_enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"band_steering",&bandsteering))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	disabled = radio_enable?0:1;

	huayi_uci_set_int(PKG_WIRELESS,radio_section,"disabled",disabled);
	huayi_uci_set_int(PKG_WIRELESS,vap_section,"disabled",disabled);
	
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"channel",channel);

	huayi_uci_set_int(PKG_WIRELESS,radio_section,"txpower",txpower);

	huayi_uci_set_int(PKG_WIRELESS,radio_section,"wifimode",hwmode);

	huayi_uci_set_int(PKG_WIRELESS,radio_section,"bw",band);

	huayi_uci_set_int(PKG_WIRELESS,vap_section,"hidden",hidden);

	huayi_uci_set_int(PKG_WIRELESS,vap_section,"noforward",noforward);

	huayi_uci_set_int(PKG_WIRELESS,vap_section,"maxstanum",maxstanum);
	
	huayi_uci_set_int(PKG_WIRELESS,"radio0","bandsteering",bandsteering);
	huayi_uci_set_int(PKG_WIRELESS,"radio1","bandsteering",bandsteering);
	
	if (bandsteering == 1)
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio0_0","ssid",ssid);

		huayi_uci_set_str(PKG_WIRELESS,"radio1_0","ssid",ssid);

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
		if(strcmp(authmode,"disable") == 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,"radio0_0","key");

			huayi_uci_set_str(PKG_WIRELESS,"radio1_0","encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,"radio1_0","key");
		}else
		{
			if(strcmp(authmode,"WPAPSK") == 0)
			{
				strcpy(encryption,"psk");
			}else if(strcmp(authmode,"WPA2PSK") == 0)
			{
				strcpy(encryption,"psk2");
			}else
			{
				strcpy(encryption,"psk-mixed");
			}

			if(strcmp(cipher,"AES") == 0)
			{
				strcat(encryption,"+ccmp");
			}else if(strcmp(cipher,"TKIP") == 0)
			{
				strcat(encryption,"+tkip");
			}else
			{
				strcat(encryption,"+ccmp+tkip");
			}

			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","encryption",encryption);
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","key",key);

			huayi_uci_set_str(PKG_WIRELESS,"radio1_0","encryption",encryption);
			huayi_uci_set_str(PKG_WIRELESS,"radio1_0","key",key);
		}
#else
		if(encryp_type == 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,"radio0_0","key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","encryption","psk-mixed");
			huayi_uci_set_str(PKG_WIRELESS,"radio0_0","key",key);
		}

		huayi_uci_set_str(PKG_WIRELESS,"radio1_0","ssid",ssid);

		if(encryp_type == 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,"radio1_0","encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,"radio1_0","key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,"radio1_0","encryption","psk-mixed");
			huayi_uci_set_str(PKG_WIRELESS,"radio1_0","key",key);
		}
#endif
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,vap_section,"ssid",ssid);

#if defined(CONFIG_HUAYI_SUPPORT_PASSWORD_OPTION)
		if(strcmp(authmode,"disable") == 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,vap_section,"key");
		}else
		{
			if(strcmp(authmode,"WPAPSK") == 0)
			{
				strcpy(encryption,"psk");
			}else if(strcmp(authmode,"WPA2PSK") == 0)
			{
				strcpy(encryption,"psk2");
			}else
			{
				strcpy(encryption,"psk-mixed");
			}

			if(strcmp(cipher,"AES") == 0)
			{
				strcat(encryption,"+ccmp");
			}else if(strcmp(cipher,"TKIP") == 0)
			{
				strcat(encryption,"+tkip");
			}else
			{
				strcat(encryption,"+ccmp+tkip");
			}

			huayi_uci_set_str(PKG_WIRELESS,vap_section,"encryption",encryption);
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"key",key);
		}
#else
		if(encryp_type == 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,vap_section,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"encryption","psk-mixed");
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"key",key);
		}
#endif
	}
	
	huayi_uci_commit(PKG_WIRELESS);
	
	system("huayi-reset wireless");

	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");

	return FCGI_RESP_CODE_SUCCESS;
}



int get_wifi_easyconf(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	cJSON *wifi2g_obj = NULL, *wifi5g_obj = NULL;
	int bndstrg_enable = 0, enable_2g = 0, enable_5g = 0, hidden_2g = 0, hidden_5g = 0;
	char ssid_2g[OPTION_STR_LEN] = { 0 }, authmode_2g[SHORT_STR_LEN] = { 0 }, wifikey_2g[OPTION_STR_LEN] = { 0 };
	char ssid_5g[OPTION_STR_LEN] = { 0 }, authmode_5g[SHORT_STR_LEN] = { 0 }, wifikey_5g[OPTION_STR_LEN] = { 0 };
	char section[OPTION_STR_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	wifi5g_obj = cJSON_CreateObject();
	wifi2g_obj = cJSON_CreateObject();
	huayi_uci_get_int(PKG_WIRELESS,"radio1","bandsteering",&bndstrg_enable);
	huayi_uci_get_int(PKG_WIRELESS,"radio0","radio",&enable_2g);
	huayi_uci_get_int(PKG_WIRELESS,"radio1","radio",&enable_5g);
	/*2.4g*/
	snprintf(section,sizeof(section),"@wifi-iface[%d]", 0);
	huayi_uci_get_str(PKG_WIRELESS,section,"ssid", ssid_2g);
	huayi_uci_get_str(PKG_WIRELESS,section,"authmode", authmode_2g);
	huayi_uci_get_str(PKG_WIRELESS,section,"key", wifikey_2g);
	huayi_uci_get_int(PKG_WIRELESS,section,"hidden", &hidden_2g);
	/*5g*/
	memset(section, 0, sizeof(section));
	snprintf(section,sizeof(section),"@wifi-iface[%d]", 2);
	huayi_uci_get_str(PKG_WIRELESS, section, "ssid", ssid_5g);
	huayi_uci_get_str(PKG_WIRELESS, section, "authmode", authmode_5g);
	huayi_uci_get_str(PKG_WIRELESS, section, "key", wifikey_5g);
	huayi_uci_get_int(PKG_WIRELESS,section,"hidden", &hidden_5g);
	huayi_add_switch_to_json(result_root,"enable5gpro", bndstrg_enable);
	huayi_add_switch_to_json(result_root,"enable2gwifi", enable_2g);
	
	cJSON_AddStringToObject(wifi2g_obj,"ssid", ssid_2g);
	cJSON_AddStringToObject(wifi2g_obj,"authmode", authmode_2g);
	cJSON_AddStringToObject(wifi2g_obj,"key", wifikey_2g);
	cJSON_AddNumberToObject(wifi2g_obj,"hidden", hidden_2g);
	cJSON_AddItemToObject(result_root, "ssid2g", wifi2g_obj);
	huayi_add_switch_to_json(result_root,"enable5gwifi", enable_5g);
	
	cJSON_AddStringToObject(wifi5g_obj,"ssid", ssid_5g);
	cJSON_AddStringToObject(wifi5g_obj,"authmode", authmode_5g);
	cJSON_AddStringToObject(wifi5g_obj,"key", wifikey_5g);
	cJSON_AddNumberToObject(wifi5g_obj,"hidden", hidden_5g);
	cJSON_AddItemToObject(result_root, "ssid5g", wifi5g_obj);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_easyconf(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int bndstrg_enable = 0;
	int enable_2g = 0, enable_5g = 0, hidden_2g = 0, hidden_5g = 0;
	cJSON *wifi_obj_2g = NULL, *wifi_obj_5g = NULL;
	char ssid_2g[OPTION_STR_LEN] = { 0 }, authmode_2g[SHORT_STR_LEN] = { 0 }, wifikey_2g[OPTION_STR_LEN] = { 0 };
	char ssid_5g[OPTION_STR_LEN] = { 0 }, authmode_5g[SHORT_STR_LEN] = { 0 }, wifikey_5g[OPTION_STR_LEN] = { 0 };
	char encryption_2g[OPTION_STR_LEN] = { 0 }, encryption_5g[OPTION_STR_LEN] = { 0 };
	char section[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"enable5gpro",&bndstrg_enable) ||
		!huayi_json_get_int(request_root,"enable2gwifi",&enable_2g) ||
		!huayi_json_get_int(request_root,"enable5gwifi",&enable_5g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_uci_set_int(PKG_WIRELESS,"radio1","bandsteering", bndstrg_enable);
	huayi_uci_set_int(PKG_WIRELESS,"radio0","radio",enable_2g); 
	huayi_uci_set_int(PKG_WIRELESS,"radio1","radio",enable_5g);
	if(!(wifi_obj_2g = cJSON_GetObjectItem(request_root,"ssid2g")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_json_get_str(wifi_obj_2g, "ssid", ssid_2g, OPTION_STR_LEN);
	huayi_json_get_str(wifi_obj_2g, "authmode", authmode_2g, SHORT_STR_LEN);
	huayi_json_get_str(wifi_obj_2g, "key", wifikey_2g, OPTION_STR_LEN);
	huayi_json_get_int(wifi_obj_2g, "hidden", &hidden_2g);
	if(!strcmp(authmode_2g, "none"))
		snprintf(encryption_2g, sizeof(encryption_2g), "%s", authmode_2g);
	else
		snprintf(encryption_2g, sizeof(encryption_2g), "%s_ccmp+tkip", authmode_2g);
	snprintf(section,sizeof(section),"@wifi-iface[%d]", 0);
	huayi_uci_set_str(PKG_WIRELESS,section,"ssid", ssid_2g);
	huayi_uci_set_str(PKG_WIRELESS,section,"encryption", encryption_2g);
	huayi_uci_set_str(PKG_WIRELESS,section,"authmode", authmode_2g);
	huayi_uci_set_str(PKG_WIRELESS,section,"key", wifikey_2g);
	huayi_uci_set_int(PKG_WIRELESS,section,"hidden", hidden_2g);
	if(1 == bndstrg_enable) {   /*2g��5g config same*/
		memset(section, 0, sizeof(section));
		/*section 2 is 5g main ssid*/
		snprintf(section,sizeof(section),"@wifi-iface[%d]", 2); 
		huayi_uci_set_str(PKG_WIRELESS,section,"ssid", ssid_2g);
		huayi_uci_set_str(PKG_WIRELESS,section,"encryption", encryption_2g);
		huayi_uci_set_str(PKG_WIRELESS,section,"key", wifikey_2g);
		huayi_uci_set_str(PKG_WIRELESS,section,"authmode", authmode_2g);
		huayi_uci_set_int(PKG_WIRELESS,section,"hidden", hidden_2g);
	} else {   //set 5g config
		if(!(wifi_obj_5g = cJSON_GetObjectItem(request_root,"ssid5g")))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		huayi_json_get_str(wifi_obj_5g, "ssid", ssid_5g, OPTION_STR_LEN);
		if(strcmp(ssid_5g,ssid_2g)==0)
		{
			snprintf(ssid_5g,sizeof(ssid_2g),"%s-5G",ssid_2g);
		}
		huayi_json_get_str(wifi_obj_5g, "authmode", authmode_5g, SHORT_STR_LEN);
		huayi_json_get_str(wifi_obj_5g, "key", wifikey_5g, OPTION_STR_LEN);
		huayi_json_get_int(wifi_obj_5g, "hidden", &hidden_5g);
		if(!strcmp(authmode_5g, "none"))
			snprintf(encryption_5g, sizeof(encryption_5g), "%s", authmode_5g);
		else
			snprintf(encryption_5g, sizeof(encryption_5g), "%s_ccmp+tkip", authmode_5g);
		memset(section, 0, sizeof(section));
		/*section 2 is 5g main ssid*/
		snprintf(section,sizeof(section),"@wifi-iface[%d]", 2);
		huayi_uci_set_str(PKG_WIRELESS,section,"ssid", ssid_5g);
		huayi_uci_set_str(PKG_WIRELESS,section,"encryption", encryption_5g);
		huayi_uci_set_str(PKG_WIRELESS,section,"authmode", authmode_5g);
		huayi_uci_set_str(PKG_WIRELESS,section,"key", wifikey_5g);
		huayi_uci_set_int(PKG_WIRELESS,section,"hidden", hidden_5g);
	}

	huayi_uci_commit(PKG_WIRELESS);
	system("huayi-reset wireless");

	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_wifi_sigmode(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	int txpower = 0;
	int sigmode = 1;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	
	huayi_uci_get_int(PKG_WIRELESS,"radio0","txpower",&txpower);
	switch (txpower) {
		case 100:
			sigmode = 1;
			break;
		case 75:
			sigmode = 2;
			break;
		case 25:
			sigmode = 3;
			break;
		default:
			break;
	}
	cJSON_AddNumberToObject(result_root,"wifisigmode", sigmode);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_sigmode(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int txpower = 100;
	int sigmode = 1;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wifisigmode",&sigmode))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	switch (sigmode) {
		case 1:
			txpower = 100;
			break;
		case 2:
			txpower = 75;
			break;
		case 3:
			txpower = 25;
			break;
		default:
			break;
	}

	huayi_uci_set_int(PKG_WIRELESS,"radio0","txpower",txpower); 
	huayi_uci_set_int(PKG_WIRELESS,"radio1","txpower",txpower);
	huayi_uci_commit(PKG_WIRELESS);
	system("huayi-reset wireless");

	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");

	return FCGI_RESP_CODE_SUCCESS;
}


int get_wifi_advance(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	char radio_section[OPTION_STR_LEN] = {0};
	int radio_id = 0, bgprotect = 0, beacon = 0, dtim = 0;
	int rtsthres = 0, fragthres = 0, txburst = 0, txpreamble = 0; 
	int wmm = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"wifi_radio",&radio_id))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	snprintf(radio_section,OPTION_STR_LEN,"radio%d",radio_id);
	
	result_root = cJSON_CreateObject();
	
	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"bgprotect",&bgprotect))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"beacon",&beacon))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"dtim",&dtim))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"rtsthres",&rtsthres))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"fragthres",&fragthres))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"txburst",&txburst))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"txpreamble",&txpreamble))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,radio_section,"wmm",&wmm))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	cJSON_AddNumberToObject(result_root,"bgprotect_mode", bgprotect);
	cJSON_AddNumberToObject(result_root,"beacon", beacon);
	cJSON_AddNumberToObject(result_root,"dtim", dtim);
	cJSON_AddNumberToObject(result_root,"rts_thres", rtsthres);
	cJSON_AddNumberToObject(result_root,"frag_thres", fragthres);
	cJSON_AddNumberToObject(result_root,"tx_burst", txburst);
	cJSON_AddNumberToObject(result_root,"tx_preamble", txpreamble);
	cJSON_AddNumberToObject(result_root,"wmm", wmm);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_advance(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char radio_section[OPTION_STR_LEN] = {0};
	int radio_id = 0, bgprotect = 0, beacon = 0, dtim = 0;
	int rtsthres = 0, fragthres = 0, txburst = 0, txpreamble = 0; 
	int wmm = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wifi_radio",&radio_id))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	snprintf(radio_section,OPTION_STR_LEN,"radio%d",radio_id);
	
	if(!huayi_json_get_int(request_root,"bgprotect_mode",&bgprotect) ||
		!huayi_json_get_int(request_root,"beacon",&beacon) ||
		!huayi_json_get_int(request_root,"dtim",&dtim) ||
		!huayi_json_get_int(request_root,"rts_thres",&rtsthres) ||
		!huayi_json_get_int(request_root,"frag_thres",&fragthres) ||
		!huayi_json_get_int(request_root,"tx_burst",&txburst) ||
		!huayi_json_get_int(request_root,"tx_preamble",&txpreamble) || 
		!huayi_json_get_int(request_root,"wmm",&wmm))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	 
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"bgprotect", bgprotect);
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"beacon", beacon);
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"dtim", dtim);
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"rtsthres", rtsthres);
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"fragthres", fragthres);
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"txburst", txburst);
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"txpreamble", txpreamble);
	huayi_uci_set_int(PKG_WIRELESS,radio_section,"wmm", wmm); 

	huayi_uci_commit(PKG_WIRELESS);
	system("huayi-reset wireless");

	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");
	return FCGI_RESP_CODE_SUCCESS;
}


int get_wifi_macfilterlist(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL, *mac_array = NULL, *maclistentry = NULL;
	char vap_section[OPTION_STR_LEN] = {0};
	int enable = 0,radio_id = 0;
	char maclist[TEMP_STR_LEN]={0};
	char *p = NULL;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wifi_radio",&radio_id))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	snprintf(vap_section,OPTION_STR_LEN,"radio%d_0",radio_id);
	
	result_root = cJSON_CreateObject();
	mac_array = cJSON_CreateArray();
	
	if(!huayi_uci_get_int(PKG_WIRELESS,vap_section,"macfilterenable",&enable))
	{
		enable = 0;
	}
	
	huayi_uci_get_str(PKG_WIRELESS,vap_section,"maclist",maclist);
	if (strlen(maclist) > 15) {		
		p = strtok(maclist,";");		
		if (p) {			
			maclistentry = cJSON_CreateObject();
			cJSON_AddStringToObject(maclistentry,"mac_addr",p);
			cJSON_AddItemToArray(mac_array,maclistentry);
			while ((p = strtok(NULL,";")) != NULL) {				
				maclistentry = cJSON_CreateObject();
				cJSON_AddStringToObject(maclistentry,"mac_addr",p);
				cJSON_AddItemToArray(mac_array,maclistentry);			
			}		
		}
	}	

	cJSON_AddNumberToObject(result_root,"wifi_admin", enable);
	
	cJSON_AddItemToObject(result_root,"macfilter_arr", mac_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;	
}

int set_wifi_macfilterlist(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *mac_array = NULL, *maclistentry = NULL;
	char vap_section[OPTION_STR_LEN] = {0};
	int enable = 0,radio_id = 0;
	int size = 0, k = 0;
	char mac_addr[MAC_STR_LEN] = {0}, macbuf[LIST_STR_LEN] = { 0 };
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wifi_radio",&radio_id))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wifi_admin",&enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!(mac_array= cJSON_GetObjectItem(request_root,"macfilter_arr")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	size = cJSON_GetArraySize(mac_array);
	for(k=0;k < size;k++)
	{
		maclistentry = cJSON_GetArrayItem(mac_array, k);
		huayi_json_get_str(maclistentry,"mac_addr",mac_addr,MAC_STR_LEN);
		strcat(macbuf, mac_addr);
		strcat(macbuf, ";");
	}
	macbuf[strlen(macbuf)-1] = '\0';

	for(k=0; k<4; k++)
	{
		snprintf(vap_section,OPTION_STR_LEN,"radio%d_%d",radio_id,k);

		huayi_uci_set_int(PKG_WIRELESS,vap_section,"macfilterenable", enable);
		
		huayi_uci_set_str(PKG_WIRELESS,vap_section,"maclist", macbuf);
	}		
	
	huayi_uci_commit(PKG_WIRELESS);
	system("huayi-reset wireless");

	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");
	return FCGI_RESP_CODE_SUCCESS;
}

int get_wifi_stamaclist(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL, *wifi_sta_array = NULL, *maclistentry = NULL;
	char vap_section[OPTION_STR_LEN] = {0},vap_ifname[OPTION_STR_LEN] = {0};
	int i = 0, socket_id = 0, radio_id = 0;
	int bandwidth = 0;
	unsigned long *num = NULL;
    struct iwreq iwr;
	RT_802_11_7603_MAC_TABLE table_24G = {0};
	RT_802_11_7663_MAC_TABLE table_58G = {0};
	char macbuff[MAC_STR_LEN]={0};
	unsigned int stauptime = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"wifi_radio",&radio_id))
	{
		;
	}
	
	snprintf(vap_section,OPTION_STR_LEN,"radio%d_0",radio_id);

	if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"ifname",vap_ifname))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	result_root = cJSON_CreateObject();
	wifi_sta_array = cJSON_CreateArray();
	
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, vap_ifname, IFNAMSIZ);

	if (radio_id == 0)
	{
		iwr.u.data.pointer = (caddr_t) &table_24G;
		num = &table_24G.Num;
	}
	else
	{
		iwr.u.data.pointer = (caddr_t) &table_58G;
		num = &table_58G.Num;
	}
	
	if (socket_id < 0) 
	{
		goto end;
	}
	
	if (ioctl(socket_id, 0x8bef, &iwr) < 0) 
	{
		close(socket_id);
		goto end;
	}
	
	close(socket_id);
	
	for (i = 0;( i < (*num)) && (i < 32); i++) 
	{
		memset(macbuff, 0, sizeof(macbuff));

		if (radio_id == 0)
		{
			sprintf(macbuff,"%02X:%02X:%02X:%02X:%02X:%02X", 
					table_24G.Entry[i].Addr[0], table_24G.Entry[i].Addr[1],
					table_24G.Entry[i].Addr[2], table_24G.Entry[i].Addr[3],
					table_24G.Entry[i].Addr[4], table_24G.Entry[i].Addr[5]);
			bandwidth = table_24G.Entry[i].TxRate.field.BW;
			stauptime = table_24G.Entry[i].ConnectedTime;
		}
		else
		{
			sprintf(macbuff,"%02X:%02X:%02X:%02X:%02X:%02X", 
					table_58G.Entry[i].Addr[0], table_58G.Entry[i].Addr[1],
					table_58G.Entry[i].Addr[2], table_58G.Entry[i].Addr[3],
					table_58G.Entry[i].Addr[4], table_58G.Entry[i].Addr[5]);
			bandwidth = table_58G.Entry[i].TxRate.field.BW;
			stauptime = table_58G.Entry[i].ConnectedTime;
		}
		
		//arplookup_ip(outmac,outip);
		//printf("outip is %s\n",outip);
		//if (!strcmp(outip, "0.0.0.0"))  continue;
		
		maclistentry = cJSON_CreateObject();
		cJSON_AddNumberToObject(maclistentry, "index", i + 1);
		cJSON_AddStringToObject(maclistentry, "bssid", macbuff);
		//cJSON_AddStringToObject(maclistentry, "bandwidth", bandwidth==0?"20M":"40M");	
		cJSON_AddNumberToObject(maclistentry,"stauptime", stauptime);
		cJSON_AddItemToArray(wifi_sta_array,maclistentry);
	}
	
end:
		
	cJSON_AddItemToObject(result_root,"wireless_clients_arr", wifi_sta_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int get_wifi_mbssid_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL, *vap_array = NULL, *vap_entry = NULL;
	char vap_section[OPTION_STR_LEN] = {0};
	int disabled = 0,vap_enable = 0;
	int noforward = 0, encryp_type = 0;
	int radio_id = 0, vap_index = 1;
	char ssid[OPTION_STR_LEN] = {0}, key[TEMP_STR_LEN] = {0};
	char encryption[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"radio_id",&radio_id))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	snprintf(vap_section,OPTION_STR_LEN,"radio%d_%d",radio_id,vap_index);

	if(!huayi_uci_get_int(PKG_WIRELESS,vap_section,"noforward",&noforward))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();
	vap_array = cJSON_CreateArray();
	
	for(vap_index = 1; vap_index < MBSSID_MAX_NUM; vap_index++)
	{
		snprintf(vap_section,OPTION_STR_LEN,"radio%d_%d",radio_id,vap_index);
	
		if(!huayi_uci_get_int(PKG_WIRELESS,vap_section,"disabled",&disabled))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		
		if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"ssid",ssid))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	
		if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"encryption",encryption))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		
		if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"key",key))
		{
			;
		}

		vap_enable = disabled?0:1;
	
		if (strcmp(encryption,"none") == 0)
		{
			encryp_type = 0;
		}
		else
		{
			encryp_type = 1;
		}

		vap_entry = cJSON_CreateObject();
	
		cJSON_AddNumberToObject(vap_entry,"enable",vap_enable);
		
		cJSON_AddStringToObject(vap_entry,"ssid",ssid);
		cJSON_AddStringToObject(vap_entry,"key",key);
		
		cJSON_AddNumberToObject(vap_entry,"encryptype",encryp_type);
					
		cJSON_AddItemToArray(vap_array,vap_entry);	
	}

	cJSON_AddNumberToObject(result_root,"noforward", noforward);
	
	cJSON_AddItemToObject(result_root,"vap_list", vap_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;	
}

int set_wifi_mbssid_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *mac_array = NULL, *maclistentry = NULL;
	char vap_section[OPTION_STR_LEN] = {0};
	int size = 0, k = 0;
	int noforward = 0, encryp_type = 0;
	int disabled = 0, vap_enable = 0;
	int radio_id = 0, vap_index = 1;
	char ssid[OPTION_STR_LEN] = {0}, key[TEMP_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"radio_id",&radio_id))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"noforward",&noforward))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!(mac_array= cJSON_GetObjectItem(request_root,"vap_list")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	size = cJSON_GetArraySize(mac_array);
	for(k=0;k < size;k++)
	{
		snprintf(vap_section,OPTION_STR_LEN,"radio%d_%d",radio_id,vap_index);
		vap_index++;
		
		maclistentry = cJSON_GetArrayItem(mac_array, k);

		if(!huayi_json_get_int(maclistentry,"enable",&vap_enable))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		if(!huayi_json_get_str(maclistentry,"ssid",ssid,OPTION_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		
		if(!huayi_json_get_int(maclistentry,"encryptype",&encryp_type))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		if(!huayi_json_get_str(maclistentry,"key",key,TEMP_STR_LEN))
		{
			;
		}

		disabled = vap_enable?0:1;

		huayi_uci_set_int(PKG_WIRELESS,vap_section,"disabled",disabled);

		huayi_uci_set_int(PKG_WIRELESS,vap_section,"noforward",noforward);

		huayi_uci_set_str(PKG_WIRELESS,vap_section,"ssid",ssid);

		if(encryp_type == 0)
		{
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"encryption","none");
			huayi_uci_delete_option(PKG_WIRELESS,vap_section,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"encryption","psk-mixed");
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"key",key);
		}
		
	}
	
	huayi_uci_commit(PKG_WIRELESS);
	system("huayi-reset wireless");

	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");
	return FCGI_RESP_CODE_SUCCESS;
}

#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7620) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7628) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
int get_wifi_vlan_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL, *vlan_array = NULL;
	cJSON *vlan_entry = NULL;
	cJSON *vlan_index0_entry[VAP_VLAN_MAX_NUM] = {0};
	cJSON *vlan_index0_array[VAP_VLAN_MAX_NUM] = {0};
	cJSON *lan_entry = NULL, *lan_array = NULL;
	char section[OPTION_STR_LEN] = {0};
	int  vlan_index = 0, vlan_id = 0;
	int  radio_index = 0;
	int  lan_index = 0;
	int  index_vid[VAP_VLAN_MAX_NUM] = {0};
#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7620) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
	int  reg94 = 0;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7628)
	int  reg70 = 0;
#endif

	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	for (vlan_index = 0; vlan_index < VAP_VLAN_MAX_NUM; vlan_index++)
	{
		snprintf(section,OPTION_STR_LEN,"vlan_%d",vlan_index);

		vlan_index0_entry[vlan_index] = cJSON_CreateObject();
		vlan_index0_array[vlan_index] = cJSON_CreateArray();
		
		if(!huayi_uci_get_int(PKG_NETWORK,section,"vlanid",&index_vid[vlan_index]))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		
		cJSON_AddNumberToObject(vlan_index0_entry[vlan_index],"vlan_id", index_vid[vlan_index]);
	}
	
	result_root = cJSON_CreateObject();
	vlan_array = cJSON_CreateArray();
	
	for(radio_index = 0; radio_index < RADIO_MAX_NUM; radio_index++)
	{
		snprintf(section,OPTION_STR_LEN,"radio%d_0",radio_index);
	
		if(!huayi_uci_get_int(PKG_WIRELESS,section,"vlanid",&vlan_id))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		if (vlan_id == 0)
			continue;
		
		for (vlan_index = 0; vlan_index < VAP_VLAN_MAX_NUM; vlan_index++)
		{	
			if (index_vid[vlan_index] == vlan_id)
			{
				vlan_entry = cJSON_CreateNumber(radio_index);
				cJSON_AddItemToArray(vlan_index0_array[vlan_index],vlan_entry);
				break;
			}
		}
	}

#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7620) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
	for (vlan_index = 0; vlan_index < VAP_VLAN_MAX_NUM; vlan_index++)
	{
		lan_entry = cJSON_CreateObject();
		snprintf(section,OPTION_STR_LEN,"vlan_%d",vlan_index);
		huayi_uci_get_hex(PKG_NETWORK,section,"reg94",&reg94);
		lan_array = cJSON_CreateArray();
		
		for (lan_index = 0; lan_index < LAN_MAX_NUM; lan_index++)
		{
			if (reg94&(0x1<<(lan_index+16)))
			{
				lan_entry = cJSON_CreateNumber(lan_index - CONFIG_HUAYI_LAN1_PORT_NUM_OFFSET);
				cJSON_AddItemToArray(lan_array,lan_entry);
			}
		}

		cJSON_AddItemToObject(vlan_index0_entry[vlan_index],"lan_list",lan_array);
	}
#endif

#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7628)
	huayi_uci_get_hex(PKG_NETWORK,"lan","reg70",&reg70);
	for (vlan_index = 0; vlan_index < VAP_VLAN_MAX_NUM; vlan_index++)
	{
		lan_entry = cJSON_CreateObject();
		
		lan_array = cJSON_CreateArray();
		
		for (lan_index = 0; lan_index < LAN_MAX_NUM; lan_index++)
		{
			if (reg70&(0x1<<(lan_index+(vlan_index*8))))
			{
				lan_entry = cJSON_CreateNumber(lan_index - CONFIG_HUAYI_LAN1_PORT_NUM_OFFSET);
				cJSON_AddItemToArray(lan_array,lan_entry);
			}
		}

		cJSON_AddItemToObject(vlan_index0_entry[vlan_index],"lan_list",lan_array);
	}
#endif


	for (vlan_index = 0; vlan_index < VAP_VLAN_MAX_NUM; vlan_index++)
	{
		cJSON_AddItemToObject(vlan_index0_entry[vlan_index],"vap_list",vlan_index0_array[vlan_index]);
		cJSON_AddItemToArray(vlan_array,vlan_index0_entry[vlan_index]);
	}
	
	cJSON_AddItemToObject(result_root,"vlan_list", vlan_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;	
}

int set_wifi_vlan_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *vlan_array = NULL, *vlan_entry = NULL;
	cJSON *vap_array = NULL, *vap_entry = NULL;
	cJSON *portlan_array = NULL, *portlan_entry = NULL;
	char vap_section[OPTION_STR_LEN] = {0};
	char lan_section[OPTION_STR_LEN] = {0};
	char vap_network[OPTION_STR_LEN] = {0};
	char vlan_eth[OPTION_STR_LEN] = {0};
	int vap_used[VAP_VLAN_MAX_NUM] = {0};
	int lan_used[LAN_MAX_NUM] = {0};
	int radio_id = 0,lan_id = 0;
	int vlan_id = 0;
	int vlan_size = 0,radio_size = 0,portlan_size = 0;
	int i = 0,k = 0;
#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7620) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
	int lan_pvid = 0x10000, reg94 = 0x10c10001, reg98 = 0x2002;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
	int  reg90 = 0x80001000;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7628)
	int lan1_pvid = 0x1001, lan2_pvid = 0x1001;
	int reg70 = 0x0, reg50 = 0x004003, reg54 = 0x006005;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!(vlan_array= cJSON_GetObjectItem(request_root,"vlan_list")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	vlan_size = cJSON_GetArraySize(vlan_array);
	for(i = 0; i < vlan_size; i++)
	{
		vlan_entry = cJSON_GetArrayItem(vlan_array, i);
		huayi_json_get_int(vlan_entry,"vlan_id",&vlan_id);

		snprintf(vap_network,OPTION_STR_LEN,"vlan_%d",i);

		if (vlan_id == 0)
		{
			huayi_uci_set_int(PKG_NETWORK,vap_network,"vlanid",0);
			huayi_uci_delete_option(PKG_NETWORK,vap_network,"ifname");
			continue;
		}

		if(!(vap_array= cJSON_GetObjectItem(vlan_entry,"vap_list")))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

		radio_size = cJSON_GetArraySize(vap_array);

		for (k = 0; k < radio_size; k++)
		{
			vap_entry = cJSON_GetArrayItem(vap_array, k);
			radio_id = atoi(cJSON_Print(vap_entry));

			snprintf(vap_section,OPTION_STR_LEN,"radio%d_0",radio_id);

			huayi_uci_set_int(PKG_WIRELESS,vap_section,"vlanid",vlan_id);
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"network",vap_network);

			vap_used[radio_id] = 1;
		}

		if(!(portlan_array= cJSON_GetObjectItem(vlan_entry,"lan_list")))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}

#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7620) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
		if (CONFIG_HUAYI_WAN_PORT_NUM == 4)
		{
			reg94 = 0x10d00001;
		}
		else
		{
			reg94 = 0x10c10001;
		}
		portlan_size = cJSON_GetArraySize(portlan_array);
		for (k = 0; k < portlan_size; k++)
		{
			portlan_entry = cJSON_GetArrayItem(portlan_array, k);
			lan_id = atoi(cJSON_Print(portlan_entry)) + CONFIG_HUAYI_LAN1_PORT_NUM_OFFSET;
			lan_used[lan_id] = 1;

			lan_pvid = 0x10000 + vlan_id;
			snprintf(lan_section,OPTION_STR_LEN,"lan%d_pvid",lan_id);
			huayi_uci_set_hex(PKG_NETWORK,"lan",lan_section,lan_pvid);

			reg94 |= 0x1<<(lan_id+16);
		}

#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
		reg90 = 0x80001000;
		reg90 |= vlan_id;
		huayi_uci_set_hex(PKG_NETWORK,vap_network,"reg90",reg90);
#endif

		snprintf(vlan_eth,OPTION_STR_LEN,"eth0.%d",vlan_id);
		huayi_uci_set_str(PKG_NETWORK,vap_network,"ifname",vlan_eth);
		huayi_uci_set_int(PKG_NETWORK,vap_network,"vlanid",vlan_id);
		huayi_uci_set_hex(PKG_NETWORK,vap_network,"reg94",reg94);
		if (CONFIG_HUAYI_WAN_PORT_NUM == 4)
		{
			reg98 = 0x2200;
		}
		else
		{
			reg98 = 0x2002;
		}
		huayi_uci_set_hex(PKG_NETWORK,vap_network,"reg98",reg98);
#endif
#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7628)
		reg70 = reg70|(0x50<<(i*8));
		portlan_size = cJSON_GetArraySize(portlan_array);
		for (k = 0; k < portlan_size; k++)
		{
			portlan_entry = cJSON_GetArrayItem(portlan_array, k);
			lan_id = atoi(cJSON_Print(portlan_entry)) + CONFIG_HUAYI_LAN1_PORT_NUM_OFFSET;
			lan_used[lan_id] = 1;

			reg70 = reg70|((1<<lan_id) << (8*i));
			if (lan_id == 1)
			{
				lan1_pvid = 0x001|(vlan_id << 12);
			}
			else if (lan_id == 2)
			{
				lan2_pvid = 0x1000|vlan_id;
			}
		}

		switch (i)
		{
			case 0:
				reg50 = reg50 & 0xfff000 | vlan_id;
				break;
			case 1:
				reg50 = reg50 & 0x000fff | (vlan_id<<12);
				break;
			case 2:
				reg54 = reg54 & 0xfff000 | vlan_id;
				break;
			case 3:
				reg54 = reg54 & 0x000fff | (vlan_id<<12);
				break;
			default:
				break;
		}

		snprintf(vlan_eth,OPTION_STR_LEN,"eth0.%d",vlan_id);
		huayi_uci_set_str(PKG_NETWORK,vap_network,"ifname",vlan_eth);
		huayi_uci_set_int(PKG_NETWORK,vap_network,"vlanid",vlan_id);
#endif	
	}

	for (i = 0; i < RADIO_MAX_NUM; i++)
	{
		if (vap_used[i] == 0)
		{
			snprintf(vap_section,OPTION_STR_LEN,"radio%d_0",i);
			huayi_uci_set_int(PKG_WIRELESS,vap_section,"vlanid",0);
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"network","lan");
		}
	}

#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7620) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
	for (i = CONFIG_HUAYI_LAN1_PORT_NUM_OFFSET; i < LAN_MAX_NUM; i++)
	{
		if (lan_used[i] == 0)
		{
			snprintf(lan_section,OPTION_STR_LEN,"lan%d_pvid",i);
			lan_pvid = 0x10001;
			huayi_uci_set_hex(PKG_NETWORK,"lan",lan_section,lan_pvid);
		}
	}
#endif

#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7628)
	huayi_uci_set_hex(PKG_NETWORK,"lan","lan1_pvid",lan1_pvid);
	huayi_uci_set_hex(PKG_NETWORK,"lan","lan2_pvid",lan2_pvid);
	huayi_uci_set_hex(PKG_NETWORK,"lan","reg70",reg70);
	huayi_uci_set_hex(PKG_NETWORK,"lan","reg50",reg50);
	huayi_uci_set_hex(PKG_NETWORK,"lan","reg54",reg54);
#endif
	
	huayi_uci_commit(PKG_WIRELESS);
	huayi_uci_commit(PKG_NETWORK);
	
	system("huayi-reset network");

	*lock_time = HUAYI_RELOAD_NETWORK_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	return FCGI_RESP_CODE_SUCCESS;
}
#endif


int set_wifi_scan_start(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char huayi_cmd[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))	
	{		
		return FCGI_RESP_CODE_DATA_MISS;	
	}	

	system("huayi_cmd set system scan_state 1");
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	sprintf(huayi_cmd,"huayi-reset scan parameter %d",WIFI_ALL_SCAN);
#else
	sprintf(huayi_cmd,"huayi-reset scan parameter %d",WIFI_24G_SCAN);
#endif

	system(huayi_cmd);

	//system("huayi_cmd set system scan_state 1");
	
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_wifi_scanstatus(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root = NULL;		
	int scan_status = 0;
	char scan_status_str[SMALL_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))	
	{		
		return FCGI_RESP_CODE_DATA_MISS;	
	}	

	result_root = cJSON_CreateObject(); 

	get_cmd_result("huayi_cmd get system scan_state",scan_status_str,SMALL_STR_LEN);

	scan_status = atoi(scan_status_str);
	
	cJSON_AddNumberToObject(result_root,"scan_status",scan_status);
	
	if (scan_status == 2)
	{
		system("huayi_cmd set system scan_state 0");
	}
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}



int get_wifi_scanlist(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	FILE *fp = NULL;	
	cJSON *result_root = NULL;	
	cJSON *scan_24G_array = NULL;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	cJSON *scan_58G_array = NULL;
#endif
	cJSON *scan_obj = NULL;	
	char buff[LIST_STR_LEN] = {0};	
	char bssidbuf[TEMP_STR_LEN] = {0},ssidbuf[TEMP_STR_LEN] = {0};
	char encryption[TEMP_STR_LEN] = {0},authmodebuf[TEMP_STR_LEN] = {0},encrypbuf[TEMP_STR_LEN] = {0};
	char *ptr = NULL,*pstrtok = NULL,*pabv = NULL;
	int channel = 0,rssi = 0,ht_extcha = 1;
	int i = 0;
	char ssid_str[OPTION_STR_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time))	
	{		
		return FCGI_RESP_CODE_DATA_MISS;	
	}		
	
	result_root = cJSON_CreateObject(); 
	scan_24G_array = cJSON_CreateArray();
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	scan_58G_array = cJSON_CreateArray();
#endif
	
	fp = fopen("/tmp/ScanList", "r");	
	
	if (NULL == fp) 
	{		
		goto end;	
	}				
	
	while(fgets(buff, sizeof(buff), fp) != NULL)	
	{
		if ((ptr = strstr(buff,"Channel:")) != NULL)
		{
			ptr = ptr + strlen("Channel:");
			channel = atoi(ptr);
		}
		else
		{
			continue;
		}

		if ((ptr = strrstr(buff,"BSSID:")) != NULL)
		{
			ptr = ptr + strlen("BSSID:");
			strncpy(bssidbuf, ptr, 17);
		}
		else
		{
			continue;
		}

		if ((ptr = strrstr(buff,"Signal:")) != NULL)
		{
			ptr = ptr + strlen("Signal:");
			rssi = atoi(ptr);
			pabv = NULL;
			if ((pabv = strstr(ptr,"ABOVE")) != NULL)
			{
				ht_extcha = 1;
			}
			else
			{
				ht_extcha = 0;
			}
		}
		else
		{
			continue;
		}

		if ((ptr = strrstr(buff,"Encryption:\"")) != NULL)
		{
			ptr = ptr + strlen("Encryption:\"");
			if ((pstrtok = strstr(ptr,"\"")) != NULL)
			{
				*pstrtok = '\0';
				strcpy(encryption, ptr);
				if(0 == strncmp(encryption, "WEP", sizeof("WEP")))
				{
					continue;
				}
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}

		if ((ptr = strstr(buff,"SSID:\"")) != NULL)
		{
			memset(ssidbuf,0,TEMP_STR_LEN);
			memset(ssid_str,0,OPTION_STR_LEN);
			ptr = ptr + strlen("SSID:\"");
			strncpy(ssid_str,ptr,59); /*59 is 65 - strlen("SSID:\"")  65 MACRO in driver*/
			if ((pstrtok = strrstr(ssid_str,"\"")) != NULL)
			{

				//FASTCGI_LOG("pstrtok-ssid_str = %d",pstrtok-ssid_str);
				if((pstrtok-ssid_str)== 0)
				{
					continue;
				}
				
				*pstrtok = '\0';
				strcpy(ssidbuf, ssid_str);
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}

		scan_obj = cJSON_CreateObject();
		
		cJSON_AddNumberToObject(scan_obj, "index", i);
		
		cJSON_AddStringToObject(scan_obj, "bssid", bssidbuf);
		
		cJSON_AddStringToObject(scan_obj, "ssid", ssidbuf);
		
		cJSON_AddNumberToObject(scan_obj, "channel", channel);
		
		cJSON_AddNumberToObject(scan_obj, "signal", rssi);

		cJSON_AddNumberToObject(scan_obj, "ht_extcha", ht_extcha);
		
		pstrtok = strtok(encryption,"/");
		snprintf(authmodebuf, TEMP_STR_LEN, "%s", pstrtok);
		pstrtok = strtok(NULL,"/");
		snprintf(encrypbuf, TEMP_STR_LEN, "%s", pstrtok);
		if (strcmp(authmodebuf,"OPEN") == 0)
		{
			strcpy(authmodebuf, "NONE");
		}
		cJSON_AddStringToObject(scan_obj, "auth_mode", authmodebuf);
		cJSON_AddStringToObject(scan_obj, "encryption", encrypbuf);
		
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		
		if (channel > 14)
		{
			cJSON_AddItemToArray(scan_58G_array,scan_obj);
		}
		else
#endif
		{
			cJSON_AddItemToArray(scan_24G_array,scan_obj);
		}
		i++;
	}		

	fclose(fp);

end:

	cJSON_AddItemToObject(result_root,"scan_24G_arr",scan_24G_array);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	cJSON_AddItemToObject(result_root,"scan_58G_arr",scan_58G_array);
#endif

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_wifi_sche_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int weeks = 0;
	int start_hour = 0;
	int start_minute = 0;
	int end_hour = 0;
	int end_minute = 0;
	int enabled = 0;
	
	int interval = 0;
	int remaining_time = 0;
	int i = 0;
	int k = 0;
	FCGI_BOOL this_week = FCGI_FALSE;
	struct tm *p = NULL;
	time_t timep;
	unsigned long time_of_day = 0;
	int cur_time = 0;
	int next_time = 0;
	int wifi_off = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_int(PKG_SCHE,"wifi_sche","enabled",&enabled))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_int(PKG_SCHE,"wifi_sche","weeks",&weeks))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_SCHE,"wifi_sche","start_hour",&start_hour))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_SCHE,"wifi_sche","start_minute",&start_minute))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_int(PKG_SCHE,"wifi_sche","end_hour",&end_hour))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	
	if(!huayi_uci_get_int(PKG_SCHE,"wifi_sche","end_minute",&end_minute))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","wifi_off",&wifi_off);

	//FASTCGI_LOG("wifi_off=%d\n",wifi_off);

	time(&timep);
				
	p=localtime(&timep);

	time_of_day = timep + p->tm_gmtoff - 28800; 
		
	if(enabled && (wifi_off == 0))
	{
		i = p->tm_wday;
		cur_time = (p->tm_wday *1440 + p->tm_hour*60 + p->tm_min);
		weeks = (weeks == 255 ? 127 : weeks);
		for(k=i;k<7;k++)
		{
			if(MyGetField32(weeks,k))
			{
				next_time = ( k * 1440 +start_hour *60+start_minute);
				if(next_time >= cur_time)
				{
					/*find a day this week*/
					this_week = FCGI_TRUE ;
					break;
				}
			}
		}

		if(this_week)
		{
			interval= (next_time - cur_time);
		}else
		{	
			/*get first day next week*/
			for(k=0;k<=i;k++)
			{
				if(MyGetField32(weeks,k))
				{
					/*first day need reboot in next  week*/
					next_time = ( k * 1440 +start_hour *60+start_minute);
					interval = next_time + ( (1440 * 7 ) - cur_time);
					break;
				}
			}
		}

		if (interval > 0)
			remaining_time = (interval * 60) - p->tm_sec;
		else
			remaining_time = 0;
	}else
	{	
		remaining_time = 0;
	}
	
	result_root = cJSON_CreateObject();

	huayi_add_switch_to_json(result_root,"enabled",enabled);
	
	weeks = (weeks == 127 ? 255 : weeks);
	
	cJSON_AddNumberToObject(result_root,"weeks",weeks);

	cJSON_AddNumberToObject(result_root,"start_hour",start_hour);

	cJSON_AddNumberToObject(result_root,"start_minute",start_minute);

	cJSON_AddNumberToObject(result_root,"end_hour",end_hour);

	cJSON_AddNumberToObject(result_root,"end_minute",end_minute);

	cJSON_AddNumberToObject(result_root,"remaining_time",remaining_time);

	cJSON_AddNumberToObject(result_root,"time_of_day",time_of_day);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int set_wifi_sche_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int enabled = 0;
	int weeks = 0;
	int start_hour = 0;
	int start_minute = 0;
	int end_hour = 0;
	int end_minute = 0;
	int start_total_min = 0;
	int end_total_min = 0;
	int i = 0,j = 0,k = 0;
	int max_id = 0;
	int week_flag_1 = 0, week_flag_2 = 0, week_flag_3 = 0;
	char section[OPTION_STR_LEN] = {0};
	char list_time[OPTION_STR_LEN] = {0};
	
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"enabled",&enabled))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"enabled",&enabled)||
		!huayi_json_get_int(request_root,"weeks",&weeks)||
		!huayi_json_get_int(request_root,"start_hour",&start_hour) ||
		!huayi_json_get_int(request_root,"start_minute",&start_minute)||
		!huayi_json_get_int(request_root,"end_hour",&end_hour) ||
		!huayi_json_get_int(request_root,"end_minute",&end_minute))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	huayi_uci_set_int(PKG_SCHE,"wifi_sche","enabled",enabled);
	huayi_uci_set_int(PKG_SCHE,"wifi_sche","weeks",weeks);
	huayi_uci_set_int(PKG_SCHE,"wifi_sche","start_hour",start_hour);
	huayi_uci_set_int(PKG_SCHE,"wifi_sche","start_minute",start_minute);
	huayi_uci_set_int(PKG_SCHE,"wifi_sche","end_hour",end_hour);
	huayi_uci_set_int(PKG_SCHE,"wifi_sche","end_minute",end_minute);

	snprintf(section,sizeof(section),"@sche_list[-1]");

	huayi_uci_get_int(PKG_SCHE,section,"id",&max_id);

	if(max_id == 0)
	{
		;/*no section*/
	}
	else
	{
		snprintf(section,sizeof(section),"@sche_list[0]");
		
		for(i=1;i<=max_id;i++)
		{
			huayi_uci_del_section(PKG_SCHE,section);
		}
	}

	start_total_min = start_hour*60 + start_minute;
	end_total_min = end_hour*60 + end_minute;
	
	if (end_total_min >= start_total_min)
	{
		huayi_uci_add_section(PKG_SCHE,"sche_list");
		snprintf(section,sizeof(section),"@sche_list[%d]",0);

		huayi_uci_set_int(PKG_SCHE,section,"week",weeks);

		snprintf(list_time,sizeof(list_time),"%d-%d",start_total_min,end_total_min);
		huayi_uci_add_list(PKG_SCHE,section,"time",list_time);

		max_id = 1;
		huayi_uci_set_int(PKG_SCHE,section,"id",max_id);
	}
	else
	{
		for (i = 0; i < 7; i++)
		{
			if ((1<<i) & weeks)
			{
				j = i-1;
				k = i+1;
				
				if (j < 0)
				{
					j = 6;
				}

				if (k > 6)
				{
					k = 0;
				}

				if ((1<<j) & weeks)
				{
					week_flag_2 |= (1<<i);
				}
				else
				{
					week_flag_3 |= (1<<i);
				}

				if ((1<<k) & weeks)
				{
					continue;
				}
				else
				{
					week_flag_1 |= (1<<k);
				}	
			}
		}

		max_id = 1;
		if (week_flag_1 > 0)
		{
			huayi_uci_add_section(PKG_SCHE,"sche_list");
			snprintf(section,sizeof(section),"@sche_list[%d]",max_id-1);

			huayi_uci_set_int(PKG_SCHE,section,"week",week_flag_1);

			snprintf(list_time,sizeof(list_time),"%d-%d",0,end_total_min);
			huayi_uci_add_list(PKG_SCHE,section,"time",list_time);
			
			huayi_uci_set_int(PKG_SCHE,section,"id",max_id);
			max_id++;
		}

		if (week_flag_2 > 0)
		{
			huayi_uci_add_section(PKG_SCHE,"sche_list");
			snprintf(section,sizeof(section),"@sche_list[%d]",max_id-1);

			huayi_uci_set_int(PKG_SCHE,section,"week",week_flag_2);

			snprintf(list_time,sizeof(list_time),"%d-%d",0,end_total_min);
			huayi_uci_add_list(PKG_SCHE,section,"time",list_time);

			snprintf(list_time,sizeof(list_time),"%d-%d",start_total_min,24*60);
			huayi_uci_add_list(PKG_SCHE,section,"time",list_time);

			huayi_uci_set_int(PKG_SCHE,section,"id",max_id);
			max_id++;
		}

		if (week_flag_3 > 0)
		{
			huayi_uci_add_section(PKG_SCHE,"sche_list");
			snprintf(section,sizeof(section),"@sche_list[%d]",max_id-1);

			huayi_uci_set_int(PKG_SCHE,section,"week",week_flag_3);

			snprintf(list_time,sizeof(list_time),"%d-%d",start_total_min,24*60);
			huayi_uci_add_list(PKG_SCHE,section,"time",list_time);

			huayi_uci_set_int(PKG_SCHE,section,"id",max_id);
			max_id++;
		}
	}

	huayi_uci_commit(PKG_SCHE);

	system("huayi-reset wifi_sche");
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_wireless_status(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *ssid_config = NULL;
	cJSON *guest = NULL;

	int disable = 0;
	int ssid_2g_enable = 0;
	char ssid_2g[TEMP_STR_LEN] = {0}, encry_2g[TEMP_STR_LEN] = {0};
	char passphrase_2g[TEMP_STR_LEN] = {0};
	int hide_2g = 0;

	int guest_ssid_2g_enable = 0;
	char guest_ssid_2g[TEMP_STR_LEN] = {0};
	int guest_hide_2g = 0;
	char guest_passphrase[TEMP_STR_LEN] = {0} ,guest_encry[TEMP_STR_LEN] = {0};

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)

	int ssid_5g_enable = 0;
	char ssid_5g[TEMP_STR_LEN] = {0}, encry_5g[TEMP_STR_LEN] = {0};
	char passphrase_5g[TEMP_STR_LEN] = {0};
	int hide_5g = 0;

	int guest_ssid_5g_enable = 0;
	char guest_ssid_5g[TEMP_STR_LEN] = {0};
	int guest_hide_5g = 0;
#endif
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_IFACE,"disabled",&disable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"encryption",encry_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"key",passphrase_2g))
	{
		;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_IFACE,"hidden",&hide_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	ssid_2g_enable = disable?0:1;

	
	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"disabled",&disable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_GUEST_IFACE,"ssid",guest_ssid_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"hidden",&guest_hide_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_GUEST_IFACE,"encryption",guest_encry))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_GUEST_IFACE,"key",guest_passphrase))
	{
		;
	}

	guest_ssid_2g_enable = disable?0:1;

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_uci_get_int(PKG_WIRELESS,W58G_IFACE,"disabled",&disable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"encryption",encry_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"key",passphrase_5g))
	{
		;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W58G_IFACE,"hidden",&hide_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	ssid_5g_enable = disable?0:1;

	if(!huayi_uci_get_int(PKG_WIRELESS,W58G_GUEST_IFACE,"disabled",&disable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_GUEST_IFACE,"ssid",guest_ssid_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W58G_GUEST_IFACE,"hidden",&guest_hide_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	guest_ssid_5g_enable = disable?0:1;
#endif


	result_root = cJSON_CreateObject();
	ssid_config = cJSON_CreateObject();
	guest = cJSON_CreateObject();

	//cJSON_AddNumberToObject(ssid_config, "ssid_2g_enable", ssid_2g_enable);	
	if(ssid_2g_enable == 0)
	{
		cJSON_AddFalseToObject(ssid_config,"ssid_2g_enable");
	}
	else
	{
		cJSON_AddTrueToObject(ssid_config,"ssid_2g_enable");
	}
	
	cJSON_AddStringToObject(ssid_config, "ssid_2g", ssid_2g);
	if (strncmp(encry_2g,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(ssid_config, "encry_2g", 0);	
	}
	else
	{
		cJSON_AddNumberToObject(ssid_config, "encry_2g", 1);	
	}
	cJSON_AddStringToObject(ssid_config, "passphrase_2g", passphrase_2g);
	//cJSON_AddNumberToObject(ssid_config, "hide_2g", hide_2g);
	if(hide_2g == 0)
	{
		cJSON_AddFalseToObject(ssid_config,"hide_2g");
	}
	else
	{
		cJSON_AddTrueToObject(ssid_config,"hide_2g");
	}

	//cJSON_AddNumberToObject(guest, "enable_2g", guest_ssid_2g_enable);
	if(guest_ssid_2g_enable == 0)
	{
		cJSON_AddFalseToObject(guest,"enable_2g");
	}
	else
	{
		cJSON_AddTrueToObject(guest,"enable_2g");
	}
	cJSON_AddStringToObject(guest, "ssid_2g", guest_ssid_2g);
	//cJSON_AddNumberToObject(guest, "hide_2g", guest_hide_2g);
	if(guest_hide_2g == 0)
	{
		cJSON_AddFalseToObject(guest,"hide_2g");
	}
	else
	{
		cJSON_AddTrueToObject(guest,"hide_2g");
	}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	//cJSON_AddNumberToObject(ssid_config, "ssid_5g_enable", ssid_5g_enable);	
	if(ssid_5g_enable == 0)
	{
		cJSON_AddFalseToObject(ssid_config,"ssid_5g_enable");
	}
	else
	{
		cJSON_AddTrueToObject(ssid_config,"ssid_5g_enable");
	}
	cJSON_AddStringToObject(ssid_config, "ssid_5g", ssid_5g);
	if (strncmp(encry_5g,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(ssid_config, "encry_5g", 0);	
	}
	else
	{
		cJSON_AddNumberToObject(ssid_config, "encry_5g", 1);		
	}
	cJSON_AddStringToObject(ssid_config, "passphrase_5g", passphrase_5g);
	//cJSON_AddNumberToObject(ssid_config, "hide_5g", hide_5g);
	if(hide_5g == 0)
	{
		cJSON_AddFalseToObject(ssid_config,"hide_5g");
	}
	else
	{
		cJSON_AddTrueToObject(ssid_config,"hide_5g");
	}

	//cJSON_AddNumberToObject(guest, "enable_5g", guest_ssid_5g_enable);
	if(guest_ssid_5g_enable == 0)
	{
		cJSON_AddFalseToObject(guest,"enable_5g");
	}
	else
	{
		cJSON_AddTrueToObject(guest,"enable_5g");
	}
	cJSON_AddStringToObject(guest, "ssid_5g", guest_ssid_5g);
	//cJSON_AddNumberToObject(guest, "hide_5g", guest_hide_5g);
	if(guest_hide_5g == 0)
	{
		cJSON_AddFalseToObject(guest,"hide_5g");
	}
	else
	{
		cJSON_AddTrueToObject(guest,"hide_5g");
	}
#endif
	if (strncmp(guest_encry,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(guest, "encry", 0);	
	}
	else
	{
		cJSON_AddNumberToObject(guest, "encry", 1);	
	}
	cJSON_AddStringToObject(guest, "passphrase", guest_passphrase);

	cJSON_AddItemToObject(result_root,"ssid_config",ssid_config);
	cJSON_AddItemToObject(result_root,"guest",guest);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wireless_status(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *ssid_config = NULL;
	cJSON *guest = NULL;

	int disabled = 0;
	int ssid_2g_enable = 0;
	char ssid_2g[TEMP_STR_LEN] = {0};
	int encry_2g = 0;
	char passphrase_2g[TEMP_STR_LEN] = {0};
	int hide_2g = 0;

	int guest_ssid_2g_enable = 0;
	char guest_ssid_2g[TEMP_STR_LEN] = {0};
	int guest_hide_2g = 0;
	int guest_security = 0;
	char guest_passphrase[TEMP_STR_LEN] = {0};

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)

	int ssid_5g_enable = 0;
	char ssid_5g[TEMP_STR_LEN] = {0};
	int encry_5g = 0;
	char passphrase_5g[TEMP_STR_LEN] = {0};
	int hide_5g = 0;

	int guest_ssid_5g_enable = 0;
	char guest_ssid_5g[TEMP_STR_LEN] = {0};
	int guest_hide_5g = 0;
#endif
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!(ssid_config = cJSON_GetObjectItem(request_root,"ssid_config")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!(guest = cJSON_GetObjectItem(request_root,"guest")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(ssid_config,"ssid_2g_enable",&ssid_2g_enable)||
		!huayi_json_get_str(ssid_config,"ssid_2g",ssid_2g,OPTION_STR_LEN)||
		!huayi_json_get_int(ssid_config,"encry_2g",&encry_2g)||
		!huayi_json_get_int(ssid_config,"hide_2g",&hide_2g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(ssid_config,"passphrase_2g",passphrase_2g,OPTION_STR_LEN))
	{
		if(encry_2g == 1)
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}

	if(!huayi_json_get_int(guest,"enable_2g",&guest_ssid_2g_enable)||
		!huayi_json_get_str(guest,"ssid_2g",guest_ssid_2g,OPTION_STR_LEN)||
		!huayi_json_get_int(guest,"hide_2g",&guest_hide_2g)||
		!huayi_json_get_int(guest,"encry",&guest_security))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(guest,"passphrase",guest_passphrase,OPTION_STR_LEN))
	{
		if(guest_security == 1)
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_int(ssid_config,"ssid_5g_enable",&ssid_5g_enable)||
		!huayi_json_get_str(ssid_config,"ssid_5g",ssid_5g,OPTION_STR_LEN)||
		!huayi_json_get_int(ssid_config,"encry_5g",&encry_5g)||
		!huayi_json_get_int(ssid_config,"hide_5g",&hide_5g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(ssid_config,"passphrase_5g",passphrase_5g,OPTION_STR_LEN))
	{
		if(encry_5g == 1)
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}

	if(!huayi_json_get_int(guest,"enable_5g",&guest_ssid_5g_enable)||
		!huayi_json_get_str(guest,"ssid_5g",guest_ssid_5g,OPTION_STR_LEN)||
		!huayi_json_get_int(guest,"hide_5g",&guest_hide_5g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
#endif

	disabled = ssid_2g_enable?0:1;
	huayi_uci_set_int(PKG_WIRELESS,W24G_IFACE,"disabled",disabled);
	if(disabled == 0)
	{
		huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"disabled",disabled);
	}
	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_2g);
	if (encry_2g == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W24G_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","mixed-psk+tkip+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"key",passphrase_2g);
	}
	huayi_uci_set_int(PKG_WIRELESS,W24G_IFACE,"hidden",hide_2g);

	disabled = guest_ssid_2g_enable?0:1;
	huayi_uci_set_int(PKG_WIRELESS,W24G_GUEST_IFACE,"disabled",disabled);
	huayi_uci_set_str(PKG_WIRELESS,W24G_GUEST_IFACE,"ssid",guest_ssid_2g);
	if (guest_security == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_GUEST_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W24G_GUEST_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_GUEST_IFACE,"encryption","mixed-psk+tkip+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W24G_GUEST_IFACE,"key",guest_passphrase);
	}
	huayi_uci_set_int(PKG_WIRELESS,W24G_GUEST_IFACE,"hidden",guest_hide_2g);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	disabled = ssid_5g_enable?0:1;
	huayi_uci_set_int(PKG_WIRELESS,W58G_IFACE,"disabled",disabled);
	if(disabled == 0)
	{
		huayi_uci_set_int(PKG_WIRELESS,W58G_DEV,"disabled",disabled);
	}
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_5g);
	if (encry_5g == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W58G_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","mixed-psk+tkip+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"key",passphrase_5g);
	}
	huayi_uci_set_int(PKG_WIRELESS,W58G_IFACE,"hidden",hide_5g);

	disabled = guest_ssid_5g_enable?0:1;
	huayi_uci_set_int(PKG_WIRELESS,W58G_GUEST_IFACE,"disabled",disabled);
	huayi_uci_set_str(PKG_WIRELESS,W58G_GUEST_IFACE,"ssid",guest_ssid_5g);
	if (guest_security == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_GUEST_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W58G_GUEST_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_GUEST_IFACE,"encryption","mixed-psk+tkip+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W58G_GUEST_IFACE,"key",guest_passphrase);
	}
	huayi_uci_set_int(PKG_WIRELESS,W58G_GUEST_IFACE,"hidden",guest_hide_5g);
#endif
	huayi_uci_commit(PKG_WIRELESS);
	system("huayi-reset wireless");
	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	return FCGI_RESP_CODE_SUCCESS;
}

int get_guest_network(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;

	int disable = 0;
	int guest_ssid_2g_enable = 0;
	char guest_ssid_2g[TEMP_STR_LEN] = {0};
	int guest_hide_2g = 0;
	char guest_passphrase[TEMP_STR_LEN] = {0} ,guest_encry[TEMP_STR_LEN] = {0};
	int guest_visible = 0, guest_access = 0;

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int guest_ssid_5g_enable = 0;
	char guest_ssid_5g[TEMP_STR_LEN] = {0};
	int guest_hide_5g = 0;
#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"disabled",&disable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_GUEST_IFACE,"ssid",guest_ssid_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"hidden",&guest_hide_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_GUEST_IFACE,"encryption",guest_encry))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_str(PKG_WIRELESS,W24G_GUEST_IFACE,"key",guest_passphrase))
	{
		;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"noforward",&guest_visible))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"local_network",&guest_access))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	guest_ssid_2g_enable = disable?0:1;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_uci_get_int(PKG_WIRELESS,W58G_GUEST_IFACE,"disabled",&disable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,W58G_GUEST_IFACE,"ssid",guest_ssid_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,W58G_GUEST_IFACE,"hidden",&guest_hide_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	guest_ssid_5g_enable = disable?0:1;

	result_root = cJSON_CreateObject();
#endif
	//cJSON_AddNumberToObject(result_root, "enable_2g", guest_ssid_2g_enable);
	if(guest_ssid_2g_enable == 0)
	{
		cJSON_AddFalseToObject(result_root,"enable_2g");
	}
	else
	{
		cJSON_AddTrueToObject(result_root,"enable_2g");
	}
	cJSON_AddStringToObject(result_root, "ssid_2g", guest_ssid_2g);
	//cJSON_AddNumberToObject(result_root, "hide_2g", guest_hide_2g);
	if(guest_hide_2g == 0)
	{
		cJSON_AddFalseToObject(result_root,"hide_2g");
	}
	else
	{
		cJSON_AddTrueToObject(result_root,"hide_2g");
	}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	//cJSON_AddNumberToObject(result_root, "enable_5g", guest_ssid_5g_enable);
	if(guest_ssid_5g_enable == 0)
	{
		cJSON_AddFalseToObject(result_root,"enable_5g");
	}
	else
	{
		cJSON_AddTrueToObject(result_root,"enable_5g");
	}
	cJSON_AddStringToObject(result_root, "ssid_5g", guest_ssid_5g);
	//cJSON_AddNumberToObject(result_root, "hide_5g", guest_hide_5g);
	if(guest_hide_5g == 0)
	{
		cJSON_AddFalseToObject(result_root,"hide_5g");
	}
	else
	{
		cJSON_AddTrueToObject(result_root,"hide_5g");
	}
#endif

	if (strncmp(guest_encry,"none",strlen("none")) == 0)
	{
		cJSON_AddNumberToObject(result_root, "encry", 0);	
	}
	else
	{
		cJSON_AddNumberToObject(result_root, "encry", 1);	
	}
	cJSON_AddStringToObject(result_root, "passphrase", guest_passphrase);
	//cJSON_AddNumberToObject(result_root, "visible", guest_visible);
	if(guest_visible == 0)
	{
		cJSON_AddTrueToObject(result_root,"visible");
	}
	else
	{
		cJSON_AddFalseToObject(result_root,"visible");
	}
	//cJSON_AddNumberToObject(result_root, "access", guest_access);
	if(guest_access == 0)
	{
		cJSON_AddFalseToObject(result_root,"access");
	}
	else
	{
		cJSON_AddTrueToObject(result_root,"access");
	}

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int set_guest_network(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int disabled = 0;
	int guest_ssid_2g_enable = 0;
	char guest_ssid_2g[TEMP_STR_LEN] = {0};
	int guest_hide_2g = 0;
	int guest_encry = 0;
	char guest_passphrase[TEMP_STR_LEN] = {0};
	int guest_visible = 0, guest_access = 0;

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int guest_ssid_5g_enable = 0;
	char guest_ssid_5g[TEMP_STR_LEN] = {0};
	int guest_hide_5g = 0;
#endif
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"enable_2g",&guest_ssid_2g_enable)||
			!huayi_json_get_str(request_root,"ssid_2g",guest_ssid_2g,OPTION_STR_LEN)||
			!huayi_json_get_int(request_root,"hide_2g",&guest_hide_2g)||
			!huayi_json_get_int(request_root,"encry",&guest_encry)||
			!huayi_json_get_int(request_root,"visible",&guest_visible)||
			!huayi_json_get_int(request_root,"access",&guest_access))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"passphrase",guest_passphrase,OPTION_STR_LEN))
	{
		if(guest_encry == 1)
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}

	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_int(request_root,"enable_5g",&guest_ssid_5g_enable)||
		!huayi_json_get_str(request_root,"ssid_5g",guest_ssid_5g,OPTION_STR_LEN)||
		!huayi_json_get_int(request_root,"hide_5g",&guest_hide_5g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
#endif

	disabled = guest_ssid_2g_enable?0:1;
	huayi_uci_set_int(PKG_WIRELESS,W24G_GUEST_IFACE,"disabled",disabled);
	if(disabled == 0)
	{
		huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"disabled",disabled);
	}
	huayi_uci_set_str(PKG_WIRELESS,W24G_GUEST_IFACE,"ssid",guest_ssid_2g);
	if (guest_encry == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_GUEST_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W24G_GUEST_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W24G_GUEST_IFACE,"encryption","mixed-psk+tkip+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W24G_GUEST_IFACE,"key",guest_passphrase);
	}
	huayi_uci_set_int(PKG_WIRELESS,W24G_GUEST_IFACE,"hidden",guest_hide_2g);
	guest_visible = (guest_visible==0?1:0);
	huayi_uci_set_int(PKG_WIRELESS,W24G_GUEST_IFACE,"noforward",guest_visible);
	huayi_uci_set_int(PKG_WIRELESS,W24G_GUEST_IFACE,"local_network",guest_access);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	disabled = guest_ssid_5g_enable?0:1;
	huayi_uci_set_int(PKG_WIRELESS,W58G_GUEST_IFACE,"disabled",disabled);
	if(disabled == 0)
	{
		huayi_uci_set_int(PKG_WIRELESS,W58G_DEV,"disabled",disabled);
	}
	huayi_uci_set_str(PKG_WIRELESS,W58G_GUEST_IFACE,"ssid",guest_ssid_5g);
	if (guest_encry == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_GUEST_IFACE,"encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,W58G_GUEST_IFACE,"key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,W58G_GUEST_IFACE,"encryption","mixed-psk+tkip+ccmp");
		huayi_uci_set_str(PKG_WIRELESS,W58G_GUEST_IFACE,"key",guest_passphrase);
	}
	huayi_uci_set_int(PKG_WIRELESS,W58G_GUEST_IFACE,"hidden",guest_hide_5g);
	huayi_uci_set_int(PKG_WIRELESS,W58G_GUEST_IFACE,"noforward",guest_visible);
#endif
	huayi_uci_commit(PKG_WIRELESS);
	system("huayi-reset wireless");
	*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	return FCGI_RESP_CODE_SUCCESS;
}


int get_wifi_wps(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	int wps_enable = 0,pincodeenable = 0, wifienable = 0;
	int pincode = 0;
	int radio2g_disable = 1;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int	radio5g_disable = 1;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	huayi_uci_get_int(PKG_WPS,"wps","wpsenable",&wps_enable);
	huayi_uci_get_int(PKG_WPS,"wps","pincodeenable",&pincodeenable);
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&radio2g_disable);

	if(0 == radio2g_disable)
	{ 
		wifienable = 1;
	}
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&radio5g_disable);
	if(0 == radio5g_disable)
	{ 
		wifienable = 1;
	}
#endif
	
	if(0 == radio2g_disable) 
	{
		huayi_uci_get_int(PKG_WPS,"wps","own2gpincode",&pincode);
	} 
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	else if(0 == radio5g_disable) 
	{
		huayi_uci_get_int(PKG_WPS,"wps","own5gpincode",&pincode);
	}
#endif
	
	huayi_add_switch_to_json(result_root,"enable", wps_enable);
	huayi_add_switch_to_json(result_root,"pinenable", pincodeenable);
	cJSON_AddNumberToObject(result_root,"wifienable", wifienable);
	cJSON_AddNumberToObject(result_root,"pincode", pincode);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_wps(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int wps_enable = 0, initpincodeflg = 0;
	char cmd[CMD_STR_LEN] = { 0 };
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_int(request_root,"enable",&wps_enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_WPS, "wps","wpsenable", wps_enable);

	huayi_uci_commit(PKG_WPS);	
	*lock_time = 0;
	huayi_uci_get_int(PKG_WPS,"wps","initpincodeflg",&initpincodeflg);
	if(0 == initpincodeflg) {
		snprintf(cmd,sizeof(cmd),"huayi-reset wps_pin parameter %d", 1);  // get initialize pincode
		system(cmd);
	}
	cJSON_AddFalseToObject(response_root,"reconnection");
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_wps_pbc(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int pbcconnect = 0;
	char cmd[CMD_STR_LEN] = { 0 };
	int radio2g_disable = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int radio5g_disable = 0;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_int(request_root,"enable",&pbcconnect))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&radio2g_disable);
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&radio5g_disable);
#endif

	huayi_uci_set_int(PKG_WPS,"wps","pbcconnect",pbcconnect);
	huayi_uci_set_int(PKG_WPS, "wps", "wpsstatus", 36);
	if(1 == pbcconnect) 
	{
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		if(0 == radio5g_disable) 
		{
			snprintf(cmd,sizeof(cmd),"huayi-reset wps_pbc parameter %d", 5);
		}
#endif
		if(0 == radio2g_disable) 
		{
			snprintf(cmd,sizeof(cmd),"huayi-reset wps_pbc parameter %d", 2);
		}
	} 
	else 
	{
		snprintf(cmd,sizeof(cmd),"huayi-reset wps_pbc parameter %d", 0);  //disconnect wps pbc
	}
	
	huayi_uci_commit(PKG_WPS);
	system(cmd);	
	*lock_time = 0;

	cJSON_AddFalseToObject(response_root,"reconnection");
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_wpsstapin(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int pinconnect = 0, pincode = 0;
	char cmd[CMD_STR_LEN] = { 0 };
	int radio2g_disable = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int radio5g_disable = 0;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_int(request_root,"enable",&pinconnect)||
		!huayi_json_get_int(request_root,"stapin",&pincode))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_WPS,"wps","stapincode",pincode);
	huayi_uci_set_int(PKG_WPS,"wps","pinconnect",pinconnect);
	
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&radio2g_disable);
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&radio5g_disable);
#endif
	
	huayi_uci_set_int(PKG_WPS, "wps", "wpsstatus", 36);
	if(1 == pinconnect) 
	{
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		if(0 == radio5g_disable) 
		{ 
			snprintf(cmd,sizeof(cmd),"huayi-reset wps_pin parameter %d", 5);
		}
#endif
		if(0 == radio2g_disable) 
		{  
			snprintf(cmd,sizeof(cmd),"huayi-reset wps_pin parameter %d", 2);
		}
	} 
	else 
	{
		snprintf(cmd,sizeof(cmd),"huayi-reset wps_pin parameter %d", 0);
	}	
	
	huayi_uci_commit(PKG_WPS);
	system(cmd);	
	*lock_time = 0;

	cJSON_AddFalseToObject(response_root,"reconnection");
	return FCGI_RESP_CODE_SUCCESS;
}

int get_wifi_wpspin(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	int pincodeenable = 0;
	int pincode = 0;
	int radio2g_disable = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int radio5g_disable = 0;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	huayi_uci_get_int(PKG_WPS,"wps","pincodeenable",&pincodeenable);
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&radio2g_disable);
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&radio5g_disable);
#endif
	
	if(0 == radio2g_disable) 
	{  
		huayi_uci_get_int(PKG_WPS,"wps","own2gpincode",&pincode);
	} 
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	else if(0 == radio5g_disable)
	{ 
		huayi_uci_get_int(PKG_WPS,"wps","own5gpincode",&pincode); 
	} 
#endif
	
	huayi_add_switch_to_json(result_root,"pinenable", pincodeenable);
	cJSON_AddNumberToObject(result_root,"pincode", pincode);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_wps_ownpin(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int pincodeenable = 0;
	char cmd[CMD_STR_LEN] = { 0 };
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_int(request_root,"enable",&pincodeenable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_WPS, "wps","pincodeenable", pincodeenable);

	huayi_uci_commit(PKG_WPS);	

	snprintf(cmd,sizeof(cmd),"huayi-reset wps_pin parameter %d", 6);
	system(cmd);
	
	*lock_time = HUAYI_WPS_ENABLE_TIME;

	cJSON_AddFalseToObject(response_root,"reconnection");
	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_wpsfirstpin(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	char cmd[CMD_STR_LEN] = { 0 };
	int pincode = 0;
	int radio2g_disable = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int radio5g_disable = 0;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&radio2g_disable);
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&radio5g_disable);
#endif

	if(0 == radio2g_disable) 
	{
		huayi_uci_get_int(PKG_WPS,"wps","initpincode2g",&pincode);
		
	} 
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	else if(0 == radio5g_disable) 
	{
		huayi_uci_get_int(PKG_WPS,"wps","initpincode5g",&pincode);
	}
	huayi_uci_set_int(PKG_WPS, "wps", "own5gpincode", pincode);
#endif

	huayi_uci_set_int(PKG_WPS, "wps", "own2gpincode", pincode);
	
	huayi_uci_commit(PKG_WPS);
	snprintf(cmd,sizeof(cmd),"huayi-reset wps_pin parameter %d", 1);
	system(cmd);	
	*lock_time = HUAYI_WPS_ENABLE_TIME;

	cJSON_AddFalseToObject(response_root,"reconnection");

	return FCGI_RESP_CODE_SUCCESS;
}

int set_wifi_wpsrandompin(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char cmd[CMD_STR_LEN] = { 0 };
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	snprintf(cmd,sizeof(cmd),"huayi-reset wps_pin parameter %d", 3);  // random pincode
	system(cmd);	
	*lock_time = HUAYI_WPS_ENABLE_TIME;

	cJSON_AddFalseToObject(response_root,"reconnection");
	return FCGI_RESP_CODE_SUCCESS;
}

int get_wifi_wpsstatus(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	int wpsstatus = 36;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	
	huayi_uci_get_int(PKG_WPS,"wps","wpsstatus",&wpsstatus);
	
	cJSON_AddNumberToObject(result_root,"wpsstatus", wpsstatus);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}


#endif
