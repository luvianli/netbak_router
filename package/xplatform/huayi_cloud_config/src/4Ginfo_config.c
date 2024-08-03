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
	int sim_slot = 0,lock = 0,simSwitch = 0,Simnum = 0,sim = 0;
	char usesiminfoBuf[1024] = {0},siminfoBuf[512] = {0},cmd[OPTION_STR_LEN]={0},tmp[OPTION_STR_LEN]={0};
	char iccid[OPTION_STR_LEN]={0};
	char product_model[OPTION_STR_LEN] = {0};

	if(NULL == response_root) 
	{
		return ;
	}

	if(!huayi_json_get_int(response_root, "simSwitch", &simSwitch)) 
	{
		return;
	}

	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	if(!strcmp(product_model, "ZC-GW01-2"))
	{
		CustomCommunicationClient("echo `uci get m_system.usesiminfo.card_slot;uci get m_system.usesiminfo.lock;uci get m_system.system.simnum;`",usesiminfoBuf);
		if(!(toParsing(usesiminfoBuf,3,&sim_slot,&lock,&Simnum))){
	        CLOUD_CLIENT_LOG("usesiminfoBuf error\n");
			return ;
	     }

		if(simSwitch > 0 && simSwitch <= Simnum && lock == 0 && simSwitch != sim_slot)
		{
			if(simSwitch == 4)
			{
				CustomCommunicationClient("echo `uci get m_system.sim4.iccid`",siminfoBuf);
				if(!(toParsing(siminfoBuf,1,iccid))){
	        		CLOUD_CLIENT_LOG("get sim4 error\n");
					return ;
	     		}
				if (strlen(iccid) < 5 || !isStringValid(iccid)) {
	            	CLOUD_CLIENT_LOG("sim4 iccid Invalid");
	            	return ;
	        	} 
			}
			snprintf(cmd,OPTION_STR_LEN,"custom_switch_sim %d %d &",simSwitch,lock);
			huayi_uci_set_str(PKG_SYSTEM,"mysocket","send",cmd);
			huayi_uci_commit(PKG_SYSTEM);
			system("huayi-reset switch_sim");
		}else CLOUD_CLIENT_LOG("simSwitch error\n");
	}
	else if(!strcmp(product_model, "ZC-GW01-3"))
	{
		huayi_uci_get_int(PKG_DISPOSITION, "system", "eSimSUM", &Simnum);
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &sim_slot);
		if(simSwitch >=0 && simSwitch < Simnum && simSwitch != sim_slot)
		{
			CLOUD_CLIENT_LOG("switch sim%d\n",simSwitch);
			sprintf(cmd, "eSimSlot%d", simSwitch);
			huayi_uci_get_int(PKG_DISPOSITION, "system", cmd, &sim);
			sprintf(tmp, "switch_sim %d", sim);
			system("killall switch_sim");
	        system(tmp);
			save_useing_siminfo(sim);
		}else CLOUD_CLIENT_LOG("simSwitch is error or using\n");
	}

	return ;
}



