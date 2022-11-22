
#include "fastcgi_common.h"
#include "oid.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

int get_pre_login_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	char web_title[OPTION_STR_LEN] = {0};
	char company_website[OPTION_STR_LEN] = {0};
	char company_copyright[OPTION_STR_LEN] = {0};
	int sys_mode = 0;
	int first_boot = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_str(PKG_ABOUT,"about_system","company_name",web_title);

	huayi_uci_get_str(PKG_ABOUT,"about_system","company_website",company_website);

	huayi_uci_get_str(PKG_ABOUT,"about_system","company_copyright",company_copyright);

	cJSON_AddStringToObject(result_root,"company_name",web_title);
	
	cJSON_AddStringToObject(result_root,"company_domain",company_website);

	cJSON_AddStringToObject(result_root,"company_info",company_copyright);

	cJSON_AddNumberToObject(result_root,"randomname",huayi_generate_randomname());

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);

	cJSON_AddNumberToObject(result_root,"sysmode",sys_mode);

	huayi_uci_get_int(PKG_SYSTEM,"global","first_boot",&first_boot);

	cJSON_AddNumberToObject(result_root,"first_boot",first_boot);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int set_pre_login_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	/*reserve */
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_language_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *support_language_array = NULL;
	int i = 0;
	char support_language_list[LIST_STR_LEN] ={0};
	char support_language[OPTION_STR_LEN] = {0};
	char language[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_str(PKG_SYSTEM,"global","support_language",support_language_list))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_SYSTEM,"global","language",language))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	support_language_array = cJSON_CreateArray();

	result_root = cJSON_CreateObject();

	while(huayi_get_nth_value(i++, support_language_list, WEB_SPACE_DELIMIT, support_language, sizeof(support_language)))
	{
		cJSON_AddItemToArray(support_language_array,cJSON_CreateString(support_language));
	}
	
	cJSON_AddItemToObject(result_root,"support_language",support_language_array);

	cJSON_AddStringToObject(result_root,"current_language",language);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}


