/*******************************************************************************************
 * Project:  huayi_utimer_functions                                                         *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "huayi_utimer_functions.h"
#include "wps.h"
#include "oid.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <errno.h>
#include "huayi_utimer_functions.h"


int huayi_get_rt_flash_usage(void)
{
	FILE *pp;
	char cmd[CMD_STR_LEN] = { 0 };
	char buf[TEMP_STR_LEN] = { 0 };

	snprintf(cmd,sizeof(cmd),"df | grep '%s' | awk '{print $5}'",HUAYI_FIRMWARE_BLOCK);
	
	if ((pp = popen(cmd, "r")) == NULL)
	{
		return 100;
	}
	
	if(fgets(buf, sizeof(buf), pp)!= NULL)
	{
		pclose(pp);
		
		return atoi(buf);
	}

	pclose(pp);

	return 100;
}


void huayi_core_update_cpu_mem_usage(struct VcoreAgent *Agent)
{
	char cmd[CMD_STR_LEN] = {0};
	unsigned long cpu_total_time = 0,cpu_idle_time =0;
	unsigned long cpu_use_ratio = 0,average_cpu_use_ratio = 0;
	int mem_rt_usage = 0;
	int mem_avg_usage = 0;
	int flash_usage = 0;

	huayi_uci_get_long(PKG_STATUS_SYSTEM,"sys","cpu_total_time",&cpu_total_time);

	huayi_uci_get_long(PKG_STATUS_SYSTEM,"sys","cpu_idle_time",&cpu_idle_time);
	
	if(Agent->cpu_total_time != 0)
	{
		if(cpu_total_time != Agent->cpu_total_time)
		{
	    	cpu_use_ratio =(unsigned int)((((cpu_total_time - Agent->cpu_total_time ) - (cpu_idle_time - Agent->cpu_idle_time))*100)/(cpu_total_time - Agent->cpu_total_time ));
		}

		cpu_use_ratio = ((cpu_use_ratio == 0) ? 1 : cpu_use_ratio); 
		
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set system cpu_rt_usage %d",cpu_use_ratio);
		
		system(cmd);
		
		Agent->cpu_use_ratio_sum += cpu_use_ratio;	
	}

	Agent->cpu_idle_time = cpu_idle_time;
	
	Agent->cpu_total_time  = cpu_total_time;

	flash_usage = huayi_get_rt_flash_usage();
	
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set system flash_usage %d",flash_usage);

	system(cmd);

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","mem_rt_usage",&mem_rt_usage);

	Agent->cpu_use_ratio_sum += cpu_use_ratio;

	Agent->mem_use_ratio_sum += mem_rt_usage;

	Agent->usage_calc_count++;
	
	if( STATISTICS_UPDATE_INTERVAL * (Agent->usage_calc_count) >= AVG_USAGE_UPDATE_INTERVAL)
	{	
		average_cpu_use_ratio = Agent->cpu_use_ratio_sum/(Agent->usage_calc_count);

		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set system cpu_avg_usage %d",average_cpu_use_ratio);

		system(cmd);
		
		mem_avg_usage = Agent->mem_use_ratio_sum/(Agent->usage_calc_count);
		
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set system mem_avg_usage %d",mem_avg_usage);

		system(cmd);
		
		Agent->usage_calc_count = 0;

		Agent->cpu_use_ratio_sum = 0;

		Agent->mem_use_ratio_sum = 0;
	}
	return;
}

void huayi_core_update_network_statistics(struct VcoreAgent *Agent)
{
	unsigned long long rx_bytes = 0;
	unsigned long long tx_bytes = 0;
	char cmd_str[CMD_STR_LEN] = {0};
	int wan_up = 0;
	int rx_bps = 0;
	int tx_bps = 0;
	long new_time = 0;
	long sys_interval = 0;
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	char wan_type[OPTION_STR_LEN] = {0};
	char ifname[OPTION_STR_LEN] = {0};
#endif

	struct timeval tv;
    gettimeofday(&tv,NULL);

	new_time = tv.tv_usec/1000 + tv.tv_sec * 1000;
	
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up);
	if(wan_up)
	{
		huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","rx_bytes",&rx_bytes);
		huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","tx_bytes",&tx_bytes);
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
		huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ifname",ifname);
		if(!strcmp(wan_type, "rep4g") && !strcmp(ifname, "apcli0"))
		{
			memset(cmd_str, 0, sizeof(cmd_str));
			get_cmd_result("ifconfig ra0 | grep \"RX bytes:\" | awk '{print $2}'", cmd_str, sizeof(cmd_str));
			rx_bytes = atoi(cmd_str+strlen("bytes:"));
			memset(cmd_str, 0, sizeof(cmd_str));
			get_cmd_result("ifconfig ra0 | grep \"TX bytes:\" | awk '{print $6}'", cmd_str, sizeof(cmd_str));
			tx_bytes = atoi(cmd_str+strlen("bytes:"));
		}
#endif

		sys_interval = new_time - Agent->old_time;
		sys_interval = ((sys_interval == 0) ? 1 : sys_interval);
		if(sys_interval < 0 || sys_interval > 60*1000)
		{
			sys_interval = 5000;
		}
		if(Agent->net_stats.rx_bytes==0)
		{
			if(rx_bytes != 0)
			{
				Agent->net_stats.rx_bytes= rx_bytes;
			}
			rx_bps = 0;
		}else
		{
			if(rx_bytes >=Agent->net_stats.rx_bytes)
			{
				rx_bps = (int)((rx_bytes - Agent->net_stats.rx_bytes)*1000/sys_interval);
			}else
			{
				rx_bps = (int)((4294967295ULL - Agent->net_stats.rx_bytes + rx_bytes)*1000/sys_interval);
				
			}
			Agent->net_stats.rx_bytes = rx_bytes;
		}
		
		if(Agent->net_stats.tx_bytes==0)
		{	
			if(tx_bytes != 0)
			{
				Agent->net_stats.tx_bytes= tx_bytes;
			}
			tx_bps = 0;
		}else
		{
			if(tx_bytes >=Agent->net_stats.tx_bytes)
			{
				tx_bps =  (int)((tx_bytes - Agent->net_stats.tx_bytes)*1000/sys_interval);
				
			}else
			{
				tx_bps = (int)((4294967295ULL - Agent->net_stats.tx_bytes + tx_bytes)*1000/sys_interval);
				
			}
			Agent->net_stats.tx_bytes = tx_bytes;
		}
	}else
	{
		Agent->net_stats.rx_bytes = rx_bytes;
		Agent->net_stats.tx_bytes = tx_bytes;
		rx_bps = 0;
		tx_bps = 0;
	}
	
	snprintf(cmd_str,sizeof(cmd_str),"huayi_cmd set network wan_rx_bps %d",rx_bps);
	system(cmd_str);
	snprintf(cmd_str,sizeof(cmd_str),"huayi_cmd set network wan_tx_bps %d",tx_bps);
	system(cmd_str);

	Agent->old_time = new_time;
	
	return;
}

void huayi_core_update_wps_led_status(struct VcoreAgent *Agent)
{
	VTBool_WPS wps_connect_status = WPS_INIT;
	int repeater_status = 0;
	
	if (WPS_SUCCESS == Agent->wps_connect_status_24g ||
		WPS_SUCCESS == Agent->wps_connect_status_58g)
	{
		wps_connect_status = WPS_SUCCESS;
	}
	else if (WPS_FALSE == Agent->wps_connect_status_24g ||
		WPS_FALSE == Agent->wps_connect_status_58g)
	{
		Agent->wps_connect_status_24g = WPS_INIT;
		Agent->wps_connect_status_58g = WPS_INIT;
		wps_connect_status = WPS_INIT;
	}
	else if (WPS_DOING == Agent->wps_connect_status_24g &&
		WPS_DOING == Agent->wps_connect_status_58g)
	{
		wps_connect_status = WPS_DOING;
	}

	if (Agent->repeater_status_24g == 1 || Agent->repeater_status_58g == 1)
	{
		repeater_status = 1;
	}

	if(HUAYI_REP_MODE == Agent->opmode && wps_connect_status != WPS_DOING)
	{
		if(Agent->sta_connect_status_old != repeater_status || wps_connect_status == WPS_SUCCESS)
		{
			if(WPS_SUCCESS == wps_connect_status || 1 == repeater_status)
			{
		#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
				do_wps_led_on();
		#endif
			}
			else if (wps_connect_status == WPS_INIT)
			{
		#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
				do_wps_led_blinks_slowly();
		#endif
			}
			
			Agent->sta_connect_status_old = repeater_status;
		}
	}
	
	return;
}

#if defined(CONFIG_HUAYI_REPEATER_LED_SUPPORT)
void huayi_core_repeater_led_status(void* arg)
{
	int RSSI = 0;
	char cmd_str[CMD_STR_LEN] = {0};
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	
	if(HUAYI_REP_MODE == Agent->opmode)
	{
		if (Agent->repeater_status_24g == 0 && Agent->repeater_status_58g == 0)
		{
			do_repeater_led_all_blinks_slowly();
		}
		else
		{
			if (Agent->repeater_status_24g == 1)
			{
				get_cmd_result("iwpriv apcli0 stat | grep RSSI | awk '{print $3}'", cmd_str, sizeof(cmd_str));
			}
			else
			{
				get_cmd_result("iwpriv apclii0 stat | grep RSSI | awk '{print $4}'", cmd_str, sizeof(cmd_str));
			}

			RSSI = atoi(cmd_str);

			if (RSSI > -60)
			{
				do_repeater_led_high_signal_or_on();
			}
			else if (RSSI > -75)
			{
				do_repeater_led_medium_signal();
			}
			else if (RSSI > -85)
			{
				do_repeater_led_weak_signal();
			}
			else
			{
				do_repeater_led_no_signal_or_off();
			}	
		}
	}
	
	huayi_core_add_task(HUAYI_REPEATER_LED_CHECK_INTERVAL,HUAYI_REPEATER_LED_CHECK_TASK,huayi_core_repeater_led_status,Agent);
	
	return;
}
#endif

#if defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
int huayi_core_repeater_rssi(struct VcoreAgent *Agent)
{
	int RSSI = 0;
	char cmd_str[CMD_STR_LEN] = {0};
	
	if(HUAYI_REP_MODE == Agent->opmode)
	{
		if (Agent->repeater_status_24g == 1)
		{
			get_cmd_result("iwpriv apcli0 stat | grep RSSI | awk '{print $3}'", cmd_str, sizeof(cmd_str));
		}
		else if(Agent->repeater_status_58g == 1)
		{
			get_cmd_result("iwpriv apclii0 stat | grep RSSI | awk '{print $4}'", cmd_str, sizeof(cmd_str));
		}
		RSSI = atoi(cmd_str);
	}
	return RSSI;
}


void huayi_core_start_special_led(struct VcoreAgent *Agent)
{
	int first_boot = 0;

	huayi_uci_get_int(PKG_SYSTEM,"global","first_boot",&first_boot);

	if (first_boot == 1)
	{
		//do_orange_led_blinks_slowly();
#if defined(CONFIG_HUAYI_REPEATER_ONLY_LED_INTERNET)
		two_color_blink_slowly();
#else
		do_orange_led_blinks_slowly();
#endif
	}
	else
	{
#if defined(CONFIG_HUAYI_REPEATER_ONLY_LED_INTERNET)
		two_color_blink_slowly();
#else
		do_white_led_blinks_slowly();
#endif	
	}
	
	Agent->special_led_status = LED_PREPARED;

	return;
}


void huayi_core_update_special_led_status(struct VcoreAgent *Agent)
{
	int network_status = 0;
	int first_boot = 0;
#if defined(CONFIG_HUAYI_REPEATER_ONLY_LED_INTERNET)
	int link_stat = 0;
	int rep_rssi = 0;
#endif

	
	huayi_uci_get_int(PKG_SYSTEM,"global","first_boot",&first_boot);
	
	if(LED_PREPARED == Agent->special_led_status || LED_RESET == Agent->special_led_status)
	{
		if (internet_do_ping_detect(INTERNET_DOMAIN, NULL) == 1)
		{
			network_status = 1;	

		}
		else
		{
			network_status = 0;
		}
#if !defined(CONFIG_HUAYI_REPEATER_ONLY_LED_INTERNET)	
		if(Agent->wan_connect_status_old != network_status || LED_RESET == Agent->special_led_status)
#endif
		{
			if(WPS_INIT == Agent->wps_downlink_status)
			{
#if defined(CONFIG_HUAYI_REPEATER_ONLY_LED_INTERNET)
				if(((Agent->opmode == HUAYI_ROUTE_MODE) || (Agent->opmode == HUAYI_AP_MODE)) && (Agent->link_port & (1<<CONFIG_HUAYI_WAN_PORT_NUM))) //AP mode
				{
					link_stat = 1;
				}

				if(Agent->opmode == HUAYI_REP_MODE) //Rep mode
				{
					if((Agent->repeater_status_24g == 1) || (Agent->repeater_status_58g == 1))
					{
						link_stat =1;
					}
				}
				if(link_stat == 0)
				{
					two_color_blink_slowly();
				}
				else
				{
					if(1 == network_status)
					{
						rep_rssi = huayi_core_repeater_rssi(Agent);
						if(Agent->opmode == HUAYI_ROUTE_MODE || Agent->opmode == HUAYI_AP_MODE || rep_rssi > -65)
						{
							do_white_led_on();
						}
						else
						{
							do_orange_led_on();
						}
					}else
					{
						do_orange_led_blinks_slowly();	
					}
				}
#else
				if(1 == network_status)
				{
					do_white_led_on();
				}
				else
				{
					if (first_boot == 1)
					{
						do_orange_led_blinks_slowly();
					}
					else
					{
						do_white_led_blinks_slowly();
					}
				}		
#endif		
			}

			if (LED_RESET == Agent->special_led_status)
			{
				Agent->special_led_status = LED_PREPARED;
			}
			
			Agent->wan_connect_status_old = network_status;

		}	
	}

	huayi_core_add_task(HUAYI_START_SPECIAL_LED_DELAY_TIME,HUAYI_START_SPECIAL_LED_TASK,huayi_core_update_special_led_status,Agent);
	
	return;
}

void huayi_core_delay_start_special_led(struct VcoreAgent *Agent)
{
	huayi_core_start_special_led(Agent);
	
	huayi_core_add_task(HUAYI_START_SPECIAL_LED_DELAY_TIME,HUAYI_START_SPECIAL_LED_TASK,huayi_core_update_special_led_status,Agent);
	
	return;
}

#endif

#if defined(CONFIG_HUAYI_INTERNET_LED_SUPPORT)
void huayi_core_start_internet_led(struct VcoreAgent *Agent)
{
	int network_status = 0;
	char detectip[OPTION_STR_LEN] = {0};
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	int i_ready = 0; 
#endif
	char product_model[OPTION_STR_LEN] = {0};
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);

	if (internet_do_ping_detect(detectip, NULL) == 1)
	{
		network_status = 1;	

	}
	else
	{
		if (internet_do_ping_detect(INTERNET_ABROAD_DOMAIN, NULL) == 1)
		{
			network_status = 1;	

		}
		else
		{
			network_status = 0;
		}
	}

	if(strcmp(product_model, "ZC-GW03-3"))
	{
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	huayi_uci_get_int(PKG_STATUS_MODEM, "modem", "sim_ready", &i_ready);

	if(i_ready == 1 && network_status == 0)
	{
		do_internet_led_slowly();
	}else if(i_ready == 1 && network_status == 1)
	{
		do_internet_led_on();
	}else
	{
		do_internet_led_off();
	}
#else
	if(1 == network_status)
	{
		do_internet_led_on();
	}else
	{
		do_internet_led_off();
	}
#endif
	}
	
#if defined(CONFIG_HUAYI_4GONLINE_LED_SUPPORT)
	huayi_uci_get_int(PKG_STATUS_MODEM, "modem", "sim_ready", &i_ready);
	if(i_ready == 1 && network_status == 1)
	{
		char ifname[IPV4_STR_LEN] = {0};
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ifname", ifname);
		if(!strcmp(ifname, "eth1") || !strcmp(ifname, "wwan0"))
		{
			do_4G_online_led_on();
		}else
		{
			do_4G_online_led_off();
		}
	}else
	{
		do_4G_online_led_off();
	}
#endif	

#if defined(CONFIG_HUAYI_SIGNAL_LED_SUPPORT)
	if(1 == network_status)
	{
		int i_signal;
		atcmd_get_signal(&i_signal);
		if(i_signal > 67)
		{

			do_signal_led_high();
		}else if(i_signal > 34)
		{

			do_signal_led_middle();
		}else if(i_signal > 0)
		{

			do_signal_led_low();
		}else
		{

			do_signal_led_off();
		}//rssi=csq*2-113,i_signal=csq*100/31
	}else
	{

		do_signal_led_off();
	}	
#endif

	huayi_core_add_task(HUAYI_INTERNET_LED_DELAY_TIME,HUAYI_START_CHECK_NETWORK_LED_TASK,huayi_core_start_internet_led,Agent);
	
	return;
}
#endif

void huayi_core_update_led_status(struct VcoreAgent *Agent)
{
#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
	huayi_core_update_wps_led_status(Agent);
#endif
	
	return;
}

#if defined(CONFIG_HUAYI_SIM_LED_SUPPORT)
void huayi_core_start_sim_led(struct VcoreAgent *Agent)
{
	int i_ready = 0;
	atcmd_get_cpin(&i_ready);
	if(i_ready == 1)
	{
		do_sim_led_on();
	}else
	{
		do_sim_led_off();
	}
	huayi_core_add_task(HUAYI_INTERNET_LED_DELAY_TIME,HUAYI_START_CHECK_SIM_LED_TASK,huayi_core_start_sim_led,Agent);
	return;
}
#endif

#if defined(CONFIG_HUAYI_SUPPORT_FLUSH_ARP)
void huayi_core_flush_arp_list(void* arg)
{
	int arp_flush = 0;
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","arp_flush",&arp_flush);
	
	if (arp_flush == 1)
	{
		if (Agent->arp_flush_count%5 == 0)
		{
			flush_arp_list();
			system("huayi_cmd set system arp_flush 0");
		}

		Agent->arp_flush_count++;
	}
	else
	{
		Agent->arp_flush_count = 0;
	}
	
	huayi_core_add_task(HUAYI_ARP_FLUSH_CHECK_INTERVAL,HUAYI_SYSTEM_FLUSH_ARP_TASK,huayi_core_flush_arp_list,Agent);
	
	return;
}
#endif

void huayi_core_update_status(void* arg)
{	
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	huayi_core_update_cpu_mem_usage(Agent);

	huayi_core_update_network_statistics(Agent);

	huayi_core_update_led_status(Agent);
	
	huayi_core_add_task(STATISTICS_UPDATE_INTERVAL,HUAYI_UPTATE_STATISTICS_TASK,huayi_core_update_status,Agent);
	
	return;
}

void huayi_core_schedule_reboot(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	int rectime = 0;
	int uptime = 0;
	int rec_uptime = 0;
	int sche_reboot_mode = HUAYI_SCHE_REBOOT_MODE_OFF;
	char cmd_str[CMD_STR_LEN] = {0};
	int weeks = 0;
	int hour = 0;
	int minute = 0;
	time_t timep;
	struct tm *p = NULL;
	int interval = 0;
	int i = 0;
	int k = 0;
	VTBool this_week = VT_FALSE;
	int cur_time = 0;
	int next_time = 0;

	huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","sche_reboot_mode",&sche_reboot_mode);
	switch (sche_reboot_mode)
	{
		case HUAYI_SCHE_REBOOT_MODE_REC_TIME:
			if(!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","rectime",&rectime))
			{
				return;
			}
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rec_uptime",&rec_uptime);
			if(rec_uptime == 0)
			{
				snprintf(cmd_str,sizeof(cmd_str),"huayi_cmd set system rec_uptime %d\n",uptime);
				system(cmd_str);
			}else if((rec_uptime+rectime) <= uptime)
			{
				system("huayi-reset reboot");
				return;
			}
			break;
		case HUAYI_SCHE_REBOOT_MODE_SPE_TIME:
			if(!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","weeks",&weeks)||
				!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","hour",&hour)||
				!huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","minute",&minute))
			{
				return;
			}
			
			if(weeks==0)
			{
				return;
			}
			
			time(&timep);
			
			p=localtime(&timep);
			
			i = p->tm_wday;
			
			cur_time = (p->tm_wday *1440 + p->tm_hour*60 + p->tm_min);

			weeks = (weeks == 255 ? 127 : weeks);

			for(k=i;k<7;k++)
			{
				if(MyGetField32(weeks,k))
				{
					next_time = ( k * 1440 +hour *60+minute);
					
					if(next_time >= cur_time)
					{
						/*find a day this week*/
						
						this_week = VT_TRUE ;
						break;
					}
				}
			}

			if(this_week)
			{
				interval= (next_time - cur_time);
			}else
			{	
				/*get first day next week*/
				for(k=0;k<=i;k++)
				{
					if(MyGetField32(weeks,k))
					{
						/*first day need reboot in next  week*/
						next_time = ( k * 1440 +hour *60+minute);
						interval = next_time + ((1440 * 7 ) - cur_time);
						break;
					}
				}
			}
			
			if(interval <= 0)
			{
				system("huayi-reset reboot");
				return;
			}
			
			break;
		case HUAYI_SCHE_REBOOT_MODE_OFF:
		default:
			system("huayi_cmd set system rec_uptime 0");
			break;
	}

	
	huayi_core_add_task(AVG_USAGE_UPDATE_INTERVAL,HUAYI_SCHE_REBOOT_TASK,huayi_core_schedule_reboot,Agent);
	return;
}


