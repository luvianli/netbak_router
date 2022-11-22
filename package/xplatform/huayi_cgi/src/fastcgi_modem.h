#ifndef __FASTCGI_MODEM_HEADER__
#define __FASTCGI_MODEM_HEADER__

#include <huayi_sqlite3.h>
#include "fastcgi_common.h"
#include "huayi_log.h"
#include "huayi_atcmd.h"
#define MAX_PHONE_NUMBER_LEN		                17

#define WAN_TYPE_4G_MOBILE							"mobile"
#define WAN_TYPE_ETH_WIRED							"wired"
#define WAN_TYPE_WIRED_PRIOROTY						"prior"
#define WAN_TYPE_WIRED_PRIOROTY						"prior"
#define WAN_TYPE_REP4G_PRIOROTY						"rep4g"
#define WAN_TYPE_REP								"rep"


typedef enum
{
	HYID_GET_OVERVIEW_CONFIG				=0,
    HYID_GET_WAN_OR_WAN4G_CONFIG			=1,
    HYID_SET_WAN_OR_WAN4G_CONFIG			=2,
    HYID_GET_NEWS_SMS_STATE					=3,
    HYID_GET_BREAF_SMS_INFO					=4,
    HYID_GET_SMS_SESSION_BY_NUMBER			=5,
    HYID_DELETE_SMS_SESSION_BY_SID			=6,
    HYID_DELETE_SMS_BY_CID					=7,
    HYID_SEND_SMS_MESSAGES					=8,
    HYID_GET_PIN_STATE						=9,
    HYID_SET_SIM_LOCK_SET_PIN				=10,
    HYID_GET_PIN_CLCK						=11,
    HYID_SET_PIN_CLCK						=12,
    HYID_CHANGE_PIN_CODE					=13,
    HYID_GET_APN_CONFIG						=14,
    HYID_SET_APN_CONFIG						=15,
    HYID_GET_4G_SIGNAL						=16,
    HUID_GET_SIM_MESSAGE 					=17,
    MAX_MODEM_HYID,
}FGI_HYID_MODEM;



int (*FastCgiHandleModemFunction[MAX_MODEM_HYID])(cJSON *request_root, cJSON *response_root,int *lock_time);


/*fastcgi_modem.c*/
int cgi_get_new_sms_state(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_get_breaf_sms_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_get_sms_session_by_number(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_delete_sms_session_by_sid(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_delete_sms_by_cid(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_send_sms_messages(cJSON *request_root, cJSON *response_root,int *lock_time);

int cgi_get_module_pin_state(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_set_sim_lock_set_pin(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_get_sim_clck(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_set_pin_clck(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_change_module_pin_code(cJSON *request_root, cJSON *response_root,int *lock_time);

int cgi_get_module_apn_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_set_module_apn_config(cJSON *request_root, cJSON *response_root,int *lock_time);

int cgi_get_overview_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_get_wan_or_wan4g_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_set_wan_or_wan4g_config(cJSON *request_root, cJSON *response_root,int *lock_time);

int cgi_get_4g_signal(cJSON *request_root, cJSON *response_root,int *lock_time);

int cgi_get_dev_message(cJSON *request_root,cJSON *response_root,int *lock_time);
#endif
