#ifndef __HUAYI_UCI_H__
#define __HUAYI_UCI_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <uci.h>
#include <huayi_log.h>
#include <huayi_lock.h>

/* Max UCI string length for UCI string construction */
#define MAX_UCI_STRLEN			70
#define MAX_UCI_VALUE_LEN		1024						/* Max len of the uci list */


/* Default UCI context timeout */
/* 5 secs */
#define DEFAULT_CTX_TIMEOUT     			3
/* 2 sec, the context will be reused if the last access if within this timeout. (To avoid changing view, when getting a large table) */
#define DEFAULT_CTX_LASTACCESS_TIMEOUT     	2

#define DEFAULT_UCI_CONFIG_PATH 		"/etc/config/"
#define DEFAULT_ROM_UCI_CONFIG_PATH 	"/rom/etc/config/"
#define DEFAULT_UCI_STATUS_PATH			"/proc/huayi/"
#define DEFAULT_UCI_TMP_PATH			"/tmp/"
#define DEFAULT_UCI_HUAYI_PATH			"/etc/huayi/brand_info/"

typedef enum{
	PKG_UNDEFINE = 0,
	PKG_ROM_WIRELESS,
	PKG_ABOUT,
	PKG_STATUS_SYSTEM,
	PKG_STATUS_NETWORK,
	PKG_STATUS_MODEM,
	PKG_STATUS_BLEND,
	PKG_DHCP_DETECT_RESLUT,
	PKG_STATUS_RESVER2,
	PKG_STATUS_RESVER3,
	PKG_STATUS_RESVER4,
	PKG_CAPABILITY,
	PKG_SYSTEM,
 	PKG_NETWORK,
 	PKG_DHCP,
 	PKG_WIRELESS,
 	PKG_HUAYI_FIREWALL,
 	PKG_DDNS,
 	PKG_UPNPD,
 	PKG_WPS,
 	PKG_HWNAT,
 	PKG_QOS,
 	PKG_THINAP,
 	PKG_MODEM,
 	PKG_PPTPD,
 	PKG_XL2TPD,
 	PKG_PPL2TP,
 	PKG_VPN_USER,
 	PKG_IGMP_PROXY,
 	PKG_USB_CONFIG,
 	PKG_PRINTER_CONFIG,
 	PKG_EASYCWMP,
 	PKG_HUAYI_UDHCPD,
	PKG_SCHE,
	PKG_TELNET,
	PKG_M_SYSTEM,
	PKG_DISPOSITION,
	PKG_NUM_OF_PKG,
}PKG_INDEX;

/* This definition of PKG_NUM_OF_PKG should be put at last, to count the total number of packages */

#define PKG_ABOUT_FILE				"about"
#define PKG_STATUS_SYSTEM_FILE		"status_system"
#define PKG_STATUS_NETWORK_FILE		"status_network"
#define PKG_STATUS_MODEM_FILE		"status_modem"
#define PKG_STATUS_BLEND_FILE		"status_blend"
#define PKG_DHCP_DETECT_RESLUT_FILE	"dhcp_detect_result"
#define PKG_STATUS_RESVER2_FILE		"status_resver2"
#define PKG_STATUS_RESVER3_FILE		"status_resver3"
#define PKG_SYSTEM_FILE         	"system"
#define PKG_NETWORK_FILE        	"network"
#define PKG_DHCP_FILE        		"dhcp"
#define PKG_WIRELESS_FILE			"wireless"
#define PKG_HUAYI_FIREWALL_FILE		"huayi_firewall"
#define PKG_DDNS_FILE				"ddns"
#define PKG_UPNPD_FILE				"upnpd"
#define PKG_ACCESS_FILE				"access"
#define PKG_WPS_FILE				"wps"
#define PKG_HWNAT_FILE				"hwnat"
#define PKG_QOS_FILE				"huayi_qos"
#define PKG_THINAP_FILE				"thinap"
#define PKG_MODEM_FILE	    		"huayi_modem"
#define PKG_PPTPD_FILE				"pptpd"
#define PKG_XL2TPD_FILE				"xl2tpd"
#define PKG_PPL2TP_FILE				"ppl2tp"
#define PKG_VPN_USER_FILE           "vpn_user"
#define PKG_IGMP_PROXY_FILE         "igmpproxy"
#define PKG_USB_CONFIG_FILE			"usb_config"
#define PKG_PRINTER_CONFIG_FILE		"p910nd"
#define PKG_EASYCWMP_FILE			"easycwmp"
#define PKG_HUAYI_UDHCPD_FILE		"huayi_udhcpd"
#define PKG_SCHE_FILE				"huayi_sche"
#define PKG_TELNET_FILE				"huayi_telnet"
#define PKG_M_SYSTEM_FILE			"m_system"
#define PKG_DISPOSITION_FILE		"disposition"

