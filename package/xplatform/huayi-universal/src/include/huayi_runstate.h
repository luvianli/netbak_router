#ifndef __HUAYI_STATUS_H__
#define __HUAYI_STATUS_H__
#include <huayi-universal/huayi_dal.h>

#define HUAYI_MODULE_STATUS_ABOUT  			"about"
#define HUAYI_MODULE_STATUS_SYSTEM 			"system"
#define HUAYI_MODULE_STATUS_NETWORK     	"network"
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
#define HUAYI_MODULE_STATUS_MODEM     		"modem"
#endif
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
#define HUAYI_MODULE_STATUS_BLEND     		"blend"
#endif


#define CSTATUS_MAX_STRING 65
#define CSTATUS_MAX_DESCRIPTION 256

#define CSTATUS_MAX_BUF    512 /* huayi_cmd */


#define CSTATUS_MAX_DEVICE_ID  10  /* DAL_MAX_DEVICE_ID */
#define CSTATUS_MAX_IFNAME     16  /* DAL_MAX_IFNAME */
#define CSTATUS_MAX_DNS_SERVER 10

#define CSTATUS_MACADDR_LEN    6   /* DAL_MACADDR_LEN */

#define CSTATUS_MAX_IPV6_ADDR 10
#define CSTATUS_MAX_IPV6_ADDR_LEN 48

#define CSTATUS_MAX_IPV6_DNS_ADDR 10


#define DEFAULT_IPV6_ADDR "0:0:0:0:0:0:0:0"

typedef struct huayi_status_about {
	char system_type[CSTATUS_MAX_STRING];
	char cpu_model[CSTATUS_MAX_STRING];
	uint8_t cpu_cores;
	uint8_t cpu_processors;
	u_int32_t flash_size;
	char fw_version[CSTATUS_MAX_STRING];
	char buildtime[CSTATUS_MAX_STRING];
	char uboot_version[CSTATUS_MAX_STRING];
	char webui_version[CSTATUS_MAX_STRING];
    char company_name[CSTATUS_MAX_STRING];
	u_int32_t enterprise_oid;
    char product_name[CSTATUS_MAX_STRING];
    char product_model[CSTATUS_MAX_STRING];
	char product_sn[CSTATUS_MAX_STRING];
	char product_desc[CSTATUS_MAX_STRING];
	char company_website[CSTATUS_MAX_STRING];
    char company_contacts[CSTATUS_MAX_STRING];
    char company_address[CSTATUS_MAX_STRING];
	char company_copyright[CSTATUS_MAX_STRING];
	char domain_name[CSTATUS_MAX_STRING];
}HUAYI_STATUS_ABOUT;

typedef struct huayi_status_system {
	u_int32_t rec_uptime;
	u_int8_t  cpu_rt_usage;
	u_int8_t  cpu_avg_usage;
	u_int8_t  mem_avg_usage;
	u_int8_t  flash_usage;
	u_int8_t  upgrade_keep_setting;
	u_int8_t  usb_storage;
	u_int8_t  ping_detect;
	u_int8_t  tracert_detect;
	u_int8_t  dhcp_detect;
#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
	u_int8_t  ddns_state;
	u_int32_t ddns_ip;
#endif
	u_int8_t  wan_proto;
	u_int8_t  internet_type;
	u_int8_t  scan_state;
	u_int8_t rep_24g_status;
	u_int8_t rep_58g_status;
	u_int8_t wifi_24g_num;
	u_int8_t wifi_58g_num;
#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	u_int8_t  pptp_link;
	u_int32_t pptp_ip;
	u_int8_t  xl2tp_link;
	u_int32_t xl2tp_ip;
#endif
    char ipv6_address[CSTATUS_MAX_IPV6_ADDR][CSTATUS_MAX_IPV6_ADDR_LEN]; //ipv6 address  can be up to 10 at most
    char dns_server_ipv6[CSTATUS_MAX_IPV6_DNS_ADDR][CSTATUS_MAX_IPV6_ADDR_LEN]; //dns server ipv6 address  can be up to 10 at most
    char default_gw_ipv6[CSTATUS_MAX_IPV6_ADDR_LEN];
#if defined(CONFIG_PACKAGE_HUAYI_CAPWAP_CLIENT)
	u_int32_t ac_ipaddr;
	u_int8_t capwap_client_status;
	u_int32_t ac_assoc_time;
#endif
#if defined(CONFIG_HUAYI_BUTTON_WIFI_SUPPORT)	
	u_int8_t  wifi_button;
#endif
	u_int8_t  arp_flush;
	u_int8_t  wifi_off;
} HUAYI_STATUS_SYSTEM;

