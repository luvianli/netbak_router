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


void huayi_get_network_config(char *gatewayip)
{
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",gatewayip);
		
	return;
}

void huayi_get_qos_config(long speeddown, long speedup)
{
	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed",&speedup);
	huayi_uci_get_int(PKG_QOS,"qos","total_dwspeed",&speeddown);
		
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

