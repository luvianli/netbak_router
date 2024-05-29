#include <stdio.h>
#include <unistd.h>
#include <huayi_uci_fun.h>

typedef enum{
	INTERNET_UNDEFINE = 0,
	INTERNET_4G,
	INTERNET_WIRED,
	
}INTERNET_MODE;

void setNetworkConfiguration(const char *device, const char *ipv4, const char *netmask, const char *gateway, const char *dns) {
    char cmd_str[CMD_STR_LEN];
    snprintf(cmd_str, CMD_STR_LEN, "huayi_cmd set network wan_device %s", device);
    system(cmd_str);
    snprintf(cmd_str, CMD_STR_LEN, "huayi_cmd set network wan_ipv4 %s", ipv4);
    system(cmd_str);
    snprintf(cmd_str, CMD_STR_LEN, "huayi_cmd set network wan_netmask %s", netmask);
    system(cmd_str);
    snprintf(cmd_str, CMD_STR_LEN, "huayi_cmd set network wan_gateway %s", gateway);
    system(cmd_str);

    char temp_dns[TEMP_STR_LEN];
    int i = 0;
	CON_LOG ("dns :%s",dns);
    while (huayi_get_nth_value(i++, dns, " ", temp_dns, sizeof(temp_dns))) {
        snprintf(cmd_str, CMD_STR_LEN, "huayi_cmd set network wan_dns_server %s", temp_dns);
        system(cmd_str);
    }
}

void routeConfiguration(const char *del_gateway, const char *wan_ifname, const char *wan_gateway) {
    char cmd_str[CMD_STR_LEN];
    if (strncmp(del_gateway, "0.0.0.0", strlen("0.0.0.0"))) {
        snprintf(cmd_str, CMD_STR_LEN, "route del default gw %s", del_gateway);
        system(cmd_str);

        snprintf(cmd_str, CMD_STR_LEN, "route -n | grep %s | grep UG | grep %s |wc -l", wan_ifname, wan_gateway);
        char resultbuf[SHORT_STR_LEN];
        get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));

        if (atoi(resultbuf) == 0 && strncmp(wan_gateway, "0.0.0.0", strlen("0.0.0.0"))) {
            snprintf(cmd_str, CMD_STR_LEN, "route add default gw %s dev %s", wan_gateway, wan_ifname);
            system(cmd_str);
        }
    }
}


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
	char wired_gateway[IPV4_STR_LEN] = {0},cpe_gateway [IPV4_STR_LEN] = {0};
	int wired_status = 0;
	int cpe_status = 0;
	int wired_link = 0;
	int cpe_link = 0;
	int old_int_mode = INTERNET_UNDEFINE;
	int now_int_mode = INTERNET_UNDEFINE;
	int  i = 0;
	char detectip[OPTION_STR_LEN] = {0};
	
	while(1)
	{
		sleep(10);
		memset(wan_type,0,sizeof(wan_type));
		huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);

		if(strcmp(wan_type,"prior") != 0)
		{
			continue;
		}
		huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
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
        huayi_uci_get_str(PKG_STATUS_BLEND, "wired", "gateway", wired_gateway);
        huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", cpe_gateway);
        if(strlen(wired_gateway) <= strlen("0.0.0.0")){
            wired_link = 0;
        }
        if(strlen(cpe_gateway) <= strlen("0.0.0.0"))
		{
            cpe_link = 0;
        }

		if(wired_link == 1 && cpe_link == 1)
		{
			wired_status = internet_do_ping_detect(detectip, wired_device);
			if(wired_status == 0)
			{
				cpe_status = internet_do_ping_detect(detectip, cpe_device);
				if(cpe_status == 0)
				{
					sprintf(section,"wired");
					now_int_mode = INTERNET_WIRED;
					old_int_mode = INTERNET_UNDEFINE;
				}else
				{
					sprintf(section,"cpe");
					now_int_mode = INTERNET_4G;
				}
			}else
			{
				sprintf(section,"wired");
				now_int_mode = INTERNET_WIRED;
			}
		}
		else if(cpe_link == 1)
		{	
			sprintf(section,"cpe");
			now_int_mode = INTERNET_4G;
		}else
		{
			sprintf(section,"wired");
			now_int_mode = INTERNET_WIRED;
		}

		{
			memset(del_gateway,0,sizeof(del_gateway));
			if(!strncmp(section,"wired",strlen("wired")))
			{
				huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", del_gateway);
			}
			else
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

			routeConfiguration( del_gateway, wan_ifname, wan_gateway);

			huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", net_ipv4);
			if(strcmp(wan_ipv4,net_ipv4))
			{
				setNetworkConfiguration( wan_ifname, wan_ipv4, wan_netmask, wan_gateway, temp_dns);	

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

	return 0;

}
