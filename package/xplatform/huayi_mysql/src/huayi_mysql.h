

#ifndef __HUAYI_MYSQL_HEADER__
#define __HUAYI_MYSQL_HEADER__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <stdarg.h>
#include "huayi_log.h"

typedef enum{
	HUAYI_MYSQL_FALSE,
	HUAYI_MYSQL_TRUE,
}HUAYI_MYSQL_BOOL;

/*mysql commmon*/
#define MYSQL_DIR "/tmp/data/"
#define MYSQL_LOCALHOST "localhost"
#define MYSQL_USERNAME "root"
#define MYSQL_PASSWORD "huayi123456"
#define MYSQL_SOCK "/var/run/mysqld.sock"
#define MYSQL_DB "ac_database"

#define DEFAULT_HEART_BEAT_INTERVAL  30

#define DEFAULT_OFFLINE_HFTIMES		6

#define SMALL_BUF_SIZE	8

#define TEMP_BUF_SIZE 128

#define CMD_BUF_SIZE 256

#define MAX_BUF_SIZE 4096

#define AC_AGENT_LISTEN_PORT	60000

#define CONFIG_FLAG_LEN		4  			/*sizeof(int)*/



/*-----------------------------------------------------------------*/


/*mysql table*/
#define MYSQL_APINFO_TABLE_NAME 				"APINFO"
#define MYSQL_GROUP_INFO_TABLE_NAME 			"GROUP_INFO"
#define MYSQL_RADIO0_CONFIG_TABLE_NAME 			"RADIO0_CONFIG"
#define MYSQL_RADIO1_CONFIG_TABLE_NAME 			"RADIO1_CONFIG"
#define MYSQL_VAP_CONFIG_NAME 					"VAP_CONFIG"
#define MYSQL_UPGRADE_INFO_TABLE_NAME 			"UPGRADE_INFO"
#define MYSQL_RADIO0_STATUS_TABLE_NAME 			"RADIO0_STATUS"
#define MYSQL_RADIO1_STATUS_TABLE_NAME 			"RADIO1_STATUS"
#define MYSQL_WLAN_STATUS_TABLE_NAME			"VAP_STATUS"


typedef enum{
	MYSQL_APINFO_TABLE=0,
	MYSQL_GROUP_INFO_TABLE,
	MYSQL_RADIO0_CONFIG_TABLE,
	MYSQL_RADIO1_CONFIG_TABLE,
	MYSQL_VAP_CONFIG,
	MYSQL_UPGRADE_INFO_TABLE,
	MYSQL_RADIO0_STATUS_TABLE,
	MYSQL_RADIO1_STATUS_TABLE,
	MYSQL_VAP_STATUS_TABLE,
	MYSQL_MAX_TABLE
}HUAYI_MYSQL_TABLE;

#define TBL_ID_TONAME(id)	 \
((id == MYSQL_APINFO_TABLE) ? MYSQL_APINFO_TABLE_NAME : \
(id == MYSQL_GROUP_INFO_TABLE) ? MYSQL_GROUP_INFO_TABLE_NAME : \
(id == MYSQL_RADIO0_CONFIG_TABLE) ? MYSQL_RADIO0_CONFIG_TABLE_NAME : \
(id == MYSQL_RADIO1_CONFIG_TABLE) ? MYSQL_RADIO1_CONFIG_TABLE_NAME : \
(id == MYSQL_VAP_CONFIG) ? MYSQL_VAP_CONFIG_NAME : \
(id == MYSQL_UPGRADE_INFO_TABLE) ? MYSQL_UPGRADE_INFO_TABLE_NAME : \
(id == MYSQL_RADIO0_STATUS_TABLE) ? MYSQL_RADIO0_STATUS_TABLE_NAME : \
(id == MYSQL_RADIO1_STATUS_TABLE) ? MYSQL_RADIO1_STATUS_TABLE_NAME : \
(id == MYSQL_VAP_STATUS_TABLE) ? MYSQL_WLAN_STATUS_TABLE_NAME: "Unknown Talbe")

