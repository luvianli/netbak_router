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
	CLOUD_CLIENT_LOG(" %s; url is : %s\n",__FUNCTION__,url);
	return;
}

void get_mifi_url(char *url,int code)
{
	char urldomain[OPTION_STR_LEN] = { 0 };

	huayi_uci_get_str(PKG_SYSTEM,"server","mifiurl", urldomain);

	if(strlen(urldomain) == 0){
		printf("use default server urldomain.\n");
		sprintf(urldomain,"http://%s", HEART_DOMAIN_DEFAULT);
		
	}

	if(code == 1)
	{
		sprintf(urldomain,"%s/MiFi/Open", urldomain);
	}else if(code == 2)
	{
		sprintf(urldomain,"%s/MiFi/Update2", urldomain);
	}
	strcpy(url, urldomain);
	CLOUD_CLIENT_LOG(" %s; url is : %s\n",__FUNCTION__,url);
	return;
}

void set_url(cJSON *response_root)
{
	char address[IPV4_STR_LEN] = {0},port[IPV4_STR_LEN] = {0};

    if (NULL == response_root)
    {
        return ;
    }

	if (!huayi_json_get_str(response_root, "address", address,SHORT_STR_LEN)||
		!huayi_json_get_str(response_root, "port", port,SHORT_STR_LEN))
    {	
    	CLOUD_CLIENT_LOG("address or port is null\n");
        return;
    }
	CLOUD_CLIENT_LOG("address:%s,port:%s\n",address,port);

	char urldomain[OPTION_STR_LEN] = { 0 };
	sprintf(urldomain,"http://%s:%s",address,port);

	huayi_uci_set_str(PKG_SYSTEM,"server","mifiurl",urldomain);
	huayi_uci_commit(PKG_SYSTEM);

	char ssid[OPTION_STR_LEN] = {0},oldssid[OPTION_STR_LEN] = {0},imei[RESULT_STR_LEN] = {0};
	if (!huayi_json_get_str(response_root, "prefix", ssid,SHORT_STR_LEN))
    {	
        return;
    }

	int len = strlen(imei),change;
	char imei_6[RESULT_STR_LEN] = {0};
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", imei);
	strcpy(imei_6, imei + strlen(imei) - 6);
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid", oldssid);
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","change", &change);
	sprintf(ssid,"%s%s",ssid,imei_6);
	if(0 != strcmp(ssid, oldssid) && change == 0)
	{	
		CLOUD_CLIENT_LOG(" ssid:%s; old: %s\n",ssid,oldssid);
		huayi_uci_set_str(PKG_WIRELESS,"radio0_0","ssid", ssid);
		huayi_uci_commit(PKG_WIRELESS); 
		system("huayi-reset wireless");
	}

	return ;
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
/*void msg_content(char *heartdata, unsigned int *heartsize)
{
	cJSON *result_root = NULL;
	char mobile_type[SHORT_STR_LEN] = {0}, iccid[OPTION_STR_LEN] = {0};
	char lac[OPTION_STR_LEN] = {0}, cellid[OPTION_STR_LEN] = {0};
	int  sig = 0,devicetype = 0,uptime =0;
	char soft_ver[SHORT_STR_LEN] = {0}, sn[OPTION_STR_LEN] = {0},productname[OPTION_STR_LEN] = {0};
	char gatewayip[RESULT_STR_LEN] = {0};
	long speeddown = 0, speedup = 0;
	int hidden_2g = 0, client_num = 0;
	char ssid_2g[OPTION_STR_LEN] = {0}; 
	char key_2g[TEMP_STR_LEN] = {0};
	char *output = NULL;
	int wifi_status = 0,wan_status = 0,channel = 0;
	char wanip[RESULT_STR_LEN] = {0},wannetmask[RESULT_STR_LEN] = {0},nettype[RESULT_STR_LEN] = {0};
	
	result_root = cJSON_CreateObject();
	
	huayi_get_4g_info(mobile_type, &sig, iccid, lac, cellid,sn);
	cJSON_AddStringToObject(result_root,"opname", mobile_type);
	cJSON_AddNumberToObject(result_root,"dbm", sig);
	cJSON_AddStringToObject(result_root,"iccid", iccid);
	cJSON_AddStringToObject(result_root,"cell_id", cellid);
	cJSON_AddStringToObject(result_root,"lac", lac);
	cJSON_AddStringToObject(result_root,"sn", sn);
	
	huayi_get_sys_info(soft_ver,productname,&uptime,&devicetype);
	cJSON_AddStringToObject(result_root,"version", soft_ver);
	//cJSON_AddStringToObject(result_root,"sn", sn);
	
	huayi_get_network_config(gatewayip);
	cJSON_AddStringToObject(result_root,"ip", gatewayip);

	huayi_get_qos_config(&speeddown, &speedup);//to do long	
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
}*/
void mifi_link_content(char *heartdata, unsigned int *heartsize)
{
	char devid[RESULT_STR_LEN] = {0};
	char mmc[SMALL_STR_LEN]={0},mnc[SMALL_STR_LEN]={0};
	char btn[OPTION_STR_LEN]={0},sim[OPTION_STR_LEN]={0},lac[OPTION_STR_LEN]={0},sign[OPTION_STR_LEN]={0};
	int mode = -1;

	huayi_get_mifi_ink(mmc,mnc,btn,sim,lac,devid,&mode,sign);
	sprintf(heartdata,"c=1101&devid=%s&sign=%s&btn=%s&sim=%s&mmc=%s&mnc=%s&lac=%s&mode=%d",
		devid,sign,btn,sim,mmc,mnc,lac,mode);
	*heartsize = strlen(heartdata);
	return ;
}

void mifi_open_content(char *heartdata, unsigned int *heartsize)
{
	cJSON *result_root = NULL,*output = NULL;
	char devid[RESULT_STR_LEN] = {0},hardvers[RESULT_STR_LEN] = {0};
	char mcc[SMALL_STR_LEN]={0},mnc[SMALL_STR_LEN]={0};
	char iccid[OPTION_STR_LEN]={0},lac[OPTION_STR_LEN]={0},sign[OPTION_STR_LEN]={0};
	char ci[SHORT_STR_LEN]={0};
	int db = 0;
	/*result_root = cJSON_CreateObject();

	huayi_get_mifi_open(mcc,mnc,iccid,lac,ci,devid,hardvers,&db,sign);
	cJSON_AddNumberToObject(result_root,"c", 1101);
	cJSON_AddStringToObject(result_root,"mcc", mcc);
	cJSON_AddStringToObject(result_root,"mnc", mnc);
	cJSON_AddStringToObject(result_root,"lac", lac);
	cJSON_AddStringToObject(result_root,"ci",ci);
	cJSON_AddStringToObject(result_root,"devid", devid);
	cJSON_AddStringToObject(result_root,"iccid", iccid);
	cJSON_AddStringToObject(result_root,"hardvers", hardvers);
	cJSON_AddNumberToObject(result_root,"db", db);
	cJSON_AddStringToObject(result_root,"sign", sign);

	output = cJSON_Print(result_root);
	strcpy(heartdata, output);
	*heartsize = strlen(output);
	free(output);*/
	huayi_get_mifi_open(mcc,mnc,iccid,lac,ci,devid,hardvers,&db,sign);
	sprintf(heartdata,"\"c\":\"1101\",\"mcc\":\"%s\",\"mnc\":\"%s\",\"lac\":\"%s\",\"ci\":\"%s\",\"devid\":\"%s\",\"iccid\":\"%s\",\"hardvers\":\"%s\",\"sign\":\"%s\",\"db\":\"%d\"",
		mcc,mnc,lac,ci,devid,iccid,hardvers,sign,db);
	*heartsize = strlen(heartdata);
	return ;
}

void mifi_update_content(char *heartdata, unsigned int *heartsize)
{
	cJSON *result_root = NULL,*output = NULL;
	char devid[RESULT_STR_LEN] = {0},reptm[RESULT_STR_LEN] = {0},imei[RESULT_STR_LEN]={0};
	int flow;
	char iccid[OPTION_STR_LEN]={0},wifipsw[OPTION_STR_LEN]={0},ssid[OPTION_STR_LEN]={0},sign[OPTION_STR_LEN]={0};
	int db = 0,vspeed = 0,battery = 0,wificount = 0;
	//result_root = cJSON_CreateObject();

	huayi_get_mifi_update(&flow,reptm,sign,devid,iccid,&db,&wificount,wifipsw,ssid,imei,&vspeed);
	/*cJSON_AddNumberToObject(result_root,"c", 1101);
	cJSON_AddStringToObject(result_root,"flow", flow);
	cJSON_AddStringToObject(result_root,"reptm", reptm);
	cJSON_AddStringToObject(result_root,"devid", devid);
	cJSON_AddStringToObject(result_root,"imei", devid);
	cJSON_AddStringToObject(result_root,"iccid", iccid);
	cJSON_AddNumberToObject(result_root,"db", db);
	cJSON_AddNumberToObject(result_root,"wificount", wificount);
	cJSON_AddStringToObject(result_root,"wifipsw", wifipsw);
	cJSON_AddStringToObject(result_root,"ssid", ssid);
	cJSON_AddNumberToObject(result_root,"vspeed", vspeed);
	cJSON_AddNumberToObject(result_root,"battery", 100);
	cJSON_AddNumberToObject(result_root,"sessid", 0);
	cJSON_AddStringToObject(result_root,"sign", sign);
	output = cJSON_Print(result_root);
	strcpy(heartdata, output);
	*heartsize = strlen(output);
	free(output);*/
	if(flow == 0)
		sprintf(heartdata,"\"c\":\"1101\",\"sign\":\"%s\",\"devid\":\"%s\",\"iccid\":\"%s\",\"db\":\"%d\",\"wificount\":\"%d\",\"wifipsw\":\"%s\",\"ssid\":\"%s\",\"imei\":\"%s\",\"vspeed\":\"%d\",\"battery\":\"100\",\"sessid\":\"0\"",
			sign,devid,iccid,db,wificount,wifipsw,ssid,imei,vspeed);
	else sprintf(heartdata,"\"c\":\"1101\",\"flow\":\"%d\",\"reptm\":\"%s\",\"sign\":\"%s\",\"devid\":\"%s\",\"iccid\":\"%s\",\"db\":\"%d\",\"wificount\":\"%d\",\"wifipsw\":\"%s\",\"ssid\":\"%s\",\"imei\":\"%s\",\"vspeed\":\"%d\",\"battery\":\"100\",\"sessid\":\"0\"",
			flow,reptm,sign,devid,iccid,db,wificount,wifipsw,ssid,imei,vspeed);
	*heartsize = strlen(heartdata);
	return ;
}

void msg_content(char *heartdata, unsigned int *heartsize)
{
	cJSON *result_root = NULL;
	char mobile_type[SHORT_STR_LEN] = {0}, iccid[OPTION_STR_LEN] = {0};
	char lac[OPTION_STR_LEN] = {0}, cellid[OPTION_STR_LEN] = {0};
	int  sig = 0,devicetype = 0,uptime = 0,simSwitch = 0;
	char soft_ver[SHORT_STR_LEN] = {0}, sn[OPTION_STR_LEN] = {0},productname[OPTION_STR_LEN] = {0};
	char gatewayip[RESULT_STR_LEN] = {0};
	long speeddown = 0, speedup = 0;
	int hidden_2g = 0, client_num = 0;
	char ssid_2g[OPTION_STR_LEN] = {0},key_2g[TEMP_STR_LEN] = {0};
	char *output = NULL;
	int status = 0,channel = 0;
	char ip[IPV4_STR_LEN] = {0},netmask[IPV4_STR_LEN] = {0},nettype[RESULT_STR_LEN] = {0};
	char startip[IPV4_STR_LEN] = {0},endip[IPV4_STR_LEN] = {0};
	int lease_time = 0;
	int rectime = 0,week = 0,hour = 0,minute = 0,rec_time = 0,type = 0;
	char mode[RESULT_STR_LEN] = {0},proto[RESULT_STR_LEN] = {0},name[RESULT_STR_LEN] = {0},gateway[IPV4_STR_LEN] = {0};
	char dns1[IPV4_STR_LEN] = {0},dns2[IPV4_STR_LEN] = {0},p_name[OPTION_STR_LEN] = {0},password[OPTION_STR_LEN] = {0},ac[OPTION_STR_LEN] = {0},service[OPTION_STR_LEN] = {0};
	char dial_apn[OPTION_STR_LEN] = {0},dial_name[OPTION_STR_LEN] = {0},dial_pass[OPTION_STR_LEN] = {0},dial_auth[OPTION_STR_LEN] = {0};
	char product_model[OPTION_STR_LEN] = {0};
        
	result_root = cJSON_CreateObject();
    huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
    if(strcmp(product_model, "WYX3C-1") == 0)
    {
        huayi_get_4g_info(mobile_type, &sig, iccid, lac, cellid,sn);
        cJSON_AddStringToObject(result_root,"sn", sn);
        cJSON_AddStringToObject(result_root,"iccid", iccid);
        cJSON_AddNumberToObject(result_root,"dbm", sig);
        cJSON_AddStringToObject(result_root,"opname", mobile_type);
        cJSON_AddNumberToObject(result_root,"simSwitch", simSwitch);
        speeddown = 0;
    	speedup = 0 ;
    	huayi_get_qos_config(&speeddown, &speedup);//to do long
    	cJSON_AddNumberToObject(result_root,"speed_downlink", speeddown);
    	cJSON_AddNumberToObject(result_root,"speed_uplink", speedup);

        huayi_get_wireless_config(ssid_2g, key_2g, &hidden_2g, &client_num,&channel,&status);
    	//cJSON_AddNumberToObject(result_root,"wifi_status",status);
    	cJSON_AddStringToObject(result_root,"ssid", ssid_2g);
    	cJSON_AddStringToObject(result_root,"password", key_2g);
    	//cJSON_AddNumberToObject(result_root,"channel",channel);
    	cJSON_AddNumberToObject(result_root,"hidden", hidden_2g);
        cJSON_AddNumberToObject(result_root,"count",client_num);
                cJSON_AddStringToObject(result_root,"cell_id", cellid);
          cJSON_AddStringToObject(result_root,"lac", lac);
        huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",gatewayip);
        cJSON_AddStringToObject(result_root,"ip", gatewayip);
        huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version", soft_ver);
        cJSON_AddStringToObject(result_root,"version", soft_ver);
    }else{
    	huayi_get_4g_info(mobile_type, &sig, iccid, lac, cellid,sn);
    	cJSON_AddStringToObject(result_root,"sn", sn);
        
    	cJSON_AddStringToObject(result_root,"iccid", iccid);
    	cJSON_AddNumberToObject(result_root,"dbm", sig);
    	cJSON_AddStringToObject(result_root,"opname", mobile_type);
    	cJSON_AddStringToObject(result_root,"cell_id", cellid);
    	cJSON_AddStringToObject(result_root,"lac", lac);
    	cJSON_AddNumberToObject(result_root,"simSwitch", simSwitch);
    	
    	huayi_get_sys_info(soft_ver,productname,&uptime,&devicetype);
    	cJSON_AddNumberToObject(result_root,"devicetype", devicetype);
    	cJSON_AddStringToObject(result_root,"productname", productname);
    	cJSON_AddStringToObject(result_root,"version", soft_ver);
    	cJSON_AddNumberToObject(result_root,"uptime", uptime);
    	
    	huayi_get_wan_confign(gatewayip,&status,ip,netmask,nettype,&speedup, &speeddown);
    	cJSON_AddStringToObject(result_root,"ip", gatewayip);
    	cJSON_AddNumberToObject(result_root,"wan_status", status);
    	cJSON_AddStringToObject(result_root,"wan_ip", ip);
    	cJSON_AddStringToObject(result_root,"wan_netmask", netmask);
    	cJSON_AddStringToObject(result_root,"nettype", nettype);
    	cJSON_AddNumberToObject(result_root,"rx_bps", speeddown);
    	cJSON_AddNumberToObject(result_root,"tx_bps", speedup);

    	memset(ip,0,sizeof(ip));
    	memset(netmask,0,sizeof(netmask));
    	status = 0;
    	huayi_get_lan_confign(ip,netmask,&status,startip,endip,&lease_time);
    	cJSON_AddStringToObject(result_root,"lan_ip", ip);
    	cJSON_AddStringToObject(result_root,"lan_netmask", netmask);
    	cJSON_AddNumberToObject(result_root,"lan_dhcp", status);
    	cJSON_AddStringToObject(result_root,"start_ip", startip);
    	cJSON_AddStringToObject(result_root,"end_ip", endip);
    	cJSON_AddNumberToObject(result_root,"lease_time", lease_time);

    	speeddown = 0;
    	speedup = 0 ;
    	huayi_get_qos_config(&speeddown, &speedup);//to do long
    	cJSON_AddNumberToObject(result_root,"speed_downlink", speeddown);
    	cJSON_AddNumberToObject(result_root,"speed_uplink", speedup);

    	status = 0;
    	huayi_get_wireless_config(ssid_2g, key_2g, &hidden_2g, &client_num,&channel,&status);
    	cJSON_AddNumberToObject(result_root,"wifi_status",status);
    	cJSON_AddStringToObject(result_root,"ssid", ssid_2g);
    	cJSON_AddStringToObject(result_root,"password", key_2g);
    	cJSON_AddNumberToObject(result_root,"channel",channel);
    	cJSON_AddNumberToObject(result_root,"count",client_num);
    	cJSON_AddNumberToObject(result_root,"hidden", hidden_2g);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
    	status = 0;
    	huayi_get_wireless_5g_config(ssid_2g, key_2g, &hidden_2g, &channel,&status);
    	cJSON_AddNumberToObject(result_root,"5gmode", 1);
    	cJSON_AddNumberToObject(result_root,"wifi_5g",status);
    	cJSON_AddStringToObject(result_root,"ssid_5g", ssid_2g);
    	cJSON_AddStringToObject(result_root,"password_5g", key_2g);
    	cJSON_AddNumberToObject(result_root,"channel_5g",channel);
    	cJSON_AddNumberToObject(result_root,"hidden_5g", hidden_2g);
#else
    	cJSON_AddNumberToObject(result_root,"5gmode", 0);
#endif

    	status = 0;
    	memset(ip,0,sizeof(ip));
    	memset(netmask,0,sizeof(netmask));

    	huayi_get_wan_or_wan4g_config(&type,mode,proto,name,&status,ip,netmask,gateway,dns1,dns2
    		,p_name,password,ac,service,dial_apn,dial_name,dial_pass,dial_auth);
    	cJSON_AddNumberToObject(result_root,"wan_type", type);
    	cJSON_AddStringToObject(result_root,"wan_mode", mode);
    	cJSON_AddStringToObject(result_root,"proto", proto);
    	cJSON_AddStringToObject(result_root,"host_name", name);
    	if(!strcmp(proto,"static"))
    	{
    		cJSON_AddStringToObject(result_root,"sta_ip", ip);
    		cJSON_AddStringToObject(result_root,"sta_netmask", netmask);
    		cJSON_AddStringToObject(result_root,"sta_gateway", gateway);
    		cJSON_AddStringToObject(result_root,"dns1", dns1);
    		cJSON_AddStringToObject(result_root,"dns2", dns2);
    	}else if(!strcmp(proto,"pppoe"))
    	{
    		cJSON_AddStringToObject(result_root,"ppp_name", p_name);
    		cJSON_AddStringToObject(result_root,"ppp_pass", password);
    		cJSON_AddStringToObject(result_root,"ppp_ac", ac);
    		cJSON_AddStringToObject(result_root,"ppp_server", service);
    	}
    	cJSON_AddNumberToObject(result_root,"dial_way", status);
    	if(status != 0)
    	{
    		cJSON_AddStringToObject(result_root,"dial_apn", dial_apn);
    		cJSON_AddStringToObject(result_root,"dial_name", dial_name);
    		cJSON_AddStringToObject(result_root,"dial_pass", dial_pass);
    		cJSON_AddStringToObject(result_root,"dial_auth", dial_auth);
    	}
    	
    	status = 0;
    	huayi_get_sche_reboot_config(&status,&rectime,&week,&hour,&minute,&rec_time);
    	cJSON_AddNumberToObject(result_root,"sch_reboot", status);
    	cJSON_AddNumberToObject(result_root,"rectime", rectime);
    	cJSON_AddNumberToObject(result_root,"week", week);
    	cJSON_AddNumberToObject(result_root,"hour", hour);
    	cJSON_AddNumberToObject(result_root,"minute", minute);
    	cJSON_AddNumberToObject(result_root,"rec_time", rec_time);
    }
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
		
	/*if(!huayi_json_get_str(response_root,"host", urldomain,TEMP_STR_LEN)) {
		CLOUD_CLIENT_LOG("wireless ssid param is not respon !\n");
	} else {
		if(0 != strlen(urldomain)) {
			strcpy(gurldomain, urldomain);
			huayi_uci_set_str(PKG_SYSTEM,"server","urldomain", urldomain);
			flag = 1;
		}
	}	*/
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
void link_msg_parse(char *heartdata, unsigned int heartsize,int *code)
{
	int ret = -1; 	
	cJSON *root = NULL;
	cJSON *format = NULL;
	char *info;

	if(strlen(heartdata) == 0) {
		return;
	}
	ret = getJson_Status(heartdata);
	*code = ret;
	CLOUD_CLIENT_LOG("%s code is %d .\n",__FUNCTION__, ret);
	root = cJSON_Parse(heartdata);	
	if(root == NULL)
		return;
	format = cJSON_GetObjectItem(root,"data");
	if(format == NULL) {
			return;
		}

	if(ret == 0)
	{	
		set_url(format);
	}else if(ret == 1 || ret == 201 || ret == 501 || ret == 502 || ret == 508)//设备无法上网
	{	
		*code = 1;
		system("ifdown wan4g");
	}
	return;
}

void msg_parse(char *heartdata, unsigned int heartsize,int *code)
{
	int ret = -1; 	
	cJSON *root = NULL;
	cJSON *format = NULL;
	char *info;

	if(strlen(heartdata) == 0) {
		return;
	}
	ret = getJson_Status(heartdata);
	*code = ret;
	CLOUD_CLIENT_LOG("%s code is %d .\n",__FUNCTION__, ret);
	root = cJSON_Parse(heartdata);	
	if(root == NULL)
		return;
	format = cJSON_GetObjectItem(root,"data");
	if(format == NULL) {
		format = cJSON_GetObjectItem(root,"json");
		if(format == NULL) {
			return;
		}
	}

	if(ret == 0)
	{	
#ifndef	CONFIG_HUAYI_SUPPORT_MEIGE_SINGLE_CARD
		huayi_set_4g_config(format);
#endif
	}else if(ret == 1 || ret == 201 || ret == 501 || ret == 502 || ret == 508)//设备无法上网
	{	
		*code = 1;
		//system("route del default");
		system("ifdown wan4g");
	}else if(ret == 509)//重启
	{	
		system("reboot");
	}/*else if(ret == 506)//限速
	{	
		huayi_set_wifi_qos();
	}else if(ret == 507)//取消限速
	{	
		int mode = -1;
		huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&mode);
		if(mode != 0)
		{
			huayi_uci_set_int(PKG_QOS,"qos","total_upspeed",0);
			huayi_uci_set_int(PKG_QOS,"qos","total_dwspeed",0);
			huayi_uci_set_int(PKG_QOS,"qos","qos_mode", 0);
			huayi_uci_commit(PKG_QOS);	
			system("huayi-reset iplimit");
		}
	}*/else if(ret == 511)//修改wifi密码
	{	
		huayi_set_wifi_psw(format);
	}else if(ret == 514)//修改wifi名
	{	
		huayi_set_wifi_ssid(format);
	}else if(ret == 515)//恢复出厂
	{	
		system("huayi-reset reset");
	}
	
	/*huayi_next_url_time(format);
	huayi_set_4g_config(format);
	huayi_set_wifi_config(format);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_set_5gwifi_config(format);
#endif
	huayi_set_lan_config(format);
	huayi_set_dhcp_config(format);
	huayi_set_qos_config(format);
	huayi_set_sche_reboot_config(format);
	huayi_set_wan_config(format);
	ret = huayi_isreboot_config(format);
	if(1 == ret) {
		//need reboot. wait other config save success.
        CON_LOG("reboot ");
        sleep(5);
        
		system("huayi-reset reboot");
        system("reboot");
		goto out;
	}
	
	huayi_remote_upgrade(format);

out:
	cJSON_Delete(root);*/
	return;
}
