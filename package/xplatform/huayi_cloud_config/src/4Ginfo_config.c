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


void huayi_get_4g_info(char *mobile_type, int *sig, char *iccid, char *lac, char *cellid, char *sn,int *simSwitch,int *band)
{
    int  signal = -121,sim = 0;
	char result[TEMP_STR_LEN];

    huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "operator", mobile_type);
    huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "iccid", iccid);
    huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "lac", lac);
    huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "ci", cellid);
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &sim);
    if (get_dev_sn(sn) < 0)
    {
        strcpy(sn, " ");
    }
    if (strncmp(sn, "FFFFFFFFFFFF", strlen(sn)) == 0)
    {
        strcpy(sn, " ");
    }
    //huayi_uci_get_str(PKG_STATUS_MODEM,"modem","imei",sn);
    atcmd_get_signal_for_cloud(&signal);
	*simSwitch = sim;
    //huayi_uci_get_int(PKG_STATUS_MODEM,"modem","signal",&signal);
    //signal = signal/5 - 120;

    *sig = signal;

    /*CustomCommunicationClient("echo `uci get m_system.usesiminfo.card_slot`",usesiminfoBuf);
    if(!(toParsing(usesiminfoBuf, 1,&sim_slot))){
        CLOUD_CLIENT_LOG("usesiminfoBuf error\n");
     }else *simSwitch = sim_slot;*/
	get_cmd_result("echo $(huayi_AT_cli at+cced=0,1) | awk -F ',' '{print $6}'", result, sizeof(result));
	*band = atoi(result);

    return;
}

/*void huayi_set_4g_config(cJSON *response_root)
{
	int  switchSim = -1, autoSlot = -1;
	//int  oldswitchSim = -1;
	int  cfgset_flag = 0;

	if(NULL == response_root)
	{
		return ;
	}
	//huayi_uci_get_int(PKG_SYSTEM, "simcard","switchSim", &oldswitchSim);
	if(!huayi_json_get_int(response_root, "simSwitch", &switchSim))
	{
		CLOUD_CLIENT_LOG("switchSim is not respon !\n");
	} else {
		if(-1 != switchSim) {
			huayi_uci_set_int(PKG_SYSTEM, "simcard","switchSim", switchSim);
			huayi_uci_set_int(PKG_SYSTEM, "simcard","cloud_cmd_flag", 1);
			cfgset_flag = 1;
		}
	}
	if(!huayi_json_get_int(response_root, "autoSlot", &autoSlot)) //autoSlot   is not use
	{
		CLOUD_CLIENT_LOG("autoSlot is not respon !\n");
	}

	if(1 == cfgset_flag ) {
		huayi_uci_commit(PKG_SYSTEM);
#if defined(CONFIG_HUAYI_SIM_CHANGE_SUPPORT)
		system("/etc/init.d/hy_sc restart");
#endif
	}
	//printf("%s, %d, sim = %d, slot = %d !\n", __FUNCTION__, __LINE__, switchSim, autoSlot);
	return;
}*/

void huayi_set_4g_config(cJSON *response_root)
{
    int at_card = 0, lock = 0, simSwitch = 0, Simnum = 0, enable = 0;
    char usesiminfoBuf[1024] = {0}, siminfoBuf[512] = {0}, cmd[OPTION_STR_LEN] = {0};
    char iccid[OPTION_STR_LEN] = {0};


    if (NULL == response_root)
    {
        return ;
    }
#ifdef CONFIG_HUAYI_SUPPORT_SPECIAL_INTERFACE
    if (!huayi_json_get_int(response_root, "mode", &simSwitch))
    {
        return;
    }

	if(simSwitch == 2)
	{
		simSwitch = 0;
	}else if(simSwitch == 4)
	{
		simSwitch = 1;
	}else if(simSwitch == 3)
	{
		simSwitch = 2;
	}else return ;
	CLOUD_CLIENT_LOG("simSwitch is %d\n",simSwitch);

	huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &at_card);
	if(simSwitch == at_card)
		return ;
#else
    if (!huayi_json_get_int(response_root, "simSwitch", &simSwitch))
    {
        return;
    }
#endif

#ifndef CONFIG_HUAYI_SUPPORT_MEIGE_AFTER_SALES
	int simnum;
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);

    huayi_uci_get_str(PKG_M_SYSTEM, "system", "external_card_enable", &enable);
    if (simSwitch == simnum -1 && !enable)
    {
        SWITCH_SIM_LOG("external_card_enable=%d,switch %d error",enable,simSwitch);
        return;
    }