/*-----------------------------------------------------------------*/
/*MYSQL common def*/
#define MAX_MYSQL_VALUE_LEN			512
#define MAX_MYSQL_QUERY_LEN			4096
#define MAX_MYSQL_GROUP_NAME_LEN	33 //32+1  32+\0
#define MYSQL_MAC_LEN				6  //row byte
#define MYSQL_MAC_STR_LEN			18 //str 17+\0
#define MYSQL_IP_STR_LEN			16 //15+\0
#define MYSQL_APNAME_LEN			33 //32+\0
#define MYSQL_APMODEL_LEN			17 //16+1
#define MYSQL_SSID_LEN				33 //32+\0
#define MYSQL_PASSPHRASE_LEN		64 //63+\0 at least 8
#define MYSQL_FWVERSION_LEN			10
#define MYSQL_FILEPATH_LEN			512


#define MySetField32(val,start)  (val |=(1<<start))

#define MyClearField32(val,start) (val &=(~(1<<start)))

#define MyGetField32(val,start) ((val&(1<<start))>>start)


/*MYSQL_APINFO_TABLE_NAME*/

#define MYSQL_APID_KEY_NAME					"apid"
#define MYSQL_MAC_KEY_NAME					"mac"
#define MYSQL_IPADDR_KEY_NAME				"ipaddr"
#define MYSQL_PUBLIC_IPADDR_KEY_NAME		"public_ip"
#define MYSQL_NETMASK_KEY_NAME				"netmask"
#define MYSQL_GW_KEY_NAME					"gw"
#define MYSQL_DNS_KEY_NAME					"dns"
#define MYSQL_MODEL_KEY_NAME				"model"
#define MYSQL_APNAME_KEY_NAME				"apname"
#define MYSQL_VERSION_KEY_NAME				"version"
#define MYSQL_UPTIME_KEY_NAME				"uptime"
#define MYSQL_RADIO_TYPE_KEY_NAME			"radio_type"
#define MYSQL_JOINTIME_KEY_NAME				"jointime"
#define MYSQL_GROUPID_KEY_NAME				"gid"
#define MYSQL_REASON_KEY_NAME				"reason"
#define	MYSQL_RUNSTATE_KEY_NAME				"runstate"

/*-----------------------------------------------------------------*/


/*MYSQL_GROUP_INFO_TABLE_NAME*/

#ifndef MYSQL_GROUPID_KEY_NAME
#define MYSQL_GROUPID_KEY_NAME				"gid"
#endif
#define	MYSQL_GROUPNAME_KEY_NAME            "groupname"
#define MYSQL_DEFAULT_GROUP_NAME			"Default"
#define MYSQL_DEFAULT_GROUP_ID				1

/*-----------------------------------------------------------------*/


/*MYSQL_WIFI_CONFIG*/

#define MYSQL_RID_KEY_NAME					"rid"
#ifndef MYSQL_GROUPID_KEY_NAME
#define MYSQL_GROUPID_KEY_NAME				"gid"
#endif
#define MYSQL_IF_ADMIN_KEY_NAME				"if_admin"
#define MYSQL_COUNTRY_KEY_NAME				"country"
#define MYSQL_WIRELESSMODE_KEY_NAME			"wirelessmode"
#define MYSQL_HTMODE_KEY_NAME				"htmode"
#define MYSQL_CHANNEL_KEY_NAME				"channel"
#define MYSQL_TXPOWER_KEY_NAME				"txpower"
#define MYSQL_BEACON_KEY_NAME				"beacon"

#define	MYSQL_RADIOID_NAME					"radioid"
#define MYSQL_DEFAULT_COUNTRY				"CN"
/*-----------------------------------------------------------------*/


