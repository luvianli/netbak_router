#ifndef __HUAYI_UCI_FUN_H__
#define __HUAYI_UCI_FUN_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <huayi_uci.h>
#include <cJSON.h>

typedef enum{
	HUAYI_FALSE,
	HUAYI_TRUE,
}HUAYI_BOOL;


#define XCMD( x, fmt, args... ) do {\
        sprintf( x, fmt, ##args ); \
        system( x); \
    } while( 0 )

#define SYSTEM_TIME_OUT     120

#define SMALL_STR_LEN                                    8
#define SHORT_STR_LEN                                    16
#define RESULT_STR_LEN                                   32
#define OPTION_STR_LEN                                   64
#define TEMP_STR_LEN                                     128
#define PASSWD_LONG_STR_LEN                              129
#define CMD_STR_LEN                                      256
#define LIST_STR_LEN                                     512
#define LONG_BUFF_LEN                                    1024
#define LONG_LIST_BUFF_LEN                               2048
#define LONGLONG_BUFF_LEN                                8192

#define MAX_SMS_ESCAPE_CHARACTER_LEN				4096
#define WEB_SINGLE_QUOTES_DELIMIT                   '\''


#define MAC_STR_LEN										18
#define MAC_ROW_LEN										6
#define IPV4_STR_LEN									16

#define HUAYI_ROUTE_MODE								0
#define HUAYI_AP_MODE									1
#define HUAYI_REP_MODE									2
#define HUAYI_WISP_MODE									3

#define VAP_VLAN_MAX_NUM								4
#define MBSSID_MAX_NUM									4
#define RADIO_MAX_NUM									2
#define LAN_MAX_NUM										3


#define ETH_SWITCH_PORT_NUM								5

#define WLAN_24G_REPEATER_PORT_NUM						8
#define WLAN_58G_REPEATER_PORT_NUM						9
#define WLAN_24G_WPS_PORT_NUM							10
#define WLAN_58G_WPS_PORT_NUM							11


#define WLAN_WPA_KEY_BUFF_LEN							65

#define WLAN_AUTHTYPE_WPAPSK      	0x01
#define WLAN_AUTHTYPE_WPA2PSK     	0x02
#define WLAN_AUTHTYPE_SHARED      	0x04
#define WLAN_AUTHTYPE_WPAPSKWPA2PSK 0x22


#define WLAN_ENCRTYPE_TKIP    		0x10
#define WLAN_ENCRTYPE_AES     		0x20
#define WLAN_ENCRTYPE_WEP     		0x40

#define PING_FILE                                  	        "/tmp/ping_file"
#define UPLOAD_BIN  									"/tmp/firmware.bin"
#define UPGRADE_FILE  									"/tmp/firmware.img"

#define UPLOAD_BACKUP_FILE 								"/tmp/upload_restore"
#define REAL_BACKUP_FILE 								"/tmp/restore.tar.gz"
#define CHECK_BACKUP_FILE_PATH							"/tmp/check_backup/"
#define CHECK_PATH										"/tmp/check_backup/etc/config"

#define PING_TMPFILE                                    "/tmp/.ping_success"
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
#define WISP4G_TMPFILE                                  "/tmp/.wisp_success"
#endif
#define INTERNET_DOMAIN									"www.baidu.com"
#define INTERNET_ABROAD_DOMAIN							"8.8.8.8"

#define HUAYI_NET_PROTO_DISABLE   						"disable"
#define HUAYI_NET_PROTO_STATIC   						"static"
#define HUAYI_NET_PROTO_DHCP   	 						"dhcp"
#define HUAYI_NET_PROTO_PPPOE	 						"pppoe"
#if defined(CONFIG_HUAYI_SUPPORT_VPN)
#define HUAYI_NET_PRORO_PPTP      						"pptp"
#define HUAYI_NET_PROPO_XL2TP							"l2tp"
#endif

#define HUAYI_DEFAULT_NET_MASK							"255.255.255.0"
#define ETH_DEV_BR_LAN									"br-lan"
#define ETH_DEV_WAN_IFNAME								"eth0.2"
#define PPPOE_DEV_WAN_IFNAME							"pppoe-wan"

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT)
#define RANDOM_KEY					"h7gtrgCB968tyj4n"
#define DEF_REP_W24G_SSID			"HUAYI-2.4G_rep"
#define W24G_DEV					"radio0"
#define W24G_IFACE					"radio0_0"
#define W24G_GUEST_IFACE			"radio0_1"
#define W24G_MAIN_IFNAME			"ra0"
#define W24G_MAIN_GUEST_IFNAME		"ra1"
#define W24G_REP_IFACE				"radio0_cli0"
#define W24G_REP_IFNAME				"apcli0"


