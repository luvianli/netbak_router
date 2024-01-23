#include "fastcgi_common.h"
#include "oid.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>


FCGI_RESPONSE_ARRAY response_code_array[] = {
	{FCGI_RESP_CODE_SUCCESS,"success"},  													/*success*/
	{FCGI_RESP_CODE_UNAUTHORIZED,"unauthorized"}, 											/*login auth failed*/
	{FCGI_RESP_CODE_DATA_FORMAT_ERROR,"data format error"}, 								/*data is not json format*/
	{FCGI_RESP_CODE_HYID_MISS,"HYID field miss"}, 											/*no HYID in json */
	{FCGI_RESP_CODE_ILLEGAL_HYID,"illegal HYID"}, 											/*illegal HYID in json */
	{FCGI_RESP_CODE_DATA_MISS,"required data miss"},										/*required key miss in json*/
	{FCGI_RESP_CODE_PERMISSION_DENY,"permission deny"},										/*password check faild*/
	{FCGI_RESP_CODE_SERVER_FAILURE,"server failure"},										/*server failure*/
	{FCGI_RESP_CODE_FILE_FORMAT_ERROR,"upload file format error"},							/*file format error*/
	{FCGI_RESP_CODE_DULICATE_DATA,"dulicate data"},											/*dulicate data*/
	{FCGI_RESP_CODE_DATA_CONTENT_ERROR,"data content error"},								/*file format error*/
	{FCGI_RESP_CODE_LOGIN_LIMIT,"limit of login users"},									/*limit of login users*/
	{FCGI_RESP_CODE_PIN_CODE_ERROR,"pin code error"},										/*pin code error*/
};

void fast_cgi_response_web(cJSON *response_root,int rest_code,int lock_time) 
{
	int i = 0; 
	char *output = NULL;
	
	for(i = 0; i < ARRAY_SIZE(response_code_array); i++ )
	{
		if (rest_code == response_code_array[i].restcode)
		{
			cJSON_AddStringToObject(response_root, "huayierrtype", response_code_array[i].errmsg);
	
		}
	}

	cJSON_AddNumberToObject(response_root, "errcode", rest_code);
		
	cJSON_AddNumberToObject(response_root, "progresstime", lock_time);

	output = cJSON_Print(response_root);
	
	FCGI_printf("Content-Type: text/plain\n\n");
	
	FCGI_printf("%s",output);

	free(output);
	
	return;
}

FCGI_BOOL huayi_add_switch_to_json(cJSON *root,char *name,int switch_value)
{
	if((root==NULL)||(name == NULL))
	{
		return FCGI_FALSE;
	}

	if(switch_value == 0)
	{
		cJSON_AddFalseToObject(root,name);
	}else if(switch_value == 1)
	{
		cJSON_AddTrueToObject(root,name);
	}else
	{
		return FCGI_FALSE;
	}

	return FCGI_TRUE;
}

FCGI_BOOL huayi_add_file_to_json(cJSON *root,char *name,char *read_file)
{
	FILE *fp = NULL;
	char *output = NULL;
	int  file_size = 0;
	char tmp[CMD_STR_LEN] = {0};
	int p = 0;

	if((root==NULL)||(name == NULL)||(read_file == NULL))
	{
		return FCGI_FALSE;
	}

	fp=fopen(read_file,"r");
	if(!fp)
	{
		return FCGI_FALSE;
	}
	
	fseek(fp,0L,SEEK_END);
	
	file_size = ftell(fp) + 1;
	
	if(file_size > 1)
	{
		output = malloc(file_size);

		memset(output,0,file_size);
	
		fseek(fp,0L,SEEK_SET);

		memset(tmp, 0, CMD_STR_LEN);
	
		while(fgets(tmp,CMD_STR_LEN,fp) != NULL)
		{
        	p += snprintf(output + p, file_size-p, "%s",tmp);
			memset(tmp, 0, CMD_STR_LEN);
    	}

		if((strcmp(name,"syslog") == 0) && strstr(output,"Thu Jan  1 08:00:00 1970 kern.emerg kernel:"))
		{
			cJSON_AddStringToObject(root,name,"");
		}else
		{
			cJSON_AddStringToObject(root,name,output);
		}
		free(output);
	}else
	{
		cJSON_AddStringToObject(root,name,"error:content is null.");
	}
	
	fclose(fp);
	fp = NULL;
	
	return FCGI_TRUE;
}