/*MYSQL_VAP_CONFIG_NAME*/
#if 0 /*Operator ssid config*/
#define MYSQL_WID_KEY_NAME					"wid"
#ifndef MYSQL_GROUPID_KEY_NAME
#define MYSQL_GROUPID_KEY_NAME				"gid"
#endif
#define MYSQL_SSID_KEY_NAME					"ssid"
#define MYSQL_HIDE_KEY_NAME					"hide"
#define MYSQL_ISOLATE_KEY_NAME				"isolate"
#define MYSQL_ENCRY_KEY_NAME				"encry"
#define MYSQL_PASSPHRASE_KEY_NAME			"passphrase"
#define MYSQL_MAXSTA_KEY_NAME				"maxsta"
#define MYSQL_VLANID_KEY_NAME				"vlanid"
#define MYSQL_WORKMODE_KEY_NAME				"workmode"
#else
#define MYSQL_WID_KEY_NAME					"wid"
#ifndef MYSQL_GROUPID_KEY_NAME
#define MYSQL_GROUPID_KEY_NAME				"gid"
#endif
#define MYSQL_SSID2G_KEY_NAME				"ssid_2g"
#define MYSQL_HIDE2G_KEY_NAME				"hide_2g"
#define MYSQL_ISOLATE2G_KEY_NAME			"isolate_2g"
#define MYSQL_ENCRY2G_KEY_NAME				"encry_2g"
#define MYSQL_PASSPHRASE2G_KEY_NAME			"passphrase_2g"
#define MYSQL_MAXSTA2G_KEY_NAME				"maxsta_2g"
#define MYSQL_VLANID2G_KEY_NAME				"vlanid_2g"

#define MYSQL_SSID5G_KEY_NAME				"ssid_5g"
#define MYSQL_HIDE5G_KEY_NAME				"hide_5g"
#define MYSQL_ISOLATE5G_KEY_NAME			"isolate_5g"
#define MYSQL_ENCRY5G_KEY_NAME				"encry_5g"
#define MYSQL_PASSPHRASE5G_KEY_NAME			"passphrase_5g"
#define MYSQL_MAXSTA5G_KEY_NAME				"maxsta_5g"
#define MYSQL_VLANID5G_KEY_NAME				"vlanid_5g"


#endif


/*-----------------------------------------------------------------*/


/*MYSQL_AP_UPGRADE*/
#define MYSQL_FID_KEY_NAME					"fid"
#ifndef	MYSQL_MODEL_KEY_NAME
#define MYSQL_MODEL_KEY_NAME				"model"
#endif
#define MYSQL_FWVERSION_KEY_NAME			"fwversion"
#define MYSQL_VERSION_ACTIVE_KEY_NAME		"active"
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

#define MYSQL_SMSID_KEY_NAME  						"message_id"
#define MYSQL_SMSNUMBER_KEY_NAME  					"phone_num"
#define MYSQL_SMS_MSG_KEY_NAME						"message_content"
#define MYSQL_SMS_COUNT_KEY_NAME					"message_count"
#define MYSQL_SMS_UNREAD_KEY_NAME					"messages_unread"
#define MYSQL_SMS_LASTTIME_KEY_NAME					"final_time"
#define MYSQL_SMS_CHILD_ID_KEY_NAME					"c_message_sid"
#define MYSQL_SMS_RECEIVE_TIME_KEY_NAME				"receive_time"
#define MYSQL_SMS_SRC_KEY_NAME						"messages_src"
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
#define ALIAS_HUAYI_TABLE       "huayi_alias"

#define MYSQL_ALIASID_KEY_NAME  					"sid"
#define MYSQL_ALIAS_MAC_KEY_NAME					"alias_mac"
#define MYSQL_ALIAS_NAME_KEY_NAME					"alias_name"
#define MYSQL_ALIAS_CHANGE_KEY_NAME					"alias_change"


/*-----------------------------------------------------------------*/

HUAYI_MYSQL_BOOL huayiMySqlOpen(MYSQL * my_con);
HUAYI_MYSQL_BOOL huayiMySqlClose(MYSQL *my_con);
HUAYI_MYSQL_BOOL huayiMySqlKeepAlive(MYSQL *my_con);
char *huayiMysqlHexString(char *string,int from_len);
void huayiMySqlErrorLog(MYSQL *my_con,const char *action);
HUAYI_MYSQL_BOOL huayiMySqlExec(MYSQL *my_con,const char *fmt,...);
int huayiMySqlGetStr(MYSQL *my_con,const char *query,char *value,int len);
void huayiMySqlGetInt(MYSQL *my_con,const char *query,int *value);
HUAYI_MYSQL_BOOL huayiMysqlGetTable(MYSQL *my_con,char *query,unsigned long *nrow,unsigned int *ncol, void **result);
HUAYI_MYSQL_BOOL huayiMysqlCheckTableExist(MYSQL *my_con,char *table_name);


#endif


