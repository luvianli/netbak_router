#ifndef _HUAYI_CORE_BASE_HEADER_
#define _HUAYI_CORE_BASE_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include "huayi_uci_fun.h"
#include "huayi_core_internal.h"
#include "huayi_config_common.h"


typedef enum {
	VT_FALSE = 0,
	VT_TRUE,
} VTBool;

typedef enum {
	WPS_INIT = 0,
	WPS_DOING,
	WPS_FALSE,
	WPS_SUCCESS,
} VTBool_WPS;

#if defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
typedef enum {
	LED_UNPREPARED = 0,
	LED_PREPARED,
	LED_RESET,
} VTBool_LED;
#endif

struct AgentIPCSocket
{
	int huayi_ipc_fd;
	struct sockaddr_un address; 
};

typedef enum{
	BUTTON_RELEASED = 0,
	BUTTON_PRESSED = 1,
}BUTTON_ACTION;

typedef enum{
	AP_WPS_INIT = 0,
	AP_WPS_2G = 1,
	AP_WPS_2G_DONE = 2,
	AP_WPS_5G = 3,
	AP_WPS_5G_DONE = 4,
	AP_WPS_TO_UP_DONE = 5,
}WPS_FREQ;

typedef enum{
	BUTTON_INIT_STATE = 0,
	BUTTON_READY_TO_RESET = 1,
	BUTTON_NEED_RESET = 2,
	BUTTON_READY_TO_WPS = 3,
	BUTTON_NEED_WPS = 4,
	BUTTON_WPS_2G_DONE = 5,
	BUTTON_WPS_5G_DONE = 6,
	BUTTON_WPS_TO_UP_DONE = 7,
}RESET_BUTTON_STATE;

typedef struct huayi_net_stats
{
	unsigned long long rx_bytes;
	unsigned long long tx_bytes;
}HUAYI_NET_STATS;

typedef struct VcoreAgent
{ 
	int huayi_timer_fd;
	int huayi_event_fd;
	int huayi_gpio_fd;
	int huayi_port_fd;
	struct AgentIPCSocket ipcSocket;
	VTBool (*CoreReloadFunction[RELOAD_FUNC_SIZE])(void * arg,cJSON *input_root);
	long cpu_idle_time;
	long cpu_total_time;
	int  cpu_use_ratio_sum;
	int  mem_use_ratio_sum;
	int usage_calc_count;
	long old_time;
	WPS_FREQ wps_freq;
	unsigned int wps_button_time;
	RESET_BUTTON_STATE reset_button_state;
	RESET_BUTTON_STATE old_reset_button_state;
	struct huayi_net_stats net_stats;
	int opmode;
	int link_port;
	int wps_timer_count;
	char wps_interface[SHORT_STR_LEN];
	VTBool wifi_sche_down;
	VTBool wpstimeout;
	VTBool_WPS wps_connect_status_24g;
	VTBool repeater_status_24g;
	VTBool_WPS wps_connect_status_58g;
	VTBool repeater_status_58g;
	VTBool sta_connect_status_old;
	VTBool_WPS wps_downlink_status;
	VTBool wan_connect_status_old;
	unsigned char encryption;
	unsigned char wpa_key[WLAN_WPA_KEY_BUFF_LEN];
#if defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
	VTBool_LED special_led_status;
#endif
#if defined(CONFIG_HUAYI_SUPPORT_FLUSH_ARP)
	unsigned int arp_flush_count;
#endif
	int wifi_scan_type;
	int wifi_scan_status;
	int wifi_scan_time;
}VcoreAgentStructure;

typedef struct huayi_handle_msg
{
	MsgType msgid;
	long rec_time;
	cJSON *input_root;
}HUAYI_HANDLE_MSG;

typedef enum
{
	PPP_HANDLE_ALL,
	PPP_HANDLE_PPTPD,
	PPP_HANDLE_XL2TPD,
}PPP_HANDLE_TYPE;

#endif