#define PKG_CAPABILITY_FILE			"capability"

/* HUAYI UCI Package FILE */
#define PKG_ID_TOFILE(id) \
 ((id == PKG_ABOUT) ? PKG_ABOUT_FILE : \
 (id == PKG_STATUS_SYSTEM) ? PKG_STATUS_SYSTEM_FILE : \
 (id == PKG_STATUS_NETWORK) ? PKG_STATUS_NETWORK_FILE : \
 (id == PKG_STATUS_MODEM) ? PKG_STATUS_MODEM_FILE : \
 (id == PKG_STATUS_BLEND) ? PKG_STATUS_BLEND_FILE : \
 (id == PKG_DHCP_DETECT_RESLUT) ? PKG_DHCP_DETECT_RESLUT_FILE : \
 (id == PKG_SYSTEM) ? PKG_SYSTEM_FILE : \
 (id == PKG_NETWORK) ? PKG_NETWORK_FILE : \
 (id == PKG_DHCP) ? PKG_DHCP_FILE : \
 (id == PKG_WIRELESS) ? PKG_WIRELESS_FILE:\
 (id == PKG_HUAYI_FIREWALL) ? PKG_HUAYI_FIREWALL_FILE:\
 (id == PKG_DDNS) ? PKG_DDNS_FILE:\
 (id == PKG_UPNPD) ? PKG_UPNPD_FILE:\
 (id == PKG_WPS) ? PKG_WPS_FILE:\
 (id == PKG_HWNAT) ? PKG_HWNAT_FILE:\
 (id ==	PKG_CAPABILITY) ? PKG_CAPABILITY_FILE: \
 (id ==	PKG_TELNET) ? PKG_TELNET_FILE: \
 (id == PKG_THINAP) ?  PKG_THINAP_FILE: \
 (id == PKG_MODEM) ?  PKG_MODEM_FILE: \
 (id ==	PKG_PPTPD) ? PKG_PPTPD_FILE: \
 (id ==	PKG_XL2TPD) ? PKG_XL2TPD_FILE: \
 (id ==	PKG_PPL2TP) ? PKG_PPL2TP_FILE: \
 (id ==	PKG_VPN_USER) ? PKG_VPN_USER_FILE: \
 (id ==	PKG_IGMP_PROXY) ? PKG_IGMP_PROXY_FILE: \
 (id ==	PKG_USB_CONFIG) ? PKG_USB_CONFIG_FILE: \
 (id ==	PKG_PRINTER_CONFIG) ? PKG_PRINTER_CONFIG_FILE: \
 (id == PKG_QOS) ? PKG_QOS_FILE: \
 (id == PKG_HUAYI_UDHCPD) ? PKG_HUAYI_UDHCPD_FILE: \
 (id ==  PKG_SCHE) ?  PKG_SCHE_FILE: \
 (id == PKG_M_SYSTEM) ?  PKG_M_SYSTEM_FILE: \
 (id == PKG_DISPOSITION) ?  PKG_DISPOSITION_FILE: \
 (id == PKG_EASYCWMP) ? PKG_EASYCWMP_FILE:"Unknown ID")

 /* HUAYI UCI Package File Path, DFEAULT_UCI_CONFIG_PATH is the default if it is not specified */
