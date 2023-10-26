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


void huayi_get_4g_info(char *mobile_type, int *sig, char *iccid, char *lac, char *cellid,char *sn)
{
	int  signal = -121;

	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","operator", mobile_type);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","iccid",iccid);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","lac",lac);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","ci",cellid);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","imei",sn);
	atcmd_get_signal_for_cloud(&signal);
	//huayi_uci_get_int(PKG_STATUS_MODEM,"modem","signal",&signal);
	//signal = signal/5 - 120;

	*sig = signal;

	/*CustomCommunicationClient("echo `uci get m_system.usesiminfo.card_slot`",usesiminfoBuf);
	if(!(toParsing(usesiminfoBuf, 1,&sim_slot))){
        CLOUD_CLIENT_LOG("usesiminfoBuf error\n");
     }else *simSwitch = sim_slot;*/
	
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
	int sim_slot = 0,lock = 0,simSwitch = 0,Simnum = 0,enable =0;
	char usesiminfoBuf[1024] = {0},siminfoBuf[512] = {0},cmd[OPTION_STR_LEN]={0};
	char iccid[OPTION_STR_LEN]={0};


	if(NULL == response_root) 
	{
		return ;
	}

	if(!huayi_json_get_int(response_root, "simSwitch", &simSwitch)) 
	{
		return;
	}
    huayi_uci_get_str(PKG_M_SYSTEM, "system", "external_card_enable", &enable);
    if(simSwitch == 2 && !enable){
        CON_LOG("external_card_enable=0,switch 3 error");
        return;
    }
    CON_LOG("switch sim %d",simSwitch);
    sprintf(cmd, "switch_sim %d", simSwitch);
    system(cmd);
    sleep(30);
    save_useing_siminfo(simSwitch);
	return ;
}