FCGI_BOOL get_interface_list(cJSON *result_root)
{
	cJSON 	*interface_array = NULL;
	cJSON	*interface_obj = NULL;
	char	inter_face[OPTION_STR_LEN] = {0};
	
	if(result_root == NULL)
	{
		return FCGI_FALSE;
	}

	interface_array = cJSON_CreateArray();

	interface_obj = cJSON_CreateObject();

	cJSON_AddNumberToObject(interface_obj,"index",0);

	snprintf(inter_face,sizeof(inter_face),"lan");

	cJSON_AddStringToObject(interface_obj,"interface",inter_face);
	
	cJSON_AddItemToArray(interface_array,interface_obj);

	interface_obj = cJSON_CreateObject();

	cJSON_AddNumberToObject(interface_obj,"index",1);

	snprintf(inter_face,sizeof(inter_face),"wan");

	cJSON_AddStringToObject(interface_obj,"interface",inter_face);
	
	cJSON_AddItemToArray(interface_array,interface_obj);

	cJSON_AddItemToObject(result_root,"interface_arr",interface_array);

	return FCGI_TRUE;
}

FCGI_BOOL is_mac_in_acl(char *mac_address)
{
	char acl[LONG_BUFF_LEN] = {0};
	int acl_count = 0;
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"acl","acl_count",&acl_count);

	if(acl_count == 0) 
	{
		return FCGI_FALSE;
	}

	huayi_uci_get_str(PKG_HUAYI_FIREWALL,"acl","acl",acl);
	
	if(strstr(acl,mac_address)) 
	{
		return FCGI_TRUE;
	}
	
	return FCGI_FALSE;
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

int check_ip_in_dhcplease(char *arp_ip)
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

int check_ipmac_in_dhcplease(char *arp_ip, char *macaddr)
{
	int ret = FCGI_FREE;
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
		if(strncmp(macaddr, hw_address,strlen(macaddr)) == 0) 
		{
	        if(strncmp(ip, arp_ip,strlen(arp_ip))) 
			{
				fclose(fp);
	            return FCGI_CONFLICT;
	        }else
	       	{
	       		ret = FCGI_DHCP;
				break;
			}
		}
    }
	fclose(fp);
	return ret;
}


int get_w_arp(char * buf,char * vap_section)
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
		return FCGI_FALSE;
	}

	if(!huayi_uci_get_str(PKG_WIRELESS,vap_section,"ifname",vap_ifname))
	{
		return FCGI_FALSE;
	}

	strncpy(iwr.ifr_name, vap_ifname, IFNAMSIZ);

	if (!strcmp(vap_section,"radio0_0"))
	{
		iwr.u.data.pointer = (caddr_t) &table_24G;
		num = &table_24G.Num;
		if(!huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&disabled))
		{
			return FCGI_FALSE;
		}
	}
	else
	{
		iwr.u.data.pointer = (caddr_t) &table_58G;
		num = &table_58G.Num;
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
	
	return FCGI_TRUE;
	
}

