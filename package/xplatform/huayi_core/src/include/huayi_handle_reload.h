#ifndef _HUAYI_HANDLE_RELOAD_HEADER_
#define _HUAYI_HANDLE_RELOAD_HEADER_

#include "huayi_core_base.h"
#define QOS_IPLIMIT_PREROUTING_CHAIN    "qos_iplimit_prerouting"
#define QOS_IPLIMIT_POSTROUTING_CHAIN   "qos_iplimit_postrouting"


VTBool huayi_reload_functions_init(struct VcoreAgent *Agent);
VTBool huayi_execute_need_reload(void);
void huayi_execute_reload(void * arg);

/*huayi_reload_system.c*/
VTBool huayi_sys_upgrade(void * arg,cJSON *input_root);
VTBool huayi_sys_reset(void * arg,cJSON *input_root);
VTBool huayi_sys_reboot(void * arg,cJSON *input_root);
VTBool huayi_reload_sche_reboot(void * arg,cJSON *input_root);
VTBool huayi_restore_system_config(void * arg,cJSON *input_root);
VTBool huayi_reload_system_ntpd(void * arg,cJSON *input_root);


/*huayi_reload_network.c*/
VTBool huayi_reload_network(void * arg,cJSON *input_root);
VTBool huayi_reload_dhcps(void * arg,cJSON *input_root);
VTBool huayi_reload_hwnat(void * arg,cJSON *input_root);
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
VTBool huayi_reload_lte4g(void * arg,cJSON *input_root);
VTBool huayi_reload_small4g_func(void * arg,cJSON * input_root);
#endif

#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
VTBool huayi_reload_save_database(void * arg,cJSON *input_root);
#endif
#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
VTBool huayi_reload_save_database(void * arg,cJSON *input_root);
#endif

VTBool huayi_reload_ftpd(void * arg,cJSON *input_root);
VTBool huayi_reload_printer(void * arg,cJSON *input_root);
VTBool huayi_reload_mount(void * arg,cJSON *input_root);
VTBool huayi_reload_udhcpd(void * arg,cJSON *input_root);

#if defined(CONFIG_HUAYI_SUPPORT_CONTROL_TELNET_OPTION)
VTBool huayi_reload_telnet(void * arg,cJSON *input_root);
#endif

/*huayi_reload_firewall.c*/
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
VTBool huayi_reload_firewall(void * arg,cJSON *input_root);
VTBool huayi_reload_wanup(void * arg,cJSON *input_root);
VTBool huayi_reload_remote_manage(void * arg,cJSON * input_root);
VTBool huayi_reload_macfilter(void * arg,cJSON * input_root);
VTBool huayi_reload_urlfilter(void * arg,cJSON * input_root);
VTBool huayi_reload_ipportfilter(void * arg,cJSON * input_root);
VTBool huayi_reload_dmz(void * arg,cJSON * input_root);
VTBool huayi_reload_port_forward(void * arg,cJSON * input_root);
VTBool huayi_reload_static_route(void * arg,cJSON *input_root);
VTBool huayi_reload_wanping(void * arg,cJSON * input_root);

VTBool huayi_reload_upnpd(void * arg,cJSON * input_root);
VTBool huayi_reload_acl(void * arg,cJSON * input_root);
VTBool huayi_reload_wifiguestfilter(void * arg,cJSON * input_root);

VTBool huayi_reload_qos_iplimit(void * arg,cJSON * input_root);

#if defined(CONFIG_HUAYI_SUPPORT_VPN) 
VTBool huayi_ppp_handler(PPP_HANDLE_TYPE type);
VTBool huayi_reload_pptpd(void * arg,cJSON *input_root);
VTBool huayi_reload_xl2tpd(void * arg,cJSON *input_root);
VTBool huayi_reload_pptp(void * arg,cJSON *input_root);
VTBool huayi_reload_xl2tp(void * arg,cJSON *input_root);
VTBool huayi_reload_vpn_user(void * arg,cJSON *input_root);
VTBool huayi_reload_vpn_route(void * arg,cJSON *input_root);
VTBool huayi_reload_vpn_server_route(void * arg,cJSON *input_root);


#endif

#endif


#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
VTBool huayi_reload_ddns(void * arg,cJSON * input_root);
#endif




/*huayi_reload_wireless.c*/
VTBool huayi_reload_wireless(void * arg,cJSON *input_root);
VTBool huayi_wps_pbc(void * arg,cJSON *input_root);
VTBool huayi_wps_pin(void * arg,cJSON *input_root);
VTBool huayi_reload_wireless_scan(void * arg,cJSON *input_root);
VTBool huayi_reload_wifi_sche(void * arg,cJSON *input_root);
VTBool huayi_reload_wifi_close(void * arg,cJSON *input_root);

//switch_sim &&save usesiminfo
VTBool huayi_switch_card(void * arg,cJSON *input_root);
VTBool huayi_check_internet_state(void * arg,cJSON *input_root);
VTBool huayi_meige_module_decryption(void * arg,cJSON *input_root);
VTBool huayi_meige_set_band(void * arg,cJSON *input_root);


#endif