int set_language_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char language[OPTION_STR_LEN] = {0};
	char LANGUAGE[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_str(request_root,"current_language",language,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!huayi_uci_get_str(PKG_SYSTEM,"global","language",LANGUAGE))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(strcmp(LANGUAGE,language) == 0)
	{
		return FCGI_RESP_CODE_SUCCESS;
	}

	huayi_uci_set_str(PKG_SYSTEM,"global","language",language);

	huayi_uci_commit(PKG_SYSTEM);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int do_login_auth(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char username[OPTION_STR_LEN]={0};
	char http_user[OPTION_STR_LEN]={0};	
	char http_pass[OPTION_STR_LEN]={0};
#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
	char sess_id_set[OPTION_STR_LEN]={0};
	int  num = 0;
	BIO *bio = NULL;
	RSA *publicRsa = NULL;
	unsigned char *encryptMsg_base64 = NULL, *encryptMsg_rsa = NULL;
	char *decrypt_password = NULL;
	int base64_len = 0, rsa_len = 0;
#else
	char password[OPTION_STR_LEN]={0};
#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"hy_username",username,OPTION_STR_LEN))
	{
		return  FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_str(PKG_SYSTEM,"login","http_user",http_user))
	{
		return  FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_str(PKG_SYSTEM,"login","http_pass",http_pass))
	{
		return	FCGI_RESP_CODE_SERVER_FAILURE;
	}
#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
	if ((bio = BIO_new_mem_buf((void *)publicKey, -1)) == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	if ((publicRsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL)) == NULL) 
	{
		BIO_free_all(bio);
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	BIO_free_all(bio);
	rsa_len = RSA_size(publicRsa);
	RSA_free(publicRsa);
	base64_len = (rsa_len/3 + (rsa_len%3?1:0)) * 4;
	
	encryptMsg_base64 = (unsigned char *)malloc(base64_len + 1);
	if(NULL == encryptMsg_base64)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	encryptMsg_rsa = (unsigned char *)malloc(rsa_len + 1);  
	if(NULL == encryptMsg_rsa)
	{
		free(encryptMsg_base64);
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	memset(encryptMsg_base64, 0, base64_len + 1);
	memset(encryptMsg_rsa, 0, rsa_len + 1);

	if(!huayi_json_get_str(request_root,"hy_userpwd",(char *)encryptMsg_base64,base64_len + 1))
	{
		free(encryptMsg_base64);
		free(encryptMsg_rsa);
		return FCGI_RESP_CODE_DATA_MISS;
	}
	base64_decode((char *)encryptMsg_base64, strlen((char *)encryptMsg_base64), (char *)encryptMsg_rsa);
	decrypt_password = rsa_decrypt((void *)encryptMsg_rsa);   

	free(encryptMsg_base64);
	free(encryptMsg_rsa);

	if(NULL != decrypt_password)
	{
		if(strcmp(http_user,username)||strcmp(http_pass,decrypt_password))
		{
			free(decrypt_password);
			return FCGI_RESP_CODE_PERMISSION_DENY;
			
		}
		free(decrypt_password);
	}
	else
	{
		return FCGI_RESP_CODE_PERMISSION_DENY;
	}
#else 
	if(!huayi_json_get_str(request_root,"hy_userpwd",password,OPTION_STR_LEN))
	{
		FASTCGI_LOG("hy_userpwd data miss");
		return	FCGI_RESP_CODE_DATA_MISS;
	}

	FASTCGI_LOG("hy_userpwd data success");
	if(strcmp(http_user,username)||strcmp(http_pass,password))
	{
		return FCGI_RESP_CODE_PERMISSION_DENY;
	}
#endif
		
#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
	num = get_login_users();
	if(num >= 0 && num < MAX_WEB_SESSION_NUM)
	{
		if (find_login_user(sess_id_set) == FCGI_FALSE)
		{
			add_session_login(sess_id_set);
		}
		FCGI_printf("Set-Cookie:SESSION_ID=%s; path=/\n", sess_id_set);
	}
	else
	{
		return FCGI_RESP_CODE_LOGIN_LIMIT;
	}
#endif

	return FCGI_RESP_CODE_SUCCESS;

}

int do_logout_clean(cJSON *request_root, cJSON *response_root,int *lock_time)
{
#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
	char *remoteip = NULL;
	
	remoteip = getenv("REMOTE_ADDR");

	if(NULL != remoteip)
	{
		session_logout(remoteip);
	}	
#endif
	return FCGI_RESP_CODE_UNAUTHORIZED;
}

int get_passwd_state(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	cJSON_AddNumberToObject(result_root,"randomname",huayi_generate_randomname());
	

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}


int change_admin_password(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char http_pass[OPTION_STR_LEN]={0};		
#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
	BIO *bio = NULL;
	RSA *publicRsa = NULL;
	unsigned char *encryptMsg_base64 = NULL, *encryptMsg_rsa = NULL;
	char *decrypt_password = NULL;
	int base64_len = 0, rsa_len = 0;
#else
	char prepassword[OPTION_STR_LEN]={0};  
	char newpassword[OPTION_STR_LEN]={0};
	char ipaddr[OPTION_STR_LEN] = {0};
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	

#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
	if ((bio = BIO_new_mem_buf((void *)publicKey, -1)) == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	if ((publicRsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL)) == NULL) 
	{
		BIO_free_all(bio);
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	BIO_free_all(bio);
	rsa_len = RSA_size(publicRsa);
	RSA_free(publicRsa);
	base64_len = (rsa_len/3 + (rsa_len%3?1:0)) * 4;

	encryptMsg_base64 = (unsigned char *)malloc(base64_len + 1);
	if(NULL == encryptMsg_base64)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	encryptMsg_rsa = (unsigned char *)malloc(rsa_len + 1);	
	if(NULL == encryptMsg_rsa)
	{
		free(encryptMsg_base64);
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	memset(encryptMsg_base64, 0, base64_len + 1);
	memset(encryptMsg_rsa, 0, rsa_len + 1);
	
	if(huayi_uci_get_str(PKG_SYSTEM,"login","http_pass",http_pass))
	{
		if(!huayi_json_get_str(request_root,"hy_prepwd",(char *)encryptMsg_base64,base64_len + 1))
		{
			free(encryptMsg_base64);
			free(encryptMsg_rsa);
			return FCGI_RESP_CODE_DATA_MISS;
		}
		base64_decode((char *)encryptMsg_base64, strlen((char *)encryptMsg_base64), (char *)encryptMsg_rsa);
		decrypt_password = rsa_decrypt((void *)encryptMsg_rsa);   

		free(encryptMsg_base64);
		free(encryptMsg_rsa);

		if(NULL != decrypt_password)
		{
			if(strcmp(decrypt_password,http_pass))	
			{
				free(decrypt_password);
				return FCGI_RESP_CODE_PERMISSION_DENY;		
			}

			free(decrypt_password);
		}
		else
		{
			return FCGI_RESP_CODE_PERMISSION_DENY;
		}
	}
	encryptMsg_base64 = (unsigned char *)malloc(base64_len + 1);
	if(NULL == encryptMsg_base64)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	encryptMsg_rsa = (unsigned char *)malloc(rsa_len + 1);	
	if(NULL == encryptMsg_rsa)
	{
		free(encryptMsg_base64);
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	memset(encryptMsg_base64, 0, base64_len + 1);
	memset(encryptMsg_rsa, 0, rsa_len + 1);

	if(!huayi_json_get_str(request_root,"hy_newpwd",(char *)encryptMsg_base64,base64_len + 1))
	{
		free(encryptMsg_base64);
		free(encryptMsg_rsa);
		return FCGI_RESP_CODE_DATA_MISS;
	}
	base64_decode((char *)encryptMsg_base64, strlen((char *)encryptMsg_base64), (char *)encryptMsg_rsa);
	decrypt_password = rsa_decrypt((void *)encryptMsg_rsa);   

	free(encryptMsg_base64);
	free(encryptMsg_rsa);

	if(NULL != decrypt_password)
	{
		huayi_uci_set_str(PKG_SYSTEM,"login","http_pass",decrypt_password);
		free(decrypt_password);
	}
	else
	{
		return FCGI_RESP_CODE_PERMISSION_DENY;
	}
	
#else
	if(huayi_uci_get_str(PKG_SYSTEM,"login","http_pass",http_pass))
	{
		if(!huayi_json_get_str(request_root,"hy_prepwd",prepassword,OPTION_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}
	
	if(!huayi_json_get_str(request_root,"hy_newpwd",newpassword,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}	

	if(strcmp(prepassword,http_pass))
	{
		return FCGI_RESP_CODE_PERMISSION_DENY;
	}

	huayi_uci_set_str(PKG_SYSTEM,"login","http_pass",newpassword);
		
#endif
	huayi_uci_commit(PKG_SYSTEM);

#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)

	session_logout("all");

	return FCGI_RESP_CODE_UNAUTHORIZED;
	
#else
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",ipaddr);
	if (strcmp(ipaddr,"0.0.0.0") == 0)
	{
		huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr);
	}
	
	cJSON_AddTrueToObject(response_root,"reconnection");

	cJSON_AddStringToObject(response_root,"reconnection_addr",ipaddr);

	return FCGI_RESP_CODE_SUCCESS;	
#endif

}

int get_quick_setting_config(cJSON * request_root,cJSON * response_root,int * lock_time)
{
	int ret = 0; 
#if defined(CONFIG_HUAYI_CGI_WIZARD_SUPPORT)
	ret =  do_get_quick_setting_config(request_root,response_root,lock_time);
	return ret;
#else
	int first_boot = 0;
	cJSON *result_root = NULL;
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	cJSON_AddNumberToObject(result_root,"first_boot",first_boot);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
#endif
}

int set_quick_setting_config(cJSON * request_root,cJSON * response_root,int * lock_time)
{
	int ret = 0;

#if defined(CONFIG_HUAYI_CGI_WIZARD_SUPPORT)
	ret = do_set_quick_setting_config(request_root,response_root,lock_time);
#else
	ret =  FCGI_RESP_CODE_SUCCESS;
#endif
	return ret;
}

#if defined(CONFIG_HUAYI_CGI_WIZARD_SUPPORT)
int get_all_quick_setting_config(cJSON * request_root,cJSON * response_root,int * lock_time)
{
	cJSON *result_root = NULL;
	int link = 0;
	int wan_proto = 0;
	int first_boot = 0;
	char proto[OPTION_STR_LEN] = {0};
	char time_zone[OPTION_STR_LEN] = {0};
	char ipaddr[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	//int sys_mode = 0;
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	char ssid[OPTION_STR_LEN] = {0}; 
	char encryption[OPTION_STR_LEN] = {0};
	char passphrase[TEMP_STR_LEN] = {0};
	int encry = 0;
#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
	
	cJSON_AddStringToObject(result_root,"proto",proto);
	
	/*Wireless	configuration*/
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)

	if(!huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid",ssid))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	if(!huayi_uci_get_str(PKG_WIRELESS,"radio0_0","encryption",encryption))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key",passphrase);

	if (strcmp(encryption,"none") == 0)
	{
		encry = 0;
	}
	else
	{
		encry = 1;
	}
	cJSON_AddStringToObject(result_root,"ssid",ssid);
	cJSON_AddNumberToObject(result_root,"encryption",encry);
	cJSON_AddStringToObject(result_root,"passphrase",passphrase);
#endif	

	/*Wire network configuration*/
	//huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);
	//cJSON_AddNumberToObject(result_root,"opmode",sys_mode);
	
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&link);
	
	cJSON_AddNumberToObject(result_root,"link",link);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","wan_proto",&wan_proto);

	cJSON_AddNumberToObject(result_root,"wan_proto",wan_proto);
	
	get_multiwan_static_config_by_index(result_root,"wan");

	get_multiwan_pppoe_config_by_index(result_root,"wan");

	get_multiwan_dhcp_config_by_index(result_root,"wan");	

	if(!huayi_uci_get_str(PKG_SYSTEM,"system","timezone",time_zone))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	cJSON_AddStringToObject(result_root,"time_zone",time_zone);

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	cJSON_AddStringToObject(result_root,"lan_ipaddr",ipaddr);

	if(!huayi_uci_get_str(PKG_NETWORK,"lan","netmask",netmask))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}	
	cJSON_AddStringToObject(result_root,"lan_netmask",netmask);
		
	huayi_uci_get_int(PKG_SYSTEM,"global","first_boot",&first_boot);

	cJSON_AddNumberToObject(result_root,"first_boot",first_boot);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int set_all_quick_setting_config(cJSON * request_root,cJSON * response_root,int * lock_time)
{
	char proto[OPTION_STR_LEN] = {0};
	int  ppp_mtu = 0;
	int  cc_mtu = 0;
	char cmd[CMD_STR_LEN] = {0};
	char time_zone[OPTION_STR_LEN] = {0};

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	char ssid[OPTION_STR_LEN] = {0}; 
	char passphrase[TEMP_STR_LEN] = {0};
	int  encry = 0;
	int  sync_manage_passwd = 0;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
	char tmpStr[OPTION_STR_LEN]={0};
#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	/*set wireless network config*/
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_2G)
	if(!huayi_json_get_str(request_root,"ssid",ssid,OPTION_STR_LEN)||
		!huayi_json_get_int(request_root,"encryption",&encry)||
		!huayi_json_get_str(request_root,"passphrase",passphrase,TEMP_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_str(PKG_WIRELESS,"radio0_0","ssid",ssid);
	
	if(encry == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio0_0","encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,"radio0_0","key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio0_0","encryption","psk-mixed");
		huayi_uci_set_str(PKG_WIRELESS,"radio0_0","key",passphrase);
	}

	huayi_json_get_int(request_root,"sync_manage_passwd",&sync_manage_passwd);
	if (sync_manage_passwd == 1)
	{
		huayi_uci_set_str(PKG_SYSTEM,"login","http_pass",passphrase);
	}
#endif

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_set_str(PKG_WIRELESS,"radio1_0","ssid",ssid);
	
	if(encry == 0)
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio1_0","encryption","none");
		huayi_uci_delete_option(PKG_WIRELESS,"radio1_0","key");
	}
	else
	{
		huayi_uci_set_str(PKG_WIRELESS,"radio1_0","encryption","psk-mixed");
		huayi_uci_set_str(PKG_WIRELESS,"radio1_0","key",passphrase);
	}
#endif

	if(!huayi_json_get_str(request_root,"proto",proto,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	/*set wire network config*/
	quick_set_lan_config(request_root,response_root);
	
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
#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
		if(huayi_uci_get_str(PKG_NETWORK,"wan6","proto",tmpStr)){
			huayi_uci_set_str(PKG_NETWORK,"wan6","proto","dhcpv6");
			huayi_uci_set_str(PKG_NETWORK,"wan6","ifname","pppoe-wan");
		}
#endif			
	}else if(!strcmp(proto,HUAYI_NET_PROTO_DHCP))
	{
		if(!set_multiwan_dhcp_config_by_index(request_root,"wan"))
		{
			huayi_uci_revert(PKG_NETWORK);
			return FCGI_RESP_CODE_DATA_MISS;
		}
		huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
		if(huayi_uci_get_str(PKG_NETWORK,"wan6","proto",tmpStr)){
			huayi_uci_set_str(PKG_NETWORK,"wan6","proto","dhcpv6");
			huayi_uci_set_str(PKG_NETWORK,"wan6","ifname","eth1");
		}
#endif			
	}

	huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);

	if(!huayi_json_get_str(request_root,"time_zone",time_zone,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}
	huayi_uci_set_str(PKG_SYSTEM,"system","timezone",time_zone);
	snprintf(cmd,sizeof(cmd),"echo '%s'> /etc/TZ",time_zone);
	system(cmd);

	huayi_uci_set_int(PKG_SYSTEM,"global","first_boot",0);

	huayi_uci_commit(PKG_WIRELESS);
	huayi_uci_commit(PKG_NETWORK);
	huayi_uci_commit(PKG_SYSTEM);

	system("huayi-reset network");

	*lock_time = HUAYI_RELOAD_NETWORK_TIME;

	//cJSON_AddTrueToObject(response_root,"reconnection");
	//cJSON_AddStringToObject(response_root,"reconnection_addr","");

	return FCGI_RESP_CODE_SUCCESS;
}

int set_skip_quick_setting(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int first_boot = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"first_boot",&first_boot))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_uci_set_int(PKG_SYSTEM,"global","first_boot",first_boot);
	huayi_uci_commit(PKG_SYSTEM);

	return FCGI_RESP_CODE_SUCCESS;
}
#endif



int set_system_reboot(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char ipaddr[OPTION_STR_LEN]		  =  {0};
	int opmode = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	*lock_time = HUAYI_SYSTEM_REBOOT_TIME;

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode);
	if(opmode == HUAYI_ROUTE_MODE)
	{
		huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",ipaddr);
		if (strcmp(ipaddr,"0.0.0.0") == 0)
		{
			huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr);
		}
	}else
	{
		huayi_uci_get_str(PKG_ABOUT,"about_system","domain_name",ipaddr);
	}
	
	system("huayi-reset reboot");

	cJSON_AddTrueToObject(response_root,"reconnection");

	cJSON_AddStringToObject(response_root,"reconnection_addr",ipaddr);

	return FCGI_RESP_CODE_SUCCESS;
}

int set_system_factory(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char	ipaddr[IPV4_STR_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	*lock_time = HUAYI_SYSTEM_RESET_TIME;

	system("huayi-reset reset");

	huayi_uci_get_str(PKG_NETWORK,"lan","def_ip",ipaddr);

	cJSON_AddTrueToObject(response_root,"reconnection");

	cJSON_AddStringToObject(response_root,"reconnection_addr",ipaddr);

	return FCGI_RESP_CODE_SUCCESS;
}

int get_upgrade_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	char product_model[OPTION_STR_LEN] = {0};
	char buildtime[OPTION_STR_LEN] = {0};
	char fw_version[OPTION_STR_LEN]={0};
	char module_version[OPTION_STR_LEN]={0};
	char tmp_str[SMALL_STR_LEN] = {0};
	int  flash_size = 0;
	int  size_tip = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	if(!huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version",fw_version))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_ABOUT,"about_software","buildtime",buildtime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_ABOUT,"about_hardware","flash_size",&flash_size))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	size_tip = flash_size/1000000;
	snprintf(tmp_str,SMALL_STR_LEN,"%dM",size_tip);
	
	cJSON_AddStringToObject(result_root,"pro_model",product_model);
	cJSON_AddStringToObject(result_root,"release_time",buildtime);
	cJSON_AddStringToObject(result_root,"fw_version",fw_version);
	cJSON_AddStringToObject(result_root,"module_version",module_version);
	cJSON_AddNumberToObject(result_root,"maxsize",flash_size);
	cJSON_AddStringToObject(result_root,"maxsize_tip",tmp_str);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}



int get_ping_detect_result(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int	ping_detct = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","ping_detect",&ping_detct);

	cJSON_AddNumberToObject(result_root,"ping_detct",ping_detct);

	if(!huayi_add_file_to_json(result_root,"action_content","/tmp/ping_result"))
	{
		if(ping_detct != 0)
			cJSON_AddStringToObject(result_root,"action_content","ping error.100% packet loss");
		else
			cJSON_AddStringToObject(result_root,"action_content","");
	}
	
	if(ping_detct == HUAYI_ACTION_COMPLETE)
	{
		system("huayi_cmd set system ping_detect 0");
		remove("/tmp/ping_result");
	}
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int do_ping_detect(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char 	dest_host[OPTION_STR_LEN] = {0};
	char	start_host[OPTION_STR_LEN] = {0};
	int		ping_counter = 0;		
	char	cmd[CMD_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"dest_host_addr",dest_host,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!huayi_json_get_str(request_root,"start_host_addr",start_host,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!huayi_json_get_int(request_root,"ping_num",&ping_counter))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(strlen(start_host))
	{
		snprintf(cmd,sizeof(cmd),"/etc/action.d/ping_detect -I %s %s -c %d >/tmp/ping_result &",start_host,dest_host,ping_counter);
	}else
	{
		snprintf(cmd,sizeof(cmd),"/etc/action.d/ping_detect %s -c %d >/tmp/ping_result &",dest_host,ping_counter);
	}
	system(cmd);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int stop_ping_detect(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	system("killall ping");

	return FCGI_RESP_CODE_SUCCESS;
}


int get_tracert_detect_result(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int	tracert_detect = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","tracert_detect",&tracert_detect);

	cJSON_AddNumberToObject(result_root,"tracert_detect",tracert_detect);

	if(!huayi_add_file_to_json(result_root,"action_content","/tmp/tracert_result"))
	{
		if(tracert_detect != 0)
			cJSON_AddStringToObject(result_root,"action_content","error.100% packet loss");
		else
			cJSON_AddStringToObject(result_root,"action_content","");
	}
	
	if(tracert_detect == HUAYI_ACTION_COMPLETE)
	{
		system("huayi_cmd set system tracert_detect 0");
		
		remove("/tmp/tracert_result");
	}
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int do_tracert_detect(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char 	dest_host[OPTION_STR_LEN] = {0};
	int		max_hops = 0;
	int		time_out = 0;		
	char	cmd[CMD_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"dest_host_addr",dest_host,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!huayi_json_get_int(request_root,"max_hops",&max_hops))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!huayi_json_get_int(request_root,"timeout",&time_out))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}
	
	snprintf(cmd,sizeof(cmd),"/etc/action.d/traceroute_detect -m %d -w %d %s >/tmp/tracert_result &",max_hops,time_out,dest_host);

	system(cmd);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int stop_tracert_detect(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	system("killall -9 traceroute");

	return FCGI_RESP_CODE_SUCCESS;
}

#if defined(CONFIG_PACKAGE_HUAYI_DHCP_DETECT)
int get_dhcp_detect_result(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *dhcp_detecet_array = NULL;
	cJSON *dhcp_server_obj = NULL;
	int	dhcp_detect = 0;
	char	section[OPTION_STR_LEN] = {0};
	int	 i = 0;
	char host_ip[IPV4_STR_LEN] = {0};
	char host_mac[MAC_STR_LEN] = {0};
	
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","dhcp_detect",&dhcp_detect);

	cJSON_AddNumberToObject(result_root,"ping_detct",dhcp_detect);

	dhcp_detecet_array = cJSON_CreateArray();
	
	if(dhcp_detect == HUAYI_ACTION_COMPLETE)
	{
		system("huayi_cmd set system dhcp_detect 0");
		while(1)
		{
			memset(section,0,sizeof(section));
			memset(host_ip,0,sizeof(host_ip));
			memset(host_mac,0,sizeof(host_mac));
			snprintf(section,sizeof(section),"@dhcp_detect[%d]",i);
			if(!huayi_uci_get_str(PKG_DHCP_DETECT_RESLUT,section,"ip",host_ip))
			{
				break;
			}
			huayi_uci_get_str(PKG_DHCP_DETECT_RESLUT,section,"mac",host_mac);
			
			dhcp_server_obj = cJSON_CreateObject();
			
			cJSON_AddStringToObject(dhcp_server_obj,"ip_addr",host_ip);
		
			cJSON_AddStringToObject(dhcp_server_obj,"mac_addr",host_mac);

			cJSON_AddItemToArray(dhcp_detecet_array,dhcp_server_obj);
			
			i++;
		}
		remove("/tmp/dhcp_detect_result");
	}

	cJSON_AddItemToObject(result_root,"action_arr",dhcp_detecet_array);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int do_dhcp_detect(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char	cmd[CMD_STR_LEN] = {0};
	int	dhcp_detect = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","dhcp_detect",&dhcp_detect);
	if(dhcp_detect != HUAYI_ACTION_DOING)
	{
		snprintf(cmd,sizeof(cmd),"/usr/bin/huayi_dhcp_detect -I br-lan &");
		system(cmd);
	}
	
	return FCGI_RESP_CODE_SUCCESS;
}


int stop_dhcp_detect(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	system("killall huayi_dhcp_detect");

	return FCGI_RESP_CODE_SUCCESS;
}
#endif


#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
int get_ddns_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	int ddns_enable = 0;
	char provider[OPTION_STR_LEN] = {0};
	char domain_name[OPTION_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	char ddns_ip[IPV4_STR_LEN] = {0};
	int	 ddns_state = 0;
	int  wan_link = 0;
	int  wan_up = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	
	huayi_uci_get_int(PKG_DDNS,"wan","enable",&ddns_enable);
	
	huayi_add_switch_to_json(result_root,"ddns_admin",ddns_enable);

	huayi_uci_get_str(PKG_DDNS,"wan","provider",provider);

	cJSON_AddStringToObject(result_root,"server_addr",provider);

	huayi_uci_get_str(PKG_DDNS,"wan","domain",domain_name);

	cJSON_AddStringToObject(result_root,"domain_name",domain_name);
	
	huayi_uci_get_str(PKG_DDNS,"wan","account",username);
	
	cJSON_AddStringToObject(result_root,"usrname",username);

	huayi_uci_get_str(PKG_DDNS,"wan","password",password);
	
	cJSON_AddStringToObject(result_root,"pwd",password);

	huayi_uci_get_str(PKG_STATUS_SYSTEM,"sys","ddns_ip",ddns_ip);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","ddns_state",&ddns_state);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&wan_link);
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up);

	if (wan_link == 1 && wan_up == 1)
	{
		cJSON_AddStringToObject(result_root,"ddns_addr",ddns_ip);
		cJSON_AddNumberToObject(result_root,"ddns_state",ddns_state);
	}
	else
	{
		cJSON_AddStringToObject(result_root,"ddns_addr","0.0.0.0");
		cJSON_AddNumberToObject(result_root,"ddns_state",0);
	}
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int set_ddns_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	int ddns_enable = 0;
	char provider[OPTION_STR_LEN] = {0};
	char domain_name[OPTION_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"ddns_admin",&ddns_enable)||
		!huayi_json_get_str(request_root,"server_addr",provider,OPTION_STR_LEN)||
		!huayi_json_get_str(request_root,"domain_name",domain_name,OPTION_STR_LEN)||
		!huayi_json_get_str(request_root,"usrname",username,OPTION_STR_LEN)||
		!huayi_json_get_str(request_root,"pwd",password,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_DDNS,"wan","enable",ddns_enable);

	huayi_uci_set_str(PKG_DDNS,"wan","provider",provider);

	huayi_uci_set_str(PKG_DDNS,"wan","domain",domain_name);

	huayi_uci_set_str(PKG_DDNS,"wan","account",username);

	huayi_uci_set_str(PKG_DDNS,"wan","password",password);

	huayi_uci_commit(PKG_DDNS);

	system("huayi-reset ddns");
	
	return FCGI_RESP_CODE_SUCCESS;
}
#endif


int get_ntp_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int ntp_enable = 0;
	unsigned long time_of_day = 0;
	char ntp_server[OPTION_STR_LEN] = {0};
	char ntp_server_list[LONG_BUFF_LEN] ={0};
	char time_zone[OPTION_STR_LEN] = {0};
	cJSON *result_root = NULL;
	cJSON *ntp_server_array = NULL;
	int i = 0;
	time_t timep;
	struct tm *p = NULL;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_int(PKG_SYSTEM,"ntp","enabled",&ntp_enable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_SYSTEM,"ntp","server",ntp_server_list))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_SYSTEM,"system","timezone",time_zone))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	time(&timep);
			
	p=localtime(&timep);

	time_of_day = timep + p->tm_gmtoff - 28800;

	result_root = cJSON_CreateObject();
	
	ntp_server_array = cJSON_CreateArray();

	while(huayi_get_nth_value(i++, ntp_server_list, WEB_SPACE_DELIMIT, ntp_server, sizeof(ntp_server)))
	{
		cJSON_AddItemToArray(ntp_server_array,cJSON_CreateString(ntp_server));
	}

	cJSON_AddItemToObject(result_root,"ntp_server_arr",ntp_server_array);

	huayi_add_switch_to_json(result_root,"ntp_admin",ntp_enable);
	
	cJSON_AddStringToObject(result_root,"time_zone",time_zone);

	cJSON_AddNumberToObject(result_root,"time_day",time_of_day);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
	
}

int set_ntp_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int ntp_enable = 0;
	char time_zone[OPTION_STR_LEN] = {0};
	cJSON *ntp_server_array = NULL;
	cJSON *ntp_server_obj = NULL;
	int i = 0;
	char cmd[CMD_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"ntp_admin",&ntp_enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}
	
	if(!huayi_json_get_str(request_root,"time_zone",time_zone,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!(ntp_server_array= cJSON_GetObjectItem(request_root,"ntp_server_arr")))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_delete_option(PKG_SYSTEM,"ntp","server");
	for(i=0;i< cJSON_GetArraySize(ntp_server_array);i++)
	{
		ntp_server_obj = cJSON_GetArrayItem(ntp_server_array,i);
		huayi_uci_add_list(PKG_SYSTEM,"ntp","server",ntp_server_obj->valuestring);
	}
	
	if(!huayi_uci_set_int(PKG_SYSTEM,"ntp","enabled",ntp_enable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_set_str(PKG_SYSTEM,"system","timezone",time_zone))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_commit(PKG_SYSTEM))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	snprintf(cmd,sizeof(cmd),"echo '%s'> /etc/TZ",time_zone);

	system(cmd);

	system("huayi-reset ntpd");
	
	return FCGI_RESP_CODE_SUCCESS;
	
}


int use_browser_time(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char browser_time[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_str(request_root,"current_time",browser_time,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}
	
	snprintf(cmd,sizeof(cmd),"date -s '%s'",browser_time);

	system("/etc/init.d/sysntpd stop");
	
	system(cmd);

	return FCGI_RESP_CODE_SUCCESS;
	
}

int get_syslog_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int syslog_enable = 0;
	cJSON *result_root = NULL;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_int(PKG_SYSTEM,"syslog","enabled",&syslog_enable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	result_root = cJSON_CreateObject();

	huayi_add_switch_to_json(result_root,"syslog_admin",syslog_enable);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int set_syslog_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int syslog_enable = 0;
	int log_size = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"syslog_admin",&syslog_enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}
	
	if(!huayi_uci_set_int(PKG_SYSTEM,"syslog","enabled",syslog_enable))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_set_int(PKG_SYSTEM,"system","log_size",log_size))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_commit(PKG_SYSTEM))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	return FCGI_RESP_CODE_SUCCESS;
}

int display_syslog(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int syslog_enable = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_SYSTEM,"syslog","enabled",&syslog_enable);
	if(syslog_enable == FCGI_TRUE)
	{
		system("logread >/tmp/logread.log");
		
		huayi_add_file_to_json(result_root,"log_content","/tmp/logread.log");

		remove("/tmp/logread.log");
		
	}else
	{
		cJSON_AddStringToObject(result_root,"log_content","");
	}
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int clean_syslog(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	system("/etc/init.d/log restart");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int get_sche_reboot_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int sche_reboot_mode= 0;
	cJSON *result_root = NULL;
	int rectime = 0;
	int weeks = 0;
	int hour = 0;
	int minute = 0;
	int rec_uptime=0;
	int uptime = 0;
	int interval = 0;
	static int remaining_time = 0;
	int i = 0;
	int k = 0;
	FCGI_BOOL this_week = FCGI_FALSE;
	struct tm *p = NULL;
	time_t timep;
	unsigned long time_of_day = 0;
	int cur_time = 0;
	int next_time = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","sche_reboot_mode",&sche_reboot_mode))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","rectime",&rectime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","weeks",&weeks))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","hour",&hour))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	
	if(!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","minute",&minute))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	time(&timep);
			
	p=localtime(&timep);

	time_of_day = timep + p->tm_gmtoff - 28800;

	switch(sche_reboot_mode)
	{
		case HUAYI_SCHE_REBOOT_MODE_REC_TIME:
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rec_uptime",&rec_uptime);
			if(rec_uptime == 0)
			{
				remaining_time = rectime;
			}else
			{
				remaining_time = rectime - (uptime - rec_uptime);
			 	if(remaining_time < 0)
			 	{
			 		remaining_time = 0;
			 	}
			}
			break;
		case HUAYI_SCHE_REBOOT_MODE_SPE_TIME:
			i = p->tm_wday;
			cur_time = (p->tm_wday *1440 + p->tm_hour*60 + p->tm_min);
			weeks = (weeks == 255 ? 127 : weeks);
			for(k=i;k<7;k++)
			{
				if(MyGetField32(weeks,k))
				{
					next_time = ( k * 1440 +hour *60+minute);
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
						next_time = ( k * 1440 +hour *60+minute);
						interval = next_time + ( (1440 * 7 ) - cur_time);
						break;
					}
				}
			}

			//interval = (interval == 0 ? 1 : interval);
			if (interval > 0)
				remaining_time = (interval * 60) - p->tm_sec;
			else
				remaining_time = 0;
			break;
		case HUAYI_SCHE_REBOOT_MODE_OFF:
		default:
			remaining_time = 0;
			break;
	}

	result_root = cJSON_CreateObject();

	cJSON_AddNumberToObject(result_root,"sche_mode",sche_reboot_mode);

	cJSON_AddNumberToObject(result_root,"rectime",rectime);
	weeks = (weeks == 127 ? 255 : weeks);
	cJSON_AddNumberToObject(result_root,"weeks",weeks);

	cJSON_AddNumberToObject(result_root,"hour",hour);

	cJSON_AddNumberToObject(result_root,"minute",minute);

	cJSON_AddNumberToObject(result_root,"remaining",remaining_time);

	cJSON_AddNumberToObject(result_root,"time_day",time_of_day);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int set_sche_reboot_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int sche_reboot_mode = 0;
	int rectime=0;
	int weeks = 0;
	int hour = 0;
	int minute = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"sche_mode",&sche_reboot_mode))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(sche_reboot_mode == 1)
	{
		if(!huayi_json_get_int(request_root,"rectime",&rectime))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}else if(sche_reboot_mode == 2)
	{
		if(!huayi_json_get_int(request_root,"weeks",&weeks)||
			!huayi_json_get_int(request_root,"hour",&hour) ||
			!huayi_json_get_int(request_root,"minute",&minute))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
	}

	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","sche_reboot_mode",sche_reboot_mode);
	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","rectime",rectime);
	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","weeks",weeks);
	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","hour",hour);
	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","minute",minute);

	system("huayi_cmd set system rec_uptime 0");

	huayi_uci_commit(PKG_SYSTEM);

	system("huayi-reset sche_reboot");
	
	return FCGI_RESP_CODE_SUCCESS;
}


