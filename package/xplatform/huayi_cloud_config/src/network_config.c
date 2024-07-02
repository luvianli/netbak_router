/*******************************************************************************************
 * Project:  huayi_clound_config                                                           *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20210114 add by rex  Version 0.0 													   *
 ******************************************************************************************/

#include "huayi_cloud_config.h"
#include <signal.h>

#define HUAYI_SPACE_DELIMIT		' '


void huayi_get_network_config(char *gatewayip)
{
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",gatewayip);
		
	return;
}

void huayi_get_wan_confign(char *gatewayip,int *status,char *wanip,char *netmask,char *nettype,long *up,long *down)
{
	char ifname[IPV4_STR_LEN] = {0};
	int rx_bps = 0,tx_bps = 0;
	int wan_up = 0;
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up);
	*status = wan_up;
	if(wan_up == 1)
	{
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",gatewayip);
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",wanip);
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","netmask",netmask);
		huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","tx_bps",&tx_bps);
		*up = ((long)tx_bps * 8);
		huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","rx_bps",&rx_bps);
		*down = ((long)rx_bps * 8);
	}else
	{
		strcpy(gatewayip,"0.0.0.0");
		strcpy(wanip,"0.0.0.0");
		strcpy(netmask,"0.0.0.0");
		*up = 0;
		*down = 0;
	}
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ifname", ifname);
	if(!strcmp(ifname, "apcli0")){
		strcpy(nettype,"Wisp");
	}else if(!strcmp(ifname, "eth1") || !strcmp(ifname, "wwan0")){
		strcpy(nettype,"4G");
	}else if(!strcmp(ifname, "eth0.2")){
		strcpy(nettype,"Wired");;
	}else{
		strcpy(nettype,"4G");
	}
	return;
}

void huayi_get_lan_confign(char *ip,char *netmask,int *dhcp,char *startip,char *endip,int *time)
{
	int start = 0,limit = 0,ipp = 0,net = 0;
	int min = 0,max = 0,status = 0,leasetime = 0;
	huayi_uci_get_int(PKG_DHCP,"lan","ignore",&status);
	huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ip);
	huayi_uci_get_str(PKG_NETWORK,"lan","netmask",netmask);
	huayi_uci_get_int(PKG_DHCP,"lan","start",&start);
	huayi_uci_get_int(PKG_DHCP,"lan","limit",&limit);
	huayi_uci_get_int(PKG_DHCP,"lan","leasetime",&leasetime);
	*dhcp = status;
	*time = leasetime;

	inet_pton(AF_INET,ip, &ipp);
	inet_pton(AF_INET,netmask, &net);
	ipp = htonl(ipp);
	net = htonl(net);

	min = htonl((ipp & net) +1);
	max = htonl((ipp | ~net) -1);

	start = htonl(ntohl(min)+start-1);
	limit = htonl(htonl(start)+limit - 1);

	inet_ntop(AF_INET, &start, startip, IPV4_STR_LEN);
	inet_ntop(AF_INET, &limit, endip, IPV4_STR_LEN);

	return;
}

void huayi_set_lan_config(cJSON *response_root)
{
	char ipaddr[IPV4_STR_LEN] = {0},oldipaddr[IPV4_STR_LEN] = {0};

	if(NULL == response_root) 
	{
		return ;
	}

	huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",oldipaddr);

	if(huayi_json_get_str(response_root,"lan_ip",ipaddr,OPTION_STR_LEN)) 
	{
		if (is_ipv4_addr(ipaddr) == 0 && 0!=strcmp(oldipaddr,ipaddr)) {
			huayi_uci_set_str(PKG_NETWORK,"lan","ipaddr",ipaddr);
			huayi_uci_commit(PKG_NETWORK);
			system("huayi-reset network");
		} 
		else {
			CLOUD_CLIENT_LOG("ipaddr error\n");
		}
	}

	return;
}