#define DEF_REP_W58G_SSID			"HUAYI-5.8G_rep"
#define W58G_DEV					"radio1"
#define W58G_IFACE					"radio1_0"
#define W58G_GUEST_IFACE			"radio1_1"
#define W58G_MAIN_IFNAME			"rai0"
#define W58G_MAIN_GUEST_IFNAME		"rai1"
#define W58G_REP_IFACE				"radio1_cli0"
#define W58G_REP_IFNAME				"apclii0"

#define HUAYI_RANDOM_APCLI_NAME		"huayi_p1stdjkdsFG"
#endif

#define WIFI_NO_SCAN 	0
#define WIFI_24G_SCAN 	1
#define WIFI_58G_SCAN 	2
#define WIFI_ALL_SCAN 	3


#define SAFE_CLOSE(fd)				if(fd > 0)	{ close(fd); fd = -1; }


typedef enum
{
	HUAYI_SCHE_REBOOT_MODE_OFF,
	HUAYI_SCHE_REBOOT_MODE_REC_TIME,
	HUAYI_SCHE_REBOOT_MODE_SPE_TIME,
}HUAYI_SCHE_REBOOT_MODE;


typedef enum
{
	QOS_DISABLE_MODE 		= 0,
	QOS_SMART_MODE 			= 1,
	QOS_IPLIMIT_MODE 		= 2,
}HUAYI_QOS_MODE;


typedef enum
{
	HUAYI_ACTION_START,
	HUAYI_ACTION_DOING,
	HUAYI_ACTION_COMPLETE,
}HUAYI_ACTION_STATE;

#define MySetField32(val,start)  (val |=(1<<start))
#define MyClearField32(val,start) (val &=(~(1<<start)))
#define MyGetField32(val,start) ((val&(1<<start))>>start)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


HUAYI_BOOL huayi_uci_get_str(int title,char *section,char *option,char *value);
HUAYI_BOOL huayi_uci_get_int(int title,char *section,char *option,int *value);
HUAYI_BOOL huayi_uci_get_hex(int title,char *section,char *option,int *value);
HUAYI_BOOL huayi_uci_get_long(int title,char *section,char *option,unsigned long *value);
HUAYI_BOOL huayi_uci_get_long_long(int title,char *section,char *option,unsigned long long *value);


HUAYI_BOOL huayi_uci_set_str(int title,char *section,char *option,char *value);
HUAYI_BOOL huayi_uci_set_int(int title,char *section,char *option,int value);
HUAYI_BOOL huayi_uci_set_hex(int title,char *section,char *option,int value);
HUAYI_BOOL huayi_uci_set_long(int title,char *section,char *option,long value);
HUAYI_BOOL huayi_uci_set_long_long(int title,char *section,char *option,unsigned long long value);
HUAYI_BOOL huayi_uci_commit(int title);
HUAYI_BOOL huayi_uci_revert(int title);

HUAYI_BOOL huayi_uci_delete_option(int title,char *section, char *option);
HUAYI_BOOL huayi_uci_add_list(int title,char *section, char *option,char *value);
HUAYI_BOOL huayi_uci_del_list(int title,char *section, char *option,char *value);
HUAYI_BOOL huayi_uci_add_section(int title,char *section_type);
HUAYI_BOOL huayi_uci_add_name_section(int title,char *section, char *value);
HUAYI_BOOL huayi_uci_del_section(int title,char *section_name);

void get_ac_ip_by_hostname(char *achostname,char *acip);
void MAC17_TO_MAC6(char * MAC_17,char * MAC_6);
void MAC6_TO_MAC17(char * MAC_6,char * MAC_17);

HUAYI_BOOL get_cmd_result(char *cmd, char *resultbuf, size_t buf_size);
HUAYI_BOOL huayi_file_exists(const char *path);
HUAYI_BOOL huayi_dir_exists(const char *path);
unsigned long huayi_file_size(const char *path);

HUAYI_BOOL huayi_json_get_int(cJSON *root, char *key, int *value);
HUAYI_BOOL huayi_json_get_str(cJSON *root, char *key,char *value,unsigned int len);
HUAYI_BOOL huayi_get_nth_value(int index, char *value, char delimit, char *result, int len);
HUAYI_BOOL huayiHanldeEscapeCharacter(char *value, char delimit, char *result);

char* strrstr(const char *haystack, const char *needle);
int is_phyconnected(int portNum);

int internet_do_ping_detect(const char *host, char *ifname);

int HuayiSystem(char *command, int printFlag);

void flush_arp_list(void);

char* str2upper(char *str);
char* str2lower(char *str);
unsigned long __attribute__((stdcall)) huayi_hex_string(char *to, const char *from, unsigned long length);

void HexStrToStr(const char* source, unsigned char* dest, int sourceLen);
void Str2Hex( const char *sSrc,  char *sDest, int nSrcLen );
#endif