int get_system_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	char product_name[OPTION_STR_LEN]={0};
	char product_model[OPTION_STR_LEN]={0};
	char software_version[OPTION_STR_LEN]={0};
	char webui_version[OPTION_STR_LEN]={0};
	char lan_mac[MAC_STR_LEN]={0};
	char wan_ip[IPV4_STR_LEN]={0};
	int  wan_link = 0;
	int  wan_up = 0;
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	char wan_type[OPTION_STR_LEN] = {0};
	int sim_state = 0;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_str(PKG_ABOUT,"about_system","product_name",product_name);

	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);

	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version",software_version);

	snprintf(webui_version,sizeof(webui_version),"WEB%s",software_version);

	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","mac_address",lan_mac);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&wan_link);
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up);

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	huayi_uci_get_int(PKG_STATUS_MODEM,"modem","sim_ready",&sim_state);
	
	if (!strcmp(wan_type,WAN_TYPE_4G_MOBILE))
	{
		if (sim_state != MODEM_SIN_READY)
			wan_up = 0;
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_WIRED_PRIOROTY))
	{
		if (wan_link == 0)
		{
			if (sim_state != MODEM_SIN_READY)
			{
				wan_up = 0;
			}
		}
	}
#elif defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_type, WAN_TYPE_REP4G_PRIOROTY))
	{
		if (wan_link == 0)
		{
			if (sim_state != MODEM_SIN_READY)
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
	
	if (wan_up == 1)
	{
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",wan_ip);
	}
	else
	{
		strcpy(wan_ip,"0.0.0.0");
	}


	cJSON_AddStringToObject(result_root,"product_name",product_name);

	cJSON_AddStringToObject(result_root,"product_model",product_model);

	cJSON_AddStringToObject(result_root,"software_version",software_version);

	cJSON_AddStringToObject(result_root,"webui_version",webui_version);

	cJSON_AddStringToObject(result_root,"lan_mac",lan_mac);

	cJSON_AddStringToObject(result_root,"ip_address",wan_ip);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int start_cloud_upgrade_detect(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	/*start check upgrade detect here*/
	system("upgrade_sys.sh check &");
	
	return FCGI_RESP_CODE_SUCCESS;
	
}

int start_cloud_firmware_upgrade(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	/*start cloud firmware upgrade here*/
	system("upgrade_sys.sh cpe &");
	
	return FCGI_RESP_CODE_SUCCESS;
	
}

int start_cloud_module_upgrade(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	/*start cloud module upgrade here*/
	system("upgrade_sys.sh modem &");
	
	return FCGI_RESP_CODE_SUCCESS;
	
}

int get_overview_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	int wan_up = 0, wan_link = 0;
	int uptime = 0, wan_uptime = 0, network_linktime = 0;
	unsigned long long rx_bytes = 0, tx_bytes = 0;
	unsigned long long tx_bbbps = 0, rx_bbbps = 0;
	unsigned long long rx_bps = 0, tx_bps = 0;
	int wlan_2g_disabled = 0;
#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int wlan_5g_disabled = 0;
#endif
	int client_num = 0;
	char proto[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&wan_link))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_uptime",&wan_uptime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","rx_bytes",&rx_bytes))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","tx_bytes",&tx_bytes))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","rx_bps",&rx_bps))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","tx_bps",&tx_bps))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&wlan_2g_disabled))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_uci_get_int(PKG_WIRELESS,"radio1","disabled",&wlan_5g_disabled))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
#endif
	get_active_arp(1,NULL,&client_num);	

	huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
	
	if (wan_link == 0)
	{
		wan_up = 0;
		cJSON_AddNumberToObject(result_root,"link_status",0);
	}
	else if(wan_up == 0 && strcmp(proto,HUAYI_NET_PROTO_PPPOE) == 0)
	{
		cJSON_AddNumberToObject(result_root,"link_status",2);
	}
	else if(wan_up == 0)
	{
		cJSON_AddNumberToObject(result_root,"link_status",3);
	}
	else
	{
		cJSON_AddNumberToObject(result_root,"link_status",1);
	}
	
	cJSON_AddNumberToObject(result_root,"wan_up",wan_up);

	if(0 == wan_uptime)
	{
		network_linktime = 0;
	}
	else
	{
		network_linktime = uptime - wan_uptime;
	}	

	tx_bbbps = ((unsigned long long)tx_bps * 8);
	rx_bbbps = ((unsigned long long)rx_bps * 8);
	
	cJSON_AddNumberToObject(result_root,"network_linktime",network_linktime);
	cJSON_AddNumberToObject(result_root,"total_rx_bytes",rx_bytes);
	cJSON_AddNumberToObject(result_root,"total_tx_bytes",tx_bytes);
	cJSON_AddNumberToObject(result_root,"rx_bps",rx_bbbps);
	cJSON_AddNumberToObject(result_root,"tx_bps",tx_bbbps);
	cJSON_AddNumberToObject(result_root,"wifi_2g",wlan_2g_disabled == 0?1:0);
