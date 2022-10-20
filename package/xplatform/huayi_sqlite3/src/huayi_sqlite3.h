

#ifndef __HUAYI_SQLITE3_HEADER__
#define __HUAYI_SQLITE3_HEADER__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdarg.h>
#include "huayi_log.h"

typedef enum{
	HUAYI_SQLITE3_FALSE,
	HUAYI_SQLITE3_TRUE,
}HUAYI_SQLITE3_BOOL;

typedef struct huayi_sqlite3{
	int column_size;
	char **column_value;
	char **column_name;
}HUAYI_SQLITE3_DATA,*P_HUAYI_SQLITE3_DATA;

/*SQLITE3 commmon*/
#define SQLITE3_DIR "/tmp/"
#define SQLITE3_LOCALHOST "localhost"
#define SQLITE3_USERNAME "root"
#define SQLITE3_PASSWORD "huayi123456"
#define SQLITE3_SOCK "/var/run/SQLITE3d.sock"
#define SQLITE3_DB "/tmp/sqlite3_data.db"

#define DEFAULT_HEART_BEAT_INTERVAL  30

#define DEFAULT_OFFLINE_HFTIMES		6

#define SMALL_BUF_SIZE	8

#define TEMP_BUF_SIZE 128

#define CMD_BUF_SIZE 256

#define MAX_BUF_SIZE 4096

#define AC_AGENT_LISTEN_PORT	60000

#define CONFIG_FLAG_LEN		4  			/*sizeof(int)*/



/*-----------------------------------------------------------------*/


/*SQLITE3 table*/
#define SQLITE3_APINFO_TABLE_NAME 				"APINFO"
#define SQLITE3_GROUP_INFO_TABLE_NAME 			"GROUP_INFO"
#define SQLITE3_RADIO0_CONFIG_TABLE_NAME 			"RADIO0_CONFIG"
#define SQLITE3_RADIO1_CONFIG_TABLE_NAME 			"RADIO1_CONFIG"
#define SQLITE3_VAP_CONFIG_NAME 					"VAP_CONFIG"
#define SQLITE3_UPGRADE_INFO_TABLE_NAME 			"UPGRADE_INFO"
#define SQLITE3_RADIO0_STATUS_TABLE_NAME 			"RADIO0_STATUS"
#define SQLITE3_RADIO1_STATUS_TABLE_NAME 			"RADIO1_STATUS"
#define SQLITE3_WLAN_STATUS_TABLE_NAME			"VAP_STATUS"


typedef enum{
	SQLITE3_APINFO_TABLE=0,
	SQLITE3_GROUP_INFO_TABLE,
	SQLITE3_RADIO0_CONFIG_TABLE,
	SQLITE3_RADIO1_CONFIG_TABLE,
	SQLITE3_VAP_CONFIG,
	SQLITE3_UPGRADE_INFO_TABLE,
	SQLITE3_RADIO0_STATUS_TABLE,
	SQLITE3_RADIO1_STATUS_TABLE,
	SQLITE3_VAP_STATUS_TABLE,
	SQLITE3_MAX_TABLE
}HUAYI_SQLITE3_TABLE;

#define TBL_ID_TONAME(id)	 \
((id == SQLITE3_APINFO_TABLE) ? SQLITE3_APINFO_TABLE_NAME : \
(id == SQLITE3_GROUP_INFO_TABLE) ? SQLITE3_GROUP_INFO_TABLE_NAME : \
(id == SQLITE3_RADIO0_CONFIG_TABLE) ? SQLITE3_RADIO0_CONFIG_TABLE_NAME : \
(id == SQLITE3_RADIO1_CONFIG_TABLE) ? SQLITE3_RADIO1_CONFIG_TABLE_NAME : \
(id == SQLITE3_VAP_CONFIG) ? SQLITE3_VAP_CONFIG_NAME : \
(id == SQLITE3_UPGRADE_INFO_TABLE) ? SQLITE3_UPGRADE_INFO_TABLE_NAME : \
(id == SQLITE3_RADIO0_STATUS_TABLE) ? SQLITE3_RADIO0_STATUS_TABLE_NAME : \
(id == SQLITE3_RADIO1_STATUS_TABLE) ? SQLITE3_RADIO1_STATUS_TABLE_NAME : \
(id == SQLITE3_VAP_STATUS_TABLE) ? SQLITE3_WLAN_STATUS_TABLE_NAME: "Unknown Talbe")

