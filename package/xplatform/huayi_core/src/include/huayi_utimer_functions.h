#ifndef HUAYI_UTIMER_FUNCTIONS_HEADER
#define HUAYI_UTIMER_FUNCTIONS_HEADER

#include "huayi_unix_socket.h"
#include "huayi_eloop_job.h"
#include "huayi_handle_reload.h"
#include "huayi_netlink.h"


#define HUAYI_FIRMWARE_BLOCK  "/dev/mtdblock6"


#define STATISTICS_UPDATE_INTERVAL			5

#define AVG_USAGE_UPDATE_INTERVAL			60

#define HUAYI_RELOAD_DELAY_TIME				3

#define HUAYI_ARP_FLUSH_CHECK_INTERVAL		60

#define HUAYI_REPEATER_LED_CHECK_INTERVAL	60

#define HUAYI_UDHCPD_DELAY_TIME				30

#define HUAYI_BUTTON_DELAY_REBOOT_TIME		3

#define HUAYI_BUTTON_DELAY_RESET_TIME		5

#define HUAYI_BUTTON_DELAY_WPS_TIMEOUT		3

#define HUAYI_WIFI_SCAN_CHECK_INTERVAL		5

#define HUAYI_AP_WPS_2G_DELAY_TIME			1

#define HUAYI_AP_WPS_5G_DELAY_TIME			5

#define HUAYI_WPS_TIME_COUNT                12

#define HUAYI_IFUP_WAN_INTERVAL				3

#define HUAYI_WPS_CHECK_TIME				5

#define HUAYI_WPS_END_TIME					120

#define HUAYI_START_SPECIAL_LED_DELAY_TIME	20

#define HUAYI_INTERNET_LED_DELAY_TIME	20

typedef enum
{
	HUAYI_RELOAD_TASK,
	HUAYI_UPTATE_STATISTICS_TASK,
	HUAYI_SCHE_REBOOT_TASK,
	HUAYI_WPS_CONNECT_TASK,
	HUAYI_IFUP_WAN_TASK,
	HUAYI_BUTTON_ONLY_RESET_TASK,
	HUAYI_BUTTON_MULT_WPS_TIMEOUT_TASK,
	HUAYI_BUTTON_MULT_TO_RESET_TASK,
	HUAYI_AP_WPS_FREQ_CHANGE_TASK,
	HUAYI_DO_AP_WPS_TASK,
	HUAYI_WPS_END_TASK,
	HUAYI_WIFI_SCAN_TASK,
	HUAYI_WIFI_SCHED_DOWN_TASK,
	HUAYI_WIFI_WPS_CHECK_TASK,
	HUAYI_START_SPECIAL_LED_TASK,
	HUAYI_SYSTEM_FLUSH_ARP_TASK,
	HUAYI_NETWORK_UDHCPD_CHECK_TASK,
	HUAYI_REPEATER_LED_CHECK_TASK,
	HUAYI_RELOAD_MOUNT_TASK,
	HUAYI_START_CHECK_NETWORK_LED_TASK,
	HUAYI_START_CHECK_4G_LED_TASK,
}HUAYI_CORE_TASK_ID;



void huayi_utimer_functions_start(struct VcoreAgent *Agent);
void huayi_core_schedule_reboot(void* arg);
void huayi_wps_connect(void* arg);
void huayi_core_timer_ifup_wan(void* arg);
void huayi_core_timer_udhcpd(void* arg);
void huayi_core_wifi_schedule_down(void* arg);
void huayi_core_mount(void* arg);
void huayi_core_wifi_timer_scan(void* arg);

#endif