#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	cJSON_AddNumberToObject(result_root,"wifi_5g",wlan_5g_disabled == 0?1:0);
#endif
	cJSON_AddNumberToObject(result_root,"client_num",client_num);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}


int get_opmode_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	char apcli_bssid[OPTION_STR_LEN]={0};
	char ip_address[OPTION_STR_LEN]={0};
	int wan_up = 0, wan_link = 0;
	int network_up = 0,linkport = 0;
	int opmode = 0;
	int uptime = 0, wan_uptime = 0, network_linktime = 0;
	unsigned long long rx_bytes = 0, tx_bytes = 0;
	unsigned long long rx_bps = 0, tx_bps = 0;
	int wlan_2g_disabled = 0;
#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int wlan_5g_disabled = 0;
#endif
	int client_num = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	if(!huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&wan_link))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_uptime",&wan_uptime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","rx_bytes",&rx_bytes))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","tx_bytes",&tx_bytes))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","rx_bps",&rx_bps))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","tx_bps",&tx_bps))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&wlan_2g_disabled))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_uci_get_int(PKG_WIRELESS,"radio1","disabled",&wlan_5g_disabled))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
#endif
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",ip_address);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"lan","linkport",&linkport);

	get_active_arp(1,NULL,&client_num);	

	if (opmode == HUAYI_ROUTE_MODE)
	{
		if (wan_link == 1 && wan_up == 1)
		{
			network_up = 1;
		}
	}
	else if (opmode == HUAYI_AP_MODE)
	{
		
		if(0 != linkport && strlen(ip_address)>=7 && strcmp("0.0.0.0", ip_address) != 0)
		{
			network_up = 1;
		}
	}
	else if (opmode == HUAYI_REP_MODE)
	{
		get_cmd_result("iwconfig apcli0 | grep 'Access Point' | awk '{print $5}'", apcli_bssid, sizeof(apcli_bssid));
		
		if (0 != strncmp("Not-Associated", apcli_bssid, strlen("Not-Associated")))
		{
			if(strlen(ip_address)>=7 && strcmp("0.0.0.0", ip_address) != 0)
			{
				network_up = 1;
			}
		}
	}
	else if (opmode == HUAYI_WISP_MODE)
	{
		get_cmd_result("iwconfig apcli0 | grep 'Access Point' | awk '{print $5}'", apcli_bssid, sizeof(apcli_bssid));
		
		if (wan_up == 1 && 0 != strncmp("Not-Associated", apcli_bssid, strlen("Not-Associated")))
		{
			network_up = 1;
		}
	}
	
	cJSON_AddNumberToObject(result_root,"network_up",network_up);

	if(0 == wan_uptime)
	{
		network_linktime = 0;
	}
	else
	{
		network_linktime = uptime - wan_uptime;
	}

	cJSON_AddNumberToObject(result_root,"sysmode",opmode);
	
	cJSON_AddNumberToObject(result_root,"network_linktime",network_linktime);
	cJSON_AddNumberToObject(result_root,"total_rx_bytes",rx_bytes);
	cJSON_AddNumberToObject(result_root,"total_tx_bytes",tx_bytes);
	cJSON_AddNumberToObject(result_root,"rx_bps",rx_bps);
	cJSON_AddNumberToObject(result_root,"tx_bps",tx_bps);
	cJSON_AddNumberToObject(result_root,"wifi_2g",wlan_2g_disabled == 0?1:0);
#if  defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	cJSON_AddNumberToObject(result_root,"wifi_5g",wlan_5g_disabled == 0?1:0);
#endif
	cJSON_AddNumberToObject(result_root,"client_num",client_num);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}


int get_all_system_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	char product_name[OPTION_STR_LEN] = {0};
	char company_website[OPTION_STR_LEN] = {0};
	char software_version[OPTION_STR_LEN]={0};
	char webui_version[OPTION_STR_LEN]={0};
	char buildtime[OPTION_STR_LEN] = {0};
	char time_of_day[OPTION_STR_LEN]={0};
	char domain_name[OPTION_STR_LEN]={0};
	int uptime = 0;
	int cpu_rt_usage = 0;
	int mem_rt_usage = 0;
	int flash_usage = 0;
	unsigned char sn[OPTION_STR_LEN]={ 0 };
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	char imei[OPTION_STR_LEN] = { 0 };
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();
	
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_name",product_name);

	cJSON_AddStringToObject(result_root,"pro_name",product_name);

	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version",software_version);
	
	cJSON_AddStringToObject(result_root,"fw_version",software_version);

	huayi_uci_get_str(PKG_ABOUT,"about_software","webui_version",webui_version);
	cJSON_AddStringToObject(result_root,"webui_version",webui_version);

	huayi_uci_get_str(PKG_ABOUT,"about_software","buildtime",buildtime);

	cJSON_AddStringToObject(result_root,"build_time",buildtime);
	
	huayi_uci_get_str(PKG_ABOUT,"about_system","company_website",company_website);
	
	cJSON_AddStringToObject(result_root,"company_domain",company_website);

	huayi_uci_get_str(PKG_ABOUT,"about_system","domain_name",domain_name);
	
	cJSON_AddStringToObject(result_root,"domain",domain_name);

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
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	if (get_dev_sn(sn) < 0) 
	{
		strcpy(sn, " ");
	}
	if(strncmp(sn, "FFFFFFFFFFFF", strlen(sn)) == 0)
	{
		strcpy(sn, " ");
	}
	cJSON_AddStringToObject(result_root,"device_umber", sn);
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", imei);
	cJSON_AddStringToObject(result_root,"IMEI", imei);
#endif
	return FCGI_RESP_CODE_SUCCESS;
}

int get_all_network_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	int i = 1,k = 0;
	int wifi_on=0;
	char section[OPTION_STR_LEN] = {0};
	cJSON *port_array = NULL;
	cJSON *port_obj = NULL;
	cJSON *rep_obj = NULL;
	cJSON *network_array = NULL;
	cJSON *wireless_array = NULL;
	cJSON *network_obj = NULL;
	cJSON *wireless_obj = NULL;
	int link = 0,speed = 0,wanup = 0;
	int netlink = 0;
	unsigned long wifi_24g_num = 0;
	unsigned long wifi_58g_num = 0;
	unsigned long long txbytes=0,rxbytes=0;
	unsigned long long tx_bps=0,rx_bps=0;
	int tx_Bps=0,rx_Bps=0;
	char mac_address[MAC_STR_LEN] = {0};
	char ip_address[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char gateway[IPV4_STR_LEN] = {0};
	char temp_dns[OPTION_STR_LEN] = {0};
	char dns[IPV4_STR_LEN] = {0};
	char bssid[OPTION_STR_LEN] = {0};
	char encryptype[OPTION_STR_LEN] = {0},ssid[OPTION_STR_LEN] = {0};
	char *ptr = NULL;
	int channel_auto = 0,wifimode = 0,channel = 0;
	int radio_id = 0;
	int	sys_mode = 0;
	int rep_status = 0, rep_24g_status = 0, rep_58g_status = 0;
	int linkport = 0;
	char portname[SMALL_STR_LEN] = {0};
	char run_cmd[CMD_STR_LEN]= {0};

	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	network_array = cJSON_CreateArray();
	
	port_array = cJSON_CreateArray();

	if (*lock_time >= FASTCGI_UPDATE_INFO_INTERVAL)
	{
		system("ifconfig > /tmp/ifconfig_info");
		system("iwconfig ra0 > /tmp/iwconfig_ra0");
		system("iwconfig rai0 > /tmp/iwconfig_rai0");

		get_wifi_stamacnum("ra0",0,&wifi_24g_num);
		snprintf(run_cmd,CMD_STR_LEN,"huayi_cmd set system wifi_24g_num %d",wifi_24g_num);	
		system(run_cmd);

		get_wifi_stamacnum("rai0",1,&wifi_58g_num);
		snprintf(run_cmd,CMD_STR_LEN,"huayi_cmd set system wifi_58g_num %d",wifi_58g_num);	
		system(run_cmd);

		*lock_time = FASTCGI_UPDATE_INFO_FLAG;
	}
	else
	{
		huayi_uci_get_int(PKG_STATUS_SYSTEM, "sys", "wifi_24g_num",&wifi_24g_num);
		huayi_uci_get_int(PKG_STATUS_SYSTEM, "sys", "wifi_58g_num",&wifi_58g_num);
	}
	
	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);
	cJSON_AddNumberToObject(result_root,"sys_mode",sys_mode);

	if (sys_mode == HUAYI_REP_MODE || sys_mode == HUAYI_WISP_MODE)
	{
		huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_24g_status",&rep_24g_status);	
		
		huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_58g_status",&rep_58g_status);	
		
		if (rep_58g_status == 1)
		{
			radio_id = 1;
		}
	}
	rep_status = rep_24g_status|rep_58g_status;
	
	if (sys_mode == HUAYI_REP_MODE)
	{
		rep_obj = cJSON_CreateObject();
		
		snprintf(section,OPTION_STR_LEN,"radio%d_cli0",radio_id);
		
		huayi_uci_get_str(PKG_WIRELESS,section,"apcli_ssid",ssid);
		cJSON_AddStringToObject(rep_obj,"ssid",ssid);
		
		huayi_uci_get_str(PKG_WIRELESS,section,"apcli_bssid",bssid);
		if(strlen(bssid) != 17)
		{
			if (radio_id == 0)
			{
				get_cmd_result("iwconfig apcli0 | grep 'Access Point' | awk '{print $5}'", run_cmd, sizeof(run_cmd));
			}
			else
			{
				get_cmd_result("iwconfig apclii0 | grep 'Access Point' | awk '{print $5}'", run_cmd, sizeof(run_cmd));
			}
			
			if(0 != strncmp("00:00:00:00:00:00", run_cmd, 17))
			{
				strncpy(bssid, run_cmd, 17);
				huayi_uci_set_str(PKG_WIRELESS,section,"apcli_bssid",bssid);
				huayi_uci_commit(PKG_WIRELESS);
			}
		}

		cJSON_AddStringToObject(rep_obj,"bssid",bssid);
		
		cJSON_AddNumberToObject(rep_obj,"rep_status",rep_status);

		cJSON_AddItemToObject(result_root,"rep_info",rep_obj);
	}

	if (sys_mode == HUAYI_ROUTE_MODE || sys_mode == HUAYI_WISP_MODE)
	{
		huayi_uci_get_int(PKG_STATUS_NETWORK,"lan","linkport",&linkport);
		
		//LAN
		for(i = HUAYI_LAN1_PORT_NUM_OFFSET;i < HUAYI_LAN1_PORT_NUM_OFFSET+(HUAYI_ETH_PORT_NUM-1);i++)
		{
			port_obj = cJSON_CreateObject();

			link = 0;
			speed = 100;

			if (linkport & (0x1 << i))
			{
				link = 1;
			}

			snprintf(portname,SMALL_STR_LEN,"LAN%d",(i-HUAYI_LAN1_PORT_NUM_OFFSET)+1);
			cJSON_AddStringToObject(port_obj,"port",portname);

			cJSON_AddNumberToObject(port_obj,"link",link);		
			cJSON_AddNumberToObject(port_obj,"speed",speed);

			cJSON_AddItemToArray(port_array,port_obj);
		}

		//WAN
		{
			port_obj = cJSON_CreateObject();
			
			i = 4;
			link = 0;
			speed = 100;

			if (linkport & (0x1 << i))
			{
				link = 1;
			}

			if (sys_mode == HUAYI_ROUTE_MODE)
			{
				cJSON_AddStringToObject(port_obj,"port","WAN");
			}
			else
			{
				cJSON_AddStringToObject(port_obj,"port","LAN4");
			}

			cJSON_AddNumberToObject(port_obj,"link",link);
			cJSON_AddNumberToObject(port_obj,"speed",speed);

			cJSON_AddItemToArray(port_array,port_obj);
		}

		cJSON_AddItemToObject(result_root,"port_arr",port_array);
	}


	network_obj = cJSON_CreateObject();

	snprintf(section,sizeof(section),"lan");
	
	cJSON_AddStringToObject(network_obj,"network_mode","lan");
	
	huayi_uci_get_str(PKG_NETWORK,"lan","def_macaddr",mac_address);
	
	cJSON_AddStringToObject(network_obj,"mac_addr",mac_address);

	huayi_uci_get_str(PKG_STATUS_NETWORK,section,"ip_address",ip_address);
	
	cJSON_AddStringToObject(network_obj,"ip_addr",ip_address);

	huayi_uci_get_str(PKG_STATUS_NETWORK,section,"netmask",netmask);
	
	cJSON_AddStringToObject(network_obj,"subnet_mask",netmask);

	cJSON_AddItemToArray(network_array,network_obj);
	
	if (sys_mode == HUAYI_ROUTE_MODE || sys_mode == HUAYI_WISP_MODE)
	{
		link = 0;
		snprintf(section,sizeof(section),"wan");
		network_obj = cJSON_CreateObject();	
		cJSON_AddStringToObject(network_obj,"network_mode","wan");
		huayi_uci_get_int(PKG_STATUS_NETWORK,section,"link",&link);	
		huayi_uci_get_int(PKG_STATUS_NETWORK,section,"wan_up",&wanup);	
		if (sys_mode == HUAYI_ROUTE_MODE)
		{
			if (link == 1 && wanup == 1)
			{
				netlink = 1;
			}	
		}
		else if (sys_mode == HUAYI_WISP_MODE)
		{
			if (rep_status == 1 && wanup == 1)
			{
				netlink = 1;
			}	
		}
		cJSON_AddNumberToObject(network_obj,"link",netlink);
		
		memset(mac_address,0,MAC_STR_LEN);			
		huayi_uci_get_str(PKG_STATUS_NETWORK,section,"mac_address",mac_address);
		cJSON_AddStringToObject(network_obj,"mac_addr",mac_address);
		memset(ip_address,0,IPV4_STR_LEN);			
		huayi_uci_get_str(PKG_STATUS_NETWORK,section,"ip_address",ip_address);	
		memset(netmask,0,IPV4_STR_LEN);			
		huayi_uci_get_str(PKG_STATUS_NETWORK,section,"netmask",netmask);			
		memset(gateway,0,IPV4_STR_LEN);			
		huayi_uci_get_str(PKG_STATUS_NETWORK,section,"gateway",gateway);			
		memset(temp_dns,0,OPTION_STR_LEN);			
		huayi_uci_get_str(PKG_STATUS_NETWORK,section,"dns_server",temp_dns);			
					
		k = 0;			
		memset(dns,0,IPV4_STR_LEN);			

		if(netlink == 1)
		{
			cJSON_AddStringToObject(network_obj,"ip_addr",ip_address);
			cJSON_AddStringToObject(network_obj,"subnet_mask",netmask);
			cJSON_AddStringToObject(network_obj,"gateway",gateway);

			while(huayi_get_nth_value(k, temp_dns, WEB_SPACE_DELIMIT, dns, sizeof(dns)))			
			{	
				if (k == 0)
				{
					cJSON_AddStringToObject(network_obj,"dns_pre",dns);
				}
				else if (k == 1)
				{
					cJSON_AddStringToObject(network_obj,"dns_sec",dns);
				}
				k++;
			}
			if (k == 1)
			{
				cJSON_AddStringToObject(network_obj,"dns_sec",dns);
			}
		}
		else
		{
			cJSON_AddStringToObject(network_obj,"ip_addr","0.0.0.0");
			cJSON_AddStringToObject(network_obj,"subnet_mask","0.0.0.0");
			cJSON_AddStringToObject(network_obj,"gateway","0.0.0.0");
			cJSON_AddStringToObject(network_obj,"dns_pre","0.0.0.0");
			cJSON_AddStringToObject(network_obj,"dns_sec","0.0.0.0");
		}
		
		huayi_uci_get_int(PKG_STATUS_NETWORK,section,"tx_bps",&tx_Bps);
		tx_bps = ((unsigned long long)tx_Bps)*8;
		cJSON_AddNumberToObject(network_obj,"tx_bps",tx_bps);
		
		huayi_uci_get_int(PKG_STATUS_NETWORK,section,"rx_bps",&rx_Bps);
		rx_bps = ((unsigned long long)rx_Bps)*8;
		cJSON_AddNumberToObject(network_obj,"rx_bps",rx_bps);
		
		huayi_uci_get_long_long(PKG_STATUS_NETWORK,section,"tx_bytes",&txbytes);
		cJSON_AddNumberToObject(network_obj,"tx_bytes",txbytes);
		
		huayi_uci_get_long_long(PKG_STATUS_NETWORK,section,"rx_bytes",&rxbytes);
		cJSON_AddNumberToObject(network_obj,"rx_bytes",rxbytes);
		
		cJSON_AddItemToArray(network_array,network_obj);
	}

	cJSON_AddItemToObject(result_root,"network_arr",network_array);

	//wifi	
	wireless_array = cJSON_CreateArray();

	//2.4g wifi
	wireless_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(wireless_obj,"section","0");

	get_cmd_result("cat /tmp/ifconfig_info | grep ra0 | wc -l", run_cmd, sizeof(run_cmd));
	wifi_on = atoi(run_cmd);
	
	cJSON_AddNumberToObject(wireless_obj,"status",wifi_on);

	if (wifi_on == 1)
	{	  
		huayi_uci_get_int(PKG_WIRELESS,W24G_DEV,"wifimode",&wifimode);
		cJSON_AddNumberToObject(wireless_obj,"band",wifimode);

		huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"encryption",encryptype);
		if (strcmp(encryptype,"none") == 0)
		{
			cJSON_AddStringToObject(wireless_obj,"auth_mode","NONE");
		}
		else
		{
			cJSON_AddStringToObject(wireless_obj,"auth_mode","WPAWPA2PSK");
		}
		
		huayi_uci_get_int(PKG_WIRELESS,W24G_DEV,"channel",&channel_auto);
		if(channel_auto == 0)
		{
			cJSON_AddNumberToObject(wireless_obj,"auto_channel",1);
		}
		else
		{
			cJSON_AddNumberToObject(wireless_obj,"auto_channel",0);
		}
		
		get_cmd_result("cat /tmp/iwconfig_ra0 | grep ESSID", run_cmd, sizeof(run_cmd));
		if ((ptr = strstr(run_cmd,"\"")) != NULL)
		{
			ptr = ptr + strlen("\"");
			if (strlen(run_cmd) > 3)
				run_cmd[strlen(run_cmd)-3] = '\0';
		}

		if(NULL != ptr)
		{
			cJSON_AddStringToObject(wireless_obj,"ssid",ptr);
		}
		else
		{
			cJSON_AddStringToObject(wireless_obj,"ssid","");
		}	

		get_cmd_result("cat /tmp/iwconfig_ra0  | grep Channel | awk '{print $2}'", run_cmd, sizeof(run_cmd));
		channel = atoi(run_cmd+strlen("Channel="));
		cJSON_AddNumberToObject(wireless_obj,"channel",channel);

		get_cmd_result("cat /tmp/iwconfig_ra0 | grep 'Access Point' | awk '{print $5}'", run_cmd, sizeof(run_cmd));
	    cJSON_AddStringToObject(wireless_obj,"bssid",run_cmd);

		//get_wifi_stamacnum("ra0",0,&wifi_24g_num);
		cJSON_AddNumberToObject(wireless_obj,"client_sum",wifi_24g_num);
	}

	cJSON_AddItemToArray(wireless_array,wireless_obj);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	//5.8g wifi
	wireless_obj = cJSON_CreateObject();
	cJSON_AddStringToObject(wireless_obj,"section","1");

	get_cmd_result("cat /tmp/ifconfig_info | grep rai0 | wc -l", run_cmd, sizeof(run_cmd));
	wifi_on = atoi(run_cmd);
	
	cJSON_AddNumberToObject(wireless_obj,"status",wifi_on);

	if (wifi_on == 1)
	{	  
		huayi_uci_get_int(PKG_WIRELESS,W58G_DEV,"wifimode",&wifimode);
		cJSON_AddNumberToObject(wireless_obj,"band",wifimode);

		huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"encryption",encryptype);
		if (strcmp(encryptype,"none") == 0)
		{
			cJSON_AddStringToObject(wireless_obj,"auth_mode","NONE");
		}
		else
		{
			cJSON_AddStringToObject(wireless_obj,"auth_mode","WPAWPA2PSK");
		}
		
		huayi_uci_get_int(PKG_WIRELESS,W58G_DEV,"channel",&channel_auto);
		if(channel_auto == 0)
		{
			cJSON_AddNumberToObject(wireless_obj,"auto_channel",1);
		}
		else
		{
			cJSON_AddNumberToObject(wireless_obj,"auto_channel",0);
		}
		
		get_cmd_result("cat /tmp/iwconfig_rai0 | grep ESSID", run_cmd, sizeof(run_cmd));
		if ((ptr = strstr(run_cmd,"\"")) != NULL)
		{
			ptr = ptr + strlen("\"");
			if (strlen(run_cmd) > 3)
				run_cmd[strlen(run_cmd)-3] = '\0';
		}

		if(NULL != ptr)
		{
			cJSON_AddStringToObject(wireless_obj,"ssid",ptr);
		}
		else
		{
			cJSON_AddStringToObject(wireless_obj,"ssid","");
		}
		
		get_cmd_result("cat /tmp/iwconfig_rai0  | grep Channel | awk '{print $2}'", run_cmd, sizeof(run_cmd));
		channel = atoi(run_cmd+strlen("Channel="));
		cJSON_AddNumberToObject(wireless_obj,"channel",channel);

		get_cmd_result("cat /tmp/iwconfig_rai0 | grep 'Access Point' | awk '{print $5}'", run_cmd, sizeof(run_cmd));
	    cJSON_AddStringToObject(wireless_obj,"bssid",run_cmd);

		//get_wifi_stamacnum("rai0",1,&wifi_58g_num);
		cJSON_AddNumberToObject(wireless_obj,"client_sum",wifi_58g_num);
	}
	
	cJSON_AddItemToArray(wireless_array,wireless_obj);
