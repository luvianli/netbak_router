#ifndef __FASTCGI_COMMON_HEADER__
#define __FASTCGI_COMMON_HEADER__
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
//#include "fcgi_stdio.h"
#include <arpa/inet.h>
#include <uci.h>
#include "huayi_log.h"
#include "huayi_uci_fun.h"

#include "huayi_config_common.h"

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
#include "huayi_atcmd.h"
#include "fastcgi_modem.h"
#endif

#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
#include <huayi_mysql.h>
#endif
#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
#include <huayi_sqlite3.h>
#endif


#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <dirent.h>

#define MAX_WEB_SESSION_NUM  5
#endif

#define FCGI_printf printf 
#define FCGI_fwrite fwrite

#define FASTCGI_MAX_RES_MSG_LEN 64

#define FASTCGI_UPDATE_INFO_FLAG 	65535
#define FASTCGI_UPDATE_INFO_INTERVAL 	5


#define ETH_SPEED_ADDR		0x3008
#define ETH_SPEED_OFFSET	0x0100
typedef struct eth_speed_info
{
	int link;
	int duplex_mode;
	int speed;
}ETH_SPEED_INFO;


typedef enum
{
	FCGI_FALSE = 0,
	FCGI_TRUE = 1,
}FCGI_BOOL;

typedef enum
{
	FCGI_FREE = 0,
	FCGI_CONFLICT = 1,
	FCGI_DHCP = 2,
}FCGI_ARP;


typedef enum
{	
	FCGI_RESP_CODE_SUCCESS 							= 2000,
	FCGI_RESP_CODE_UNAUTHORIZED 					= 4001,
	FCGI_RESP_CODE_DATA_FORMAT_ERROR 				= 4002,
	FCGI_RESP_CODE_HYID_MISS						= 4003,
	FCGI_RESP_CODE_ILLEGAL_HYID 					= 4004,
	FCGI_RESP_CODE_DATA_MISS						= 4005,
	FCGI_RESP_CODE_PERMISSION_DENY 					= 4006,
	FCGI_RESP_CODE_SERVER_FAILURE 					= 4007,
	FCGI_RESP_CODE_FILE_FORMAT_ERROR				= 4008,
	FCGI_RESP_CODE_DULICATE_DATA					= 4009,
	FCGI_RESP_CODE_DATA_CONTENT_ERROR				= 4010,
	FCGI_RESP_CODE_LOGIN_LIMIT						= 4011,
	FCGI_RESP_CODE_PIN_CODE_ERROR					= 4012,
}FCGI_RESP_CODE;

typedef enum {
	FCGI_CONFIG_OK,
	FCGI_CONFIG_ERROR
}FCGI_CONFIG_STATUS;


typedef struct fcgi_response_array
{
	FCGI_RESP_CODE restcode;
	char errmsg[FASTCGI_MAX_RES_MSG_LEN];
}FCGI_RESPONSE_ARRAY;


