/*******************************************************************************************
 * Project:  huayi_cloud_client                                                           *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20220112 add by rex  Version 0.0 													   *
 ******************************************************************************************/

#include "huayi_cloud_client.h"
#include <signal.h>

extern int gTimeWait;
extern char gurldomain[CMD_STR_LEN];

void get_url(char *url)
{
	char urldomain[OPTION_STR_LEN] = { 0 };
	//char port[SHORT_STR_LEN] = { 0 };

	huayi_uci_get_str(PKG_SYSTEM,"server","urldomain", urldomain);
	//huayi_uci_get_str(PKG_SYSTEM,"server","port",port);
	//printf("%s, %d, urldomain = %s !\n", __FUNCTION__, __LINE__, urldomain);
	if(strlen(urldomain) == 0){
		printf("use default server urldomain.\n");
		sprintf(urldomain,"http://%s", HEART_DOMAIN_DEFAULT);
		
	} 
	strcpy(url, urldomain);
	
	return;
}

int get_server(char *ip,int *port)
{
	char serverurl[OPTION_STR_LEN] = { 0 },serverip[OPTION_STR_LEN] = { 0 };
	int serverport;
	int status = 1;
	huayi_uci_get_str(PKG_SYSTEM,"server","serverurl", serverurl);
	huayi_uci_get_int(PKG_SYSTEM,"server","port",&serverport);
	huayi_uci_get_str(PKG_SYSTEM,"server","ip",serverip);
	if(0 == strcmp(serverip, "null"))
	{
		status = change_ip(serverurl,serverip);
		if(status != 0)
		{
			CLOUD_CLIENT_LOG("get server ip error\n");
			return -1;
		}
		huayi_uci_set_str(PKG_SYSTEM,"server","ip",serverip);
		huayi_uci_commit(PKG_SYSTEM);
	}
	//CLOUD_CLIENT_LOG("%s%d,ip:%s,port:%d\n",__FUNCTION__,__LINE__,serverip,serverport);

	strcpy(ip, serverip);
	*port = serverport;
	
	return 0;
}

int change_ip(char *url,char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
	
    if ((he = gethostbyname(url)) == NULL) {
        // 获取host信息失败
        herror("gethostbyname");
		CLOUD_CLIENT_LOG("%s%d,changeip error\n",__FUNCTION__,__LINE__);
        return -1;
    }

	// 将hostent结构体中的地址转换为IP地址
	addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
        // 返回第一个找到的IP地址
        strcpy(ip,inet_ntoa(*addr_list[i]));
    }

	return 0;
}
/*
{
	"sn":"123456",				//must need
	"iccid":"1234444",			//must need
	"dbm":-54,					//must need
	"opname":"46003",			//must need
	"speed_downlink":0,			//must need
	"speed_uplink":0,			//must need
	"ssid":"admin",				//must need
	"password":"12345678",		//must need
	"hidden":0,					//must need
	"count":10,					//must need
	"cell_id":"5272",			//must need
	"lac":"9365",				//must need
	"ip":"0.0.0.0",				//must need
	"version":"1.0"				//must need
}
*/
void msg_content(char *heartdata, unsigned int *heartsize,int serial_num)
{
	cJSON *result_root = NULL;
	/*char mobile_type[SHORT_STR_LEN] = {0}, iccid[OPTION_STR_LEN] = {0};
	char lac[OPTION_STR_LEN] = {0}, cellid[OPTION_STR_LEN] = {0};
	int  sig = 0;
	char soft_ver[SHORT_STR_LEN] = {0}, sn[OPTION_STR_LEN] = {0};
	long speeddown = 0, speedup = 0;*/
	char gatewayip[RESULT_STR_LEN] = {0};
	char mac_address[MAC_STR_LEN] = {0},ip_address[IPV4_STR_LEN] = {0};
	char sn[RESULT_STR_LEN] = {0};
	int hidden_2g = 0, client_num = 0;
	char ssid_2g[OPTION_STR_LEN] = {0}; 
	char key_2g[TEMP_STR_LEN] = {0};
	char *output = NULL;
	int speed = 0;
	char account[RESULT_STR_LEN] = {0},key[RESULT_STR_LEN] = {0};
	int status = 0;
	long start=0,end=0; 

	result_root = cJSON_CreateObject();
	
	/*huayi_get_4g_info(mobile_type, &sig, iccid, lac, cellid);
	cJSON_AddStringToObject(result_root,"opname", mobile_type);
	cJSON_AddNumberToObject(result_root,"dbm", sig);
	cJSON_AddStringToObject(result_root,"iccid", iccid);
	cJSON_AddStringToObject(result_root,"cell_id", cellid);
	cJSON_AddStringToObject(result_root,"lac", lac);
	
	huayi_get_sys_info(sn, soft_ver);
	cJSON_AddStringToObject(result_root,"version", soft_ver);
	cJSON_AddStringToObject(result_root,"sn", sn);
	
	huayi_get_network_config(gatewayip);
	cJSON_AddStringToObject(result_root,"ip", gatewayip);*/

	huayi_get_network_config(gatewayip,mac_address,ip_address,sn,&start,&end);
	cJSON_AddStringToObject(result_root,"SN", sn);
	cJSON_AddStringToObject(result_root,"MAC", mac_address);
	cJSON_AddStringToObject(result_root,"net_ipaddr", ip_address);
	cJSON_AddStringToObject(result_root,"gateway_ipaddr", gatewayip);
	cJSON_AddNumberToObject(result_root,"amound_start", start);
	cJSON_AddNumberToObject(result_root,"amound_end", end);
	
	huayi_get_wireless_config(ssid_2g, key_2g, &hidden_2g, &client_num);
	cJSON_AddStringToObject(result_root,"ssid", ssid_2g);
	cJSON_AddStringToObject(result_root,"Key", key_2g);
	cJSON_AddNumberToObject(result_root,"conn_cnt",client_num);
	cJSON_AddNumberToObject(result_root,"hidden", hidden_2g);

	huayi_get_qos_config(&speed);
	cJSON_AddNumberToObject(result_root,"limitspeed", speed);

	cJSON_AddNumberToObject(result_root,"serial_num", serial_num);

	huayi_get_vpn_config(account,key,&status);
	cJSON_AddStringToObject(result_root,"vpn_account", account);
	cJSON_AddStringToObject(result_root,"vpn_key", key);
	cJSON_AddNumberToObject(result_root,"vpn_status",status);
	
	output = cJSON_Print(result_root);
	strcpy(heartdata, output);
	*heartsize = strlen(output);
	free(output);

	return ;
}

