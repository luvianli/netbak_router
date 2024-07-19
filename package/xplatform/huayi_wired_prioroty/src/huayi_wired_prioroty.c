#include <stdio.h>
#include <unistd.h>
#include <huayi_uci_fun.h>

typedef enum{
	INTERNET_UNDEFINE = 0,
	INTERNET_4G,
	INTERNET_WIRED,
	
}INTERNET_MODE;


int main(int argc,char *argv[])
{
	char wired_device[SHORT_STR_LEN] = {0};
	char cpe_device[SHORT_STR_LEN] = {0};
	char wan_ifname[SHORT_STR_LEN] = {0};
	char wan_ipv4[IPV4_STR_LEN] = {0};
	char wan_netmask[IPV4_STR_LEN] = {0};
	char wan_gateway[IPV4_STR_LEN] = {0};
	char cmd_str[CMD_STR_LEN] = {0};
	char temp_dns[TEMP_STR_LEN] = {0};
	char dns[IPV4_STR_LEN] = {0};
	char section[SHORT_STR_LEN] = {0};
	char del_gateway[IPV4_STR_LEN] = {0};
	char resultbuf[SHORT_STR_LEN] = {0};

	char wan_type[OPTION_STR_LEN] = {0};
	char net_ipv4[IPV4_STR_LEN] = {0};

	int wired_status = 0;
	int cpe_status = 0;
	int wired_link = 0;
	int cpe_link = 0;
	int old_int_mode = INTERNET_4G;
	int now_int_mode = INTERNET_UNDEFINE;
	int  i = 0;
	char detectip[OPTION_STR_LEN] = {0};
	
	while(1)
	{
		sleep(10);
		memset(wan_type,0,sizeof(wan_type));
		huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
		huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);

		if(strcmp(wan_type,"prior") == 0)
		{
		memset(wired_device,0,sizeof(wired_device));
		memset(cpe_device,0,sizeof(cpe_device));
		wired_link = 0;
		cpe_link = 0;
		wired_status = 0;
		cpe_status = 0;
		huayi_uci_get_str(PKG_STATUS_BLEND, "wired", "device", wired_device);
		huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "device", cpe_device);
		huayi_uci_get_int(PKG_STATUS_BLEND, "wired", "link", &wired_link);
		huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &cpe_link);

		if(wired_link == 1 && cpe_link == 1)//同时在线时，使用有线优先策略
		{
			wired_status = first_internet_do_ping(detectip, wired_device);
			if(wired_status == 0)
			{
				cpe_status = first_internet_do_ping(detectip, cpe_device);
				if(cpe_status == 0)//使用有线上网方式
				{
					sprintf(section,"wired");
					now_int_mode = INTERNET_WIRED;
					old_int_mode = INTERNET_UNDEFINE;
				}else//使用4G上网方式
				{
					sprintf(section,"cpe");
					now_int_mode = INTERNET_4G;
				}
			}else//使用有线上网方式
			{
				sprintf(section,"wired");
				now_int_mode = INTERNET_WIRED;
			}

		}else if(cpe_link == 1)//使用4G上网方式
		{	
			sprintf(section,"cpe");
			now_int_mode = INTERNET_4G;
		}else//使用有线上网方式
		{
			sprintf(section,"wired");
			now_int_mode = INTERNET_WIRED;
		}
		//printf("old_int_mode = %d\n",old_int_mode);
		//printf("now_int_mode = %d\n",now_int_mode);
		//if(old_int_mode != now_int_mode || )
		{
			memset(cmd_str,0,sizeof(cmd_str));
			memset(del_gateway,0,sizeof(del_gateway));
			if(!strncmp(section,"wired",strlen("wired")))//有线上网方式，删除4G默认路由
			{
				huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", del_gateway);
			}
			else//4G上网方式，删除有线默认路由
			{
				huayi_uci_get_str(PKG_STATUS_BLEND, "wired", "gateway", del_gateway);
			}

			memset(wan_ifname,0,sizeof(wan_ifname));
			memset(wan_ipv4,0,sizeof(wan_ipv4));
			memset(wan_netmask,0,sizeof(wan_netmask));
			memset(wan_gateway,0,sizeof(wan_gateway));
			huayi_uci_get_str(PKG_STATUS_BLEND, section, "device", wan_ifname);
			huayi_uci_get_str(PKG_STATUS_BLEND, section, "ipv4", wan_ipv4);
			huayi_uci_get_str(PKG_STATUS_BLEND, section, "netmask", wan_netmask);
			huayi_uci_get_str(PKG_STATUS_BLEND, section, "gateway", wan_gateway);
			huayi_uci_get_str(PKG_STATUS_BLEND, section, "dns_server",temp_dns);
			//printf("del_gateway = %s\n",del_gateway);
			if(strncmp(del_gateway,"0.0.0.0",strlen("0.0.0.0")))
			{
				sprintf(cmd_str,"route del default gw %s",del_gateway);
				system(cmd_str);
				//printf("cmd_str = %s\n",cmd_str);

				memset(cmd_str,0,sizeof(cmd_str));
				sprintf(cmd_str,"route -n | grep %s | grep UG | grep %s |wc -l",wan_ifname,wan_gateway);
				get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));

				if(atoi(resultbuf) == 0 && strncmp(wan_gateway,"0.0.0.0",strlen("0.0.0.0")))
				{
					memset(cmd_str,0,sizeof(cmd_str));
					sprintf(cmd_str,"route add default gw %s dev %s",wan_gateway,wan_ifname);
					system(cmd_str);
				}
			}

			huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", net_ipv4);
			//if(strcmp(wan_ipv4,net_ipv4))
			{	
				memset(cmd_str,0,sizeof(cmd_str));
				snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
				system(cmd_str);

				memset(cmd_str,0,sizeof(cmd_str));
				snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_ipv4 %s",wan_ipv4);
				system(cmd_str);

				memset(cmd_str,0,sizeof(cmd_str));
				snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_netmask %s",wan_netmask);
				system(cmd_str);

				memset(cmd_str,0,sizeof(cmd_str));
				snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_gateway %s",wan_gateway);
				system(cmd_str);

				memset(dns,0,IPV4_STR_LEN);
				i = 0;
				//printf("temp_dns = %s\n",temp_dns);
				while(huayi_get_nth_value(i++, temp_dns, " ", dns, sizeof(dns)))
				{
					//printf("dns = %s\n",dns);
					memset(cmd_str,0,sizeof(cmd_str));
					snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_dns_server %s",dns);
					//printf("cmd_str = %s\n",cmd_str);
					system(cmd_str);
				}

				if(now_int_mode == INTERNET_4G)
				{
					system("ifup wan4g");					
				}else if(now_int_mode == INTERNET_WIRED)
				{
					system("ifup wan");
				}
			}

			old_int_mode = now_int_mode;
		}
		}
		/*else if(strcmp(wan_type,"mobile") == 0)
		{	
			if(!first_internet_do_ping(detectip, "eth1"))
			{
				memset(net_ipv4,0,sizeof(net_ipv4));
				huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address", net_ipv4);
				memset(wan_ipv4,0,sizeof(wan_ipv4));
				if(atcmd_get_cgpaddr(wan_ipv4))
				{
					if(strcmp(net_ipv4,wan_ipv4))
					{
						memset(cmd_str,0,TEMP_STR_LEN);
						sprintf(cmd_str,"huayi_cmd set network wan_ipv4 %s",wan_ipv4);
						system(cmd_str);
					}
				}
			}
			sleep(20);
		}*/
	}

	return 0;

}