FCGI_BOOL get_active_arp(int type,cJSON *stalist_array,int *number)
{
	cJSON *stalistentry = NULL;
	FILE *fp = NULL,*fp_w_arp = NULL;
	char buf[CMD_STR_LEN];
	char ip_entry[RESULT_STR_LEN] = { 0 }, hw_type[SMALL_STR_LEN] = { 0 },flags[SMALL_STR_LEN] = { 0 };
	char host_name[OPTION_STR_LEN] = { 0 }, hw_address[RESULT_STR_LEN] = { 0 };
	char mask[SMALL_STR_LEN] = {0},device[RESULT_STR_LEN] = {0};
	char arp_w_mac[LONG_LIST_BUFF_LEN] = {0};
	
	if((type == 0)&&( stalist_array == NULL))
	{
		return FCGI_FALSE;
		/*get list*/
		
	}else if((type != 0)&&(stalist_array !=NULL))
	{
		*number = 0;
		return FCGI_FALSE;
		/*get number*/
	}

	system("rm -rf /tmp/w_arp");
	get_w_arp(arp_w_mac,"radio0_0");
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	get_w_arp(arp_w_mac,"radio1_0");
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
		return FCGI_FALSE;
	}

	fwrite(arp_w_mac, strlen(arp_w_mac)+1, 1, fp_w_arp);
	fclose(fp_w_arp);
	
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

			if(!check_ip_in_dhcplease(ip_entry))
			{
				continue;
			}
			
		}
	
		if(type == 0)
		{
			memset(host_name, 0, sizeof(host_name));
			get_hostname(host_name, str2lower(hw_address));

			if(is_mac_in_acl(hw_address))
			{
				continue;
			}
			
			stalistentry = cJSON_CreateObject();
			cJSON_AddStringToObject(stalistentry,"ip_addr",ip_entry);
			cJSON_AddStringToObject(stalistentry,"mac_addr",hw_address);
			cJSON_AddStringToObject(stalistentry,"host_name",host_name);
			cJSON_AddItemToArray(stalist_array,stalistentry);
		}

		(*number)++;		
	}
	fclose(fp);
	
	system("huayi_cmd set system arp_flush 1");
	
	return FCGI_TRUE;
}

int set_wireless_main_def(void)
{
	char ssid_def[OPTION_STR_LEN] = {0};

	huayi_uci_get_str(PKG_WIRELESS,W24G_IFACE,"def_ssid",ssid_def);

	huayi_uci_set_str(PKG_WIRELESS,W24G_DEV,"channel","auto");
	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"ssid",ssid_def);
	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"encryption","none");

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_str(PKG_WIRELESS,W58G_IFACE,"def_ssid",ssid_def);

	huayi_uci_set_str(PKG_WIRELESS,W58G_DEV,"channel","auto");
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"ssid",ssid_def);
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"encryption","none");
#endif
	return 0;
}

char *get_wireless_config_authmode(char *authmode)
{	
	static char tmp[OPTION_STR_LEN]={0};

	if (strncmp(authmode,"WPAPSK",strlen("WPAPSK ")) == 0)
	{
		sprintf(tmp,"%s","WPAPSK");
	}
	else if (strncmp(authmode,"WPA2PSK",strlen("WPA2PSK ")) == 0)
	{
		sprintf(tmp,"%s","WPA2PSK");
	}
	else if (strncmp(authmode,"WPA1PSKWPA2PSK",strlen("WPA1PSKWPA2PSK ")) == 0 || strncmp(authmode,"WPAPSKWPA2PSK",strlen("WPAPSKWPA2PSK ")) == 0)
	{
		sprintf(tmp,"%s","WPA2PSK");
	}
	else
	{
		sprintf(tmp,"%s","OPEN");
	}
	return tmp;
}

char *get_wireless_config_encryptype(char *encryptype)
{	
	static char tmp[OPTION_STR_LEN]={0};

	if (strncmp(encryptype,"TKIP",strlen("TKIP ")) == 0)
	{
		sprintf(tmp,"%s","TKIP");
	}
	else if (strncmp(encryptype,"AES",strlen("AES ")) == 0)
	{
		sprintf(tmp,"%s","AES");
	}
	else if (strncmp(encryptype,"TKIPAES",strlen("TKIPAES ")) == 0)
	{
		sprintf(tmp,"%s","AES");
	}
	else
	{
		sprintf(tmp,"%s","NONE");
	}
	return tmp;
}

