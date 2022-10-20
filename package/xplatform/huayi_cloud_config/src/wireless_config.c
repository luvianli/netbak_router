/*******************************************************************************************
 * Project:  huayi_clound_config                                                           *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20210114 add by rex  Version 0.0 													   *
 ******************************************************************************************/

#include "huayi_cloud_config.h"
#include "oid.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <signal.h>


int ismac_in_acl(char *mac_address)
{
	char acl[LONG_BUFF_LEN] = {0};
	int acl_count = 0;
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"acl","acl_count",&acl_count);

	if(acl_count == 0) 
	{
		return 0;
	}

	huayi_uci_get_str(PKG_HUAYI_FIREWALL,"acl","acl",acl);
	
	if(strstr(acl,mac_address)) 
	{
		return 1;
	}
	
	return 0;
}


void get_hostname(char *host_name, char *macaddr)
{
	char macsuffix[TEMP_STR_LEN]={0}, buff[CMD_STR_LEN] = { 0 };
	char tmpbuf[TEMP_STR_LEN], ip[RESULT_STR_LEN],hostname[TEMP_STR_LEN], hw_address[RESULT_STR_LEN];
	FILE *fp = NULL;

	fp = fopen("/var/dhcp.leases", "r");
	if(!fp) {
        return;
    }
	while(fgets(buff, CMD_STR_LEN, fp)) 
	{
        sscanf(buff, "%s %s %s %s %s", tmpbuf, hw_address, ip, hostname, macsuffix);
        if(!strncmp(hw_address, macaddr,17)) 
		{
            strcpy(host_name, hostname);
            break;
        }
    }
	fclose(fp);
	if(strlen(host_name) == 0)
	{
		strcpy(host_name, "*");
	}
	return;
}


int checkip_in_dhcplease(char *arp_ip)
{
	char macsuffix[TEMP_STR_LEN]={0}, buff[CMD_STR_LEN] = { 0 };
	char tmpbuf[TEMP_STR_LEN], ip[RESULT_STR_LEN],hostname[TEMP_STR_LEN], hw_address[RESULT_STR_LEN];
	FILE *fp = NULL;
	
	fp = fopen("/var/dhcp.leases", "r");
	if(!fp) {
        return 0;
    }
	while(fgets(buff, CMD_STR_LEN, fp)) 
	{
        sscanf(buff, "%s %s %s %s %s", tmpbuf, hw_address, ip, hostname, macsuffix);
        if(!strncmp(ip, arp_ip,strlen(arp_ip))) 
		{
			fclose(fp);
            return 1;
        }
    }
	fclose(fp);
	return 0;
}

int get_warp(char * buf,char * vap_section)
{
	char vap_ifname[OPTION_STR_LEN] = {0};
	int i = 0, socket_id = 0;
	unsigned long *num = NULL;
    struct iwreq iwr;
	RT_802_11_7603_MAC_TABLE table_24G = {0};
	RT_802_11_7663_MAC_TABLE table_58G = {0};
	char macbuff[MAC_STR_LEN]={0};
	int disabled = 0;

	if(NULL == vap_section && NULL == buf)
	{
		return 0;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"ifname",vap_ifname))
	{
		return 0;
	}

	strncpy(iwr.ifr_name, vap_ifname, IFNAMSIZ);

	if (!strcmp(vap_section,"radio0_0"))
	{
		iwr.u.data.pointer = (caddr_t) &table_24G;
		num = &table_24G.Num;
		if(!huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&disabled))
		{
			return 0;
		}
	}
	else
	{
		iwr.u.data.pointer = (caddr_t) &table_58G;
		num = &table_58G.Num;
		if(!huayi_uci_get_int(PKG_WIRELESS,"radio1","disabled",&disabled))
		{
			return 0;
		}
	}

	if(disabled == 1)
	{
		return 0;
	}

	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_id < 0) 
	{
		return 0;
	}
	
	if (ioctl(socket_id, 0x8bef, &iwr) < 0) 
	{
		close(socket_id);
		return 0;
	}
	
	close(socket_id);

	for (i = 0;( i < (*num)) && (i < 32); i++) 
	{
		memset(macbuff, 0, sizeof(macbuff));

		if (!strcmp(vap_section,"radio0_0"))
		{
			sprintf(macbuff,"%02X:%02X:%02X:%02X:%02X:%02X ", 
					table_24G.Entry[i].Addr[0], table_24G.Entry[i].Addr[1],
					table_24G.Entry[i].Addr[2], table_24G.Entry[i].Addr[3],
					table_24G.Entry[i].Addr[4], table_24G.Entry[i].Addr[5]);
		}
		else
		{
			sprintf(macbuff,"%02X:%02X:%02X:%02X:%02X:%02X ", 
					table_58G.Entry[i].Addr[0], table_58G.Entry[i].Addr[1],
					table_58G.Entry[i].Addr[2], table_58G.Entry[i].Addr[3],
					table_58G.Entry[i].Addr[4], table_58G.Entry[i].Addr[5]);
		}

		strcat(buf,macbuff);
		
	}
	
	return 1;
	
}


