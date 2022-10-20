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
void msg_content(char *heartdata, unsigned int *heartsize)
{
	cJSON *result_root = NULL;
	char mobile_type[SHORT_STR_LEN] = {0}, iccid[OPTION_STR_LEN] = {0};
	char lac[OPTION_STR_LEN] = {0}, cellid[OPTION_STR_LEN] = {0};
	int  sig = 0;
	char soft_ver[SHORT_STR_LEN] = {0}, sn[OPTION_STR_LEN] = {0};
	char gatewayip[RESULT_STR_LEN] = {0};
	long speeddown = 0, speedup = 0;
	int hidden_2g = 0, client_num = 0;
	char ssid_2g[OPTION_STR_LEN] = {0}; 
	char key_2g[TEMP_STR_LEN] = {0};
	char *output = NULL;
	
	result_root = cJSON_CreateObject();
	
	huayi_get_4g_info(mobile_type, &sig, iccid, lac, cellid);
	cJSON_AddStringToObject(result_root,"opname", mobile_type);
	cJSON_AddNumberToObject(result_root,"dbm", sig);
	cJSON_AddStringToObject(result_root,"iccid", iccid);
	cJSON_AddStringToObject(result_root,"cell_id", cellid);
	cJSON_AddStringToObject(result_root,"lac", lac);
	
	huayi_get_sys_info(sn, soft_ver);
	cJSON_AddStringToObject(result_root,"version", soft_ver);
	cJSON_AddStringToObject(result_root,"sn", sn);
	
	huayi_get_network_config(gatewayip);
	cJSON_AddStringToObject(result_root,"ip", gatewayip);

	huayi_get_qos_config(speeddown, speedup);//to do long
	cJSON_AddNumberToObject(result_root,"speed_downlink", speeddown);
	cJSON_AddNumberToObject(result_root,"speed_uplink", speedup);
	
	huayi_get_wireless_config(ssid_2g, key_2g, &hidden_2g, &client_num);
	cJSON_AddStringToObject(result_root,"ssid", ssid_2g);
	cJSON_AddStringToObject(result_root,"password", key_2g);
	cJSON_AddNumberToObject(result_root,"count",client_num);
	cJSON_AddNumberToObject(result_root,"hidden", hidden_2g);

	output = cJSON_Print(result_root);
	strcpy(heartdata, output);
	*heartsize = strlen(output);
	free(output);
	
	return ;
}

int getJson_Status(char *json_data)
{
	cJSON *root = NULL;
	cJSON *node = NULL;
	int value;

	if(strlen(json_data) == 0) {
		return -1;
	}
	root = cJSON_Parse(json_data);	
	if(root == NULL) {
		return -1;
	}

	node =  cJSON_GetObjectItem(root, "code");
	if(node == NULL) {
		return -1;
	}
	cJSON_Delete(root);
	value = node->valueint;
	
	return value;
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
void msg_parse(char *heartdata, unsigned int heartsize)
{
	int ret = -1; 	
	cJSON *root = NULL;
	cJSON *format = NULL;

	if(strlen(heartdata) == 0) {
		return;
	}
	ret = getJson_Status(heartdata);
	//printf("### respon code is %d .\n", ret);
	if(0 != ret){
		CLOUD_CLIENT_LOG("### respon code is %d .\n", ret);
		return;
	}

	root = cJSON_Parse(heartdata);	
	if(root == NULL)
		return;
	format = cJSON_GetObjectItem(root,"data");
	if(format == NULL) {
		return;
	}

	huayi_next_url_time(format);
	huayi_set_4g_config(format);
	huayi_set_wifi_config(format);
	huayi_set_qos_config(format);
	ret = huayi_isreboot_config(format);
	if(1 == ret) {
		//need reboot. wait other config save success.
		sleep(5);
		system("huayi-reset reboot");
		goto out;
	}
	
	huayi_remote_upgrade(format);

out:
	cJSON_Delete(root);
	return;
}