void huayi_set_dhcp_config(cJSON *response_root)
{
	char ipaddr[IPV4_STR_LEN] = {0},netmask[IPV4_STR_LEN] = {0};
	int lanset_flag = 0,dhcpset_flag = 0;
	char startip[IPV4_STR_LEN] = {0},endip[IPV4_STR_LEN] = {0};
	int ignore	= -1,oldignore = -1,leasetime = 0,config_start = 0;
	int ip = 0,net = 0,min = 0,start = 0,end = 0,limit = 0;
	int oldstart = -1,oldlimit = -1,oldleasetime = -1;

	if(NULL == response_root) 
	{
		return ;
	}

	huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",ipaddr);
	huayi_uci_get_str(PKG_NETWORK,"lan","netmask",netmask);
	huayi_uci_get_int(PKG_DHCP,"lan","ignore",&oldignore);
	huayi_uci_get_int(PKG_DHCP,"lan","leasetime",&oldleasetime);

	if(huayi_json_get_int(response_root,"lan_dhcp",&ignore))
	{
		if((ignore != oldignore)&&((ignore == 0)||(ignore == 1))) 
		{
			huayi_uci_set_int(PKG_DHCP,"lan","ignore",ignore);
			dhcpset_flag = 1;
		}
	}
	
	if(huayi_json_get_str(response_root,"start_ip",startip,IPV4_STR_LEN)&&
		huayi_json_get_str(response_root,"end_ip",endip,IPV4_STR_LEN))
	{
		huayi_uci_get_int(PKG_DHCP,"lan","start",&oldstart);
		huayi_uci_get_int(PKG_DHCP,"lan","limit",&oldlimit);
		
		inet_pton(AF_INET,ipaddr, &ip);
		inet_pton(AF_INET,netmask, &net);
		inet_pton(AF_INET,startip, &start);
		inet_pton(AF_INET,endip, &end);

		ip = htonl(ip);
		net = htonl(net);
		start = htonl(start);
		end = htonl(end);
		min = htonl(ip & net);

		if((min != htonl(start & net)) || (min != htonl(end & net)))
			goto out;
		
		config_start = (start-ntohl(min));
		limit = (end-start) + 1;
		if(config_start != oldstart)
		{
			huayi_uci_set_int(PKG_DHCP,"lan","start",config_start);
			dhcpset_flag = 1;
		}
		if(limit != oldlimit)
		{
			huayi_uci_set_int(PKG_DHCP,"lan","limit",limit);
			dhcpset_flag = 1;
		}
	}
out:
	if(huayi_json_get_int(response_root,"lease_time",&leasetime))
	{
		if((leasetime >= 300 && leasetime <= 604800) && (leasetime != oldleasetime)) 
		{
			huayi_uci_set_int(PKG_DHCP,"lan","leasetime",leasetime);
			dhcpset_flag = 1;
		}
	}

	if(dhcpset_flag ==1)
	{
		huayi_uci_commit(PKG_DHCP);
		system("huayi-reset dhcps");
	}

	return;
}


void huayi_get_wan_or_wan4g_config(int *type,char *mode,char *access,char *name,int *dial_way
,char *ipaddr,char *netmask,char *gateway,char *dns1,char *dns2
,char *p_name,char *password,char *ac,char *service
,char *dial_apn,char *dial_name,char *dial_pass,char *dial_auth)
{
	char hostname[OPTION_STR_LEN] = {0},wan_type[OPTION_STR_LEN] = {0};
	int dial_mode = 0,opmode = 0;
	char dns[IPV4_STR_LEN] = {0},temp_dns[OPTION_STR_LEN] = {0};

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode);
	*type = opmode;
	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	strcpy(mode,wan_type);
	if (!strcmp(wan_type,"mobile"))
	{
		huayi_uci_get_str(PKG_NETWORK,"wan4g","proto",access);
		huayi_uci_get_str(PKG_NETWORK,"wan4g","hostname",hostname);
		if(strlen(hostname) <= 0)
		{
			huayi_uci_get_str(PKG_SYSTEM,"system","hostname",name);
		}else strcpy(name,hostname);
	}
	else
	{
		huayi_uci_get_str(PKG_NETWORK,"wan","proto",access);
		huayi_uci_get_str(PKG_NETWORK,"wan","hostname",hostname);
		if(strlen(hostname) <= 0)
		{
			huayi_uci_get_str(PKG_SYSTEM,"system","hostname",name);
		}else strcpy(name,hostname);
	}

	huayi_uci_get_int(PKG_MODEM,"status","mode",&dial_mode);
	*dial_way = dial_mode;

	huayi_uci_get_str(PKG_NETWORK,"wan","ipaddr",ipaddr);
	huayi_uci_get_str(PKG_NETWORK,"wan","netmask",netmask);
	huayi_uci_get_str(PKG_NETWORK,"wan","gateway",gateway);
	huayi_uci_get_str(PKG_NETWORK,"wan","dns",temp_dns);
	huayi_get_nth_value(0, temp_dns,HUAYI_SPACE_DELIMIT, dns, sizeof(dns));
	strcpy(dns1,dns);
	huayi_get_nth_value(1, temp_dns,HUAYI_SPACE_DELIMIT, dns, sizeof(dns));
	strcpy(dns2,dns);

	huayi_uci_get_str(PKG_NETWORK,"wan","username",p_name);
	huayi_uci_get_str(PKG_NETWORK,"wan","password",password);
	huayi_uci_get_str(PKG_NETWORK,"wan","ac",ac);
	huayi_uci_get_str(PKG_NETWORK,"wan","service",service);

	huayi_uci_get_str(PKG_MODEM,"status","validation",dial_auth);
	huayi_uci_get_str(PKG_MODEM,"status","apn",dial_apn);
	huayi_uci_get_str(PKG_MODEM,"status","user4g",dial_name);
	huayi_uci_get_str(PKG_MODEM,"status","pass4g",dial_pass);

	return ;
}