int getJson_Status(cJSON *root)
{
	cJSON *node = NULL;
	int value;

	node =  cJSON_GetObjectItem(root, "status");
	if(node == NULL) {
		return -1;
	}
	value = node->valueint;
	
	return value;
}

void set_next_time(cJSON *root)
{
	cJSON *node = NULL;
	int value,interval = 60;

	node =  cJSON_GetObjectItem(root, "nexttime");
	if(node == NULL) {
		return ;
	}

	value = node->valueint;

	if(value > 0)
	{
		huayi_uci_get_int(PKG_SYSTEM,"server","interval", &interval);
		if(value != interval)
		{	
			CLOUD_CLIENT_LOG("change nexttime:%d!\n",value);
			huayi_uci_set_int(PKG_SYSTEM, "server","interval", value);
			huayi_uci_commit(PKG_SYSTEM);
		}
	}
	
	return ;
}

void huayi_next_url_time(cJSON *response_root)
{	
	char urldomain[TEMP_STR_LEN] = {0};
	int nextRptTime = 0;
	int flag = 0;

	if(NULL == response_root) 
	{
		return ;
	}

	if(!huayi_json_get_int(response_root,"nextRptTime", &nextRptTime)) {
		CLOUD_CLIENT_LOG("nextRptTime param is not respon !\n");
	} else {
		if(0 != nextRptTime) {
			gTimeWait = nextRptTime;
			huayi_uci_set_int(PKG_SYSTEM, "server","interval", nextRptTime);
			flag = 1;
		}
	}
		
	if(!huayi_json_get_str(response_root,"host", urldomain,TEMP_STR_LEN)) {
		CLOUD_CLIENT_LOG("wireless ssid param is not respon !\n");
	} else {
		if(0 != strlen(urldomain)) {
			strcpy(gurldomain, urldomain);
			huayi_uci_set_str(PKG_SYSTEM,"server","urldomain", urldomain);
			flag = 1;
		}
	}	
	//printf("%s, %d, time = %d, host = %s !\n", __FUNCTION__, __LINE__, nextRptTime, urldomain);
	if(1 == flag ) {
		huayi_uci_commit(PKG_SYSTEM);
	}
	
	return;
}


/*
{
	"msg":"seccuss",    							//must need
	"code":0,          							//must need
	"data":{
		"switchSim":0,    						//not must need
		"autoSlot":1,     						//not must need
		"ssid":"admin",							//not must need
		"password":"87654321",					//not must need
		"hidden":1,							//not must need
		"rebot":0,								//not must need
		"speed_downlink":0,						//not must need
		"speed_uplink":0,						//not must need
		"nextRptTime":"1636611289151",				//not must need
		"host":"192.168.0.1"				//not must need
		"firmware_url":"http://xx.xx.xx.xx/xxxx"		//not must need
		"firmware_md5":"gsg3434xdfwerrx"			//not must need
	}
}
fail:
{
	"msg":"SYSTEMERROR",
	"code":500
}
*/
void msg_parse(char *heartdata,int *status)
{
	int ret = -1; 	
	cJSON *root = NULL;
	cJSON *format = NULL;
	int nexttime = 0,interval = 60;

	if(strlen(heartdata) == 0) {
		CLOUD_CLIENT_LOG("strlen(heartdata) == 0");
		return;
	}
	root = cJSON_Parse(heartdata);	
	if(root == NULL)
	{
		CLOUD_CLIENT_LOG("root == NULL");
		return;
	}

	
	ret = getJson_Status(root);
	*status = ret;
	CLOUD_CLIENT_LOG("### respon status is %d .\n", ret);
	if(0 != ret){
		return;
	}

	set_next_time(root);
	
	format = cJSON_GetObjectItem(root,"command");
	if(format == NULL) {
		return;
	}else
		CLOUD_CLIENT_LOG("### respon get command.\n");

	//huayi_next_url_time(format);
	//huayi_set_4g_config(format);
	huayi_set_wifi_config(format);
	set_next_ip_port(format);
	huayi_set_qos_config(format);
	huayi_set_vpn_config(format);
	huayi_set_amound_config(format);
	/*ret = huayi_isreboot_config(format);
	if(1 == ret) {
		//need reboot. wait other config save success.
		sleep(5);
		system("huayi-reset reboot");
		goto out;
	}
	
	huayi_remote_upgrade(format);*/

out:
	cJSON_Delete(root);
	return;
}


