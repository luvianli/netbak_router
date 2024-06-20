#include "huayi_handle_reload.h"

#if defined(CONFIG_HUAYI_SUPPORT_VPN)

#define VPN_STATUS_FILE "/tmp/vpn_status"
#define VPN_USER_FILE	"/var/etc/chap-secrets"

typedef struct __vpn_client
{
	int pid;
	char username[OPTION_STR_LEN];	
}vpn_client;

typedef struct __vpn_user
{
	char username[OPTION_STR_LEN];	
}vpn_user;

int Eliminate_client()
{
	vpn_client*  client = NULL;
	vpn_user*    user = NULL;
	int ret = 0;
	char result[RESULT_STR_LEN] = {0};

	char account_type[OPTION_STR_LEN] = {0};
	int pid = 0;
	char ip[IPV4_STR_LEN] = {0};
	char device_ip[IPV4_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	int uptime = 0;
	char buff[TEMP_STR_LEN] = {0};
	char tmp[TEMP_STR_LEN] = {0};
	char cmd[OPTION_STR_LEN] = {0};
	int client_num = 0,user_num = 0;
	int i = 0,j = 0;
	int flag = 0;
	FILE *fp = NULL;
	
	get_cmd_result("cat /tmp/vpn_status|wc -l",result,RESULT_STR_LEN);
	client_num = atoi(result);
	memset(result,0,RESULT_STR_LEN);

	get_cmd_result("cat /var/etc/chap-secrets| wc -l",result,RESULT_STR_LEN);
	user_num = atoi(result);

	if(client_num == 0)
	{
		return 0;
	}

	client = malloc(sizeof(vpn_client) * client_num);
	if(!client)
	{
		ret = -1;
		goto end;
	}
	
	user = malloc(sizeof(vpn_user) * user_num);
	if(!user)
	{
		ret = -1;
		goto end;
	}

	fp = fopen(VPN_STATUS_FILE,"r");
	if(NULL == fp)
	{
		ret = -1;
		goto end;
	}

	while(fgets(buff, sizeof(buff), fp) != NULL) 
	{
		pid = 0;
		memset(username,0,OPTION_STR_LEN);
		if(i >= client_num)
		{
			break;
		}
		
		sscanf(buff,"%s %d %*s %s %s %s %d",account_type,&pid,device_ip,ip,username,&uptime);
		client[i].pid = pid;
		strcpy(client[i].username,username);
		i ++ ;
		memset(buff,0,sizeof(buff));
	}
	fclose(fp);

	fp = fopen(VPN_USER_FILE,"r");
	if(NULL == fp)
	{
		ret = -1;
		goto end;
	}

	i = 0;
	while(fgets(buff, sizeof(buff), fp) != NULL)
	{
		memset(username,0,OPTION_STR_LEN);
		if(i >= user_num)
		{
			break;
		}
		sscanf(buff,"%s %s",username,tmp);
		strcpy(user[i].username,username);
		i++;
		memset(buff,0,sizeof(buff));
	}
	fclose(fp);

	for(i = 0; i < client_num; i++)
	{
		flag = 0;
		for(j = 0; j < user_num; j++)
		{
			if(0 == strcmp(client[i].username,user[j].username))
			{
				flag = 1;
				break;
			}
		}

		if(0 == flag)
		{
			memset(cmd,0,OPTION_STR_LEN);
			snprintf(cmd,sizeof(cmd),"kill %d",client[i].pid);
			system(cmd);
		}
	}

	ret = 0;

end:

	if(client != NULL)
	{
		free(client);
	}

	if(user != NULL)
	{
		free(user);
	}

	return ret;
	
}


VTBool huayi_reload_pptpd(void * arg,cJSON *input_root)
{
	int pptpd_enable = 0;
	FILE * fp = NULL;
	char buff[OPTION_STR_LEN] = {0};
	char account_type[OPTION_STR_LEN] = {0};
	int pid = 0;
	char cmd[CMD_STR_LEN] = {0};

	//huayi_uci_get_int(PKG_PPTPD,"pptpd","enable",&pptpd_enable);
		
	//if(pptpd_enable == 0)
	{
		fp = fopen("/tmp/vpn_status", "r");
			
		if (NULL != fp)
		{
			while(fgets(buff, sizeof(buff), fp) != NULL) 
			{
				sscanf(buff,"%s %d %*s %*s %*s %*s %*s",account_type,&pid);
		
				if(!strcmp(account_type, "pptp"))
				{
					snprintf(cmd,sizeof(cmd),"kill %d",pid);
					system(cmd);
				}
			}
			fclose(fp);
			fp = NULL;
		}	
	}
		
	HuayiSystem("/etc/init.d/pptpd restart", 0);
	
	huayi_ppp_handler(PPP_HANDLE_PPTPD);

	return VT_TRUE;
}


VTBool huayi_reload_xl2tpd(void * arg,cJSON *input_root)
{
	HuayiSystem("/etc/init.d/xl2tpd restart", 0);
	HuayiSystem("/etc/init.d/ipsec restart", 0);
	huayi_ppp_handler(PPP_HANDLE_XL2TPD);
	
	return VT_TRUE;
}


VTBool huayi_reload_pptp(void * arg,cJSON *input_root)
{
	HuayiSystem("/etc/init.d/pptp restart", 0); 

	return VT_TRUE;
}

VTBool huayi_reload_xl2tp(void * arg,cJSON *input_root)
{
	HuayiSystem("/etc/init.d/xl2tp restart", 0); 

	return VT_TRUE;
}

VTBool huayi_reload_vpn_user(void * arg,cJSON *input_root)
{
	int ret = 0;
	HuayiSystem("/etc/init.d/vpn_user restart", 0); 
	//system("huayi-reset pptpd"); 
	//system("huayi-reset xl2tpd"); 
	if(access(VPN_USER_FILE,F_OK) == 0 && access(VPN_STATUS_FILE,F_OK) == 0)
	{
		ret = Eliminate_client();
	}

	if(ret == -1)
	{
		return VT_FALSE;
	}

	return VT_TRUE;
}

VTBool huayi_reload_vpn_route(void * arg,cJSON *input_root)
{
	char section[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char gateway[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};
	char pptp_gateway[IPV4_STR_LEN] = {0};
	char xl2tp_gateway[IPV4_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0}; 
	int i = 0;
	int xl2tp_route_type = 0;
	int pptp_route_type = 0;
	huayi_uci_get_int(PKG_PPL2TP,"xl2tp","route_type",&xl2tp_route_type);
	huayi_uci_get_int(PKG_PPL2TP,"pptp","route_type",&pptp_route_type);
	
	huayi_uci_get_str(PKG_STATUS_SYSTEM,"sys","pptp_ip",pptp_gateway);
	huayi_uci_get_str(PKG_STATUS_SYSTEM,"sys","xl2tp_ip",xl2tp_gateway);
	while(1)
	{
		snprintf(section,sizeof(section),"@route[%d]",i++);
		
		if(!huayi_uci_get_str(PKG_PPL2TP,section,"interface",inter_face))
		{
			break;
		}
		if(strcmp(inter_face,"XL2TP") == 0)
		{
			strcpy(gateway,xl2tp_gateway);
			if(xl2tp_route_type == 1)
			{
				continue;
			}
		}
		else
		{
			strcpy(gateway,pptp_gateway);
			if(pptp_route_type == 1)
			{
				continue;
			}
		}
		huayi_uci_get_str(PKG_PPL2TP,section,"target",target);
		huayi_uci_get_str(PKG_PPL2TP,section,"netmask",netmask);

		snprintf(cmd,sizeof(cmd),"route add -net %s netmask %s gw %s metric %d",target,netmask,gateway,1);
		system(cmd);
	}
	
	return VT_TRUE;
}

VTBool huayi_reload_vpn_server_route(void * arg,cJSON *input_root)
{
	FILE * fp = NULL;
	char buff[OPTION_STR_LEN] = {0};
	char account_type[OPTION_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};
	char vpn_ip[IPV4_STR_LEN] = {0};
	char wan_ip[IPV4_STR_LEN] = {0};
	char vpn_username[OPTION_STR_LEN] = {0};
	char section[OPTION_STR_LEN] = {0};
	int pid = 0;
	int i = 0;
	int uptime = 0;
	char cmd[CMD_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};

	fp = fopen("/tmp/vpn_status", "r");
	if (NULL != fp)
	{
		while(fgets(buff, sizeof(buff), fp) != NULL) 
		{
			i = 0;
			sscanf(buff,"%s %d %s %s %s %s %d",account_type,&pid,inter_face,vpn_ip,wan_ip,vpn_username,&uptime);
			while(1)
			{
				snprintf(section,sizeof(section),"@route[%d]",i++);
				
				if(!huayi_uci_get_str(PKG_VPN_USER,section,"vpn_username",username))
				{
					break;
				}
				if(strcmp(username,vpn_username) != 0)
				{
					continue;
				}

				huayi_uci_get_str(PKG_VPN_USER,section,"target",target);
				huayi_uci_get_str(PKG_VPN_USER,section,"netmask",netmask);
				snprintf(cmd,sizeof(cmd),"route add -net %s netmask %s gw %s metric %d",target,netmask,vpn_ip,1);
				system(cmd);
			}	
		}
		fclose(fp);
		fp = NULL;
	}
	return VT_TRUE;
}
VTBool huayi_reload_openvpn(void * arg,cJSON *input_root)
{
	int enable = 0;
	huayi_uci_get_int(PKG_PPL2TP,"openvpn","enable",&enable);
	if(!enable){
		system("/etc/init.d/openVPN stop");
		//system("rm -f /path/to/*");
	}
	else
		system("/etc/init.d/openVPN start");

	sleep(5);
	system("/etc/init.d/network restart");
	return VT_TRUE;
}


#endif