#define PKG_FILE_PATH(id) \
 ((id == PKG_ABOUT)? DEFAULT_UCI_STATUS_PATH:\
 (id == PKG_STATUS_SYSTEM)? DEFAULT_UCI_STATUS_PATH:\
 (id == PKG_STATUS_NETWORK)? DEFAULT_UCI_STATUS_PATH:\
 (id == PKG_STATUS_MODEM)? DEFAULT_UCI_STATUS_PATH:\
 (id == PKG_STATUS_BLEND)? DEFAULT_UCI_STATUS_PATH:\
 (id == PKG_DHCP_DETECT_RESLUT)? DEFAULT_UCI_TMP_PATH:\
 (id == PKG_CAPABILITY)? DEFAULT_UCI_HUAYI_PATH:\
   DEFAULT_UCI_CONFIG_PATH)
 
/* Status action should not be cached */
#define IS_PKG_CAN_CACHE(id) ((id == PKG_UNDEFINE)? 0 : 1)


struct huayi_uci_get_context {
	unsigned char package;
    struct uci_context* get_uci_context;
    a_mutex_t get_ctx_lock;
	long ctx_lastupdate;
	long ctx_lastaccess;
};

/* Function declaration */

/* Initialize HUAYI UCI, including cache */
void huayi_uci_init(void);
/* Release all UCI resources */
void huayi_uci_release(void);

/* Cleanup the UCI context cacahe */
/* @package UCI package ID, 0 to clean all cache */
/* @clean_useless_only Only clean the context that are useless (both timed out and not being accessed for a timeout period) */
void huayi_uci_clean_uci_context_cache(unsigned char package, bool clean_useless_only);

/**
 * Get the UCI context
 * The ctx cache will be  refreshed after timeout (DEFAULT_CTX_TIMEOUT, 5s)
 * @package UCI package ID (defined in huayi_uci.h)
 * @path UCI config file location, pass NULL to use default
 */
struct huayi_uci_get_context* huayi_uci_get_uci_context(unsigned char package, const char* path);

/**
 * Deprecated, please use huayi_uci_get_uci_context.
 * Get the UCI context WITHOUT the last access protection
 * The cache will be refreshed regardless of the DEFAULT_CTX_LASTACCESS_TIMEOUT, 
 * so that ctx cache will be  refreshed after timeout (DEFAULT_CTX_TIMEOUT, 5s)
 * @package UCI package ID (defined in huayi_uci.h)
 * @path UCI config file location, pass NULL to use default
 */
struct huayi_uci_get_context* huayi_uci_get_uci_context_nla(unsigned char package, const char* path);

/**
 * Get the UCI context WITH the last access protection, it will be useful, if you need
 * to get the same snapshot in a consecutive api calls (e.g. get association list)
 * The cache will be refreshed only if the cache is timed out (DEFAULT_CTX_TIMEOUT), 
 * and the last cache access time is long than the DEFAULT_CTX_LASTACCESS_TIMEOUT 
 * @package UCI package ID (defined in huayi_uci.h)
 * @path UCI config file location, pass NULL to use default
 */
struct huayi_uci_get_context* huayi_uci_get_uci_context_lap(unsigned char package, const char* path);

/**
 * Get the uci option using the uci string
 * If the uci option is a list, a concatenated string will be returned with the space as the delimiter
 * It is caller's responsibility to ensure the result has enough space to carry result
 * @ctx  		UCI context
 * @uci_str  	UCI option string
 * @result		UCI result value string
 * @return 0, if failed
 */
int huayi_uci_get_option(struct huayi_uci_get_context *ctx,  char *uci_str, char *result);


/**
 * Check if the item in uci list
 * @ctx  		UCI context
 * @uci_str  	UCI option string
 * @cur_str	    Current item, null if want to get the first item
 * @case_sensitive Ignore case, if the case_sensitive is set to false
 * @return 0, if not found
 */
int huayi_uci_is_list_item_found(struct huayi_uci_get_context *ctx,  char *uci_str, char *cur_str, bool case_sensitive);

/**
 * Get the next item in uci list
 * It is caller's responsibility to ensure the result has enough space to carry result
 * @ctx  		UCI context
 * @uci_str  	UCI option string
 * @cur_str	    Current item, null if want to get the first item
 * @case_sensitive Ignore case, if the case_sensitive is set to false 
 * @result		UCI result value string
 * @return 0, if failed
 */
int huayi_uci_get_list_next(struct huayi_uci_get_context *ctx,  char *uci_str, char *cur_str, bool case_sensitive, char *result);

#endif