void get_activearp(int type, cJSON *stalist_array, int *number)
{
	FILE *fp = NULL,*fp_w_arp = NULL;
	char buf[CMD_STR_LEN];
	char ip_entry[RESULT_STR_LEN] = { 0 }, hw_type[SMALL_STR_LEN] = { 0 },flags[SMALL_STR_LEN] = { 0 };
	char host_name[OPTION_STR_LEN] = { 0 }, hw_address[RESULT_STR_LEN] = { 0 };
	char mask[SMALL_STR_LEN] = {0},device[RESULT_STR_LEN] = {0};
	char arp_w_mac[LONG_LIST_BUFF_LEN] = {0};
	
	if((type == 0)&&( stalist_array == NULL))
	{
		return;
		/*get list*/
		
	}else if((type != 0)&&(stalist_array !=NULL))
	{
		*number = 0;
		return;
		/*get number*/
	}

	system("rm -rf /tmp/w_arp");
	get_warp(arp_w_mac,"radio0_0");
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	get_warp(arp_w_mac,"radio1_0");
#endif
	
	/*system("echo 0 > /proc/wl_stalist");
	system("iwpriv ra0 show stamac");
	system("cat /proc/wl_stalist > /tmp/w_arp");
	system("echo 0 > /proc/wl_stalist");
	system("iwpriv rai0 show stamac");
	system("cat /proc/wl_stalist >> /tmp/w_arp");*/

	fp_w_arp = fopen("/tmp/w_arp", "w+");
	if(!fp_w_arp)
	{
		return;
	}

	fwrite(arp_w_mac, strlen(arp_w_mac)+1, 1, fp_w_arp);
	fclose(fp_w_arp);
	
	fp = fopen("/proc/net/arp", "r");
	
	if(!fp)
	{
		return;
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
		if(strcmp(device,"br-lan"))
		{
			continue;
		}

		if(strcmp(flags, "0x0") == 0)
		{
			if(strstr(str2upper(arp_w_mac),str2upper(hw_address)) == NULL)
			{
				continue;
			}

			if(!checkip_in_dhcplease(ip_entry))
			{
				continue;
			}
			
		}
	
		if(type == 0)
		{
			memset(host_name, 0, sizeof(host_name));
			get_hostname(host_name, str2lower(hw_address));

			if(ismac_in_acl(hw_address))
			{
				continue;
			}
		}

		(*number)++;		
	}
	fclose(fp);
	
	system("huayi_cmd set system arp_flush 1");
	
	return;
}

void huayi_get_wireless_config(char *ssid, char *key, int *hidd, int *num)
{
	int hidden = 0;
	int client_num = 0 ;
	
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid",ssid);	
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key", key);
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","hidden",&hidden);
	*hidd = hidden;
	get_activearp(1, NULL ,&client_num);
	*num = client_num;
	
	return;
}

void huayi_set_wifi_config(cJSON *response_root)
{
	char vap_section[OPTION_STR_LEN] = {0}, radio_section[OPTION_STR_LEN] = {0};	
	char ssid[TEMP_STR_LEN] = {0}, key[TEMP_STR_LEN] = {0};
	char oldssid[OPTION_STR_LEN] = {0}, oldkey[TEMP_STR_LEN] = {0};
	int radio_id = 0, hidden = -1, oldhidden = -1;
	int wifireset_flag = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	char vap_section5g[OPTION_STR_LEN] = {0};
	char tmp[TEMP_STR_LEN] = { 0 };
#endif

	if(NULL == response_root) 
	{
		return ;
	}

	/*to do distinguish 2.4g or 5.8g*/
	radio_id = 0; //2.4g
	snprintf(vap_section, OPTION_STR_LEN,"radio%d_0",radio_id);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	snprintf(vap_section5g, OPTION_STR_LEN,"radio%d_0",1);
#endif
	huayi_uci_get_int(PKG_WIRELESS,vap_section,"hidden",&oldhidden);
	huayi_uci_get_str(PKG_WIRELESS,vap_section,"ssid", oldssid);
	huayi_uci_get_str(PKG_WIRELESS,vap_section,"key", oldkey);
	//printf("%s, %d, oldhidden = %d, oldssid = %s oldkey = %s!\n", __FUNCTION__, __LINE__, oldhidden, oldssid, oldkey);
	if(!huayi_json_get_int(response_root,"hidden",&hidden)) {
		printf("wireless hidden param is not respon !\n");
	} else {
		if((-1 != hidden) && (hidden != oldhidden)) {
			huayi_uci_set_int(PKG_WIRELESS,vap_section,"hidden",hidden);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			huayi_uci_set_int(PKG_WIRELESS,vap_section5g,"hidden",hidden);
#endif
			wifireset_flag = 1;
		}
	}
		
	if(!huayi_json_get_str(response_root,"ssid",ssid,OPTION_STR_LEN)) {
		printf("wireless ssid param is not respon !\n");
	} else {
		if((0 != strlen(ssid)) && (0 != strcmp(oldssid, ssid))) {
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"ssid",ssid);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			snprintf(tmp, TEMP_STR_LEN, "%s_5.8g", ssid);
			huayi_uci_set_int(PKG_WIRELESS,vap_section5g,"ssid",tmp);
#endif

			wifireset_flag = 1;
		}
	}
	
	if(!huayi_json_get_str(response_root,"password",key,TEMP_STR_LEN)) {
		printf("wireless password param is not respon !\n");;
	} else {
		if(0 != strlen(ssid) && (0 != strcmp(key, oldkey))) {
			huayi_uci_set_str(PKG_WIRELESS,vap_section,"key", key);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			huayi_uci_set_str(PKG_WIRELESS,vap_section5g,"key", key);
#endif

			wifireset_flag = 1;
		}
	}	
	
	//printf("%s, %d, hidden = %d, ssid = %s key = %s!\n", __FUNCTION__, __LINE__, hidden, ssid, key);
	if(1 == wifireset_flag ) {
		huayi_uci_commit(PKG_WIRELESS);	
		system("huayi-reset wireless");
	}
	
	return;
}