void huayi_core_timer_ifup_wan(void* arg)
{
	system("ifup wan");

	return;
}

void huayi_core_timer_udhcpd(void* arg)
{
	system("/etc/init.d/huayi_udhcpd restart");
	
	return;
}

int getWscStatus(char *interface)
{
	int socket_id;
	struct iwreq wrq;
	int data = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(wrq.ifr_name, interface);
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;
	if( ioctl(socket_id, RT_PRIV_IOCTL, &wrq) == -1)
		printf("ioctl error\n");
	close(socket_id);
	return data;
}

void huayi_wps_connect(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	int status = 0;
	char strcmdbuf[TEMP_STR_LEN] = {0 };
	
	status = getWscStatus(Agent->wps_interface);

	Agent->wps_timer_count++;
	
	if(status == 2 || status == 0) {	// 2 == "WSC Process Fail"; 0 == not used
		huayi_uci_set_int(PKG_WPS, "wps", "wpsstatus", status);
		huayi_uci_commit(PKG_WPS);
		snprintf(strcmdbuf, sizeof(strcmdbuf), "iwpriv %s set WscConfStatus=1", Agent->wps_interface);
		system(strcmdbuf);
		snprintf(strcmdbuf, sizeof(strcmdbuf), "iwpriv %s set WscConfMode=0", Agent->wps_interface);
		system(strcmdbuf);
		snprintf(strcmdbuf, sizeof(strcmdbuf), "iwpriv %s set WscStop=1", Agent->wps_interface);
		system(strcmdbuf);
		Agent->wps_timer_count = 0;
		return;
	} else if(status == 34) {
		huayi_uci_set_int(PKG_WPS, "wps", "wpsstatus", status);
		huayi_uci_commit(PKG_WPS);
		Agent->wps_timer_count = 0;
		Agent->old_reset_button_state = BUTTON_INIT_STATE;
#if defined(CONFIG_HUAYI_WPS_LED_SUPPORT)
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
#elif defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
		Agent->special_led_status = LED_RESET;	
		Agent->wps_downlink_status = WPS_INIT;
#endif
		return;
	}
	if(Agent->wps_timer_count > HUAYI_WPS_TIME_COUNT) {
		huayi_uci_set_int(PKG_WPS, "wps", "wpsstatus", 35);
		huayi_uci_commit(PKG_WPS);
		snprintf(strcmdbuf, sizeof(strcmdbuf), "iwpriv %s set WscConfStatus=1", Agent->wps_interface);
		system(strcmdbuf);
		snprintf(strcmdbuf, sizeof(strcmdbuf), "iwpriv %s set WscConfMode=0", Agent->wps_interface);
		system(strcmdbuf);
		snprintf(strcmdbuf, sizeof(strcmdbuf), "iwpriv %s set WscStop=1", Agent->wps_interface);
		system(strcmdbuf);
		Agent->wps_timer_count = 0;
		return;
	}
	
	huayi_core_add_task(10, HUAYI_WPS_CONNECT_TASK, huayi_wps_connect,Agent);
	return;
}

