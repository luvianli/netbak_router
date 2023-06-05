#ifndef _HUAYI_HANDLE_EVENTS_HEADER_
#define _HUAYI_HANDLE_EVENTS_HEADER_

#include "huayi_core_base.h"
#include "huayi_utimer_functions.h"
//#include "huayi_utimer_functions.h"

#define HUAYI_PORT_NETLINK_MSG_TYPE   		30
#define HUAYI_PORT_NETLINK_PORT_ID    		50

#define HUAYI_GPIO_NETLINK_MSG_TYPE			29
#define HUAYI_GPIO_NETLINK_PORT_ID			49

typedef struct huayi_port_event
{ 
	u_int8_t port_id;          
	u_int8_t status;
	u_int8_t encryption;
	u_int8_t key[WLAN_WPA_KEY_BUFF_LEN];
} port_change_event;

typedef struct huayi_gpio_event
{ 
	u_int8_t gpio_id;          
	u_int8_t status; 			
} gpio_change_event;

void huayi_port_event_process(struct VcoreAgent *Agent,struct huayi_port_event *event);
#if defined(CONFIG_HUAYI_SUPPORT_GPIO_EVENT)
void huayi_gpio_event_process(struct VcoreAgent *Agent,struct huayi_gpio_event *event);
#endif
#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
void do_wps_led_blinks_slowly(void);
void do_wps_led_on(void);
#endif

#if defined(CONFIG_HUAYI_POWER_LED_SUPPORT)
void do_power_led_off(void);
#endif

#if defined(CONFIG_HUAYI_POWER_LED_SUPPORT)
void do_repeater_led_all_blinks_slowly(void);
void do_repeater_led_no_signal_or_off(void);
void do_repeater_led_weak_signal(void);
void do_repeater_led_medium_signal(void);
void do_repeater_led_high_signal_or_on(void);
#endif

#if defined(CONFIG_HUAYI_INTERNET_LED_SUPPORT)
void do_internet_led_on(void);
void do_internet_led_off(void);
void do_internet_led_slowly(void);
#endif


#endif