char *get_wireless_config_encryption(char *authmode,char *encryptype)
{	
	static char encryption[OPTION_STR_LEN]={0};

	if (strncmp(authmode,"WPA2PSK",strlen("WPA2PSK ")) == 0)
	{
		if (strncmp(encryptype,"TKIPAES",strlen("TKIPAES "))==0 || strncmp(encryptype,"AESTKIP",strlen("AESTKIP "))==0)
		{
			sprintf(encryption,"%s","psk2+tkip+ccmp");
		}
		else if (strncmp(encryptype,"TKIP",strlen("TKIP "))==0)
		{
			sprintf(encryption,"%s","psk2+tkip");
		}
		else if (strncmp(encryptype,"AES",strlen("AES "))==0)
		{
			sprintf(encryption,"%s","psk2+ccmp");
		}
		else
		{
			sprintf(encryption,"%s","none");
		}
	}
	else if (strncmp(authmode,"WPAPSK",strlen("WPAPSK ")) == 0)
	{
		if (strncmp(encryptype,"TKIPAES",strlen("TKIPAES "))==0 || strncmp(encryptype,"AESTKIP",strlen("AESTKIP "))==0)
		{
			sprintf(encryption,"%s","psk+tkip+ccmp");
		}
		else if (strncmp(encryptype,"TKIP",strlen("TKIP "))==0)
		{
			sprintf(encryption,"%s","psk+tkip");
		}
		else if (strncmp(encryptype,"AES",strlen("AES "))==0)
		{
			sprintf(encryption,"%s","psk+ccmp");
		}
		else
		{
			sprintf(encryption,"%s","none");
		}
	}
	else if (strncmp(authmode,"WPA1PSKWPA2PSK",strlen("WPA1PSKWPA2PSK ")) == 0)
	{
		if (strncmp(encryptype,"TKIPAES",strlen("TKIPAES "))==0  || strncmp(encryptype,"AESTKIP",strlen("AESTKIP "))==0)
		{
			sprintf(encryption,"%s","mixed-psk+tkip+ccmp");
		}
		else if (strncmp(encryptype,"TKIP",strlen("TKIP "))==0)
		{
			sprintf(encryption,"%s","mixed-psk+tkip");
		}
		else if (strncmp(encryptype,"AES",strlen("AES "))==0)
		{
			sprintf(encryption,"%s","mixed-psk+ccmp");
		}
		else
		{
			sprintf(encryption,"%s","none");
		}
	}
	else
	{
		sprintf(encryption,"%s","none");
	}
	return encryption;
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

FCGI_BOOL set_interface_static_config(cJSON *request_root,char *iface)
{
	char ipaddr[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char gateway[IPV4_STR_LEN] = {0};
	cJSON *dns_array = NULL;
	cJSON *dns_obj = NULL;
	int i = 0;
	int dns_count = 0;

	if(NULL==iface)
	{
		return FCGI_FALSE;
	}

	if( !huayi_json_get_str(request_root,"ip_addr",ipaddr,IPV4_STR_LEN)||
		!huayi_json_get_str(request_root,"subnet_mask",netmask,IPV4_STR_LEN)||
		!huayi_json_get_str(request_root,"gateway",gateway,IPV4_STR_LEN))
	{
		return FCGI_FALSE;
	}
	
	dns_array = cJSON_GetObjectItem(request_root, "dns_arr");

	if(dns_array == NULL)
	{
		return FCGI_FALSE;
	}
	
	huayi_uci_set_str(PKG_NETWORK,iface,"ipaddr",ipaddr);
	huayi_uci_set_str(PKG_NETWORK,iface,"netmask",netmask);
	huayi_uci_set_str(PKG_NETWORK,iface,"gateway",gateway);

	dns_count = cJSON_GetArraySize(dns_array);

	huayi_uci_delete_option(PKG_NETWORK,iface,"dns");

	for(i=0;i<dns_count;i++)
	{
		dns_obj = cJSON_GetArrayItem(dns_array,i);
		huayi_uci_add_list(PKG_NETWORK,iface,"dns",dns_obj->valuestring);
	}

	return FCGI_TRUE;
}

FCGI_BOOL set_interface_dhcp_config(cJSON *request_root,char *iface)
{
	char hostname[OPTION_STR_LEN] = {0};	
	
	huayi_json_get_str(request_root,"host_name",hostname,OPTION_STR_LEN);

	if(strlen(hostname)>0)
	{
		huayi_uci_set_str(PKG_NETWORK,iface,"hostname",hostname);
	}
	
	huayi_uci_delete_option(PKG_NETWORK,iface,"dns");
	huayi_uci_set_str(PKG_NETWORK,iface,"ipaddr", " ");
	huayi_uci_set_str(PKG_NETWORK,iface,"netmask", " ");
	huayi_uci_set_str(PKG_NETWORK,iface,"gateway"," ");
	
	return FCGI_TRUE;
}

FCGI_BOOL set_interface_pppoe_config(cJSON *request_root,char *iface)
{
	char username[PASSWD_LONG_STR_LEN] = {0};
	char password[PASSWD_LONG_STR_LEN] = {0};
	char ac[OPTION_STR_LEN] = {0};
	char service[OPTION_STR_LEN] = {0};
	
	if(!huayi_json_get_str(request_root,"usr",username,PASSWD_LONG_STR_LEN)||
		!huayi_json_get_str(request_root,"pwd",password,PASSWD_LONG_STR_LEN)||
		!huayi_json_get_str(request_root,"ac",ac,OPTION_STR_LEN)||
		!huayi_json_get_str(request_root,"service",service,OPTION_STR_LEN))
	{
		return FCGI_FALSE;
	}

	huayi_uci_set_str(PKG_NETWORK,iface,"username",username);
	huayi_uci_set_str(PKG_NETWORK,iface,"password",password);
	huayi_uci_set_str(PKG_NETWORK,iface,"ac",ac);
	huayi_uci_set_str(PKG_NETWORK,iface,"service",service);
	huayi_uci_delete_option(PKG_NETWORK,iface,"dns");
	huayi_uci_set_str(PKG_NETWORK,iface,"ipaddr", "");
	huayi_uci_set_str(PKG_NETWORK,iface,"netmask", "");
	huayi_uci_set_str(PKG_NETWORK,iface,"gateway", "");
	
	return FCGI_TRUE;
}

int huayi_generate_randomname(void)
{
	int randomname = 0;
	char http_pass[OPTION_STR_LEN] = {0};

	huayi_uci_get_str(PKG_SYSTEM,"login","http_pass",http_pass);
	

	if(strlen(http_pass)>0)
	{		
		randomname = random()%(8192-HUAYI_PRDY_MIDDLE_NUMBER) + HUAYI_PRDY_MIDDLE_NUMBER+1;
	}else
	{
		randomname = random()%HUAYI_PRDY_MIDDLE_NUMBER;
	}
	return randomname;
}

int get_eth_info(struct eth_speed_info *eth, int addr, int offset)
{
	char run_cmd[CMD_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};
	char *ptr = NULL;
	int value = 0;
	int speed_value = 0;
	int eth_addr = 0;
	
	if(NULL == eth || 0 == addr)
	{
		return FCGI_FALSE;
	}

	eth_addr = addr + offset;
	snprintf(cmd, sizeof(cmd), "switch reg r 0x%X", eth_addr);
	get_cmd_result(cmd, run_cmd, sizeof(run_cmd));
	if((ptr=strstr(run_cmd, "value=")) != NULL)
	{
		sscanf(ptr+strlen("value="),"%x",&value);
		eth->link = value & 0x01;
		
		if(2 == (value & 0x02))
		{
			eth->duplex_mode = 1;
		}
		else
		{
			eth->duplex_mode = 0;
		}
		
		speed_value = value & 0x0c;
		if(8 == speed_value)
		{
			eth->speed = 1000;
		}else if(4 == speed_value)
		{
			eth->speed = 100;
		}
		else
		{
			eth->speed = 10;
		}
		
	}else
	{
		return FCGI_FALSE;
	}

	return FCGI_TRUE;
}

FCGI_BOOL get_vpn_type_list(cJSON *result_root)
{
	cJSON * authenticate_array = NULL;
	cJSON * authenticate_obj = NULL;

#if defined(CONFIG_PACKAGE_HUAYI_PPPOE_SERVER)
		char vpntype_list[4][SMALL_STR_LEN] = {"all","pptp","l2tp","pppoe"};
#else
		char vpntype_list[3][SMALL_STR_LEN] = {"all","pptp","l2tp"};
#endif

	int id = 0;

	if(result_root == NULL)
	{
		return FCGI_FALSE;
	}

	authenticate_array = cJSON_CreateArray();
	
	for(id = 0; id < ARRAY_SIZE(vpntype_list); id++)
	{
		authenticate_obj= cJSON_CreateObject();
		cJSON_AddNumberToObject(authenticate_obj,"index",id+1);
		cJSON_AddStringToObject(authenticate_obj,"authenticate",vpntype_list[id]);
		cJSON_AddItemToArray(authenticate_array,authenticate_obj);
	}

	cJSON_AddItemToObject(result_root,"vpn_type_arr",authenticate_array);

	return FCGI_TRUE;
}

void get_maxdelay(char *checksignalips,char *lte_value)
{
	char cmd[TEMP_STR_LEN] = {0};
	char value[OPTION_STR_LEN] = { 0 };
	
	sprintf(cmd, "ping -c 1 -W 1 %s | grep \"time=\" | awk -F \"=\" '{print $NF}' > /tmp/pingtmp.txt", checksignalips);
	//printf("rex =========  cmd = %s\n", cmd);
	system(cmd);
	memset(cmd, 0, TEMP_STR_LEN);
	memset(value, 0, sizeof(value));
	strcpy(cmd, "cat /tmp/pingtmp.txt | awk -F \" \" '{print $1}'");
	get_cmd_result(cmd, value, sizeof(value));
	if(!strstr(value, "ping: unknown host") || !strstr(value, "ping: bad address") ||
		!strstr(value, "Network is unreachable"))
	{
		
		sprintf(lte_value,"%s",value);
	}
	else
	{
		
		sprintf(lte_value,"%s","0");
	}

	return;
}

void get_relay_wf(char *lte_vaule)
{
	char cmd[OPTION_STR_LEN]={0},value[OPTION_STR_LEN]={0};
	//iwpriv apcli0 stat | grep RSSI | awk '{print $3}'
	sprintf(cmd,"iwpriv apcli0 stat | grep RSSI | awk '{print $3}'");
	system(cmd);
	get_cmd_result(cmd,value,sizeof(value));
	sprintf(lte_vaule,"%s",value);

	return;
}


#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
/**
 * If head file  is not existed in your system, you could get information of IPv6 address
 * in file /proc/net/if_inet6.
 *
 * Contents of file "/proc/net/if_inet6" like as below:
 * 00000000000000000000000000000001 01 80 10 80       lo
 * fe8000000000000032469afffe08aa0f 08 40 20 80     ath0
 * fe8000000000000032469afffe08aa0f 07 40 20 80    wifi0
 * fe8000000000000032469afffe08aa0f 05 40 20 80     eth1
 * fe8000000000000032469afffe08aa0f 03 40 20 80      br0
 * fe8000000000000032469afffe08aa10 04 40 20 80     eth0
 *
 * +------------------------------+ ++ ++ ++ ++    +---+
 * |                                |  |  |  |     |
 * 1                                2  3  4  5     6
 *
 * There are 6 row items parameters:
 * 1 => IPv6 address without ':'
 * 2 => Interface index
 * 3 => Length of prefix
 * 4 => Scope value (see kernel source "include/net/ipv6.h" and "net/ipv6/addrconf.c")
 * 5 => Interface flags (see kernel source "include/linux/rtnetlink.h" and "net/ipv6/addrconf.c" "linux/include/if_addr.h")
 * 6 => Device name
 *
 * Note that all values of row 1~5 are hexadecimal string
 */
int get_ipv6addr(char *addr6, char *iface, int addr_type)
{
#define IF_INET6 "/proc/net/if_inet6"
    char str[128], address[64];
    char *addr,  *scope,  *name;
    char *delim = " \t\n", *p, *q;
    FILE *fp;
    int count;
    
    if (!addr6 || !iface) {
        printf("addr6 and iface can't be NULL!\n");
        return 0;
    }
    
    if (NULL == (fp = fopen(IF_INET6, "r"))) {
        perror("fopen error");
        return 0;
    }
    
    while (fgets(str, sizeof(str), fp)) {
        //DG("str:%s", str);
        addr = strtok(str, delim);
        strtok(NULL, delim);
        strtok(NULL, delim);
        scope = strtok(NULL, delim);
        strtok(NULL, delim);
        name = strtok(NULL, delim);

        if (strcmp(name, iface))
            continue;
        
        /* Just get IPv6 linklocal address */
        if (addr_type != (unsigned int)strtoul(scope, NULL, 16))
            continue;
        
        memset(address, 0x00, sizeof(address));
        p = addr;
        q = address;
        count = 0;
        while (*p != '\0') {
            if (count == 4) {
                *q++ = ':';
                count = 0;
            }
            *q++ = *p++;
            count++;
        }
		strcpy(addr6,address);
		fclose(fp);
		return 1;
    }
    
    fclose(fp);
    return 0;
}
int ipv6_reduce(char *str)
{
    int i,j;
	char *s=NULL, *e=NULL;
    //qiandao0
    for(i=0,j=0; str[j]!='\0'; i++,j++) {
        if(str[j]=='0'
                &&(j==0||str[i-1]==':')) {// the first 0
            if(str[j+1]!='0') {// 0XXX
                j++;
            } else if(str[j+2]!='0') { //00XX
                j=j+2;
            } else { //000X 0000
                j=j+3;
            }
        }
        str[i]=str[j];
    }
    str[i]='\0';//≤π…œΩ·Œ≤
    
	s = strstr(str,":0:0");
	if(s){
		*++s = ':';// ::
		e = s+4;
		while(*e=='0'||*e==':'){
			e++;
		}
		while(*e != '\0'){
			*++s = *e++;
		}
		*++s = '\0';
	}
	
    return 0;
}

int is_validipv6(const char *hostname)
{
	struct sockaddr_in6 addr;
	char *p=NULL, tmpStr[64]={0};
	int tmpInt=0,len=strlen(hostname);
	
	if (!hostname) 
		return -0;
	if (strlen(hostname)==0) 
		return -0;
	p = strchr(hostname,'/');
	if(p){
		tmpInt = atoi(p+1);
		if(tmpInt<1 || tmpInt>128)
			return 0;
		else
			strncpy(tmpStr,hostname,len-(hostname+len-p));
	}
	else
		strcpy(tmpStr,hostname);
	if (inet_pton(AF_INET6, tmpStr, &addr.sin6_addr) != 1) 
		return 0;
    	
	return 1;
}
int disable_ip6()
{
	huayi_uci_delete_option(PKG_NETWORK,"wan6","ifname");
	huayi_uci_delete_option(PKG_NETWORK,"wan6","proto");
	system("/etc/init.d/odhcpd disable");
	system("/etc/init.d/odhcpd stop");
	return 0;
}
int enable_ip6()
{
	system("/etc/init.d/odhcpd enable");
	system("/etc/init.d/odhcpd start");
	return 0;
}
#endif