#endif
	
	cJSON_AddItemToObject(result_root,"wireless_arr",wireless_array);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

#if defined(CONFIG_PACKAGE_HUAYI_CAPWAP_CLIENT)
int get_thinap_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int apmode = 0, capclistatus = 0;
	char wtp_name[OPTION_STR_LEN]= {0}, ac_ip[SHORT_STR_LEN] = {0};
	int assoctime = 0, uptime = 0, runtime = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}
	
	result_root = cJSON_CreateObject();

	if(!huayi_uci_get_int(PKG_THINAP,"global","enable",&apmode))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_THINAP,"global","wtp_name",wtp_name))
	{
		strcpy(wtp_name, "Unknow");
		//return CGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_STATUS_SYSTEM,"sys","ac_ipaddr",ac_ip))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","capwap_client_status",&capclistatus))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","ac_assoc_time",&assoctime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(assoctime > 0 && uptime > assoctime)
		runtime = uptime - assoctime;
	
	cJSON_AddNumberToObject(result_root,"apmode",apmode);//1:FIT;0:FAT

	cJSON_AddStringToObject(result_root,"wtpname",wtp_name);

	cJSON_AddStringToObject(result_root,"acipaddr",ac_ip);

	cJSON_AddNumberToObject(result_root,"capclistatus",capclistatus);

	cJSON_AddNumberToObject(result_root,"runtime",runtime);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int set_thinap_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int apmode = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"apmode",&apmode))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}
	
	if(!huayi_uci_set_int(PKG_THINAP,"global","enable",apmode))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_uci_commit(PKG_THINAP);

	*lock_time = HUAYI_SYSTEM_REBOOT_TIME;
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");

	system("huayi-reset reboot");
	
	return FCGI_RESP_CODE_SUCCESS;
}
#endif

int get_internet_status(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON 	*dns_array = NULL;
	int 	i = 0;
	char mac_addr[MAC_STR_LEN] = {0};
	char ip_addr[IPV4_STR_LEN] = {0},netmask[IPV4_STR_LEN] = {0},dns[OPTION_STR_LEN] = {0};
	int wan_up = 0,wan_link = 0;
	int mtu = 0;
	char temp_dns[OPTION_STR_LEN] = {0};
	unsigned long long rx_bps = 0, tx_bps = 0;
	char ip6Addr[OPTION_STR_LEN]={0}, ifName[16]={0};
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}
	
	result_root = cJSON_CreateObject();

	if(!huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "mac_address", mac_addr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", ip_addr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK, "wan", "link", &wan_link))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK, "wan", "wan_up", &wan_up))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_int(PKG_STATUS_NETWORK, "wan", "if_mtu", &mtu))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	if(!huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "netmask", netmask))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK, "wan", "rx_bps", &rx_bps))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_long_long(PKG_STATUS_NETWORK, "wan", "tx_bps", &tx_bps))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","dns_server",temp_dns))
	{
		
	}
	dns_array = cJSON_CreateArray();
	//cJSON_AddItemToObject(result_root,"dns",dns_array);

	if(wan_link == 0)
	{
		wan_up = 0;
	}

	cJSON_AddStringToObject(result_root, "mac_addr", mac_addr);
	cJSON_AddNumberToObject(result_root, "wan_up", wan_up);
	if(wan_up == 1)
	{
		cJSON_AddStringToObject(result_root, "ip_addr", ip_addr);
		cJSON_AddStringToObject(result_root, "netmask", netmask);
		cJSON_AddNumberToObject(result_root, "mtu", mtu);
		
		while(huayi_get_nth_value(i++, temp_dns, WEB_SPACE_DELIMIT, dns, sizeof(dns)))
		{
			cJSON_AddItemToArray(dns_array,cJSON_CreateString(dns));
		}
		cJSON_AddItemToObject(result_root,"dns",dns_array);
		cJSON_AddNumberToObject(result_root, "rx_bps", rx_bps);
		cJSON_AddNumberToObject(result_root, "tx_bps", tx_bps);
	}else
	{
		cJSON_AddStringToObject(result_root, "ip_addr", "0.0.0.0");
		cJSON_AddStringToObject(result_root, "netmask", "0.0.0.0");
		cJSON_AddNumberToObject(result_root, "mtu", 0);
		cJSON_AddItemToArray(dns_array,cJSON_CreateString("0.0.0.0"));
		cJSON_AddItemToObject(result_root,"dns",dns_array);
		cJSON_AddNumberToObject(result_root, "rx_bps", 0);
		cJSON_AddNumberToObject(result_root, "tx_bps", 0);
	}
	
#if defined(CONFIG_PACKAGE_HUAYI_IPV6)

	if(!huayi_uci_get_str(PKG_NETWORK,"wan6","ifname",ifName))
		strcpy(ifName,"eth1");
	
	if(!get_ipv6addr(ip6Addr,ifName,IPV6_ADDR_GLOBAL))
		get_ipv6addr(ip6Addr,ifName,IPV6_ADDR_LINKLOCAL);
	
	ipv6_reduce(ip6Addr);
	cJSON_AddStringToObject(result_root, "ip6addr", ip6Addr);
#endif	
	cJSON_AddItemToObject(response_root, WEB_RETURN_DATA_NAME, result_root);
	 
	return FCGI_RESP_CODE_SUCCESS;
}