void huayi_set_mac(cJSON *response_root)
{
	char mac[RESULT_STR_LEN] = {0},oldmac[RESULT_STR_LEN] = {0};
	char cmd[256] = {0};

	if(NULL == response_root) 
	{
		return ;
	}

	if(huayi_json_get_str(response_root,"mac",mac,OPTION_STR_LEN)) 
	{		
		sprintf(cmd,"huayi_mac read wlan_2g |grep \"wlan_2g\" |awk -F' ' '{print$2}' |tr '[a-z]' '[A-Z]'");
		get_cmd_result(cmd, oldmac, sizeof(oldmac));
		CLOUD_CLIENT_LOG("change mac:%s\n",mac);
		if (0 != strcmp(mac, oldmac)) {
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd,"huayi_mac write %s",mac);
			system(cmd);
		}
	}

	return;
}

void huayi_set_wan_config(cJSON *response_root)
{
	char wan_mode[RESULT_STR_LEN] = {0},oldwan_mode[RESULT_STR_LEN] = {0};
	char lan_ifname[OPTION_STR_LEN] = {0},wan_ifname[OPTION_STR_LEN] = {0},pmodel[SHORT_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0},proto[OPTION_STR_LEN] = {0};;
	int  dial_way = -1,olddial_way = 0;
	char auth_type[OPTION_STR_LEN] = {0},apn_name[OPTION_STR_LEN] = {0},username[OPTION_STR_LEN] = {0},password[OPTION_STR_LEN] = {0};
	char hostname[OPTION_STR_LEN] = {0};
	int  opmode = 5,wanset = 0,dialset = 0;
	

	if(NULL == response_root) 
	{
		return ;
	}

	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",oldwan_mode);
	if(!huayi_json_get_str(response_root,"wan_mode",wan_mode,RESULT_STR_LEN)){
		goto set4g;
	}

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode);
	
	if(opmode == 3)
	{
		if(!strcmp(wan_mode, "rep4g")||!strcmp(wan_mode, "rep")||!strcmp(wan_mode, "mobile")){
			goto setwan ;
		}else {
			CLOUD_CLIENT_LOG("wan_mode error\n");
			goto set4g;
		}
	}else if (opmode == 0)
	{
		if(!strcmp(wan_mode, "prior")||!strcmp(wan_mode, "wired")||!strcmp(wan_mode, "mobile")){
			goto setwan ;
		}else{
			CLOUD_CLIENT_LOG("wan_mode error\n");
			goto set4g;
		}
	}else return ;