/*please do not put your config macro in this enum*/
typedef enum
{
	HYID_GET_PRE_LOGIN_CONFIG,				         	/*= 0*/
	HYID_SET_PRE_LOGIN_CONFIG,						 	/*= 1*/
	HYID_SET_SYSTEM_REBOOT,								/*=	2*/
	HYID_GET_LANGUAGE_CONFIG,						 	/*= 3*/
	HYID_SET_LANGUAGE_CONFIG,						 	/*= 4*/
	HYID_DO_LOGIN_AUTH,       						 	/*= 5*/
	HYID_DO_LOGOUT_CLEAN,						     	/*= 6*/
	HYID_GET_PASSWORD_STATE,							/*= 7*/
	HYID_CHANGE_ADMIN_PASSWORD,							/*= 8*/
	HYID_GET_QUICK_SETTING_CONFIG,						/*= 9*/
	HYID_SET_QUICK_SETTING_CONFIG,						/*= 10*/
	/*11 -20 变成 21 -30*/
	HYID_SET_ETH_WAN_CONFIG,
	HYID_GET_STATIC_ROUTER_CONFIG,					
	HYID_SET_STATIC_ROUTER_CONFIG,							
	HYID_GET_REMOTE_MANAGE_CONFIG,			
	HYID_SET_REMOTE_MANAGE_CONFIG,						/*=15*/	
	HYID_GET_MAC_FILTER_CONFIG,		
	HYID_SET_MAC_FILTER_CONFIG,						
	HYID_GET_URL_FILTER_CONFIG,										
	HYID_SET_URL_FILTER_CONFIG,			
	HYID_GET_IP_PORT_FILTER_CONFIG,						/*=20*/
	HYID_SET_SYSTEM_FACTORY,					
	HYID_GET_UPGRADE_INFO,					
	HYID_GET_CURRETN_USER_INFO,									
	HYID_GET_LAN_ARP_LIST,					
	HYID_GET_NETWORK_INFO,								/*=25*/								
	HYID_GET_DHCPS_SERVER_CONFIG,				
	HYID_SET_DHCPS_SERVER_CONFIG,
	HYID_GET_LAN_CONFIG,												
	HYID_SET_LAN_CONFIG,
	HYID_GET_WAN_CONFIG,								/*=30*/
	/*31 - 50 变成51-70*/
	HYID_GET_DHCP_DETECT_RESULT,				
	HYID_DO_DHCP_DETECT,			
	HYID_STOP_DHCP_DETECT,								
	HYID_GET_NTP_CONFIG,  	  			
	HYID_SET_NTP_CONFIG,								/*=35*/
	HYID_USE_BROWSER_TIME,					
	HYID_GET_SYSLOG_CONFIG,												
	HYID_SET_SYSLOG_CONFIG,																				
	HYID_DISPLAY_SYSLOG,				
	HYID_CLEAN_SYSLOG,									/*=40*/
	HYID_GET_SCHE_REBOOT_CONFIG,				
	HYID_SET_SCHE_REBOOT_CONFIG,						
	HYID_GET_HWNAT_CONFIG,											
	HYID_SET_HWNAT_CONFIG,	
	HYID_GET_SYSTEM_INFO,								/*=45*/
	HYID_GET_ALLSTALIST,
	HYID_GET_QOS_IPLIMIT_CONFIG,
	HYID_SET_QOS_IPLIMIT_CONFIG,						
	HYID_GET_WIFI_BASIC_CONFIG,
	HYID_SET_WIFI_BASIC_CONFIG,							/*=50*/
	HYID_SET_IP_PORT_FILTER_CONFIG,		
	HYID_GET_DMZ_CONFIG,							
	HYID_SET_DMZ_CONFIG,											
	HYID_GET_PORT_FORWARD_CONFIG,			
	HYID_SET_PORT_FORWARD_CONFIG,						/*=55*/
	HYID_GET_UPNPD_CONFIG,			
	HYID_SET_UPNPD_CONFIG,							
	HYID_GET_WAN_PING_CONFIG,										
	HYID_SET_WAN_PING_CONFIG,
	HYID_GET_ACL_LIST,                   				/*=60*/
	HYID_ADD_ACL_LIST,                     			
	HYID_DEL_ACL_LIST,
	HYID_GET_DDNS_CONFIG,																		
	HYID_SET_DDNS_CONFIG,									
	HYID_GET_PING_DETECT_RESULT,						/*=65*/				   		
	HYID_DO_PING_DETECT,								
	HYID_STOP_PING_DETECT,					
	HYID_GET_TRACERT_DETECT_RESULT,							
	HYID_DO_TRACERT_DETECT,		
	HYID_STOP_TRACERT_DETECT,							/*=70*/	
	HYID_GET_OVERVIEW_INFO,	
	
	HYID_GET_WIFI_ADVANCE								= 83, 		                
	HYID_SET_WIFI_ADVANCE,       
	/*75到84变成85到94*/
	HYID_DO_WAN_PROTO_DETECT,							/*=85*/
	HYID_SET_WIFI_SCAN,									
	HYID_GET_WIFI_SCAN_STATUS,
	HYID_GET_WIFI_SCAN_LIST,							
	HYID_GET_NETWORK_CONFIG,
	HYID_SET_NETWORK_CONFIG,							/*=90*/
	HYID_GET_SYSMODE_PAGE_INFO,
	HYID_GET_BASIC_PAGE_ALL_INFO,
	HYID_GET_BASIC_PAGE_STATUS_INFO,					
	HYID_SET_BASIC_PAGE_WIFI_CONFIG,
	HYID_GET_WIFI_MACFILTERLIST,      					/*=95*/   
	HYID_SET_WIFI_MACFILTERLIST,							
	HYID_GET_WIFI_STAMACLIST,
	HYID_GET_WFII_MBSSID, 								               
	HYID_SET_WIFI_MBSSID,               
	HYID_GET_WIFI_VAP_VLAN,  							/*=100*/           
	HYID_SET_WIFI_VAP_VLAN,                                  	
	HYID_GET_SYSTEM_OPMODE,
	HYID_SET_SYSTEM_OPMODE,															
	HYID_GET_GATEWAY_MODE_CONFIG,
	HYID_GET_WIFI_EASY_CONFIG,							/*=105*/
	HYID_SET_WIFI_EASY_CONFIG,
	HYID_GET_PAGE_SYSTEM_STATUS,												
	HYID_GET_PAGE_NETWORK_STATUS,											
	HYID_GET_WIFI_SCHE_CONFIG,												
	HYID_SET_WIFI_SCHE_CONFIG,							/*=110*/	
	HYID_GET_THINAP_CONFIG,	
	HYID_SET_THINAP_CONFIG,								/*=112*/
	HYID_GET_ALL_QUICK_SETTING_CONFIG,					/*=113*/
	HYID_SET_ALL_QUICK_SETTING_CONFIG,					/*=114*/
	HYID_GET_WIRLESS_GUEST_CONFIG,						/*=115*/
	HYID_SET_WIRLESS_GUEST_CONFIG,						/*=116*/
	HYID_GET_INTERNET_STATUS,                       	/*=117*/
	HYID_GET_DEVICE_INFO,
	HYID_SET_OPTIMIZE,
	HYID_GET_HOE_DEVICES,								/*=120*/
	HYID_SET_BLOCK,
	HYID_SET_DEVICE_NAME,
	HYID_SET_WIRELESS_CLOCK,
	HYID_SET_GUEST_CLOCK,
	HYID_GET_WIFI_WPS_CONFIG,							/*=125*/
	HYID_SET_WIFI_WPS_CONFIG,
	HYID_SET_WIFI_WPS_PBC,
	HYID_SET_WIFI_WPS_PIN,
	HYID_GET_WIFI_WPS_LOCAL_PIN,
	HYID_SET_WIFI_WPS_FIRST_PIN,
	HYID_GET_WIFI_WPS_STATUS,
	HYID_SET_WIFI_WPS_PIN_CONFIG,
	HYID_SET_WIFI_WPS_RANDOM_PIN,						/*=133*/
	/*以下cgid变化set到get get到set*/
	HYID_SET_SYSTEM_MODE_CONFIG			= 154,
	HYID_GET_SYSTEM_MODE_CONFIG			= 155,
	HYID_SET_WIRELESS_STATUS			= 156,
	HYID_GET_WIRELESS_STATUS			= 157,							/*=127*/
	HYID_SET_PPTPD_CONFIG				= 158,
	HYID_GET_PPTPD_CONFIG				= 159,
	HYID_SET_XL2TPD_CONFIG              = 160,
	HYID_GET_XL2TPD_CONFIG              = 161,
	HYID_SET_PPTP_CONFIG				= 162,
	HYID_GET_PPTP_CONFIG				= 163,
	HYID_SET_L2TP_CONFIG 				= 164,
	HYID_GET_L2TP_CONFIG 				= 165,
	HYID_SET_VPN_USER_CONFIG			= 166,
	HYID_GET_VPN_USER_CONFIG			= 167,
	HYID_GET_VPN_USER_STATUS			= 168,
	HYID_SET_IPV6_CONFIG				= 169,
	HYID_GET_IPV6_CONFIG				= 170,							/*140*/
	HYID_DO_KICK_VPN_USER 				= 171,
	HYID_SET_MULTICAST_CONFIG 			= 172,
	HYID_GET_MULTICAST_CONFIG 			= 173,
	HYID_SET_FTP_CONFIG 				= 176,
	HYID_GET_FTP_CONFIG 				= 177,
	HYID_SET_PRINTER_CONFIG				= 178,
	HYID_GET_PRINTER_CONFIG 			= 179,
	HYID_SET_TR069_CONFIG 				= 180,
	HYID_GET_TR069_CONFIG 				= 181,
	HYID_SET_FORCE_FIX_DHCP 			= 182,
	HYID_GET_FORCE_FIX_DHCP 			= 183,
	HYID_SET_SKIP_QUICK_SETTING 		= 185,
	HYID_GET_WANMODE_INFO				= 186,
	HYID_GET_OPENVPN_CONFIG             = 191,
	HYID_SET_OPENVPN_CONFIG             = 192,
	MAX_HUAYI_HYID,
}FCGI_HYID_ENUM;