typedef struct huayi_status_network {
    u_int8_t        lan_device[CSTATUS_MAX_DEVICE_ID];
	u_int32_t		lan_ipv4;
	u_int32_t		lan_netmask;
	u_int32_t		lan_gateway;
	u_int32_t		wan_ipv4;
	u_int32_t		wan_netmask;
	u_int32_t		wan_gateway;
	u_int8_t 		wan_link;
	u_int32_t		wan_dns_server[CSTATUS_MAX_DNS_SERVER];
    u_int32_t 	    wan_rx_bps;
    u_int32_t 	    wan_tx_bps;
	u_int8_t		wan_device[CSTATUS_MAX_IFNAME];
	u_int8_t 		wan_up;
	u_int64_t		wan_uptime;
	u_int8_t 		linkport;
} HUAYI_STATUS_PORTS;

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
typedef struct huayi_status_modem {
    u_int8_t        modemModel[CSTATUS_MAX_STRING];
	u_int8_t		sim_ready;
	u_int8_t		network[CSTATUS_MAX_STRING];
	u_int8_t		operator[CSTATUS_MAX_STRING];
	u_int8_t		mobile_type[CSTATUS_MAX_STRING];
	u_int8_t		registStatus[CSTATUS_MAX_STRING];
	u_int8_t		imei[CSTATUS_MAX_STRING];
	u_int8_t		imsi[CSTATUS_MAX_STRING];
	u_int8_t		phoneNumber[CSTATUS_MAX_STRING];
	u_int8_t		center_number[CSTATUS_MAX_STRING];
	u_int8_t		bandLte[CSTATUS_MAX_STRING];
	u_int8_t		signal[CSTATUS_MAX_STRING];
	u_int8_t 		lteMode[CSTATUS_MAX_STRING];
	u_int8_t 		iccid[CSTATUS_MAX_STRING];
	u_int8_t 		lac[CSTATUS_MAX_STRING];
	u_int8_t 		ci[CSTATUS_MAX_STRING];
	u_int8_t 		dialresetcount[CSTATUS_MAX_STRING];
} HUAYI_STATUS_MODEM;
#endif

#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
typedef struct huayi_status_blend {
	u_int8_t   	wired_device_id[DAL_MAX_DEVICE_ID];
    u_int32_t  	wired_ipv4_addr;
	u_int32_t   wired_ipv4_mask;
	u_int32_t   wired_gateway;
	u_int8_t	wired_link;
	u_int32_t   wired_dns_server[CSTATUS_MAX_DNS_SERVER];

	u_int8_t   	cpe_device_id[DAL_MAX_DEVICE_ID];
    u_int32_t  	cpe_ipv4_addr;
	u_int32_t   cpe_ipv4_mask;
	u_int32_t   cpe_gateway;
	u_int8_t	cpe_link;
	u_int32_t   cpe_dns_server[CSTATUS_MAX_DNS_SERVER];
}HUAYI_STATUS_BLEND;
#endif

typedef struct huayi_runstate {
	struct huayi_status_about 	about;
    struct huayi_status_system 	system;
	struct huayi_status_network network;
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	struct huayi_status_modem   modem;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	struct huayi_status_blend   blend;
#endif
} HUAYI_RUNSTATE_DATA;

#define HUAYI_RUNSTATE_GET() ({ \
    struct huayi_runstate *ret = NULL; \
    dal_sys_mod_ops func = dal_get_sys_hooks(EN_HUAYI_STATUS); \
    if (NULL != func)  ret = (struct huayi_runstate *)(*func)(NULL,0); ret;})
        
#define HUAYI_RUNSTATE_GET_DEV_BY_DID(device_id) ({ \
    struct huayi_runstate *ret = NULL; \
    dal_sys_mod_ops func = dal_get_sys_hooks(EN_HUAYI_STATUS_DID); \
    if (NULL != func) ret = (struct HUAYI_STATUS_port *)(*func)((const char *)device_id,0); ret;})

#endif /* HUAYI_RUNSTATE_H */


