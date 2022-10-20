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


void huayi_get_4g_info(char *mobile_type, int *sig, char *iccid, char *lac, char *cellid)
{
	int  signal = -121;

	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","operator", mobile_type);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","iccid",iccid);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","lac",lac);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","ci",cellid);
	atcmd_get_signal_for_cloud(&signal);
	//huayi_uci_get_int(PKG_STATUS_MODEM,"modem","signal",&signal);
	//signal = signal/5 - 120;

	*sig = signal;
	
	return;
}

void huayi_set_4g_config(cJSON *response_root)
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
}