#define WEB_SPACE_DELIMIT								' '
#define WEB_RETURN_DATA_NAME							"huayi_r"

#define HUAYI_SYSTEM_UPGRADE_TIME						150
#define HUAYI_SYSTEM_RESET_TIME							120
#define HUAYI_SYSTEM_REBOOT_TIME						90
#define HUAYI_SYSTEM_CHANGE_MODE_TIME					80
#define HUAYI_SYSTEM_ROUTE_MODE_TIME					100
#define HUAYI_SYSTEM_REP_MODE_TIME					    90
#define HUAYI_MODULE_UPGRADE_TIME						100
#define HUAYI_RELOAD_NETWORK_TIME						50
#define HUAYI_RELOAD_WIRELESS_TIME                      30
#define HUAYI_RELOAD_VPN_SERVER_TIME                    10
#define HUAYI_WPS_ENABLE_TIME                           5
#define HUAYI_USB_ENABLE_TIME                           5
#define HUAYI_RELOAD_4G_MODE							75
#define HUAYI_SET_CARD									30
#define HUAYI_SET_XL2TPD_IPSEC							10



#define HUAYI_DEFAULT_DHCP_START						1
#define HUAYI_DEFAULT_DHCP_LIMIT						254

#define HUAYI_PRDY_MIDDLE_NUMBER						1594

