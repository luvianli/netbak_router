#include "huayi_mqtt.h"
#include "oid.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
//#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
//#include <sys/mman.h>
//#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <wait.h>
#include <stdio.h>
#include <string.h>

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

void get_current_time(char *tmpbuf)
{
	FILE *fp = NULL;
	char *p = NULL;
	char buf[128]={0};
	
	fp = popen("date +\"%Y-%m-%d %H:%M:%S\"", "r");
	if(fp == NULL) 
	{
		return;
	}
	
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		if((p=strstr(buf, "\n")))
		{
			p[0]='\0';
		}
	}
	
	pclose(fp);
	
	strcpy(tmpbuf,buf);
	
	return;
}

void get_pub_content(char *heartdata)
{
	cJSON *result_root = NULL;
	cJSON *arp_array = NULL,*arp_obj = NULL;
	int simNum = 0,hidden = 0,connCnt = 0,remainPwr = 0,rssi = 0,at_card = -1,i = 0;
	char imei[RESULT_STR_LEN]={0},sn[OPTION_STR_LEN] = {0},iccid[RESULT_STR_LEN]={0},simiccid[RESULT_STR_LEN]={0};
	char currentTime[OPTION_STR_LEN] = {0};
	char ssid[RESULT_STR_LEN] = {0},password[RESULT_STR_LEN] = {0};
	char version[SMALL_STR_LEN] = {0};
	int speedLimit = 0,heartbeat = 0;
	unsigned long long dayFlow = 0,monthFlow = 0;
	unsigned long long rx=0,tx=0;
	char cmd[SMALL_STR_LEN] = {0};
	int disconn = 0;
	char *output = NULL;
	int  signal = -121,down = 0,up = 0,status = 0;
	char mobile_type[SHORT_STR_LEN] = {0},band[TEMP_STR_LEN],gatewayip[RESULT_STR_LEN] = {0};
	char lac[OPTION_STR_LEN] = {0}, cellid[OPTION_STR_LEN] = {0};

#if 0
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", imei);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "iccid", iccid);
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simNum);
	get_current_time(currentTime);
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid",ssid);	
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key", password);
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","hidden",&hidden);
	get_activearp(1, NULL ,&connCnt);
	atcmd_get_signal_for_cloud(&rssi);
	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version", version);
	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed",&speedLimit);
	huayi_uci_get_long_long(PKG_SYSTEM, "network", "rx_bytes",&rx);
	huayi_uci_get_long_long(PKG_SYSTEM, "network", "tx_bytes",&tx);
	dayFlow = rx + tx;
	huayi_uci_get_long_long(PKG_SYSTEM, "network", "monthFlow",&monthFlow);
	huayi_uci_get_int(PKG_SYSTEM,"server","heartbeat", &heartbeat);
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &at_card);
	huayi_uci_get_int(PKG_SYSTEM,"network","disconn",&disconn);	
#else
	get_dev_sn(sn);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "iccid", iccid);
	atcmd_get_signal_for_cloud(&signal);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "operator", mobile_type);
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &at_card);
	get_cmd_result("echo $(huayi_AT_cli at+cced=0,1) | awk -F ',' '{print $6}'", band, sizeof(band));	
	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed",&up);
	huayi_uci_get_int(PKG_QOS,"qos","total_dwspeed",&down);
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid",ssid);	
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key", password);
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","hidden",&hidden);
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&status);
	get_activearp(1, NULL ,&connCnt);
    huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "lac", lac);
    huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "ci", cellid);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",gatewayip);
	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version", version);
#endif

	result_root = cJSON_CreateObject();

#if 0	
	cJSON_AddStringToObject(result_root,"imei", imei);
	cJSON_AddStringToObject(result_root,"iccid", iccid);
	cJSON_AddNumberToObject(result_root,"simNum", simNum);
	cJSON_AddStringToObject(result_root,"currentTime", currentTime);
	cJSON_AddStringToObject(result_root,"ssid", ssid);
	cJSON_AddStringToObject(result_root,"password", password);
	if(hidden == 0)
		cJSON_AddNumberToObject(result_root,"hidden", 1);
	else if(hidden == 1)
		cJSON_AddNumberToObject(result_root,"hidden", 0);
	cJSON_AddNumberToObject(result_root,"connCnt", connCnt);
	cJSON_AddNumberToObject(result_root,"rssi", rssi);
	cJSON_AddNumberToObject(result_root,"dayFlow", dayFlow);
	cJSON_AddNumberToObject(result_root,"monthFlow", monthFlow);
	cJSON_AddStringToObject(result_root,"version", version);
	cJSON_AddNumberToObject(result_root,"speedLimit", speedLimit);
	cJSON_AddNumberToObject(result_root,"heartbeat", heartbeat);
	cJSON_AddNumberToObject(result_root,"disconn", disconn);

	arp_array = cJSON_CreateArray();
	for(i;i<simNum-1;i++)
	{
		sprintf(cmd, "sim%d", i);
		huayi_uci_get_str(PKG_M_SYSTEM, cmd, "iccid", simiccid);
		arp_obj = cJSON_CreateObject();
		cJSON_AddStringToObject(arp_obj, "iccid", simiccid);
		if(i == at_card)
			cJSON_AddStringToObject(arp_obj, "isLine", "true");
		else
			cJSON_AddStringToObject(arp_obj, "isLine", "false");
		cJSON_AddItemToArray(arp_array,arp_obj);
	}
	cJSON_AddItemToObject(result_root,"simList",arp_array);
#else
	cJSON_AddStringToObject(result_root,"sn", sn);
	cJSON_AddStringToObject(result_root,"iccid", iccid);
	cJSON_AddNumberToObject(result_root,"dbm", signal);
	cJSON_AddStringToObject(result_root,"opname", mobile_type);
	cJSON_AddNumberToObject(result_root,"simSwitch", at_card);
	cJSON_AddStringToObject(result_root,"band", band);
	cJSON_AddNumberToObject(result_root,"speed_downlink", down);
	cJSON_AddNumberToObject(result_root,"speed_uplink", up);
	cJSON_AddStringToObject(result_root,"ssid", ssid);
	cJSON_AddStringToObject(result_root,"password", password);
	cJSON_AddNumberToObject(result_root,"hidden", hidden);
	cJSON_AddNumberToObject(result_root,"status", status);
	cJSON_AddNumberToObject(result_root,"count",connCnt);
	cJSON_AddStringToObject(result_root,"cell_id", cellid);
	cJSON_AddStringToObject(result_root,"lac", lac);
	cJSON_AddStringToObject(result_root,"ip", gatewayip);
	cJSON_AddStringToObject(result_root,"version", version);
#endif

	output = cJSON_Print(result_root);
	strcpy(heartdata, output);
	free(output);

	return ;
}

int main(int argc,char *argv[])
{
	char buff[1024] = {0};
	int heartbeat = 30;

	while(1)
	{
		memset(buff,0,sizeof(buff));
		huayi_uci_get_int(PKG_SYSTEM,"mqtt","interval", &heartbeat);
		get_pub_content(buff);
		FILE *file = fopen("/tmp/log/mqtt_pub_cont", "w");
    	if (file == NULL) {
        	MQTT_LOG("Error opening file");
			usleep(2*1000000);
        	continue;
    	} 
    	fprintf(file, "%s", buff);
    	fclose(file);
		usleep(heartbeat * 1000);
	}
	return 0;
}