void huayi_core_wifi_schedule_up(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	Agent->wifi_sche_down = VT_FALSE;

	system("huayi_cmd set system wifi_off 0");

	/*use relod wireless avoid issue*/
	system("huayi-reset wireless");

	//huayi_core_wifi_schedule_down(Agent);

	return;
	
}

void huayi_core_wifi_schedule_down(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	
	int i = 0,j = 0,k = 0;
	
	int week = 0;
	int enabled = 0;
	int start_time = 0,end_time = 0;	
	char section[OPTION_STR_LEN] = {0};
	char time_list[TEMP_STR_LEN] = {0};
	char period_time[OPTION_STR_LEN] = {0};
	char tmp[OPTION_STR_LEN] = {0};

	time_t timep;
	struct tm *p = NULL;
	int cur_time = 0;
	int flag = 0;
	int is_down = 0;
	int day = 0;

	huayi_uci_get_int(PKG_SCHE,"wifi_sche","enabled",&enabled);

	if(0==enabled)
	{
		//huayi_core_delete_first_task_by_id(HUAYI_WIFI_SCHED_DOWN_TASK);
		if(VT_TRUE == Agent->wifi_sche_down)
		{
			Agent->wifi_sche_down = VT_FALSE;
			system("huayi_cmd set system wifi_off 0");
			system("huayi-reset wireless");
		}
		return;
	}
	while(1)
	{
		memset(section,0,OPTION_STR_LEN);
		memset(time_list,0,TEMP_STR_LEN);
		j = 0;
		week = 0;
		is_down = 0;
		snprintf(section,sizeof(section),"@sche_list[%d]",i);
		i++;

		if(!huayi_uci_get_int(PKG_SCHE,section,"week",&week))
		{
			break;
		}

		if(!huayi_uci_get_str(PKG_SCHE,section,"time",time_list))
		{
			break;
		}

		while(huayi_get_nth_value(j++, time_list, ' ', period_time, sizeof(period_time)))
		{
			flag = 0;
			k = 0;
			while(huayi_get_nth_value(k++, period_time, '-', tmp, sizeof(tmp)))
			{
				if(k == 1)
				{
					start_time = atoi(tmp);
				}else
				{
					end_time = atoi(tmp);
					flag = 1;
				}
			}

			if(flag == 1)
			{
				time(&timep);
				p=localtime(&timep);

				day = p->tm_wday;
				cur_time = (p->tm_hour*60 + p->tm_min);

				week = (week == 255 ? 127 : week);

				if(MyGetField32(week,day))
				{
					if(cur_time < end_time && cur_time >= start_time)
					{

					   if(Agent->wifi_sche_down  == VT_FALSE)
					   {
							system("ifconfig ra0 down");
							system("ifconfig ra1 down");
							system("ifconfig ra2 down");
							system("ifconfig ra3 down");
							system("huayi_cmd set system wifi_off 1");
							#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)		
							system("ifconfig rai0 down");
							system("ifconfig rai1 down");
							system("ifconfig rai2 down");
							system("ifconfig rai3 down");
							#endif
							Agent->wifi_sche_down = VT_TRUE;
						}

						is_down = 1;
						goto end;

					}
				
				}
				
			}
		}
		
	}

end:
	if(is_down == 0 && Agent->wifi_sche_down  == VT_TRUE)
	{
		huayi_core_wifi_schedule_up(Agent);
	}
	//huayi_core_delete_first_task_by_id(HUAYI_WIFI_SCHED_DOWN_TASK);

	huayi_core_add_task(30,HUAYI_WIFI_SCHED_DOWN_TASK,huayi_core_wifi_schedule_down,Agent);

	return;
}



