#include "huayi_reload.h"

ReloadCmdStructure CmdTable[] = {
	{"upgrade",HUAYI_MSG_SYS_UPGRADE,0,"--To upgrade the firmware"},
	{"reset",HUAYI_MSG_SYS_RESET,0,"--Device reset"},
	{"reboot",HUAYI_MSG_SYS_REBOOT,0,"--Equipment to restart"},
	{"sche_reboot",HUAYI_MSG_SCHE_REBOOT,0,"--Periodic Device Restart"},
	{"restore_config",HUAYI_MSG_RESTORE_CONFIG,0,"--Restarting the System Configuration"},
	{"network",HUAYI_MSG_RELOAD_NETWORK,0,"--Reset network"},
	{"dhcps",HUAYI_MSG_RELOAD_DHCPS,0,"--Reset dhcps"},
	{"wireless",HUAYI_MSG_RELOAD_WIRELESS,0,"--Reset wireless"},
	{"ntpd",HUAYI_MSG_RELOAD_NTPD,0,"--Reset system ntpd"},
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
	{"firewall",HUAYI_MSG_RELOAD_FIREWALL,0,"--Reset firewall"},
	{"wanup",HUAYI_MSG_RELOAD_WANUP,0,"--Reset wanup"},
	{"static_route",HUAYI_MSG_RELOAD_STATIC_ROUTE,0,"--Reset static route"},
	{"remotemanage",HUAYI_MSG_RELOAD_FIREWALL_REMOTE_MANAGE,0,"--Reset remote management"},
	{"macfilter",HUAYI_MSG_RELOAD_FIREWALL_MACFILTER,0,"--Reset mac filter"},
	{"urlfilter",HUAYI_MSG_RELOAD_FIREWALL_URLFILTER,0,"--Reset url filter"},
	{"ipportfilter",HUAYI_MSG_RELOAD_FIREWALL_IPPORTFILTER,0,"--Reset ip/port filter"},
	{"dmz",HUAYI_MSG_RELOAD_FIREWALL_DMZ,0,"--Reset dmz"},
	{"portforward",HUAYI_MSG_RELOAD_FIREWALL_PORTFORWARD,0,"--Reset port forward"},
	{"wanping",HUAYI_MSG_RELOAD_WANPING,0,"--Reset wanping"},
	{"upnpd",HUAYI_MSG_RELOAD_UPNPD,0,"--Reset upnpd"},
	{"acl", HUAYI_MSG_RELOAD_ACL,0,"--Reset acl"},
	{"wifiguesfilter", HUAYI_MSG_RELOAD_WIFIGUESTFILTER,0,"--Reset wifiguestfilter"},
	{"iplimit",HUAYI_MSG_RELOAD_QOS_IPLIMIT,0,"--Reset qos iplimit"},
#endif
#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
	{"ddns",HUAYI_MSG_RELOAD_DDNS,0,"--Reset ddns"},
#endif
	{"wps_pbc", HUAYI_MSG_RELOAD_WPSPBC,1,"--Reset wps pbc"},
	{"wps_pin", HUAYI_MSG_RELOAD_WPSPIN,1,"--Reset wps pin"},
	{"hwnat", HUAYI_MSG_RELOAD_HWNAT,0,"--Reset hardware nat"},
	{"scan",HUAYI_MSG_RELOAD_WIRELESS_SCAN,1,"--Reset wireless scan"},
	{"wifi_sche",HUAYI_MSG_RELOAD_WIFI_SCHE,0,"--Reset wifi schedule"},
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	{"lte4g",HUAYI_MSG_RELOAD_LTE_4G,1,"--Reset LTE 4G, 1 restart model,0 is not"},
#endif
#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
	{"save_database",HUAYI_MSG_RELOAD_SAVE_DATABASE,0,"--Reset save database"},
#endif
#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
	{"save_database",HUAYI_MSG_RELOAD_SAVE_DATABASE,0,"--Reset save database"},
#endif
#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	{"pptpd",HUAYI_MSG_RELOAD_PPTPD,0,"--Reset pptpd server"},
	{"xl2tpd",HUAYI_MSG_RELOAD_XL2TPD,0,"--Reset xl2tpd server"},
	{"pptp",HUAYI_MSG_RELOAD_PPTP,0,"--Reset pptp client"},
	{"xl2tp",HUAYI_MSG_RELOAD_XL2TP,0,"--Reset xl2tp client"},
	{"vpn_user",HUAYI_MSG_RELOAD_VPN_USER,0,"--Reset vpn_user"},
	{"vpn_route",HUAYI_MSG_RELOAD_VPN_ROUTE,0,"--Reset vpn route"},
	{"vpn_server_route",HUAYI_MSG_RELOAD_SERVER_VPN_ROUTE,0,"--Reset vpn server route"},
#endif
	{"ftpd",HUAYI_MSG_RELOAD_FTPD,0,"--Reset ftpd"},
	{"printer",HUAYI_MSG_RELOAD_PRINTER,0,"--Reset printer"},
	{"mount",HUAYI_MSG_RELOAD_MOUNT,0,"--Reset mount"},
	{"udhcpd",HUAYI_MSG_RELOAD_UDHCPD,0,"--Reset udhcpd"},
#if defined(CONFIG_HUAYI_SUPPORT_CONTROL_TELNET_OPTION)
	{"huayi_telnet",HUAYI_MSG_RELOAD_TELNET,0,"--Reset huayi_telnet"},
#endif
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	{"small4gfunc",HUAYI_MSG_RELOAD_SMALL_4GFUNC,0,"--Reset 4G Minimum functionality"},
#endif
    {"switch_sim",HUAYI_MSG_SWITCH_CARD,0,"--Start the checker and save the configuration file usesiminfo"},
        
    {"check_internet_state",HUAYI_MSG_CHECK_INTERNET_STATE,0,"check_internet_state"},
        
    {"zry",HUAYI_MEIGE_MODULE_DECRYPTION_STATE,0,"check_internet_state"},

};