int get_device_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *router_obj = NULL;
	cJSON *port_array = NULL;
	cJSON *port_obj = NULL;
	cJSON *wireless_2g_obj = NULL;
	cJSON *wireless_5g_obj = NULL;
	cJSON *guest_2g_obj = NULL;
	cJSON *guest_5g_obj = NULL;
	cJSON *performance_obj = NULL;

	char product_model[OPTION_STR_LEN] = {0};
	char mac_addr[MAC_STR_LEN] = {0};
	char IPV4_addr[IPV4_STR_LEN] = {0};
	char IPV6_addr[OPTION_STR_LEN] = {0};

	int i = 0;
	int link = 0;
	int speed = 0;
	char portname[SMALL_STR_LEN] = {0};
	struct eth_speed_info eth;

	int wifi_on = 0;
	char run_cmd[CMD_STR_LEN]= {0};
	char *ptr = NULL;
	int channel = -1;
	char key[TEMP_STR_LEN] = {0};

	int cpu_rt_usage = 0;
	int mem_rt_usage = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	result_root = cJSON_CreateObject();
	router_obj = cJSON_CreateObject();
	port_array = cJSON_CreateArray();
	performance_obj = cJSON_CreateObject();

	if(!huayi_uci_get_str(PKG_ABOUT, "about_system", "product_name", product_model))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_STATUS_NETWORK, "lan", "mac_address", mac_addr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayi_uci_get_str(PKG_STATUS_NETWORK, "lan", "ip_address", IPV4_addr))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
	if(!get_ipv6addr(IPV6_addr,"br-lan",IPV6_ADDR_GLOBAL))
		get_ipv6addr(IPV6_addr,"br-lan",IPV6_ADDR_LINKLOCAL);
	ipv6_reduce(IPV6_addr);
#endif
	cJSON_AddStringToObject(router_obj, "product_model", product_model);
	cJSON_AddStringToObject(router_obj, "mac_addr", mac_addr);
	cJSON_AddStringToObject(router_obj, "IPV4_addr", IPV4_addr);
	cJSON_AddStringToObject(router_obj, "IPV6_addr", IPV6_addr);
	cJSON_AddItemToObject(result_root,"router_info",router_obj);
	{
		
		port_obj = cJSON_CreateObject();
		memset(&eth, 0, sizeof(ETH_SPEED_ADDR));
		if (CONFIG_HUAYI_WAN_PORT_NUM == 4)
		{
			get_eth_info(&eth, ETH_SPEED_ADDR, 4*ETH_SPEED_OFFSET);
		}
		else
		{
			get_eth_info(&eth, ETH_SPEED_ADDR, 0);
		}
		link = eth.link;
		speed = eth.speed;

		cJSON_AddStringToObject(port_obj, "port_name", "WAN");

		cJSON_AddNumberToObject(port_obj, "link", link);
		cJSON_AddNumberToObject(port_obj, "speed", speed);

		cJSON_AddItemToArray(port_array, port_obj);
	}

	for(i = 0;i <= LAN_MAX_NUM;i++)
	{
		port_obj = cJSON_CreateObject();
		memset(&eth, 0, sizeof(ETH_SPEED_ADDR));
		if (CONFIG_HUAYI_WAN_PORT_NUM == 4)
		{
			get_eth_info(&eth, ETH_SPEED_ADDR, i*ETH_SPEED_OFFSET);
		}
		else
		{
			get_eth_info(&eth, ETH_SPEED_ADDR, (i + 1)* ETH_SPEED_OFFSET);
		}
		link = eth.link;
		speed = eth.speed;

		snprintf(portname, SMALL_STR_LEN, "LAN%d",i);
		cJSON_AddStringToObject(port_obj, "port_name", portname);

		cJSON_AddNumberToObject(port_obj, "link", link);		
		cJSON_AddNumberToObject(port_obj, "speed", speed);

		cJSON_AddItemToArray(port_array, port_obj);
	}
	cJSON_AddItemToObject(result_root,"port_info",port_array);
		
	wireless_2g_obj = cJSON_CreateObject();

	get_cmd_result("ifconfig | grep ra0 | wc -l", run_cmd, sizeof(run_cmd));
	wifi_on = atoi(run_cmd);
	
	//cJSON_AddNumberToObject(wireless_2g_obj, "enable", wifi_on);
	
	if (wifi_on == 1)
	{	 
		cJSON_AddTrueToObject(wireless_2g_obj, "enable");
		memset(run_cmd, 0, sizeof(run_cmd));
		get_cmd_result("iwconfig ra0 | grep ESSID", run_cmd, sizeof(run_cmd));
		if ((ptr = strstr(run_cmd, "\"")) != NULL)
		{
			ptr = ptr + strlen("\"");
			if (strlen(run_cmd) > 3)
				run_cmd[strlen(run_cmd)-3] = '\0';
		}
		
		if(NULL != ptr)
		{
			cJSON_AddStringToObject(wireless_2g_obj, "ssid", ptr);
		}
		else
		{
			cJSON_AddStringToObject(wireless_2g_obj, "ssid", "");
		}

		if(!huayi_uci_get_str(PKG_WIRELESS, "radio0_0", "key", key))
		{
			;
		}

		cJSON_AddStringToObject(wireless_2g_obj, "key", key);

		memset(run_cmd, 0, sizeof(run_cmd));
		get_cmd_result("iwconfig ra0  | grep Channel | awk '{print $2}'", run_cmd, sizeof(run_cmd));
		channel = atoi(run_cmd+strlen("Channel="));
		cJSON_AddNumberToObject(wireless_2g_obj, "channel", channel);
	}
	else
	{
		cJSON_AddFalseToObject(wireless_2g_obj, "enable");
	}
	cJSON_AddItemToObject(result_root,"wireless_2g",wireless_2g_obj);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	
	wireless_5g_obj = cJSON_CreateObject();
	memset(run_cmd, 0, sizeof(run_cmd));
	get_cmd_result("ifconfig | grep rai0 | wc -l", run_cmd, sizeof(run_cmd));
	wifi_on = atoi(run_cmd);
	//cJSON_AddNumberToObject(wireless_5g_obj, "enable", wifi_on);
	
	if (wifi_on == 1)
	{	
		cJSON_AddTrueToObject(wireless_5g_obj, "enable");
		memset(run_cmd, 0, sizeof(run_cmd));
		get_cmd_result("iwconfig rai0 | grep ESSID", run_cmd, sizeof(run_cmd));
		if ((ptr = strstr(run_cmd, "\"")) != NULL)
		{
			ptr = ptr + strlen("\"");
			if (strlen(run_cmd) > 3)
				run_cmd[strlen(run_cmd)-3] = '\0';
		}
		if(NULL != ptr)
		{
			cJSON_AddStringToObject(wireless_5g_obj, "ssid", ptr);
		}
		else
		{
			cJSON_AddStringToObject(wireless_5g_obj, "ssid", "");
		}
		
		memset(key,0,TEMP_STR_LEN);
		if(!huayi_uci_get_str(PKG_WIRELESS, "radio1_0", "key", key))
		{
			;
		}

		cJSON_AddStringToObject(wireless_5g_obj, "key", key);
		memset(run_cmd, 0, sizeof(run_cmd));
		get_cmd_result("iwconfig rai0  | grep Channel | awk '{print $2}'", run_cmd, sizeof(run_cmd));
		channel = atoi(run_cmd+strlen("Channel="));
		cJSON_AddNumberToObject(wireless_5g_obj, "channel", channel);
	}
	else
	{
		cJSON_AddFalseToObject(wireless_5g_obj, "enable");
	}
	cJSON_AddItemToObject(result_root,"wireless_5g",wireless_5g_obj);
#endif

#if defined(CONFIG_HUAYI_WIRELESS_GUEST_SUPPORT_2G)
	guest_2g_obj = cJSON_CreateObject();
	memset(run_cmd, 0, sizeof(run_cmd));
	get_cmd_result("ifconfig | grep ra1 | wc -l", run_cmd, sizeof(run_cmd));
	wifi_on = atoi(run_cmd);
	//cJSON_AddNumberToObject(guest_2g_obj, "enable", wifi_on);
	
	if (wifi_on == 1)
	{	 
		cJSON_AddTrueToObject(guest_2g_obj, "enable");
		memset(run_cmd, 0, sizeof(run_cmd));
		get_cmd_result("iwconfig ra1 | grep ESSID", run_cmd, sizeof(run_cmd));
		if ((ptr = strstr(run_cmd, "\"")) != NULL)
		{
			ptr = ptr + strlen("\"");
			if (strlen(run_cmd) > 3)
				run_cmd[strlen(run_cmd)-3] = '\0';
		}
		if(NULL != ptr)
		{
			cJSON_AddStringToObject(guest_2g_obj, "ssid", ptr);
		}
		else
		{
			cJSON_AddStringToObject(guest_2g_obj, "ssid", "");
		}

		memset(key,0,TEMP_STR_LEN);
		if(!huayi_uci_get_str(PKG_WIRELESS, "radio0_1", "key", key))
		{
			;
		}
		cJSON_AddStringToObject(guest_2g_obj, "key", key);
		
	}
	else
	{
		cJSON_AddFalseToObject(guest_2g_obj, "enable");
	}
	cJSON_AddItemToObject(result_root,"guest_2g",guest_2g_obj);
#endif
#if defined(CONFIG_HUAYI_WIRELESS_GUEST_SUPPORT_5G)
	guest_5g_obj = cJSON_CreateObject();
	memset(run_cmd, 0, sizeof(run_cmd));
	get_cmd_result("ifconfig | grep rai1 | wc -l", run_cmd, sizeof(run_cmd));
	wifi_on = atoi(run_cmd);
	
	if (wifi_on == 1)
	{	  
		cJSON_AddTrueToObject(guest_5g_obj, "enable");
		memset(run_cmd, 0, sizeof(run_cmd));
		get_cmd_result("iwconfig rai1 | grep ESSID", run_cmd, sizeof(run_cmd));
		if ((ptr = strstr(run_cmd, "\"")) != NULL)
		{
			ptr = ptr + strlen("\"");
			if (strlen(run_cmd) > 3)
				run_cmd[strlen(run_cmd)-3] = '\0';
		}
		if(NULL != ptr)
		{
			cJSON_AddStringToObject(guest_5g_obj, "ssid", ptr);
		}
		else
		{
			cJSON_AddStringToObject(guest_5g_obj, "ssid", "");
		}

		
		memset(key,0,TEMP_STR_LEN);
		if(!huayi_uci_get_str(PKG_WIRELESS, "radio1_1", "key", key))
		{
			;
		}
		cJSON_AddStringToObject(guest_5g_obj, "key", key);
	}
	else
	{
		cJSON_AddFalseToObject(guest_5g_obj, "enable");
	}
	cJSON_AddItemToObject(result_root,"guest_5g",guest_5g_obj);
#endif
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","cpu_rt_usage",&cpu_rt_usage);
		
	cJSON_AddNumberToObject(performance_obj,"cpu_rt_usage",cpu_rt_usage);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","mem_rt_usage",&mem_rt_usage);
		
	cJSON_AddNumberToObject(performance_obj,"mem_rt_usage",mem_rt_usage);

	cJSON_AddItemToObject(result_root,"performance",performance_obj);
	cJSON_AddItemToObject(response_root, WEB_RETURN_DATA_NAME, result_root);
	return FCGI_RESP_CODE_SUCCESS;
}

int set_optimize(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	
	system("echo 3 > /proc/sys/vm/drop_caches");
	return FCGI_RESP_CODE_SUCCESS;
}

#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
int mysql_add_ailas(char *macaddr,char *host_name)
{
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	int sid = 0;
	char *sql_hex = NULL;

	if(NULL == macaddr || NULL == host_name)
	{
		return FCGI_FALSE;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_FALSE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlCheckTableExist(my_con,ALIAS_HUAYI_TABLE))
	{
		snprintf(query,sizeof(query),"create table if not exists `%s` ("
			"%s int unsigned not null primary key auto_increment,"
			"%s varchar(17),"
			"%s varchar(256),"
			"%s int unsigned not null default 0,"
			"UNIQUE (%s)"
			");",ALIAS_HUAYI_TABLE,MYSQL_ALIASID_KEY_NAME,MYSQL_ALIAS_MAC_KEY_NAME,MYSQL_ALIAS_NAME_KEY_NAME,
			MYSQL_ALIAS_CHANGE_KEY_NAME,MYSQL_ALIASID_KEY_NAME);

			huayiMySqlExec(my_con,query);	
	}

	sql_hex = huayiMysqlHexString(host_name,strlen(host_name));
	if(NULL == sql_hex)
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}
	snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",MYSQL_ALIASID_KEY_NAME,ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,macaddr);
	huayiMySqlGetInt(my_con,query,&sid);
	if(sid == 0)
	{
		huayiMySqlExec(my_con,"insert into `%s` (%s,%s,%s) values('%s',0x%s,0);",ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,MYSQL_ALIAS_NAME_KEY_NAME,
																				MYSQL_ALIAS_CHANGE_KEY_NAME,macaddr,sql_hex);
	}
	else
	{
		huayiMySqlExec(my_con,"update `%s` set %s=0x%s where %s=%d;",ALIAS_HUAYI_TABLE,MYSQL_ALIAS_NAME_KEY_NAME,sql_hex,
																				MYSQL_ALIASID_KEY_NAME,sid);

	}

	free(sql_hex);
	huayiMySqlClose(my_con);
	system("huayi-reset save_database");
	return FCGI_TRUE;
}

int mysql_update_alias(char *macaddr,char *host_name)
{
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	char * sql_hex = NULL;
	int sid = 0;

	if(NULL == macaddr || NULL == host_name)
	{
		return FCGI_FALSE;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_FALSE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlCheckTableExist(my_con,ALIAS_HUAYI_TABLE))
	{
		huayiMySqlClose(my_con);
		return FCGI_TRUE;
	}

	sql_hex = huayiMysqlHexString(host_name,strlen(host_name));
	if(NULL == sql_hex)
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}

	snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",MYSQL_ALIASID_KEY_NAME,ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,macaddr);
	huayiMySqlGetInt(my_con,query,&sid);
	if(sid == 0)
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}

	huayiMySqlExec(my_con,"update `%s` set %s=0x%s %s=1 where %s=%d;",ALIAS_HUAYI_TABLE,MYSQL_ALIAS_NAME_KEY_NAME,sql_hex,MYSQL_ALIAS_CHANGE_KEY_NAME,
																				MYSQL_ALIASID_KEY_NAME,sid);
	free(sql_hex);
	huayiMySqlClose(my_con);
	system("huayi-reset save_database");
	return FCGI_TRUE;	
}