void huayi_core_mount(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	int is_usb = 0;
	int up_time = 0;
	char strcmdbuf[TEMP_STR_LEN] = {0};
	char resultbuf[CMD_STR_LEN] = {0};
	char mnt_path[OPTION_STR_LEN] = {0};
	char drive_path[OPTION_STR_LEN] = {0};
	char ftp_path[OPTION_STR_LEN] = {0};
	huayi_uci_get_int(PKG_USB_CONFIG,"ftp_service","is_usb",&is_usb);
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&up_time);
	if(is_usb == 0)
	{
		return;
	}

	huayi_uci_get_str(PKG_USB_CONFIG,"ftp_service","drive_path",drive_path);
	huayi_uci_get_str(PKG_USB_CONFIG,"ftp_service","mnt_path",mnt_path);
	huayi_uci_get_str(PKG_USB_CONFIG,"ftp_service","ftp_path",ftp_path);

	sprintf(strcmdbuf,"mount | grep \"%s\"",mnt_path);
	get_cmd_result(strcmdbuf,resultbuf,sizeof(resultbuf));
	if(strlen(resultbuf) > 0)
	{
		return;
	}

	if (up_time > 90)
	{
		memset(strcmdbuf,0,sizeof(strcmdbuf));
		sprintf(strcmdbuf,"/usr/bin/ntfs-3g -o umask=000 %s %s",drive_path,mnt_path);
		system(strcmdbuf);
		system("huayi-reset ftpd");
	}

	huayi_core_add_task(10,HUAYI_RELOAD_MOUNT_TASK,huayi_core_mount,Agent);
	return;
}