/*-----------------------------------------------------------------*/
/*SQLITE3 common def*/
#define MAX_SQLITE3_VALUE_LEN			512
#define MAX_SQLITE3_QUERY_LEN			4096
#define MAX_SQLITE3_GROUP_NAME_LEN	33 //32+1  32+\0
#define SQLITE3_MAC_LEN				6  //row byte
#define SQLITE3_MAC_STR_LEN			18 //str 17+\0
#define SQLITE3_IP_STR_LEN			16 //15+\0
#define SQLITE3_APNAME_LEN			33 //32+\0
#define SQLITE3_APMODEL_LEN			17 //16+1
#define SQLITE3_SSID_LEN				33 //32+\0
#define SQLITE3_PASSPHRASE_LEN		64 //63+\0 at least 8
#define SQLITE3_FWVERSION_LEN			10
#define SQLITE3_FILEPATH_LEN			512


#define MySetField32(val,start)  (val |=(1<<start))

#define MyClearField32(val,start) (val &=(~(1<<start)))

#define MyGetField32(val,start) ((val&(1<<start))>>start)


/*SQLITE3_APINFO_TABLE_NAME*/

#define SQLITE3_APID_KEY_NAME					"apid"
#define SQLITE3_MAC_KEY_NAME					"mac"
#define SQLITE3_IPADDR_KEY_NAME				"ipaddr"
#define SQLITE3_PUBLIC_IPADDR_KEY_NAME		"public_ip"
#define SQLITE3_NETMASK_KEY_NAME				"netmask"
#define SQLITE3_GW_KEY_NAME					"gw"
#define SQLITE3_DNS_KEY_NAME					"dns"
#define SQLITE3_MODEL_KEY_NAME				"model"
#define SQLITE3_APNAME_KEY_NAME				"apname"
#define SQLITE3_VERSION_KEY_NAME				"version"
#define SQLITE3_UPTIME_KEY_NAME				"uptime"
#define SQLITE3_RADIO_TYPE_KEY_NAME			"radio_type"
#define SQLITE3_JOINTIME_KEY_NAME				"jointime"
#define SQLITE3_GROUPID_KEY_NAME				"gid"
#define SQLITE3_REASON_KEY_NAME				"reason"
#define	SQLITE3_RUNSTATE_KEY_NAME				"runstate"

/*-----------------------------------------------------------------*/


/*SQLITE3_GROUP_INFO_TABLE_NAME*/

#ifndef SQLITE3_GROUPID_KEY_NAME
#define SQLITE3_GROUPID_KEY_NAME				"gid"
#endif
#define	SQLITE3_GROUPNAME_KEY_NAME            "groupname"
#define SQLITE3_DEFAULT_GROUP_NAME			"Default"
#define SQLITE3_DEFAULT_GROUP_ID				1

/*-----------------------------------------------------------------*/


/*SQLITE3_WIFI_CONFIG*/

#define SQLITE3_RID_KEY_NAME					"rid"
#ifndef SQLITE3_GROUPID_KEY_NAME
#define SQLITE3_GROUPID_KEY_NAME				"gid"
#endif
#define SQLITE3_IF_ADMIN_KEY_NAME				"if_admin"
#define SQLITE3_COUNTRY_KEY_NAME				"country"
#define SQLITE3_WIRELESSMODE_KEY_NAME			"wirelessmode"
#define SQLITE3_HTMODE_KEY_NAME				"htmode"
#define SQLITE3_CHANNEL_KEY_NAME				"channel"
#define SQLITE3_TXPOWER_KEY_NAME				"txpower"
#define SQLITE3_BEACON_KEY_NAME				"beacon"

#define	SQLITE3_RADIOID_NAME					"radioid"
#define SQLITE3_DEFAULT_COUNTRY				"CN"
/*-----------------------------------------------------------------*/


/*SQLITE3_VAP_CONFIG_NAME*/
#if 0 /*Operator ssid config*/
#define SQLITE3_WID_KEY_NAME					"wid"
#ifndef SQLITE3_GROUPID_KEY_NAME
#define SQLITE3_GROUPID_KEY_NAME				"gid"
#endif
#define SQLITE3_SSID_KEY_NAME					"ssid"
#define SQLITE3_HIDE_KEY_NAME					"hide"
#define SQLITE3_ISOLATE_KEY_NAME				"isolate"
#define SQLITE3_ENCRY_KEY_NAME				"encry"
#define SQLITE3_PASSPHRASE_KEY_NAME			"passphrase"
#define SQLITE3_MAXSTA_KEY_NAME				"maxsta"
#define SQLITE3_VLANID_KEY_NAME				"vlanid"
#define SQLITE3_WORKMODE_KEY_NAME				"workmode"
#else
#define SQLITE3_WID_KEY_NAME					"wid"
#ifndef SQLITE3_GROUPID_KEY_NAME
#define SQLITE3_GROUPID_KEY_NAME				"gid"
#endif
#define SQLITE3_SSID2G_KEY_NAME				"ssid_2g"
#define SQLITE3_HIDE2G_KEY_NAME				"hide_2g"
#define SQLITE3_ISOLATE2G_KEY_NAME			"isolate_2g"
#define SQLITE3_ENCRY2G_KEY_NAME				"encry_2g"
#define SQLITE3_PASSPHRASE2G_KEY_NAME			"passphrase_2g"
#define SQLITE3_MAXSTA2G_KEY_NAME				"maxsta_2g"
#define SQLITE3_VLANID2G_KEY_NAME				"vlanid_2g"