#define WEB_DEFAULT_DELIMIT								';'

#define WEB_SESS_COOKIE_PATH "/tmp/web_cookies"
#define WEB_RELEASE_TIME 300


#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
/*fastcgi_security.c*/
extern const char *publicKey;
extern const char *privateKey;

void *rsa_encryption(char *plaintext);
char *rsa_decrypt(void *ciphertext);
FCGI_BOOL web_cookie_init(void);
FCGI_BOOL get_cookie_from_session(char cookie_sess_id[]);
FCGI_BOOL check_session_active(char *checkip, char *cookie_sess_id);
FCGI_RESP_CODE check_landing_status(void);
int get_login_users(void);
FCGI_BOOL add_session_login(char *sess_id_set);
FCGI_BOOL find_login_user(char *sess_id_set);
FCGI_BOOL session_logout(char *remoteip);
int base64_encode(char *in_str, int in_len, char *out_str);
int base64_decode(char *in_str, int in_len, char *out_str);
#endif

/*fastcgi_main.c*/
FCGI_BOOL FastCgiHandleFunctionInit(void);

/*fastcgi_common.c*/
void fast_cgi_response_web(cJSON *response_root,int rest_code,int lock_time);
FCGI_BOOL huayi_add_switch_to_json(cJSON *root,char *name,int switch_value);
FCGI_BOOL huayi_add_file_to_json(cJSON *root,char *name,char *read_file);
FCGI_BOOL get_interface_list(cJSON *result_root);
FCGI_BOOL is_mac_in_acl(char *mac_address);
FCGI_BOOL get_active_arp(int type,cJSON *stalist_array,int *number);
int set_wireless_main_def(void);
char *get_wireless_config_authmode(char *authmode);
char *get_wireless_config_encryptype(char *encryptype);
char *get_wireless_config_encryption(char *authmode,char *encryptype);
void get_current_time(char *tmpbuf);
FCGI_BOOL set_interface_static_config(cJSON *request_root,char *iface);
FCGI_BOOL set_interface_dhcp_config(cJSON *request_root,char *iface);
FCGI_BOOL set_interface_pppoe_config(cJSON *request_root,char *iface);
int huayi_generate_randomname(void);
void get_hostname(char *host_name, char *macaddr);
int get_eth_info(struct eth_speed_info *eth, int addr, int offset);
int check_ipmac_in_dhcplease(char *arp_ip, char *macaddr);







