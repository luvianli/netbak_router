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


void huayi_get_network_config(char *gatewayip,char *mac_address,char *ip_address,char *sn,char *start,char *end)
{
	char cmd[128] = {0},ssn[24] = {0};
	char resultbuf[128] = {0};

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",gatewayip);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",ip_address);

	sprintf(cmd,"huayi_mac read wlan_2g |grep \"wlan_2g\" |awk -F' ' '{print$2}' |tr '[a-z]' '[A-Z]'");
	get_cmd_result(cmd, resultbuf, sizeof(resultbuf));

	sprintf(ssn,"%c%c%c%c%c%c%c%c%c%c%c%c",resultbuf[0],resultbuf[1],resultbuf[3],resultbuf[4],resultbuf[6],resultbuf[7],resultbuf[9],resultbuf[10],resultbuf[12],resultbuf[13],resultbuf[15],resultbuf[16]);

	strcpy(sn, ssn);
	strcpy(mac_address, resultbuf);

	huayi_uci_get_str(PKG_SYSTEM,"server","amound_start",start);
	huayi_uci_get_str(PKG_SYSTEM,"server","amound_end",end);

	return;
}

void huayi_get_vpn_config(char *account,char *key,int *status)
{
	int enable = 0;

	huayi_uci_get_int(PKG_PPL2TP,"xl2tp","enable",&enable);
	huayi_uci_get_str(PKG_PPL2TP,"xl2tp","username",account);
	huayi_uci_get_str(PKG_PPL2TP,"xl2tp","password",key);
	*status = enable;
	
	return;
}

void huayi_get_qos_config(int *speed)
{
	int qos,limit;
	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed",&qos);
	huayi_uci_get_int(PKG_SYSTEM,"server","limitspeed",&limit);
	if(limit == 0 || limit == 1)
		*speed = limit;
	else *speed = qos;
		
	return;
}

int huayi_set_amound_config(cJSON *response_root)
{
	char start[OPTION_STR_LEN] = {0},end[OPTION_STR_LEN] = {0}; 

	if(!huayi_json_get_str(response_root,"amound_end",end,OPTION_STR_LEN)){
		return ;
	}

	huayi_uci_get_str(PKG_SYSTEM,"server","amound_end",start);
	if((0 != strlen(end)) && (0 != strcmp(end, start)))
	{
		huayi_uci_set_str(PKG_SYSTEM,"server","amound_start",start);
		huayi_uci_set_str(PKG_SYSTEM,"server","amound_end",end);
		huayi_uci_commit(PKG_SYSTEM);
	}
	return ;
}

int huayi_set_qos_config(cJSON *response_root)
{
	int oldspeed = -1,enable = 0,limit = 0,oldlimit = 0;
	int flat = 0;
	
	huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&enable);

	if(!huayi_json_get_int(response_root,"limitspeed", &limit)) {
		return ;
	}

	huayi_uci_get_int(PKG_SYSTEM,"server","limitspeed",&oldlimit);
	if(limit != oldlimit)
	{
		huayi_uci_set_int(PKG_SYSTEM,"server","limitspeed",&limit);
		huayi_uci_commit(PKG_SYSTEM);
	}

	if(limit == 0 && enable != limit)
	{
		huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 0);
		flat = 1;
	}
	if(limit != 0)
	{
		if(limit == 1)
			limit = 0;
		huayi_uci_get_int(PKG_QOS,"qos","total_upspeed", &oldspeed);
		if(enable == 0 || oldspeed != limit)
		{
			huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 1);
			huayi_uci_set_int(PKG_QOS,"qos","total_upspeed", &limit);
			huayi_uci_set_int(PKG_QOS,"qos","total_upspeed", &limit);
			flat = 1;
		}
	}

	if(flat == 1)
	{
		huayi_uci_commit(PKG_QOS);
		system("huayi-reset iplimit");
	}
	return ;
}

int huayi_set_vpn_config(cJSON *response_root)
{
	int status,enable;
	char server_ip[LIST_STR_LEN ] = {0},oldserver_ip[LIST_STR_LEN ] = {0};
	char username[OPTION_STR_LEN] = {0},oldusername[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0},oldpassword[OPTION_STR_LEN] = {0};
	char ipsec_key[OPTION_STR_LEN] = {0},oldipsec_key[OPTION_STR_LEN] = {0};
	int flat = 0;

	if(!huayi_json_get_int(response_root,"vpn_status", &status))
	{
		return ;
	}	

	if(status == 0)
	{
		huayi_uci_get_int(PKG_PPL2TP,"xl2tp","enable",&enable);
		if(status != enable)
		{
			CLOUD_CLIENT_LOG("close VPN\n");
			huayi_uci_set_int(PKG_PPL2TP,"xl2tp","enable",status);
			flat = 1;
		}
	}else if(status == 1)
	{
		if(!huayi_json_get_str(response_root,"vpn_server",server_ip,OPTION_STR_LEN)||
			!huayi_json_get_str(response_root,"vpn_account",username,OPTION_STR_LEN)||
			!huayi_json_get_str(response_root,"vpn_key",password,OPTION_STR_LEN)||
			!huayi_json_get_str(response_root,"vpn_shard_key",ipsec_key,OPTION_STR_LEN)) 
		{
			return ;
		}
		huayi_uci_get_int(PKG_PPL2TP,"xl2tp","enable",&enable);
		huayi_uci_get_str(PKG_PPL2TP,"xl2tp","server_ip",oldserver_ip);
		huayi_uci_get_str(PKG_PPL2TP,"xl2tp","username",oldusername);
		huayi_uci_get_str(PKG_PPL2TP,"xl2tp","password",oldpassword);
		huayi_uci_get_str(PKG_PPL2TP,"xl2tp","ipsec_key",oldipsec_key);

		if(status != enable || 0 != strcmp(oldserver_ip,server_ip) || 0 != strcmp(oldusername,username)
			|| 0 != strcmp(oldpassword,password) || 0 != strcmp(oldipsec_key,ipsec_key))
		{
			CLOUD_CLIENT_LOG("open VPN\n");
			huayi_uci_set_int(PKG_PPL2TP,"xl2tp","enable",status);
			huayi_uci_set_str(PKG_PPL2TP,"xl2tp","server_ip",server_ip);
			huayi_uci_set_str(PKG_PPL2TP,"xl2tp","username",username);
			huayi_uci_set_str(PKG_PPL2TP,"xl2tp","password",password);
			huayi_uci_set_str(PKG_PPL2TP,"xl2tp","ipsec_key",ipsec_key);
			flat = 1;
		}
	}	

	if(flat == 1)
	{
		huayi_uci_commit(PKG_PPL2TP);
		system("huayi-reset xl2tpd_ipsec");
	}
	return ;

}

/*int huayi_set_qos_config(cJSON *response_root)
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
}*/