#define SQLITE3_SSID5G_KEY_NAME				"ssid_5g"
#define SQLITE3_HIDE5G_KEY_NAME				"hide_5g"
#define SQLITE3_ISOLATE5G_KEY_NAME			"isolate_5g"
#define SQLITE3_ENCRY5G_KEY_NAME				"encry_5g"
#define SQLITE3_PASSPHRASE5G_KEY_NAME			"passphrase_5g"
#define SQLITE3_MAXSTA5G_KEY_NAME				"maxsta_5g"
#define SQLITE3_VLANID5G_KEY_NAME				"vlanid_5g"


#endif


/*-----------------------------------------------------------------*/


/*SQLITE3_AP_UPGRADE*/
#define SQLITE3_FID_KEY_NAME					"fid"
#ifndef	SQLITE3_MODEL_KEY_NAME
#define SQLITE3_MODEL_KEY_NAME				"model"
#endif
#define SQLITE3_FWVERSION_KEY_NAME			"fwversion"
#define SQLITE3_VERSION_ACTIVE_KEY_NAME		"active"
#define MSYQL_FILE_KEY_NAME					"file"

/*-----------------------------------------------------------------*/
#define OPTION_HERATBEAT_FAIL				"heartbeat_fail"
#define OPTION_HERATBEAT_INTERVAL			"heartbeat_interval"
#define OPTION_GLOBAL_ROAMING				"gloabal_roaming"
#define OPTION_ROAMING_THRESHOLD_2G			"roaming_threshold_2g"
#define OPTION_ROAMING_THRESHOLD_5G			"roaming_threshold_5g"
#define OPTION_GLOBAL_AUTO_CHANNEL			"gloabal_auto_channel"

/*-----------------------------------------------------------------*/


/*-----------------------------------------------------------------*/
#define SMS_HUAYI_TABLE       "huayi_sms"

#define SQLITE3_SMSID_KEY_NAME  						"message_id"
#define SQLITE3_SMSNUMBER_KEY_NAME  					"phone_num"
#define SQLITE3_SMS_MSG_KEY_NAME						"message_content"
#define SQLITE3_SMS_COUNT_KEY_NAME					"message_count"
#define SQLITE3_SMS_UNREAD_KEY_NAME					"messages_unread"
#define SQLITE3_SMS_LASTTIME_KEY_NAME					"final_time"
#define SQLITE3_SMS_CHILD_ID_KEY_NAME					"c_message_sid"
#define SQLITE3_SMS_RECEIVE_TIME_KEY_NAME				"receive_time"
#define SQLITE3_SMS_SRC_KEY_NAME						"sms_src"
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
#define ALIAS_HUAYI_TABLE       "huayi_alias"

#define SQLITE3_ALIASID_KEY_NAME  					"sid"
#define SQLITE3_ALIAS_MAC_KEY_NAME					"alias_mac"
#define SQLITE3_ALIAS_NAME_KEY_NAME					"alias_name"
#define SQLITE3_ALIAS_CHANGE_KEY_NAME					"alias_change"


/*-----------------------------------------------------------------*/

HUAYI_SQLITE3_BOOL huayiSqlite3Open(sqlite3 ** my_con);
HUAYI_SQLITE3_BOOL huayiSqlite3Close(sqlite3 *my_con);
HUAYI_SQLITE3_BOOL huayiSqlite3KeepAlive(sqlite3 *my_con);
char *huayiSqlite3HexString(char *string,int from_len);
void huayiSqlite3ErrorLog(sqlite3 *my_con,const char *action);
HUAYI_SQLITE3_BOOL huayiSqlite3Exec(sqlite3 *my_con,const char *fmt,...);
int huayiSqlite3GetStr(sqlite3 *my_con,const char *query,char *value,int len);
void huayiSqlite3GetInt(sqlite3 *my_con,const char *query,int *value);
HUAYI_SQLITE3_BOOL huayiSqlite3GetTable(sqlite3 *my_con,char *query,unsigned long *nrow,unsigned int *ncol, void **result);
HUAYI_SQLITE3_BOOL huayiSqlite3CheckTableExist(sqlite3 *my_con,char *table_name);


#endif


