/*******************************************************************************************
 * Project:  huayi_clound_config                                                           *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20210114 add by rex  Version 0.0 													   *
 ******************************************************************************************/

#include "huayi_cloud_config.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <wait.h>
#include <stdio.h>
#include <string.h>
#include "libutils.h"


void huayi_get_network_config(char *gatewayip,char *mac_address,char *ip_address,char *sn,unsigned long long *start,unsigned long long *end)
{
	char cmd[128] = {0},ssn[24] = {0};
	char resultbuf[128] = {0};
	unsigned long long rx_bytes=0,tx_bytes=0,all=0,endd=0,newend=0;
	unsigned long long enddd;

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",gatewayip);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",ip_address);

	sprintf(cmd,"huayi_mac read wlan_2g |grep \"wlan_2g\" |awk -F' ' '{print$2}' |tr '[a-z]' '[A-Z]'");
	get_cmd_result(cmd, resultbuf, sizeof(resultbuf));

	sprintf(ssn,"%c%c%c%c%c%c%c%c%c%c%c%c",resultbuf[0],resultbuf[1],resultbuf[3],resultbuf[4],resultbuf[6],resultbuf[7],resultbuf[9],resultbuf[10],resultbuf[12],resultbuf[13],resultbuf[15],resultbuf[16]);

	strcpy(sn, ssn);
	strcpy(mac_address, resultbuf);

	huayi_uci_get_long_long(PKG_SYSTEM,"network","amound_end",&endd);
	huayi_uci_get_long_long(PKG_SYSTEM,"network","amound_end",&enddd);
	huayi_uci_get_long_long(PKG_SYSTEM,"network","all",&all);
	huayi_uci_get_long_long(PKG_SYSTEM, "network", "rx_bytes", &rx_bytes);
	huayi_uci_get_long_long(PKG_SYSTEM, "network", "tx_bytes", &tx_bytes);
	
	*start = endd;
	newend = all + rx_bytes + tx_bytes;
	*end = newend;

	huayi_uci_set_long_long(PKG_SYSTEM,"network","amound_end",newend);	
	CLOUD_CLIENT_LOG("%ld,%lld",endd,enddd);
	return;
}

void huayi_get_vpn_config(char *account,char *key,int *status)
{
	int enable = 0,userlen = 0,passlen = 0;
	char username[OPTION_STR_LEN] = {0},*user;
	char password[OPTION_STR_LEN] = {0},*pass;

	huayi_uci_get_int(PKG_PPL2TP,"openvpn","enable",&enable);
	if(enable == 1)
	{
	    FILE *file = fopen("/path/to/credentials.txt", "r");
	    if (file == NULL) {
	        CLOUD_CLIENT_LOG("Error opening file");
			*status = enable;
			return ;
	    }else
	    {
			fgets(username, sizeof(username), file);
			fgets(password, sizeof(password), file);
		}
	}else
	{
		*status = enable;
		return ;
	}

	user = strtok(username,"\n");
	pass = strtok(password,"\n");
	strcpy(account,user);
	strcpy(key,pass);
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

	huayi_uci_get_str(PKG_SYSTEM,"network","amound_end",start);
	if((0 != strlen(end)) && (0 != strcmp(end, start)))
	{
		huayi_uci_set_str(PKG_SYSTEM,"network","amound_end",end);
		huayi_uci_set_long(PKG_SYSTEM, "network", "rx_bytes", 0);
		huayi_uci_set_long(PKG_SYSTEM, "network", "tx_bytes", 0);
		huayi_uci_commit(PKG_SYSTEM);
		system("/etc/init.d/hy_get_bytes restart");
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
	CLOUD_CLIENT_LOG("limitspeed:%d\n",limit);

	huayi_uci_get_int(PKG_SYSTEM,"server","limitspeed",&oldlimit);
	if(limit != oldlimit)
	{
		huayi_uci_set_int(PKG_SYSTEM,"server","limitspeed",limit);
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
			huayi_uci_set_int(PKG_QOS,"qos","total_upspeed", limit);
			huayi_uci_set_int(PKG_QOS,"qos","total_dwspeed", limit);
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

void huayi_set_vpn_config(cJSON *response_root)
{
	int status,enable,fale=0;
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0},url[CMD_STR_LEN] = {0};

	if(huayi_json_get_int(response_root,"vpn_status", &status))
	{	
		huayi_uci_set_int(PKG_PPL2TP,"openvpn","enable",status);	
		if(status == 0)
		{
			CLOUD_CLIENT_LOG("close VPN\n");
			huayi_uci_delete_option(PKG_NETWORK,"wan","dns");
			huayi_uci_delete_option(PKG_NETWORK,"wan4g","dns");
		}else if(status == 1)
		{	
			CLOUD_CLIENT_LOG("open VPN\n");
			huayi_uci_set_int(PKG_PPL2TP,"openvpn","option",1);
			huayi_uci_delete_option(PKG_NETWORK,"wan","dns");
			huayi_uci_delete_option(PKG_NETWORK,"wan4g","dns");
			huayi_uci_add_list(PKG_NETWORK,"wan","dns","8.8.8.8");
			huayi_uci_add_list(PKG_NETWORK,"wan","dns","8.8.4.4");
			huayi_uci_add_list(PKG_NETWORK,"wan4g","dns","8.8.8.8");
			huayi_uci_add_list(PKG_NETWORK,"wan4g","dns","8.8.4.4");
		}
		fale=1;
	}

	if(huayi_json_get_str(response_root,"vpn_account",username,OPTION_STR_LEN)&&
		huayi_json_get_str(response_root,"vpn_key",password,OPTION_STR_LEN))
	{	
		CLOUD_CLIENT_LOG("account:%s,key:%s\n",username,password);
		char *filename = "/path/to/credentials.txt";
	    FILE *file = fopen(filename, "r");
	    if (file == NULL) {
			CLOUD_CLIENT_LOG("Error opening file");
	        //system("mkdir -p /path/to/");
	        sprintf(cmd,"touch /path/to/credentials.txt");
			system(cmd);
	    }else
			fclose(file);
		sprintf(cmd,"echo %s > /path/to/credentials.txt",username);
		system(cmd);
		memset(cmd, 0, sizeof(cmd));	
		sprintf(cmd,"echo %s >> /path/to/credentials.txt",password);
		system(cmd);
		fale=1;
	}

	memset(cmd, 0, sizeof(cmd));

	if(huayi_json_get_str(response_root,"vpn_file",url,CMD_STR_LEN)) 
	{
		int iState = -1;
		CLOUD_CLIENT_LOG("url:%s\n",url);
		sprintf(cmd, "curl \"%s\" -s -L -o %s --connect-timeout 60 -m 43200 --speed-limit 5000 --speed-time 300", url,SCRIPT_FILE);
    	iState = system(cmd);
	    if ((WIFEXITED(iState)) && (WEXITSTATUS(iState) == 0) && (iState != -1))
	    {
			CLOUD_CLIENT_LOG("download success\n");
		}else CLOUD_CLIENT_LOG("download fail,iState:%d\n",iState);
		fale=1;
	}

	if(fale == 1)
	{
		huayi_uci_commit(PKG_NETWORK);
		huayi_uci_commit(PKG_PPL2TP);
		system("huayi-reset openvpn");
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