#endif

    CON_LOG("switch sim %d", simSwitch);
    sprintf(cmd, "switch_sim %d", simSwitch);
    huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "card_slot", simSwitch);
    huayi_uci_commit(PKG_M_SYSTEM);
    system(cmd);
    sleep(30);
    save_useing_siminfo(simSwitch);
    return ;
}

void huayi_set_band_config(cJSON *response_root)
{
	int  band = 0,my_band = 0;

	if (NULL == response_root)
    {
        return ;
    }

	if (huayi_json_get_int(response_root, "band", &band))
    {
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "band", &my_band);
		if(my_band != band){
			huayi_uci_set_int(PKG_M_SYSTEM, "system", "band", band);
			system("huayi-reset band");
		}
    }

	return;
}

void huayi_get_mifi_open(char *mcc,char *mnc,char *iccid,char *lac,char *ci,char *devid,char *hardvers,int *db,char *sign)
{
	char ccmnc[SHORT_STR_LEN]={0},llac[SHORT_STR_LEN]={0},cci[SHORT_STR_LEN]={0};
	int  signal = -121;
	char key[TEMP_STR_LEN]={0};

	atcmd_get_mccmnc(ccmnc);
	strncpy(mcc, ccmnc, 3);
	strncpy(mnc, ccmnc+3, 3);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "lac", llac);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "ci", cci);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "iccid", iccid);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", devid);
	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version",hardvers);
	atcmd_get_signal_for_cloud(&signal);
	*db = signal;

	sprintf(key,"%s%s",SKEY,devid);
	get_sign(key,sign);

	return ;
}

void huayi_get_mifi_ink(char *mcc,char *mnc,char *btn,char *sim,char *lac,char *devid,int *mode,char *sign)
{
	char ccmnc[SHORT_STR_LEN]={0},llac[SHORT_STR_LEN]={0};
	char key[TEMP_STR_LEN]={0};
	int simnum = 0;
	int card;
	
	atcmd_get_mccmnc(ccmnc);
	strncpy(mcc, ccmnc, 3);
	strncpy(mnc, ccmnc+3, 3);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "lac", lac);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", devid);
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &card);
#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_AFTER_SALES
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);
	if(card == simnum-1)
	{	
		huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "iccid", btn);
	}else
#endif
	{	
		huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "iccid", sim);
	}

	if(card == 0)
	{
		*mode = 2;//内置卡1
	}else if(card == 1)
	{
		*mode = 4;//内置卡2
	}else if(card == 2)
	{
		*mode = 3;//外插卡
	}

	sprintf(key,"%s%s%s",SKEY,clientId,devid);
	get_sign(key,sign);
	return ;
}


void huayi_get_mifi_update(unsigned long long *flow,char *reptm,char *sign,char *devid,char *iccid,int *db,
				int *wificount,char *wifipsw,char *ssid,char *imei,int *vspeed)
{
	int signal = -121,wifi=0;
	char key[TEMP_STR_LEN]={0};
	unsigned long long rx=0,tx=0;

#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_AFTER_SALES
	int simnum = 0,card;
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &card);
	if(card != simnum-1)
	{	
		huayi_uci_get_str(PKG_SYSTEM, "network", "time", reptm);
		huayi_uci_get_long_long(PKG_SYSTEM, "network", "rx_bytes",&rx);
		huayi_uci_get_long_long(PKG_SYSTEM, "network", "tx_bytes",&tx);
		*flow=(rx+tx);
	}
#else
	huayi_uci_get_str(PKG_SYSTEM, "network", "time", reptm);
	huayi_uci_get_long_long(PKG_SYSTEM, "network", "rx_bytes",&rx);
	huayi_uci_get_long_long(PKG_SYSTEM, "network", "tx_bytes",&tx);
	*flow=(rx+tx);
#endif

	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", devid);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "iccid", iccid);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", imei);

	atcmd_get_signal_for_cloud(&signal);
	*db = signal;

	huayi_uci_get_int(PKG_STATUS_SYSTEM, "sys", "wifi_24g_num",&wifi);
	*wificount=wifi;
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid",ssid);
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key",wifipsw);

	int down = 0,mode = -1;
	huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&mode);
	if(mode == 0)
	{
		*vspeed = 0;
	}else {
		huayi_uci_get_int(PKG_QOS,"qos","total_dwspeed",&down);
		*vspeed = down / 8;
	}

	sprintf(key,"%s0",SKEY);
	get_sign(key,sign);

	return ;
}


void get_sign(char* key, char* sign)
{
	unsigned char md5[16]={0};
	char content[RESULT_STR_LEN]={0};

	MDString(key,strlen(key),md5);

	int i =0;
	for(i=0;i<16;i++)  
	{   
		sprintf(content, "%02x", md5[i]);
		strcat(sign,content);
	}

	return ;
}