setwan:
	if((0 != strlen(wan_mode)) && (0 != strcmp(oldwan_mode, wan_mode))) 
	{
		huayi_uci_set_str(PKG_SYSTEM,"mode","wan_type",wan_mode);
	}

	if (!strcmp(wan_mode,"mobile"))
	{
		if(0 != strcmp(oldwan_mode, wan_mode))
		{
			huayi_uci_delete_option(PKG_NETWORK,"wan","ifname");
			huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_4g_ifname",lan_ifname);
			if(strlen(lan_ifname)>0)
			{
				huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
			}
			memset(cmd,0,sizeof(cmd));
			huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
			if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
				&& strncmp("EC200",pmodel, 5) != 0)
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device wwan0");
			}
			else
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device eth1");
			}
			system(cmd);
            #if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
				huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","1"); //off
				huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFNAME,"apcli_enable","1");
            #endif
			wanset = 1;
		}
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_mode,"prior"))
	{
		if(!huayi_json_get_str(response_root,"proto",proto,OPTION_STR_LEN))
		{
			huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
		}
		huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);
		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend wired_device %s",wan_ifname);
		system(cmd);
	
		if(!strcmp(proto,"static"))
		{
			if(!huayi_set_static_config(response_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return ;
			}
		}else if(!strcmp(proto,"pppoe"))
		{
			if(!huayi_set_pppoe_config(response_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return ;
			}
		}else if(!strcmp(proto,"dhcp"))
		{
			huayi_json_get_str(response_root,"host_name",hostname,OPTION_STR_LEN);
			if(strlen(hostname)>0)
			{
				huayi_uci_set_str(PKG_NETWORK, "wan", "hostname",hostname);
			}
			huayi_uci_delete_option(PKG_NETWORK, "wan", "dns");
		}else
		{
			CLOUD_CLIENT_LOG("proto error\n");
			goto set4g;
		}
	
		huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
		{
			memset(cmd,0,sizeof(cmd));
			huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
			if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
				&& strncmp("EC200",pmodel, 5) != 0)
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device wwan0");
			}
			else
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device eth1");
			}
			system(cmd);
		}
		wanset = 1;
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_mode,"rep4g"))
	{
		if(0 != strcmp(oldwan_mode, wan_mode))
		{
			huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);
			huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
			huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
			huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend wisp_device %s",wan_ifname);
			system(cmd);

			memset(cmd,0,sizeof(cmd));
			huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
			if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
					&& strncmp("EC200",pmodel, 5) != 0)
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device wwan0");
			}
			else
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device eth1");
			}
			system(cmd);

			huayi_uci_set_str(PKG_NETWORK,"wan","proto", "dhcp");
			huayi_uci_set_str(PKG_NETWORK,"wan4g","proto", "dhcp");
			wanset = 1;
		}
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else{
		if(0 != strcmp(oldwan_mode, wan_mode))
		{
			huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);
			huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
			huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
			huayi_uci_delete_option(PKG_NETWORK,"wan4g","ifname");
			huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
			system(cmd);
			huayi_uci_set_str(PKG_NETWORK,"wan","proto", "dhcp");
			wanset = 1;
		}
	}
