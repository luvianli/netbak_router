/*******************************************************************************************
 * Project:  huayi_core                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/

#include "huayi_utimer_functions.h"
#include "huayi_eloop_job.h"
#include "huayi_core_queue.h"
#include "huayi_netlink.h"
#include <signal.h>

VTBool gRunAgent;
VTBool gNeedReload;

VTBool huayi_core_main_init(struct VcoreAgent *Agent)
{
	memset(Agent,0,sizeof(struct VcoreAgent));
	gRunAgent = VT_TRUE;
	gNeedReload = VT_FALSE;
	return VT_TRUE;
}

void huayi_core_main_graceful_exit(int sig)
{
	gRunAgent = VT_FALSE;

	(void) signal(SIGTERM, SIG_IGN);

	return;
}

VTBool huayi_led_init(struct VcoreAgent *Agent)
{
	int opmode = 0;
	int repeater_status_24g = 0;
	int repeater_status_58g = 0;
	char product_model[OPTION_STR_LEN] = {0};

	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	huayi_uci_get_int(PKG_SYSTEM, "mode", "opmode", &opmode);
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_24g_status",&repeater_status_24g);		
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rep_58g_status",&repeater_status_58g);

	Agent->repeater_status_24g = repeater_status_24g;
	Agent->repeater_status_58g = repeater_status_58g;
	Agent->opmode = opmode;

	if(!strcmp(product_model, "ZC-GW03-3"))
	{
		char cmd[CMD_STR_LEN] = {0};
		strcpy(cmd, "devmem 0x10000060 32 0x55150451"); 
		system(cmd);
	}

#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
	if(HUAYI_REP_MODE == Agent->opmode)
	{
		do_wps_led_blinks_slowly();
		Agent->sta_connect_status_old = 0;
	}
	if(HUAYI_ROUTE_MODE == Agent->opmode || HUAYI_AP_MODE == Agent->opmode)
	{
		if (CONFIG_HUAYI_WPS_LED_DEFAULT_STATUS == 1)
		{
			do_wps_led_on();
		}
		else
		{
			butto_do_wps_led_off();
		}
	}
#endif

#if defined(CONFIG_HUAYI_POWER_LED_SUPPORT)
		do_power_led_off();
#endif

#if defined(CONFIG_HUAYI_SIGNAL_LED_SUPPORT)
	do_signal_led_off();
#endif

#if defined(CONFIG_HUAYI_REPEATER_LED_SUPPORT)
	if (Agent->opmode == HUAYI_ROUTE_MODE || Agent->opmode == HUAYI_AP_MODE)
		do_repeater_led_high_signal_or_on();
#endif
	
	return VT_TRUE;
}
#if defined(CONFIG_HUAYI_SUPPORT_CUSTOM_MAC_CHECK)
int custom_mac_check()
{
	char cmd[256] = {0};
	char resultbuf[128] = {0};
	//char str_mac[17] = {0};
	char mac_post[32] = {0};
	char custom_company_mac[18] = {0};
	char custom_mac_start[18] = {0};
	char custom_mac_end[18] = {0};
	unsigned long mac_value = 0;
	unsigned long mac_start = 0;
	unsigned long mac_end = 0;

	strncpy(custom_company_mac,CONFIG_HUAYI_CUSTOM_COMPANY_MAC,8);
	strncpy(custom_mac_start,CONFIG_HUAYI_CUSTOM_MAC_START,17);
	strncpy(custom_mac_end,CONFIG_HUAYI_CUSTOM_MAC_END,17);

	str2upper(custom_company_mac);
	str2upper(custom_mac_start);
	str2upper(custom_mac_end);

	if(strlen(custom_company_mac) != 8|| strlen(custom_mac_start) != 17 || strlen(custom_mac_end) != 17)
	{
		system("huayi-reset reboot");
	}

	sprintf(cmd,"huayi_mac read wlan_2g |grep \"wlan_2g\" |awk -F' ' '{print$2}' |tr '[a-z]' '[A-Z]'");
	get_cmd_result(cmd, resultbuf, sizeof(resultbuf));
	//MAC17_TO_MAC12(resultbuf, str_mac, ':');

	if(strncmp(resultbuf,custom_company_mac,8) != 0)
	{
		system("huayi-reset reboot");
	}

	memset(mac_post,0,sizeof(mac_post));
	sprintf(mac_post,"0x%c%c%c%c%c%c",custom_mac_start[9],custom_mac_start[10],custom_mac_start[12],custom_mac_start[13],custom_mac_start[15],custom_mac_start[16]);
	mac_start = strtoul(mac_post,NULL,16);

	memset(mac_post,0,sizeof(mac_post));
	sprintf(mac_post,"0x%c%c%c%c%c%c",custom_mac_end[9],custom_mac_end[10],custom_mac_end[12],custom_mac_end[13],custom_mac_end[15],custom_mac_end[16]);
	mac_end = strtoul(mac_post,NULL,16);

	memset(mac_post,0,sizeof(mac_post));
	sprintf(mac_post,"0x%c%c%c%c%c%c",resultbuf[9],resultbuf[10],resultbuf[12],resultbuf[13],resultbuf[15],resultbuf[16]);
	mac_value = strtoul(mac_post,NULL,16);

	if(mac_value > mac_end || mac_value < mac_start)
	{
		system("huayi-reset reboot");
	}

	return 0;
}
#endif

int main(int argc, char *argv[])
{

	struct VcoreAgent Agent;
	pthread_t reloadThread;

#if defined(CONFIG_HUAYI_SUPPORT_DNS_TRAP_IPV6_DROP)
	/*add by huayi for drop IPV6*/	
	/*we should remove code bellow if ipv6 support*/	
	system("ebtables -A FORWARD -p ipv6 -j DROP");