int mysql_del_alias(char *macaddr)
{
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};

	if(NULL == macaddr)
	{
		return FCGI_FALSE;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_FALSE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlCheckTableExist(my_con,ALIAS_HUAYI_TABLE))
	{
		huayiMySqlClose(my_con);
		return FCGI_TRUE;
	}

	snprintf(query,sizeof(query),"delete from `%s` where %s='%s';",ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,macaddr);
	if(!huayiMySqlExec(my_con,query))
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}
	return FCGI_TRUE;
}

int mysql_get_host_name(char *macaddr, char *host_name, int len)
{
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	int sid = 0;
	char name[OPTION_STR_LEN] = {0};
	
	if(NULL == macaddr || NULL == host_name)
	{
		return FCGI_FALSE;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_FALSE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlCheckTableExist(my_con,ALIAS_HUAYI_TABLE))
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}

	snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",MYSQL_ALIASID_KEY_NAME,ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,macaddr);
	huayiMySqlGetInt(my_con,query,&sid);
	if(sid != 0)
	{
		snprintf(query,sizeof(query),"select %s from `%s` where %s=%d;",MYSQL_ALIAS_NAME_KEY_NAME,ALIAS_HUAYI_TABLE,MYSQL_ALIASID_KEY_NAME,sid);
		//huayiMySqlGetInt(my_con,query,&sid);
		huayiMySqlGetStr( my_con,query,name,sizeof(name));
		memcpy(host_name,name,len);
	}
	else
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}

	huayiMySqlClose(my_con);

	return FCGI_TRUE;
	
}

/*int clean_mysql_alias(char *mac_buf)
{
	
}*/
#endif

#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
int mysql_add_ailas(char *macaddr,char *host_name)
{
#if 0
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	int sid = 0;
	char *sql_hex = NULL;

	if(NULL == macaddr || NULL == host_name)
	{
		return FCGI_FALSE;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_FALSE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlCheckTableExist(my_con,ALIAS_HUAYI_TABLE))
	{
		snprintf(query,sizeof(query),"create table if not exists `%s` ("
			"%s int unsigned not null primary key auto_increment,"
			"%s varchar(17),"
			"%s varchar(256),"
			"%s int unsigned not null default 0,"
			"UNIQUE (%s)"
			");",ALIAS_HUAYI_TABLE,MYSQL_ALIASID_KEY_NAME,MYSQL_ALIAS_MAC_KEY_NAME,MYSQL_ALIAS_NAME_KEY_NAME,
			MYSQL_ALIAS_CHANGE_KEY_NAME,MYSQL_ALIASID_KEY_NAME);

			huayiMySqlExec(my_con,query);	
	}

	sql_hex = huayiMysqlHexString(host_name,strlen(host_name));
	if(NULL == sql_hex)
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}
	snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",MYSQL_ALIASID_KEY_NAME,ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,macaddr);
	huayiMySqlGetInt(my_con,query,&sid);
	if(sid == 0)
	{
		huayiMySqlExec(my_con,"insert into `%s` (%s,%s,%s) values('%s',0x%s,0);",ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,MYSQL_ALIAS_NAME_KEY_NAME,
																				MYSQL_ALIAS_CHANGE_KEY_NAME,macaddr,sql_hex);
	}
	else
	{
		huayiMySqlExec(my_con,"update `%s` set %s=0x%s where %s=%d;",ALIAS_HUAYI_TABLE,MYSQL_ALIAS_NAME_KEY_NAME,sql_hex,
																				MYSQL_ALIASID_KEY_NAME,sid);

	}

	free(sql_hex);
	huayiMySqlClose(my_con);
	system("huayi-reset save_database");
#endif
	return FCGI_TRUE;
}

int mysql_update_alias(char *macaddr,char *host_name)
{
#if 0
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	char * sql_hex = NULL;
	int sid = 0;

	if(NULL == macaddr || NULL == host_name)
	{
		return FCGI_FALSE;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_FALSE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlCheckTableExist(my_con,ALIAS_HUAYI_TABLE))
	{
		huayiMySqlClose(my_con);
		return FCGI_TRUE;
	}

	sql_hex = huayiMysqlHexString(host_name,strlen(host_name));
	if(NULL == sql_hex)
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}

	snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",MYSQL_ALIASID_KEY_NAME,ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,macaddr);
	huayiMySqlGetInt(my_con,query,&sid);
	if(sid == 0)
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}

	huayiMySqlExec(my_con,"update `%s` set %s=0x%s %s=1 where %s=%d;",ALIAS_HUAYI_TABLE,MYSQL_ALIAS_NAME_KEY_NAME,sql_hex,MYSQL_ALIAS_CHANGE_KEY_NAME,
																				MYSQL_ALIASID_KEY_NAME,sid);
	free(sql_hex);
	huayiMySqlClose(my_con);
	system("huayi-reset save_database");
#endif
	return FCGI_TRUE;	
}

int mysql_del_alias(char *macaddr)
{
#if 0
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};

	if(NULL == macaddr)
	{
		return FCGI_FALSE;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_FALSE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlCheckTableExist(my_con,ALIAS_HUAYI_TABLE))
	{
		huayiMySqlClose(my_con);
		return FCGI_TRUE;
	}

	snprintf(query,sizeof(query),"delete from `%s` where %s='%s';",ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,macaddr);
	if(!huayiMySqlExec(my_con,query))
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}
#endif
	return FCGI_TRUE;
}

int mysql_get_host_name(char *macaddr, char *host_name, int len)
{
#if 0
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	int sid = 0;
	char name[OPTION_STR_LEN] = {0};
	
	if(NULL == macaddr || NULL == host_name)
	{
		return FCGI_FALSE;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_FALSE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlCheckTableExist(my_con,ALIAS_HUAYI_TABLE))
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}

	snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",MYSQL_ALIASID_KEY_NAME,ALIAS_HUAYI_TABLE,MYSQL_ALIAS_MAC_KEY_NAME,macaddr);
	huayiMySqlGetInt(my_con,query,&sid);
	if(sid != 0)
	{
		snprintf(query,sizeof(query),"select %s from `%s` where %s=%d;",MYSQL_ALIAS_NAME_KEY_NAME,ALIAS_HUAYI_TABLE,MYSQL_ALIASID_KEY_NAME,sid);
		//huayiMySqlGetInt(my_con,query,&sid);
		huayiMySqlGetStr( my_con,query,name,sizeof(name));
		memcpy(host_name,name,len);
	}
	else
	{
		huayiMySqlClose(my_con);
		return FCGI_FALSE;
	}

	huayiMySqlClose(my_con);
#endif
	return FCGI_TRUE;
	
}

/*int clean_mysql_alias(char *mac_buf)
{
	
}*/
#endif

int get_sta_speed(cJSON *stalist_obj, char *macaddr,char *host_name ,int network_type)
{
	FILE *fp = NULL;
	char buff[CMD_STR_LEN] = { 0 };
	char  ip[RESULT_STR_LEN] = {0}, hw_address[RESULT_STR_LEN] = {0},name[OPTION_STR_LEN] = {0};
	char upBytes[RESULT_STR_LEN] = {0}, downBytes[RESULT_STR_LEN] = {0}, uprate[RESULT_STR_LEN] = {0}, downrate[RESULT_STR_LEN] = {0};
	unsigned int timeout = 0, connect = 0, active = 0,livetime = 0 ;
	int flag = 0;
	int ret = 0;

	if(NULL == stalist_obj || NULL == macaddr || NULL == host_name)
	{
		return FCGI_FALSE;
	}

	fp = popen("cat /proc/net/client_list", "r");

	if(!fp) 
	{
		return FCGI_FALSE;
	}
	
	while(fgets(buff, CMD_STR_LEN, fp)) 
	{
        sscanf(buff, "%s %s %u %u %u %u %s %s %s %s", ip, hw_address, &timeout, &livetime, &connect, &active, upBytes, downBytes, uprate, downrate);
		
        if(!strncmp(hw_address, macaddr,17)) 
		{
            flag = 1;
            break;
        }
    }

#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
	ret = mysql_get_host_name(macaddr,name,sizeof(name));
#endif
#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
	/*todo sqlite3 rex, 2021-12-28*/
		ret = mysql_get_host_name(macaddr,name,sizeof(name));
#endif

	if(ret != 0)
	{
		cJSON_AddStringToObject(stalist_obj,"name",name);
	}
	else
	{
		cJSON_AddStringToObject(stalist_obj,"name",host_name);
	}
	
	
	if(flag == 1)
	{
		cJSON_AddStringToObject(stalist_obj,"realtime_rate_up", uprate);
		cJSON_AddStringToObject(stalist_obj,"realtime_rate_down", downrate);
		cJSON_AddStringToObject(stalist_obj,"TX_Rate", upBytes);
		cJSON_AddStringToObject(stalist_obj,"RX_Rate", downBytes);
		if(network_type == 1)
		{
			cJSON_AddNumberToObject(stalist_obj,"uptime", livetime);
		}
		
	}
	else
	{
		cJSON_AddStringToObject(stalist_obj,"realtime_rate_up", "0");
		cJSON_AddStringToObject(stalist_obj,"realtime_rate_down", "0");
		cJSON_AddStringToObject(stalist_obj,"TX_Rate", "0");
		cJSON_AddStringToObject(stalist_obj,"RX_Rate", "0");
		if(network_type == 1)
		{
			cJSON_AddNumberToObject(stalist_obj,"uptime", 0);
		}
	}

	pclose(fp);

	return FCGI_TRUE;

}

int get_wifi_sta(cJSON *stalist_array,char * mac_buf,char * vap_section)
{
	cJSON  *wifi_sta_obj = NULL;
	char vap_ifname[OPTION_STR_LEN] = {0};
	int i = 0, socket_id = 0;
	unsigned long *num = NULL;
    struct iwreq iwr;
	RT_802_11_7603_MAC_TABLE table_24G = {0};
	RT_802_11_7663_MAC_TABLE table_58G = {0};
	char macbuff[MAC_STR_LEN]={0};
	unsigned int connectedtime = 0;
	int type = -1;
	char host_name[OPTION_STR_LEN] = { 0 };
	char ip_entry[RESULT_STR_LEN] = { 0 }, hw_type[SMALL_STR_LEN] = { 0 },flags[SMALL_STR_LEN] = { 0 };
	char hw_address[RESULT_STR_LEN] = { 0 };
	char mask[SMALL_STR_LEN] = {0},device[RESULT_STR_LEN] = {0};
	char buf[CMD_STR_LEN] = {0};
	bool block = false;
	FILE *fp = NULL;
	int flag = 0;
	int disabled = 0;

	if(NULL == stalist_array || NULL == vap_section || NULL == mac_buf)
	{
		return FCGI_FALSE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"ifname",vap_ifname))
	{
		return FCGI_FALSE;
	}

	strncpy(iwr.ifr_name, vap_ifname, IFNAMSIZ);

	if (!strncmp(vap_section,"radio0_",strlen("radio0_")))
	{
		iwr.u.data.pointer = (caddr_t) &table_24G;
		num = &table_24G.Num;
		type = 1;
		if(!huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&disabled))
		{
			return FCGI_FALSE;
		}
	}
	else
	{
		iwr.u.data.pointer = (caddr_t) &table_58G;
		num = &table_58G.Num;
		type = 2;
		if(!huayi_uci_get_int(PKG_WIRELESS,"radio1","disabled",&disabled))
		{
			return FCGI_FALSE;
		}
	}

	if(disabled == 1)
	{
		return FCGI_FALSE;
	}

	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_id < 0) 
	{
		return FCGI_FALSE;
	}
	
	if (ioctl(socket_id, 0x8bef, &iwr) < 0) 
	{
		close(socket_id);
		return FCGI_FALSE;
	}
	
	close(socket_id);

	fp = fopen("/proc/net/arp", "r");
	
	if(!fp)
	{
		return FCGI_FALSE;
	}

	for (i = 0;( i < (*num)) && (i < 32); i++) 
	{
		memset(macbuff, 0, sizeof(macbuff));
		flag = 0;

		if (!strncmp(vap_section,"radio0_",strlen("radio0_")))
		{
			sprintf(macbuff,"%02X:%02X:%02X:%02X:%02X:%02X", 
					table_24G.Entry[i].Addr[0], table_24G.Entry[i].Addr[1],
					table_24G.Entry[i].Addr[2], table_24G.Entry[i].Addr[3],
					table_24G.Entry[i].Addr[4], table_24G.Entry[i].Addr[5]);
			connectedtime = table_24G.Entry[i].ConnectedTime;
		}
		else
		{
			sprintf(macbuff,"%02X:%02X:%02X:%02X:%02X:%02X", 
					table_58G.Entry[i].Addr[0], table_58G.Entry[i].Addr[1],
					table_58G.Entry[i].Addr[2], table_58G.Entry[i].Addr[3],
					table_58G.Entry[i].Addr[4], table_58G.Entry[i].Addr[5]);
			connectedtime = table_58G.Entry[i].ConnectedTime;
		}

		memset(host_name, 0, sizeof(host_name));
		get_hostname(host_name, str2lower(macbuff));

		if(is_mac_in_acl(str2upper(macbuff)))
		{
			block = false;
		}
		else
		{
			block = true;
		}

		while(fgets(buf, CMD_STR_LEN, fp)) 
		{
			if(strstr(buf, "IP"))
			{
				continue;
			}
			
			sscanf(buf, "%s %s %s %s %s %s", ip_entry, hw_type, flags, hw_address,mask,device);
			
			if((strlen(hw_address) != 17) || (strcmp(hw_address, "00:00:00:00:00:00") == 0))
			{
				continue;
			}
			if(strcmp(device, "br-lan"))
			{
				continue;
			}
			if(strncmp(str2upper(macbuff),str2upper(hw_address),17) == 0)
			{
				flag = 1;
				break;
			}

		}
		fseek(fp,0,SEEK_SET);

		if(0 == flag)
		{
			continue;
		}
		
		wifi_sta_obj = cJSON_CreateObject();
		get_sta_speed(wifi_sta_obj,str2upper(macbuff),host_name,0);
		
		cJSON_AddNumberToObject(wifi_sta_obj,"type",type);
		cJSON_AddNumberToObject(wifi_sta_obj,"uptime",connectedtime);
		cJSON_AddBoolToObject(wifi_sta_obj,"block",block);
		cJSON_AddStringToObject(wifi_sta_obj,"mac_addr",macbuff);
		cJSON_AddStringToObject(wifi_sta_obj,"ip_addr",ip_entry);
		cJSON_AddItemToArray(stalist_array,wifi_sta_obj);
		
		strcat(mac_buf,macbuff);
		strcat(mac_buf," ");
		
	}
	fclose(fp);
	return FCGI_TRUE;	
}


