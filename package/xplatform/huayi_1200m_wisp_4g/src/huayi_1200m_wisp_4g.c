#include <stdio.h>
#include <unistd.h>
#include <huayi_uci_fun.h>

typedef enum{
	INTERNET_UNDEFINE = 0,
	INTERNET_4G,
	INTERNET_WIRED,
	
}INTERNET_MODE;
static int delaycount = 1;


void get_maxdelay(char *checksignalips, char *ifname)
{
	char cmd[TEMP_STR_LEN] = {0};
	char value[OPTION_STR_LEN] = { 0 };
	int i = 0, maxdelay = 0;
	int timevalue = 0;

	huayi_uci_get_int(PKG_SYSTEM,"mode","detect_time", &maxdelay);
	for(i = 0; i < 5; i++)
	{
		sprintf(cmd, "ping -c 1 -W 1 %s -I %s | grep \"time=\" | awk '{print $7}' > /tmp/pingtmp.txt", checksignalips, ifname);
		system(cmd);
	//	CON_LOG("cmd = %s\n", cmd);
		
		memset(cmd, 0, TEMP_STR_LEN);
		memset(value, 0, sizeof(value));
		strcpy(cmd, "cat /tmp/pingtmp.txt | awk -F \" \" '{print $1}'");
		get_cmd_result(cmd, value, sizeof(value));
	//	CON_LOG("ping time value  = %s\n", value);
		timevalue = atoi(value+strlen("time="));
		if(timevalue != 0)
		{
		//	CON_LOG("ping time value  = %d, maxdelay = %d\n", timevalue, maxdelay);
			if(timevalue > maxdelay)
			{
				delaycount += 1;
			}
		}
		else
		{
		//	CON_LOG("strlen(value) = %d , ping time value  = %s \n", strlen(value), value);
			delaycount += 1;
		}
	}

	return;
}


int wisp4g_internet_do_ping(const char *host, char *ifname, char *del_gateway)
{
	char cmd[CMD_STR_LEN] = {0};
	char wan_gateway[IPV4_STR_LEN] = {0};
	char resultbuf[SHORT_STR_LEN] = {0};
	int i = 0;

	if ((NULL == host) || (strlen(host)<7))
		goto fail;

	
	if(strncmp(del_gateway,"0.0.0.0",strlen("0.0.0.0")))
	{
		sprintf(cmd,"route del default gw %s",del_gateway);
		system(cmd);
		//printf("cmd_str = %s\n",cmd_str);
		if(strcmp(ifname, "apcli0") == 0 ||strcmp(ifname, "apclii0") == 0)
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "gateway", wan_gateway);
		}
		else
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", wan_gateway);
		}
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"route -n | grep %s | grep UG | grep %s |wc -l", ifname, wan_gateway);
		get_cmd_result(cmd, resultbuf, sizeof(resultbuf));

		if(atoi(resultbuf) == 0 && strncmp(wan_gateway,"0.0.0.0",strlen("0.0.0.0")))
		{
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"route add default gw %s dev %s",wan_gateway,ifname);
			system(cmd);
		}
	}
	else
	{
		goto fail;
	}

	memset(cmd,0,sizeof(cmd));
	for(i = 0;i < 3; i++)
	{
		if (ifname == NULL)
		{
			snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s > /dev/null && touch %s", 1, host, PING_TMPFILE);
		}
		else
		{
			snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s -I %s > /dev/null && touch %s", 1, host, ifname, PING_TMPFILE);
		}
		system(cmd);
		usleep(500);
		if ( access(PING_TMPFILE, F_OK) == 0)
		{
			unlink(PING_TMPFILE);
			return 1;
		}
	}
fail:
	return 0;
}