void huayi_reload_usage()
{
	int i = 0,k = 0;	

	printf("\n");
	printf("\n");
	printf("*********welcome huayi-reset*********\n");
	printf("HUAYI Reload configuration\n");
	printf("Usage:huayi-reset [module_name]\n");
	printf("\n");
	printf("\n");
	
	for(i=0;i<ARRAY_SIZE(CmdTable);i++)
	{
		if(CmdTable[i].agrc == 0)
		{
			printf("%-15s %-20s\n",CmdTable[i].CmdStr,CmdTable[i].HelpInfo);
		}
		else
		{
			printf("%-15s",CmdTable[i].CmdStr);
			for(k=0;k<CmdTable[i].agrc;k++)
			{
				printf(" [argv%d] [value]",k);
			}
			printf(" %s\n",CmdTable[i].HelpInfo);
		}
		
	}

	return;
}

void huayi_reload_error()
{
	printf("huayi reload execution failed\n");
	printf("Please check core service\n");
	return;
}


void huayi_reload_init(struct ReloadAgent *Agent)
{
	memset(Agent,0,sizeof(struct ReloadAgent));
	Agent->input_root = NULL;
	return;
}

VRBool huayi_reload_check_cmd(struct ReloadAgent *Agent,int argc,char **argv)
{
	int i = 0,k = 0;
	
	for(i=0;i<ARRAY_SIZE(CmdTable);i++)
	{
		if(!strcmp(argv[1],CmdTable[i].CmdStr))
		{
			if(argc != (CmdTable[i].agrc * 2 + 2))
			{
				return VR_FALSE;
			}
			Agent->input_root = cJSON_CreateObject();
			
			cJSON_AddNumberToObject(Agent->input_root,HUAYI_OPTION_MSGTYPE_KEY,CmdTable[i].msg);
			
			if(CmdTable[i].agrc > 0)
			{
				for(k=0;k<CmdTable[i].agrc;k++)
				{
					cJSON_AddStringToObject(Agent->input_root,argv[k*2+2],argv[k*2+3]);
				}
			}
			
			return VR_TRUE;
		}
	}
	
	return VR_FALSE;
}