#endif
		
	if(!huayi_core_main_init(&Agent))
	{
		printf("huayi core init failed,service stop!!!\n");
		return VT_FALSE;
	}
	
	(void) signal(SIGTERM,huayi_core_main_graceful_exit);

#if defined(CONFIG_HUAYI_SUPPORT_GPIO_EVENT)
	hook_gpio_init();
#endif
	
	if(!huayi_core_eloop_init())
	{
		printf("huayi core eloop init failed,service stop!!!\n");
		return VT_FALSE;
	}

	if(!huayi_core_timer_init(&Agent))
	{
		printf("huayi core timerfd init failed,service stop!!!\n");
		huayi_core_eloop_destory();
		return VT_FALSE;
	}

	if(!huayi_core_ipc_init(&Agent))
	{
		printf("huayi core ipc init failed,service stop!!!\n");
		huayi_core_eloop_destory();
		huayi_core_timer_destory(&Agent);
		return VT_FALSE;
	}

	if(!huayi_port_event_init(&Agent))
	{
		printf("huayi core port event init failed,service stop!!!\n");
		huayi_core_eloop_destory();
		huayi_core_timer_destory(&Agent);
		huayi_core_ipc_destory(&Agent);
		return VT_FALSE;
	}

#if defined(CONFIG_HUAYI_SUPPORT_GPIO_EVENT)
	if(!huayi_gpio_event_init(&Agent))
	{
		printf("huayi core gpio event init failed,service stop!!!\n");
		huayi_core_eloop_destory();
		huayi_core_timer_destory(&Agent);
		huayi_core_ipc_destory(&Agent);
		huayi_port_event_destory(&Agent);
		return VT_FALSE;
	}
#endif

#if defined(CONFIG_HUAYI_SUPPORT_CUSTOM_MAC_CHECK)
	custom_mac_check();
#endif
	huayi_reload_functions_init(&Agent);

	huayi_led_init(&Agent);

	huayi_msg_queue_init();
	
	if(!huayi_core_eloop_register_socket(Agent.ipcSocket.huayi_ipc_fd,(void*)huayi_core_receive_ipc_msg,&Agent))
	{
		printf("huayi core eloop ipc socket register failed, service stop!!!\n");
		goto service_stop;
	}

	if(!huayi_core_eloop_register_socket(Agent.huayi_port_fd,(void*)huayi_port_event_receive,&Agent))
	{
		printf("huayi core eloop port fd register failed, service stop!!!\n");
		goto service_stop;
	}

#if defined(CONFIG_HUAYI_SUPPORT_GPIO_EVENT)
	if(!huayi_core_eloop_register_socket(Agent.huayi_gpio_fd,(void*)huayi_gpio_event_receive,&Agent))
	{
		printf("huayi core eloop gpio fd register failed, service stop!!!\n");
		goto service_stop;
	}
#endif

	if(!huayi_core_eloop_register_socket(Agent.huayi_timer_fd,(void*)huayi_core_timer_process,&Agent))
	{
		printf("huayi core eloop timer fd register failed, service stop!!!\n");
		goto service_stop;
	}

	pthread_create(&reloadThread, NULL, (void*)huayi_execute_reload, (void *)&Agent);

	huayi_utimer_functions_start(&Agent);
	
	while(VT_TRUE == gRunAgent)
	{
		huayi_core_eloop_execution();
	}

service_stop:
	huayi_core_eloop_remove_socket(Agent.huayi_timer_fd);

	huayi_core_eloop_remove_socket(Agent.ipcSocket.huayi_ipc_fd);

	huayi_core_eloop_remove_socket(Agent.huayi_port_fd);

	huayi_core_ipc_destory(&Agent);

	huayi_msg_queue_destory();

	huayi_port_event_destory(&Agent);

#if defined(CONFIG_HUAYI_SUPPORT_GPIO_EVENT)
	huayi_gpio_event_destory(&Agent);
#endif

	huayi_core_timer_destory(&Agent);
	
	huayi_core_eloop_destory();

	return 0;
}


