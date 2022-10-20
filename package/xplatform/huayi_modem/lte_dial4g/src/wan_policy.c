#include <stdio.h>
#include <uci.h>

#include "huayi_uci_fun.h"


#define DEFAULT_CHECK_DOMAIN  "114.114.114.114"
#define DNS_RESOLVE_CHECK_CMD "cat /tmp/resolv.conf.auto | grep wan4g"
int main(int argc, char *argv[])
{
	int strategy = WIRE_FRIST;
	int wan_flag = ONLY_WIRE;
	int metric = 1;
	int wan_fail_num = 0;
	int wan_phy_num = 4;
	int wan_link = 0;
	int wan4g_link = 0;
	int route_metric=1;
	unsigned long long ifupwan_num = 0;
	unsigned long long ifupwan4g_num = 0;
	char wanifname[RESULT_STR_LEN] = {0};
	char domainName[RESULT_STR_LEN] = {0};
	char strWanMode[RESULT_STR_LEN] = {0};
	char registStatus[RESULT_STR_LEN] = {0};
	char wanGw[RESULT_STR_LEN] = {0};
	char tmpBuff[TEMP_STR_LEN] = {0};
	char cmdBuff[TEMP_STR_LEN] = {0};
	
	while(1)
	{
		sleep(5);
		Uci_Get_Int(PKG_NETWORK_CONFIG,"wan", "metric",&metric);
		Uci_Get_Str(PKG_MODEM_CONFIG, "netcheck", "domain", domainName);
		if(strlen(domainName) == 0)
		{
			snprintf(domainName,RESULT_STR_LEN,DEFAULT_CHECK_DOMAIN);
		}
		Uci_Get_Int(PKG_SYSTEM_CONFIG,"opmode", "strategy",&strategy);
		Uci_Get_Str(PKG_NETWORK_CONFIG,"wan", "proto",strWanMode);
		if (strncmp(strWanMode, "pppoe", strlen("pppoe")) == 0)
		{
			snprintf(wanifname,RESULT_STR_LEN,WAN_PPPOE_IFNAME);	
		}
		else
		{
			snprintf(wanifname,RESULT_STR_LEN,WAN_IFNAME);
		}

		if (strategy == WIRE_FRIST)
		{
			snprintf(cmdBuff,sizeof(cmdBuff),"route -n | grep %s | grep UG | awk -F \" \" '{print $5}'",wanifname);
			route_metric = getCmdVal(cmdBuff);
			
			if (do_ping_detect(domainName, wanifname) == 1 && route_metric != 0)
			{
				wan_fail_num = 0;
				if (wan_flag == ONLY_4G || metric == 3)
				{	
					Uci_Set_Str(PKG_NETWORK_CONFIG,"wan","metric","1");
					Uci_Commit(PKG_NETWORK_CONFIG);
					
					system("ifup wan");
					
					wan_flag = ONLY_WIRE;
					continue;
				}	
			}
			else
			{
				if (wan_flag == ONLY_WIRE || metric == 1)
				{
					if (wan_fail_num > 0)
					{
						wan_fail_num = 0;
						Uci_Set_Str(PKG_NETWORK_CONFIG,"wan","metric","3");
						Uci_Commit(PKG_NETWORK_CONFIG);
						system("ifup wan4g");
						wan_flag = ONLY_4G;
						continue;
					}
				}
				wan_fail_num++;
			}
			
			if(wan_flag == ONLY_4G)
			{
				getCmdResult(DNS_RESOLVE_CHECK_CMD,tmpBuff,TEMP_STR_LEN);
				if(strstr(tmpBuff,"wan4g"))
				{
					memset(tmpBuff, 0 , TEMP_STR_LEN);
					ifupwan4g_num = 0;
					Uci_Get_Int(PKG_NETWORK_STATUS, "sys", "wan4g_link",&wan4g_link);
					if (wan4g_link == 1)
					{
						system("huayi-reset wan4gup");
					}
					else
					{
						system("huayi-reset network");
					}
					continue;
				}
				else 
				{
					ifupwan4g_num++;
					if ((ifupwan4g_num%60) == 10)
					{
						Uci_Get_Int(PKG_NETWORK_STATUS, "sys", "wan4g_link",&wan4g_link);
						if (wan4g_link  == 0)
						{
							system("huayi-reset network");
							continue;
						}
					}
				}
			}
			
			if (route_metric != metric)
			{
				if (metric == 1 && route_metric == 3)
				{
					system("ifup wan");
					wan_flag = ONLY_WIRE;
					continue;
				}
				else if (metric == 3 && route_metric == 1)
				{
					system("ifup wan4g");
					wan_flag = ONLY_4G;
					continue;
				}
			}
			
			if (route_metric == 0)
			{
				Uci_Get_Int(PKG_PRODUCT_CONFIG,"hardware","WanPhyNum",&wan_phy_num);
				wan_link = is_phyconnected(wan_phy_num);
				ifupwan_num++;
				
				if (wan_link == 1 && ((ifupwan_num%60) < 5))
				{
					Uci_Set_Str(PKG_NETWORK_CONFIG,"wan","metric","1");
					Uci_Commit(PKG_NETWORK_CONFIG);
					system("ifup wan");
					continue;
				}
			}
			else
			{
				ifupwan_num = 0;
			}
		}	
	}
	
	return 0;
}