int main(int argc,char *argv[])
{
	char wired_device[SHORT_STR_LEN] = {0};
	char cpe_device[SHORT_STR_LEN] = {0};
	char wan_ifname[SHORT_STR_LEN] = {0};
	char wan_ipv4[IPV4_STR_LEN] = {0};
	char wisp_ipv4[IPV4_STR_LEN] = {0};
	char wan4g_ipv4[IPV4_STR_LEN] = {0};
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
	int  i = 0;
	char detectip[OPTION_STR_LEN] = {0};

	delaycount = 1;
	while(1)
	{
		sleep(10);
		memset(wan_type,0,sizeof(wan_type));
		huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);

		if(strcmp(wan_type,"rep4g") != 0)
		{
			continue;
		}
		huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
		memset(wired_device,0,sizeof(wired_device));
		memset(cpe_device,0,sizeof(cpe_device));
		memset(wisp_ipv4, 0, sizeof(wisp_ipv4));
		memset(wan4g_ipv4, 0, sizeof(wan4g_ipv4));
		wired_link = 0;
		cpe_link = 0;
		wired_status = 0;
		cpe_status = 0;
		huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "device", wired_device);
		huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "device", cpe_device);
		huayi_uci_get_int(PKG_STATUS_BLEND, "wisp", "link", &wired_link);
		huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &cpe_link);
		huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "ipv4", wisp_ipv4);
		huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "ipv4", wan4g_ipv4);
		if(wired_link == 0 && cpe_link == 0)
			continue;
		if(strncmp(wisp_ipv4, "0.0.0.0", strlen("0.0.0.0")) == 0
			&& strncmp(wan4g_ipv4, "0.0.0.0", strlen("0.0.0.0")) == 0)
			continue;
		memset(wisp_ipv4, 0, sizeof(wisp_ipv4));
		memset(wan4g_ipv4, 0, sizeof(wan4g_ipv4));
		if(wired_link == 1 && cpe_link == 1)//同时在线时，使用wisp优先策略
		{
			memset(del_gateway, 0, sizeof(del_gateway));
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", del_gateway);
			wired_status = wisp4g_internet_do_ping(detectip, wired_device, del_gateway);
			if(wired_status == 0)
			{
				memset(del_gateway, 0, sizeof(del_gateway));
				huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "gateway", del_gateway);
				cpe_status = wisp4g_internet_do_ping(detectip, cpe_device, del_gateway);
				if(cpe_status == 0)//使用wisp上网方式
				{
					sprintf(section,"wisp");
				}else//使用4G上网方式
				{
					sprintf(section,"cpe");
				}
			}
			else
			{
				get_maxdelay(detectip, wired_device);
				if(delaycount >= 5)
				{
					//使用4G上网方式
					sprintf(section,"cpe");
				}
				else  //使用wisp上网方式
				{
					sprintf(section,"wisp");
				}
				delaycount = 1;
			}
		}
		else if(cpe_link == 1)//使用4G上网方式
		{	
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "ipv4", wan4g_ipv4);
			if(strncmp(wan4g_ipv4, "0.0.0.0", strlen("0.0.0.0")) == 0)
				continue;
			sprintf(section,"cpe");
		}
		else//使用wisp上网方式
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "ipv4", wisp_ipv4);
			if(strncmp(wisp_ipv4, "0.0.0.0", strlen("0.0.0.0")) == 0)
				continue;
			sprintf(section,"wisp");
		}

		memset(cmd_str,0,sizeof(cmd_str));
		memset(del_gateway,0,sizeof(del_gateway));
		if(!strncmp(section,"wisp",strlen("wisp")))//使用wisp上网方式，删除4G默认路由
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", del_gateway);
		}
		else//4G上网方式，删除有线默认路由
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "gateway", del_gateway);
		}
		//printf("section = %s\n",section);
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
		}
		else
		{
			/*判断当前使用的默认路由是否为0.0.0.0，如果为0.0.0.0，则删除*/
			memset(cmd_str,0,sizeof(cmd_str));
			memset(resultbuf, 0, sizeof(resultbuf));
			strcpy(cmd_str,"route | grep \"default\" | awk {'print $2'}");
			get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));
			if(strstr(resultbuf, "0.0.0.0"))
			{
				system("route del default");
				sleep(1);
				system("route del default");
			}
		}

		memset(cmd_str,0,sizeof(cmd_str));
		sprintf(cmd_str,"route -n | grep %s | grep UG | grep %s |wc -l",wan_ifname,wan_gateway);
		get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));

		if(atoi(resultbuf) == 0 && strncmp(wan_gateway,"0.0.0.0",strlen("0.0.0.0")))
		{
			memset(cmd_str,0,sizeof(cmd_str));
			sprintf(cmd_str,"route add default gw %s dev %s",wan_gateway,wan_ifname);
			system(cmd_str);
			//printf("add route cmd_str = %s\n",cmd_str);
		}

	//	huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", net_ipv4);
		//printf("net_ipv4 = %s, wan_ipv4 = %s\n",net_ipv4, wan_ipv4);
		/*add update  status_network info，解决上网方式错乱的问题，实时更新上网方式接口*/
		//if(strcmp(wan_ipv4,net_ipv4))  //更新proc下的网络状态信息
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
		}
	}

	return 0;

}