void huayi_core_ping(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
    
	system("huayi_detect_network www.baidu.com www.bjzhongyi.com www.haidilao.com");

	huayi_core_add_task(60,HUAYI_RELOAD_MOUNT_TASK,huayi_core_ping,Agent);
	return;
}


void huayi_utimer_functions_start(struct VcoreAgent *Agent)
{
	int sys_mode = 0;
	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&sys_mode);

	huayi_core_update_status(Agent);

	//huayi_core_schedule_reboot(Agent);
	huayi_core_add_task(120,HUAYI_SCHE_REBOOT_TASK,huayi_core_schedule_reboot,Agent);

	huayi_core_wifi_schedule_down(Agent);

#if defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
	huayi_core_delay_start_special_led(Agent);
#endif

#if defined(CONFIG_HUAYI_INTERNET_LED_SUPPORT)
	huayi_core_start_internet_led(Agent);
#endif

#if defined(CONFIG_HUAYI_SIM_LED_SUPPORT)
	huayi_core_start_sim_led(Agent);
#endif

#if defined(CONFIG_HUAYI_SUPPORT_FLUSH_ARP)
	huayi_core_flush_arp_list(Agent);
#endif
	
#if defined(CONFIG_HUAYI_REPEATER_LED_SUPPORT)
	huayi_core_repeater_led_status(Agent);
#endif

#if defined(CONFIG_PACKAGE_VSFTPD)
	huayi_core_add_task(60,HUAYI_RELOAD_MOUNT_TASK,huayi_core_mount,Agent);
#endif

#if defined(CONFIG_HUAYI_DETECTION_NETWORK_SUPPORT)
        huayi_core_add_task(60,HUAYI_RELOAD_MOUNT_TASK,huayi_core_ping,Agent);
#endif

	
	if(sys_mode == HUAYI_WISP_MODE || sys_mode == HUAYI_REP_MODE)
	{
		//huayi_core_wifi_timer_scan(Agent);
		huayi_core_add_task(180,HUAYI_WIFI_SCAN_TASK,huayi_core_wifi_timer_scan,Agent);
	}
	return;
}