/*fastcgi_system.c*/
int get_pre_login_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_pre_login_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_language_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_language_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int do_login_auth(cJSON *request_root, cJSON *response_root,int *lock_time);
int do_logout_clean(cJSON *request_root, cJSON *response_root,int *lock_time);
int change_admin_password(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_quick_setting_config(cJSON *request_root, cJSON *response_root,int *lock_time);	
int set_quick_setting_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_system_reboot(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_system_factory(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_upgrade_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_ping_detect_result(cJSON *request_root, cJSON *response_root,int *lock_time);
int do_ping_detect(cJSON *request_root, cJSON *response_root,int *lock_time);
int stop_ping_detect(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_tracert_detect_result(cJSON *request_root, cJSON *response_root,int *lock_time);
int do_tracert_detect(cJSON *request_root, cJSON *response_root,int *lock_time);
int stop_tracert_detect(cJSON *request_root, cJSON *response_root,int *lock_time);
#if defined(CONFIG_PACKAGE_HUAYI_DHCP_DETECT)
int get_dhcp_detect_result(cJSON *request_root, cJSON *response_root,int *lock_time);
int do_dhcp_detect(cJSON *request_root, cJSON *response_root,int *lock_time);
int stop_dhcp_detect(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif
#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
int get_ddns_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_ddns_config(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif
int get_ntp_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_ntp_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int use_browser_time(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_syslog_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_syslog_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int display_syslog(cJSON *request_root, cJSON *response_root,int *lock_time);
int clean_syslog(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_sche_reboot_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_sche_reboot_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_system_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_overview_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_passwd_state(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_opmode_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_all_system_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_all_network_info(cJSON *request_root, cJSON *response_root,int *lock_time);
#if defined(CONFIG_PACKAGE_HUAYI_CAPWAP_CLIENT)
int get_thinap_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_thinap_config(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif
#if defined(CONFIG_HUAYI_CGI_WIZARD_SUPPORT)
int get_all_quick_setting_config(cJSON *request_root, cJSON *response_root,int *lock_time);	
int set_all_quick_setting_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_skip_quick_setting(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif


/*fastcgi_network.c*/
int get_multiwan_static_config_by_index(cJSON *wan_obj,char *section);
int set_multiwan_static_config_by_index(cJSON *wan_obj,char *section);
int	get_multiwan_pppoe_config_by_index(cJSON *wan_obj,char *section);
int	set_multiwan_pppoe_config_by_index(cJSON *wan_obj,char *section);
int	get_multiwan_dhcp_config_by_index(cJSON *wan_obj,char *section);
int	set_multiwan_dhcp_config_by_index(cJSON *wan_obj,char *section);
int quick_set_lan_config(cJSON *request_root, cJSON * response_root);

int get_current_user_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_arp_list(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_network_info(cJSON *request_root, cJSON *response_root,int *lock_time);
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
int get_wan_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wan_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_static_router_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_static_router_config(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif
int get_lan_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_lan_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_dhcps_server_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_dhcps_server_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_hwnat_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_hwnat_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_allstalist(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_system_opmode(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_system_opmode(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_gateway_mode_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int do_wan_proto_detect(cJSON *request_root, cJSON *response_root,int *lock_time);
int do_get_quick_setting_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int do_set_quick_setting_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_network_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_network_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_multicast_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_multicast_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_ftp_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_ftp_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_printer_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_printer_config(cJSON *request_root, cJSON *response_root,int *lock_time);
#if defined(CONFIG_HUAYI_SUPPORT_REPEATER_DHCPD)
int get_force_fix_dhcp(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_force_fix_dhcp(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif
int get_wanmode_info(cJSON *request_root, cJSON *response_root,int *lock_time);

/*fastcgi_wireless.c*/
int get_wifi_stamacnum(char *vap_ifname, int radio_id, unsigned long *stanum);
int get_wifi_easyconf(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_easyconf(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_sigmode(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_sigmode(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_basic_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_basic_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_advance(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_advance(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_macfilterlist(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_macfilterlist(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_stamaclist(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_mbssid_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_mbssid_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_scan_start(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_scanstatus(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_scanlist(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_sche_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_sche_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_wps(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_wps(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_wps_pbc(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_wpsstapin(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_wpspin(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_wpsfirstpin(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_wps_ownpin(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_wpsrandompin(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wifi_wpsstatus(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_guest_network(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_guest_network(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wireless_status(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wireless_status(cJSON *request_root, cJSON *response_root,int *lock_time);
#if defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7620) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7628) || defined(CONFIG_PACKAGE_HUAYI_VLAN_MTK7621)
int get_wifi_vlan_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wifi_vlan_config(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif
#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
#define IPV6_ADDR_LINKLOCAL 0x0020U
#define IPV6_ADDR_GLOBAL 0x0000U

int get_ipv6_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_ipv6_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_ipv6addr(char *addr6, char *iface, int addr_type);
int disable_ip6();
int enable_ip6();
int is_validipv6(const char *hostname);
int ipv6_reduce(char *str);

#endif

/*fastcgi_firewall.c*/
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
int get_remote_manage_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_remote_manage_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_mac_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_mac_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_url_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_url_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_ip_port_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_ip_port_filter_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_dmz_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_dmz_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_port_forward_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_port_forward_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_upnpd_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_upnpd_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_wan_ping_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wan_ping_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_acl_list(cJSON *request_root, cJSON *response_root,int *lock_time);
int add_acl_list(cJSON *request_root, cJSON *response_root,int *lock_time);
int delete_acl_list(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_qos_iplimit_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_qos_iplimit_config(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif


/*fastcgi_basic.c*/
int get_basic_page_status_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_basic_page_all_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_basic_page_wifi_config(cJSON *request_root, cJSON *response_root,int *lock_time);

int get_internet_status(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_device_info(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_optimize(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_device_name(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_wireless_clock(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_guest_clock(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_home_devices(cJSON *request_root, cJSON *response_root,int *lock_time);

#ifdef CONFIG_PACKAGE_EASYCWMP
int get_tr069_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_tr069_config(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif

/*fastcgi_vpn.c*/
#if defined(CONFIG_HUAYI_SUPPORT_VPN)
int get_pptpd_server_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_pptpd_server_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_xl2tpd_server_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_xl2tpd_server_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_pptp_client_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_pptp_client_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_l2tp_client_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_l2tp_client_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_vpn_user_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_vpn_user_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int get_vpn_user_status(cJSON *request_root, cJSON *response_root, int *lock_time);
FCGI_BOOL get_vpn_type_list(cJSON *result_root);
int do_kick_vpn_user(cJSON *request_root, cJSON *response_root, int *lock_time);
int get_openvpn_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int set_openvpn_config(cJSON *request_root, cJSON *response_root, int *lock_time);

#endif
#endif
