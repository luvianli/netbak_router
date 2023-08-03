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
	char wired_gateway[IPV4_STR_LEN] = {0},cpe_gateway [IPV4_STR_LEN] = {0};

	int wired_status = 0;
	int cpe_status = 0;
	int wired_link = 0;
	int cpe_link = 0;
	int old_int_mode = INTERNET_UNDEFINE;
	int now_int_mode = INTERNET_UNDEFINE;
	int  i = 0;
	char detectip[OPTION_STR_LEN] = {0};
	int count = 1;

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
        if(strstr(wired_device,"eth0.2") == NULL || strlen(wired_gateway) <= strlen("0.0.0.0")){
            wired_link = 0;
        }
        if(strstr(cpe_device,"eth1") == NULL || strlen(cpe_gateway) <= strlen("0.0.0.0"))
		{
            cpe_link = 0;
        }
		if(wired_link == 1 && cpe_link == 1)//Í¬Ê±ÔÚÏßÊ±£¬Ê¹ÓÃÓÐÏßÓÅÏÈ²ßÂÔ
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", del_gateway);
			wired_status = first_internet_do_ping(detectip, wired_device,del_gateway);
			if(wired_status == 0)
			{
				count = (10 * 60);
                DETECTION_NETWORK_LOG("wired internet error");
                huayi_uci_get_str(PKG_STATUS_BLEND, "wired", "gateway", del_gateway);
				cpe_status = first_internet_do_ping(detectip, cpe_device,del_gateway);
				if(cpe_status == 0)//Ê¹ÓÃÓÐÏßÉÏÍø·½Ê½
				{
					DETECTION_NETWORK_LOG("cpe internet error");
					sprintf(section,"wired");
					now_int_mode = INTERNET_WIRED;
					old_int_mode = INTERNET_UNDEFINE;
				}else//Ê¹ÓÃ4GÉÏÍø·½Ê½
				{
				 	DETECTION_NETWORK_LOG("cpe internet");
					sprintf(section,"cpe");
					now_int_mode = INTERNET_4G;
				}
			}else//Ê¹ÓÃÓÐÏßÉÏÍø·½Ê½
			{
				DETECTION_NETWORK_LOG("wired internet");
				sprintf(section,"wired");
				now_int_mode = INTERNET_WIRED;
			}

		}
		else if(cpe_link == 1)//Ê¹ÓÃ4GÉÏÍø·½Ê½
		{	
			sprintf(section,"cpe");
			now_int_mode = INTERNET_4G;
		}else//Ê¹ÓÃÓÐÏßÉÏÍø·½Ê½
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
			if(!strncmp(section,"wired",strlen("wired")))//ÓÐÏßÉÏÍø·½Ê½£¬É¾³ý4GÄ¬ÈÏÂ·ÓÉ
			{
				huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", del_gateway);
			}
			else//4GÉÏÍø·½Ê½£¬É¾³ýÓÐÏßÄ¬ÈÏÂ·ÓÉ
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
			if(strncmp(del_gateway,"0.0.0.0",strlen("0.0.0.0")))
			{
				sprintf(cmd_str,"route del default gw %s",del_gateway);
				DETECTION_NETWORK_LOG("%s",cmd_str);
				system(cmd_str);
				//printf("cmd_str = %s\n",cmd_str);
			}else{
    			/*ï¿½Ð¶Ïµï¿½Ç°Ê¹ï¿½Ãµï¿½Ä¬ï¿½ï¿½Â·ï¿½ï¿½ï¿½Ç·ï¿½Îª0.0.0.0ï¿½ï¿½ï¿½ï¿½ï¿½Îª0.0.0.0ï¿½ï¿½ï¿½ï¿½É¾ï¿½ï¿½*/
    			memset(cmd_str,0,sizeof(cmd_str));
    			memset(resultbuf, 0, sizeof(resultbuf));
    			strcpy(cmd_str,"route | grep \"default\" | awk {'print $2'}");
    			get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));
    			if(strstr(resultbuf, "0.0.0.0"))
    			{
    				system("route del default");
    				sleep(1);
    				system("route del default");
                    DETECTION_NETWORK_LOG("route del default");
    			}
        	}
			memset(cmd_str,0,sizeof(cmd_str));
			sprintf(cmd_str,"route -n | grep %s | grep UG | grep %s |wc -l",wan_ifname,wan_gateway);
			get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));

			if(atoi(resultbuf) == 0 && strncmp(wan_gateway,"0.0.0.0",strlen("0.0.0.0")))
			{
				memset(cmd_str,0,sizeof(cmd_str));
				sprintf(cmd_str,"route add default gw %s dev %s",wan_gateway,wan_ifname);
				DETECTION_NETWORK_LOG("%s",cmd_str);
				system(cmd_str);
			}

			huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", net_ipv4);
			if(strcmp(wan_ipv4,net_ipv4))
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
					DETECTION_NETWORK_LOG("ifup wan4g");
					sleep(count);
				}else if(now_int_mode == INTERNET_WIRED)
				{
					system("ifup wan");
					DETECTION_NETWORK_LOG("ifup wan");
				}
			}

			old_int_mode = now_int_mode;
		}
		
	}

	return 0;

}
