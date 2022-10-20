/**
 * @file vsf_control.h
 * 
 * IMBS CONTROL
 * 
 * control Module for vsf platform
 *  
 *  Created by huayi 20180223
 */

#ifndef VSF_CONTROL_H
#define VSF_CONTROL_H
#include <huayi-universal/huayi_dal.h>
#include <net/netfilter/pa_qos.h>
#include "qos_def.h"


#define VSF_MAX_STRING 	65

#define HUAYI_MODULE_FILTER_CONFIG    "filter_config"


#define VSF_MODULE_CONTROL_SYSTEM 		"system"

#define VSF_MODULE_CONTROL_APPFILTER 	"appfilter"

#define VSF_MODULE_CONTROL_GAMESPEED 	"gameSpeed"

#define VSF_MODULE_CONTROL_QOS_IPLIMIT  "qos_iplimit"

#define VSF_MODULE_CONTROL_PROTOCOL_PRIO   "proto_prio"

#define VSF_MODULE_CONTROL_P2P   "p2p"

#if 0 /*comment by huayi,no need in new platform*/
#define VSF_MODULE_CONTROL_UCI   "uci"
#endif

#define VSF_MODULE_CONTROL_STATUS   "status"

#define VSF_DEFAULT_BANDWIDTH			100 // Mbps(12.5MBps)

#define VSF_MAX_DNS_SERVER			10

typedef enum
{
	STANDBY_STATE,
	DOING_STATE,
	COMPELETE_STATE,
}ACTION_STATE;

typedef struct vsf_system_config {
    u_int8_t auto_qos;
	u_int32_t wan_up_bandwidth[WAN_CNT];
	u_int32_t wan_down_bandwidth[WAN_CNT];
	u_int8_t qos_iplimit;
	u_int8_t appfilter_enable;
	u_int8_t gameSpeed_enable;
	u_int8_t load_appConfig;
} VSF_SYSTEM_CONFIG;

typedef struct vsf_control {
    struct vsf_system_config  			system;
} VSF_CONTROL;

extern int Mbps_to_Bps(int Mbps);
extern int Mbps_to_KBps(int Mbps);
extern int Bps_to_KBps(int Bps);
extern int Bps_to_Mbps(int Bps);



#define HUAYI_FILTER_CONFIG_GET() ({ \
		struct vsf_system_config *ret = NULL; \
		dal_sys_mod_ops func = dal_get_sys_hooks(EN_HUAYI_FILTER_CONFIG); \
		if (NULL != func)  ret = (struct vsf_system_config *)(*func)(NULL,0); ret;})


#endif /* VSF_CONFIG_H */