int get_sta_list(cJSON *stalist_array)
{
	cJSON *stalistentry = NULL;
	FILE *fp = NULL;
	char buf[CMD_STR_LEN];
	char ip_entry[RESULT_STR_LEN] = { 0 }, hw_type[SMALL_STR_LEN] = { 0 },flags[SMALL_STR_LEN] = { 0 };
	char host_name[OPTION_STR_LEN] = { 0 }, hw_address[RESULT_STR_LEN] = { 0 };
	char mask[SMALL_STR_LEN] = {0},device[RESULT_STR_LEN] = {0};
	char arp_w_mac[LONG_LIST_BUFF_LEN] = {0};
	bool block = false;

	if(stalist_array == NULL)
	{
		return FCGI_FALSE;
	}

	get_wifi_sta(stalist_array,	arp_w_mac, "radio0_0");
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)	
	get_wifi_sta(stalist_array, arp_w_mac, "radio1_0");
#endif

//访客模式下终端列表
	//get_wifi_sta(stalist_array, arp_w_mac, "radio0_1");
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)	
	//get_wifi_sta(stalist_array, arp_w_mac, "radio1_1");
#endif

	fp = fopen("/proc/net/arp", "r");
	
	if(!fp)
	{
		return FCGI_FALSE;
	}
	
	while(fgets(buf, CMD_STR_LEN, fp)) 
	{
		if(strstr(buf, "IP"))
		{
			continue;
		}
		
		sscanf(buf, "%s %s %s %s %s %s", ip_entry, hw_type, flags, hw_address,mask,device);

		if(strncmp(ip_entry,"169.254",strlen("169.254")))
		{
			continue;
		}
		
		if((strlen(hw_address) != 17) || (strcmp(hw_address, "00:00:00:00:00:00") == 0))
		{
			continue;
		}
		if(strcmp(device, "br-lan"))
		{
			continue;
		}

		if(strcmp(flags, "0x0") == 0)
		{
			continue;
		}

		if(strstr(str2upper(arp_w_mac),str2upper(hw_address)) != NULL)
		{
			continue;
		}
	
	
		memset(host_name, 0, sizeof(host_name));
		get_hostname(host_name, str2lower(hw_address));

		if(is_mac_in_acl(str2upper(hw_address)))
		{
			block = false;
		}
		else
		{
			block = true;
		}

		str2upper(hw_address);
		stalistentry = cJSON_CreateObject();
		get_sta_speed(stalistentry,str2upper(hw_address),host_name,1);
		cJSON_AddNumberToObject(stalistentry,"type",0);
		cJSON_AddBoolToObject(stalistentry,"block",block);
		cJSON_AddStringToObject(stalistentry,"mac_addr",hw_address);
		cJSON_AddStringToObject(stalistentry,"ip_addr",ip_entry);
		cJSON_AddItemToArray(stalist_array,stalistentry);	
	}
	fclose(fp);
	
	return FCGI_TRUE;
}


int get_home_devices(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *connected_list = NULL;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}
	
	result_root = cJSON_CreateObject();
	connected_list = cJSON_CreateArray();
	get_sta_list(connected_list);
	cJSON_AddItemToObject(result_root, "connected_list", connected_list);
	cJSON_AddItemToObject(response_root, WEB_RETURN_DATA_NAME, result_root);
	return FCGI_RESP_CODE_SUCCESS;
}

int set_device_name(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char mac_addr[MAC_STR_LEN] = {0};
	char host_name[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	if(!huayi_json_get_str(request_root,"mac_addr",mac_addr,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"name",host_name,OPTION_STR_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
	if(!mysql_add_ailas(mac_addr,host_name))
	{
		return FCGI_RESP_CODE_DATA_CONTENT_ERROR;
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
		/*todo sqlite3 rex, 2021-12-28*/
	
		/*if(!mysql_add_ailas(mac_addr,host_name))
		{
			return FCGI_RESP_CODE_DATA_CONTENT_ERROR;
		}*/
#endif

	return FCGI_RESP_CODE_SUCCESS;
}

int set_wireless_clock(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int enable_2g = 0;
	int disabled = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int	enable_5g = 0;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	if(!huayi_json_get_int(request_root,"enable_2g",&enable_2g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	disabled = enable_2g?0:1;
	huayi_uci_set_int(PKG_WIRELESS,"radio0_0","disabled",disabled);
	if(disabled)
	{
		system("ifconfig ra0 down");
	}
	else
	{
		huayi_uci_set_int(PKG_WIRELESS,"radio0","disabled",disabled);
		system("ifconfig ra0 up");
		system("brctl addif br-lan ra0");
	}
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_int(request_root,"enable_5g",&enable_5g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	disabled = enable_5g?0:1;
	huayi_uci_set_int(PKG_WIRELESS,"radio1_0","disabled",disabled);
	if(disabled)
	{
		system("ifconfig rai0 down");
	}
	else
	{
		huayi_uci_set_int(PKG_WIRELESS,"radio1","disabled",disabled);
		system("ifconfig rai0 up");
		system("brctl addif br-lan rai0");
	}
#endif
	huayi_uci_commit(PKG_WIRELESS);
	
	//system("huayi-reset wireless");

	//*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
	return FCGI_RESP_CODE_SUCCESS;

}

int set_guest_clock(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int enable_2g = 0;
	int disabled = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int	enable_5g = 0;
#endif
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	if(!huayi_json_get_int(request_root,"clock_guest_2g",&enable_2g))
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}

	disabled = enable_2g?0:1;
	huayi_uci_set_int(PKG_WIRELESS,"radio0_1","disabled",disabled);
	if(disabled)
	{
		system("ifconfig ra1 down");
	}
	else
	{
		huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"disabled",disabled);
		system("ifconfig ra1 up");
		system("brctl addif br-lan ra1");
	}
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(!huayi_json_get_int(request_root,"clock_guest_5g",&enable_5g))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	disabled = enable_5g?0:1;
	huayi_uci_set_int(PKG_WIRELESS,"radio1_1","disabled",disabled);
	if(disabled)
	{
		system("ifconfig rai1 down");
	}
	else
	{
		huayi_uci_set_int(PKG_WIRELESS,W58G_DEV,"disabled",disabled);
		system("ifconfig rai1 up");
		system("brctl addif br-lan rai1");
	}
#endif
	huayi_uci_commit(PKG_WIRELESS);

	system("huayi-reset wifiguesfilter");
	//system("huayi-reset wireless");

	//*lock_time = HUAYI_RELOAD_WIRELESS_TIME;
		
	return FCGI_RESP_CODE_SUCCESS;
}
#ifdef CONFIG_PACKAGE_EASYCWMP
#define LV_CWMP_STAT_FILE "/tmp/cwmp_link_stat"
int lv_get_cwmp_link_stat()
{
	FILE *fp=NULL;
	char buff[16]={0};
	int ret=0;
	fp = fopen(LV_CWMP_STAT_FILE,"r");
	if(fp){
		fgets(buff,sizeof(buff),fp);
		fclose(fp);
		ret = atoi(buff);
	}
	
	return ret;
}

int get_tr069_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int enable=0, portNum=0;
	char iface[16]={0}, Luname[32]={0}, Lpass[32]={0}, Lauth[16]={0};
	char url[64], Auname[32]={0}, Apass[32]={0};
	int heart_beat=0,interval=0;
	char serialNum[16]={0};
	int link=0;
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}
	result_root = cJSON_CreateObject();
//local config	
	huayi_uci_get_int(PKG_EASYCWMP,"local","enable",&enable);
	huayi_uci_get_int(PKG_EASYCWMP,"local","port",&portNum);
	huayi_uci_get_str(PKG_EASYCWMP,"local","interface",iface);
	if(!strcmp(iface,"br-lan")){
		memset(iface,0x00,sizeof(iface));
		strcpy(iface,"LAN");
	}
	else if(!strcmp(iface,"eth1") || !strncmp(iface,"ppp",3)){
		memset(iface,0x00,sizeof(iface));
		strcpy(iface,"WAN");
	}
		
	huayi_uci_get_str(PKG_EASYCWMP,"local","authentication",Lauth);
	huayi_uci_get_str(PKG_EASYCWMP,"local","username",Luname);
	huayi_uci_get_str(PKG_EASYCWMP,"local","password",Lpass);

// acs config
	huayi_uci_get_str(PKG_EASYCWMP,"acs","url",url);
	huayi_uci_get_str(PKG_EASYCWMP,"acs","username",Auname);
	huayi_uci_get_str(PKG_EASYCWMP,"acs","password",Apass);
	huayi_uci_get_int(PKG_EASYCWMP,"acs","periodic_enable",&heart_beat);
	huayi_uci_get_int(PKG_EASYCWMP,"acs","periodic_interval",&interval);

// device config
	huayi_uci_get_str(PKG_EASYCWMP,"device","serial_number",serialNum);
	link = lv_get_cwmp_link_stat();


	cJSON_AddNumberToObject(result_root,"enable",enable);
	cJSON_AddNumberToObject(result_root,"port",portNum);
	cJSON_AddStringToObject(result_root,"iface",iface);
	cJSON_AddStringToObject(result_root,"auth",Lauth);
	cJSON_AddStringToObject(result_root,"cpe_uname",Luname);
	cJSON_AddStringToObject(result_root,"cpe_passwd",Lpass);
	
	cJSON_AddStringToObject(result_root,"url",url);
	cJSON_AddStringToObject(result_root,"acs_uname",Auname);
	cJSON_AddStringToObject(result_root,"acs_passwd",Apass);
	cJSON_AddNumberToObject(result_root,"heart_beat",heart_beat);
	cJSON_AddNumberToObject(result_root,"interval",interval);
	cJSON_AddStringToObject(result_root,"serial",serialNum);
	cJSON_AddNumberToObject(result_root,"link_stat",link);
	cJSON_AddItemToObject(response_root, WEB_RETURN_DATA_NAME, result_root);

	return FCGI_RESP_CODE_SUCCESS;
}
int set_tr069_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int enable=0, portNum=0;
	char iface[16]={0}, Luname[32]={0}, Lpass[32]={0}, Lauth[16]={0};
	char url[64]={0},tmpStr[64]={0}, Auname[32]={0}, Apass[32]={0};
	int heart_beat=0,interval=0;
	char serialNum[16]={0};
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_HYID_MISS;
	}
	
	if(!huayi_json_get_int(request_root,"enable",&enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_int(request_root,"port",&portNum))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_str(request_root,"iface",iface,sizeof(iface)))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_str(request_root,"auth",Lauth,sizeof(Lauth)))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_str(request_root,"cpe_uname",Luname,sizeof(Luname)))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_str(request_root,"cpe_passwd",Lpass,sizeof(Lpass)))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_str(request_root,"url",tmpStr,sizeof(tmpStr)))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	else{
		if(strncmp(tmpStr,"http://",strlen("http://")) && strncmp(tmpStr,"https://",strlen("https://"))){
			sprintf(url,"http://%s",tmpStr);
		}
		else{
			strcpy(url,tmpStr);
		}
	}
	if(!huayi_json_get_str(request_root,"acs_uname",Auname,sizeof(Auname)))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_str(request_root,"acs_passwd",Apass,sizeof(Apass)))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	if(!huayi_json_get_int(request_root,"heart_beat",&heart_beat))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}	
	if(!huayi_json_get_int(request_root,"interval",&interval))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
/*
	if(!huayi_json_get_int(request_root,"serial",serialNum))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}	
*/

//local config	
	huayi_uci_set_int(PKG_EASYCWMP,"local","enable",enable);
	huayi_uci_set_int(PKG_EASYCWMP,"local","port",portNum);
//	huayi_uci_set_str(PKG_EASYCWMP,"local","interface",iface);
	huayi_uci_set_str(PKG_EASYCWMP,"local","authentication",Lauth);
	huayi_uci_set_str(PKG_EASYCWMP,"local","username",Luname);
	huayi_uci_set_str(PKG_EASYCWMP,"local","password",Lpass);

// acs config
	huayi_uci_set_str(PKG_EASYCWMP,"acs","url",url);	
	huayi_uci_set_str(PKG_EASYCWMP,"acs","username",Auname);
	huayi_uci_set_str(PKG_EASYCWMP,"acs","password",Apass);
	huayi_uci_set_int(PKG_EASYCWMP,"acs","periodic_enable",heart_beat);
	huayi_uci_set_int(PKG_EASYCWMP,"acs","periodic_interval",interval);

	huayi_uci_commit(PKG_EASYCWMP);
	system("/etc/init.d/easycwmpd restart 2>/dev/null");
	return FCGI_RESP_CODE_SUCCESS;
}

#endif