VRBool huayi_reload_ipc_init(struct ReloadAgent *Agent)
{
    if ((Agent->sendSocket.reload_ipc_fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		printf("huayi_core_ipc_init Error Creating Socket...\n");
		return VR_FALSE;
	}
	
	memset(&(Agent->sendSocket.address),0,sizeof(Agent->sendSocket.address));

	unlink (HUAYI_RELOAD_DOMAIN);
	
	strncpy(Agent->sendSocket.address.sun_path,HUAYI_RELOAD_DOMAIN,sizeof(Agent->sendSocket.address.sun_path)-1);
	
    Agent->sendSocket.address.sun_family = AF_UNIX;

	/*we use recv buf default,because our msg is short*/
	#if 0
   	ret = setsockopt(Agent->sendSocket.reload_ipc_fd, SOL_SOCKET, SO_RCVBUF, &recvBufSize, len);
   	if(ret == -1)
   	{
	   perror("setsockopt error.");
 	}
	#endif
	
	if( bind(Agent->sendSocket.reload_ipc_fd,( struct sockaddr * )&Agent->sendSocket.address, sizeof(Agent->sendSocket.address)) < 0 )
	{
		printf("huayi_core_ipc_init Error Binding Socket...\n");
		return VR_FALSE;
	}

	memset(&(Agent->sendSocket.address),0,sizeof(Agent->sendSocket.address));

	strncpy(Agent->sendSocket.address.sun_path,HUAYI_CORE_DOMAIN,sizeof(Agent->sendSocket.address.sun_path)-1);
	
    Agent->sendSocket.address.sun_family = AF_UNIX;
	
	return VR_TRUE;
}

VRBool reload_send_msg_to_core(struct ReloadAgent* Agent)
{
	int sendbyte = 0;
	int len = 0;
	char *output = NULL;	

	output= cJSON_Print(Agent->input_root);
	len = strlen(output);

	sendbyte = sendto(Agent->sendSocket.reload_ipc_fd,output,len,0,(struct sockaddr*)&Agent->sendSocket.address, sizeof(Agent->sendSocket.address));

	if (sendbyte != len)
	{
		printf("reload_send_msg_to_core sent a different number of bytes than expected, send %d byte \n",sendbyte);
		free(output);
		return VR_FALSE;
	}

	free(output);
	return VR_TRUE;
}

VRBool huayi_reload_ipc_destory(struct ReloadAgent *Agent)
{
	close(Agent->sendSocket.reload_ipc_fd);

	return VR_TRUE;
}

VRBool huayi_reload_exit(struct ReloadAgent *Agent)
{
	huayi_reload_ipc_destory(Agent);
	cJSON_Delete(Agent->input_root);
	return VR_TRUE;
}

int main(int argc, char *argv[])
{
	struct ReloadAgent Agent;

	if (argc < 2)
	{
		goto invaild_command;
	}

	huayi_reload_init(&Agent);

	if(!huayi_reload_check_cmd(&Agent,argc,argv))
	{
		goto invaild_command;
	}
	
	if(!huayi_reload_ipc_init(&Agent))
	{
		goto system_error;
	}
	
	if(!reload_send_msg_to_core(&Agent))
	{
		goto system_error;
	}

	huayi_reload_exit(&Agent);
	
	return 0;

invaild_command:
	huayi_reload_usage();
	return 1;


system_error:
	huayi_reload_exit(&Agent);
	huayi_reload_error();
	return 1;
	
}