#else
	else
	{
		if(!huayi_json_get_str(response_root,"proto",proto,OPTION_STR_LEN))
		{
			huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
		}
		huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);
		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend wired_device %s",wan_ifname);
		system(cmd);
			
		if(!strcmp(proto,"static"))
		{
			if(!huayi_set_static_config(response_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return ;
			}
		}else if(!strcmp(proto,"pppoe"))
		{
			if(!huayi_set_pppoe_config(response_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return ;
			}
		}else if(!strcmp(proto,"dhcp"))
		{
			huayi_json_get_str(response_root,"host_name",hostname,OPTION_STR_LEN);
			if(strlen(hostname)>0)
			{
				huayi_uci_set_str(PKG_NETWORK, "wan", "hostname",hostname);
			}
			huayi_uci_delete_option(PKG_NETWORK, "wan", "dns");
		}else
		{
			CLOUD_CLIENT_LOG("proto error\n");
			goto set4g;
		}
		huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
		wanset = 1;
	}
#endif

set4g:
	if(huayi_json_get_int(response_root,"dial_way",&dial_way))
	{
		huayi_uci_get_int(PKG_MODEM,"status","mode",&olddial_way);
		if((dial_way == 0 || dial_way == 1) && (dial_way != olddial_way)) {
			huayi_uci_set_int(PKG_MODEM,"status","mode",&dial_way);
			dialset = 1;
		}

		if(dial_way == 1)
		{
		huayi_json_get_str(response_root,"dial_auth",auth_type,SHORT_STR_LEN);
		huayi_json_get_str(response_root,"dial_apn",apn_name,OPTION_STR_LEN);
		huayi_json_get_str(response_root,"dial_name",username,OPTION_STR_LEN);
		huayi_json_get_str(response_root,"dial_pass",password,OPTION_STR_LEN);
		
		huayi_uci_set_str(PKG_MODEM,"status","validation",auth_type);
		huayi_uci_set_str(PKG_MODEM,"status","apn",apn_name);
		huayi_uci_set_str(PKG_MODEM,"status","user4g",username);
		huayi_uci_set_str(PKG_MODEM,"status","pass4g",password);
		dialset = 1;
		}
	}

	if(wanset == 1)
	{
		huayi_uci_commit(PKG_SYSTEM);
		huayi_uci_commit(PKG_NETWORK);
		huayi_uci_commit(PKG_WIRELESS);
		if (!strcmp(wan_mode, "mobile")||!strcmp(wan_mode, "prior")||
			!strcmp(wan_mode, "rep4g"))
		{
			system("huayi-reset lte4g parameter 0");
		}
	}

	if(dialset == 1)
		huayi_uci_commit(PKG_MODEM);
	
	if(wanset == 1 ||dialset == 1)
		system("huayi-reset network");

	return ;
}

int huayi_set_static_config(cJSON *response_root,char *iface)
{
	char ipaddr[IPV4_STR_LEN] = {0},netmask[IPV4_STR_LEN] = {0},gateway[IPV4_STR_LEN] = {0};
	char dns1[IPV4_STR_LEN] = {0},dns2[IPV4_STR_LEN] = {0};

	if(NULL==iface)
	{
		return 0;
	}

	if( !huayi_json_get_str(response_root,"sta_ip",ipaddr,IPV4_STR_LEN)||
		!huayi_json_get_str(response_root,"sta_network",netmask,IPV4_STR_LEN)||
		!huayi_json_get_str(response_root,"sta_gateway",gateway,IPV4_STR_LEN)||
		!huayi_json_get_str(response_root,"dns1",dns1,IPV4_STR_LEN))
	{
		return 0;
	}
	
	if(!is_ipv4_addr(ipaddr) == 0||!is_ipv4_addr(netmask) == 0||!is_ipv4_addr(gateway) == 0||
		!is_ipv4_addr(dns1) == 0)
	{
		CLOUD_CLIENT_LOG("ipaddr error\n");
		return 0;
	}
		
	huayi_uci_set_str(PKG_NETWORK,iface,"ipaddr",ipaddr);
	huayi_uci_set_str(PKG_NETWORK,iface,"netmask",netmask);
	huayi_uci_set_str(PKG_NETWORK,iface,"gateway",gateway);

	huayi_uci_delete_option(PKG_NETWORK,iface,"dns");
	huayi_uci_add_list(PKG_NETWORK,iface,"dns",dns1);
	if(huayi_json_get_str(response_root,"dns2",dns2,IPV4_STR_LEN))
	huayi_uci_add_list(PKG_NETWORK,iface,"dns",dns2);

	return 1;
}

int huayi_set_pppoe_config(cJSON *request_root,char *iface)
{
	char username[PASSWD_LONG_STR_LEN] = {0};
	char password[PASSWD_LONG_STR_LEN] = {0};
	char ac[OPTION_STR_LEN] = {0};
	char service[OPTION_STR_LEN] = {0};
	
	if(!huayi_json_get_str(request_root,"ppp_name",username,PASSWD_LONG_STR_LEN)||
		!huayi_json_get_str(request_root,"ppp_pass",password,PASSWD_LONG_STR_LEN)||
		!huayi_json_get_str(request_root,"ppp_ac",ac,OPTION_STR_LEN)||
		!huayi_json_get_str(request_root,"ppp_server",service,OPTION_STR_LEN))
	{
		return 0;
	}

	huayi_uci_set_str(PKG_NETWORK,iface,"username",username);
	huayi_uci_set_str(PKG_NETWORK,iface,"password",password);
	huayi_uci_set_str(PKG_NETWORK,iface,"ac",ac);
	huayi_uci_set_str(PKG_NETWORK,iface,"service",service);
	huayi_uci_delete_option(PKG_NETWORK,iface,"dns");
	huayi_uci_set_str(PKG_NETWORK,iface,"ipaddr", "");
	huayi_uci_set_str(PKG_NETWORK,iface,"netmask", "");
	huayi_uci_set_str(PKG_NETWORK,iface,"gateway", "");
	
	return 1;
}

void huayi_get_qos_config(long *speeddown, long *speedup)
{
	int down = 0,up = 0;
	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed",&up);
	huayi_uci_get_int(PKG_QOS,"qos","total_dwspeed",&down);
	*speeddown = (long)down;
	*speedup = (long)up;

	return;
}

int huayi_set_qos_config(cJSON *response_root)
{
	char vap_section[OPTION_STR_LEN] = {0}, radio_section[OPTION_STR_LEN] = {0};	
	char ssid[OPTION_STR_LEN] = {0}, key[TEMP_STR_LEN] = {0};
	int dwspeed = -1, upspeed = -1, enable = 0, upflag = 0, dwflag = 0;
	int olddwspeed = -1, oldupspeed = -1;


	if(NULL == response_root) 
	{
		return ;
	}
	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed", &oldupspeed);
	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed", &oldupspeed);
	
	if(!huayi_json_get_int(response_root,"speed_downlink", &dwspeed)) {
		printf("qos dwspeed param is not respon !\n");
		upflag = -1;
	} else {
		if ((-1 != dwspeed) && (olddwspeed != dwspeed)) {
			huayi_uci_set_int(PKG_QOS,"qos","total_dwspeed",dwspeed);
			upflag = 1;
		}
	}
		
	if(!huayi_json_get_int(response_root,"speed_uplink", &upspeed)) {
		printf("qos upspeed param is not respon !\n");
		dwflag = -1;
	} else {
		if ((-1 != upspeed) && (oldupspeed != upspeed)) {
			huayi_uci_set_int(PKG_QOS,"qos","total_upspeed",upspeed);
			dwflag = 1;
		}
	}
	
	//no config
	if((-1 == upflag) && (-1 == dwflag))
		return;
	//config not update
	if((0 == dwflag) && (0 == upflag))	
		return;
	
	huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&enable);
	if((0 == upspeed) && (0 == dwspeed)) {
		//can't speed limit
		if(1 == enable) {//on --.off
			huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 0);
		}
	} else {
		if(0 == enable) {  //off ---. on
			huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 1);
		}
	}
	//printf("%s, %d, dwspeed = %d, upspeed = %d !\n", __FUNCTION__, __LINE__, dwspeed, upspeed);
	huayi_uci_commit(PKG_QOS);	
	system("huayi-reset iplimit");

	return;
}

int is_ipv4_addr(char *ip)//检验地址是否合法
{
	int ip_num[4] = {-1, -1, -1, -1};
	char ip_s[4][4];
	int i = 0,count = 0;

	if (ip == NULL || ip[0] == '0' || ip[0] == '\0') {
		return -1;
	}

	for (i = 0, count = 0; i < strlen(ip); i++) {
		if ((ip[i] != '.') && (ip[i] < '0' || ip[i] > '9')) {
			return -1;
		}
		if (ip[i] == '.') {
			count++;
			if (count > 3) {
				return -1;
			}
		}
	}

	memset(ip_s, 0, sizeof(char[4]) * 4);

	sscanf(ip, "%[^.].%[^.].%[^.].%[^ ]", ip_s[0], ip_s[1], ip_s[2], ip_s[3]);
	sscanf(ip_s[0], "%d", &ip_num[0]);
	sscanf(ip_s[1], "%d", &ip_num[1]);
	sscanf(ip_s[2], "%d", &ip_num[2]);
	sscanf(ip_s[3], "%d", &ip_num[3]);

	for (i = 0; i < 4; i++) {
		if (strlen(ip_s[i]) == 0 || (ip_s[i][0] == '0' && ip_s[i][1] != '\0') || ip_num[i] < 0 || ip_num[i] > 255) {
			return -1;
		}
	}

	return 0;
}


