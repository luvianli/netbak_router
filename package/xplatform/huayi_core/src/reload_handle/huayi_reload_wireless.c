#include "huayi_handle_reload.h"
#include "wps.h"
#include "oid.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <errno.h>
#include "huayi_utimer_functions.h"


unsigned int get_wsc_pincode(char *interface)
{
	int socket_id;
	struct iwreq wrq;
	unsigned int data = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(wrq.ifr_name, interface);
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_PIN_CODE;
	if( ioctl(socket_id, RT_PRIV_IOCTL, &wrq) == -1)
		printf("ioctl error\n");
	close(socket_id);
	return data;
}

VTBool huayi_wps_pbc(void * arg,cJSON *input_root)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	int para = 0;
	char strcmdbuf[CMD_STR_LEN] = { 0 };
	char devname2g[SMALL_STR_LEN] = "ra0";
	int radio2g_disable = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int radio5g_disable = 0;
	char devname5g[SMALL_STR_LEN] = "rai0";
	
	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&radio5g_disable);
#endif

	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&radio2g_disable);

	huayi_json_get_int(input_root, "parameter", &para);
	switch (para) {
		case 0:   //disconnect
			if (radio2g_disable == 0)
			{
				XCMD(strcmdbuf, "iwpriv %s set WscConfStatus=1", devname2g);
				XCMD(strcmdbuf, "iwpriv %s set WscConfMode=0", devname2g);
				XCMD(strcmdbuf, "iwpriv %s set WscStop=1", devname2g);
			}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			if (radio5g_disable == 0)
			{
				XCMD(strcmdbuf, "iwpriv %s set WscConfStatus=1", devname5g);
				XCMD(strcmdbuf, "iwpriv %s set WscConfMode=0", devname5g);
				XCMD(strcmdbuf, "iwpriv %s set WscStop=1", devname5g);
			}
#endif
			break;
		case 2:
			XCMD(strcmdbuf, "iwpriv %s set WscConfStatus=2", devname2g);
			XCMD(strcmdbuf, "iwpriv %s set WscConfMode=7", devname2g);
			XCMD(strcmdbuf, "iwpriv %s set WscMode=2", devname2g);
			XCMD(strcmdbuf, "iwpriv %s set WscGetConf=1", devname2g);
			huayi_core_delete_first_task_by_id(HUAYI_WPS_CONNECT_TASK);
			Agent->wps_timer_count = 0;
			strcpy(Agent->wps_interface, "ra0");
			huayi_wps_connect(arg);
			break;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		case 5:
			XCMD(strcmdbuf, "iwpriv %s set WscConfStatus=2", devname5g);
			XCMD(strcmdbuf, "iwpriv %s set WscConfMode=7", devname5g);
			XCMD(strcmdbuf, "iwpriv %s set WscMode=2", devname5g);
			XCMD(strcmdbuf, "iwpriv %s set WscGetConf=1", devname5g);
			huayi_core_delete_first_task_by_id(HUAYI_WPS_CONNECT_TASK);
			Agent->wps_timer_count = 0;
			strcpy(Agent->wps_interface, "rai0");
			huayi_wps_connect(arg);
			break;
#endif
		default:
			break;
	}

	return VT_TRUE;
}

VTBool huayi_wps_pin(void * arg,cJSON *input_root)
{
	int para = 0, initflag = 0;
	char strcmdbuf[CMD_STR_LEN] = { 0 };
	int pincode = 0, stapincode = 0;
	int radio2g_disable = 0;
	int pincodeenable = 0;
	char devname2g[SMALL_STR_LEN] = "ra0";
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int radio5g_disable = 0;
	char devname5g[SMALL_STR_LEN] = "rai0";

	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&radio5g_disable);
#endif
	
	huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&radio2g_disable);
	
	huayi_json_get_int(input_root, "parameter", &para);
	huayi_uci_get_int(PKG_WPS,"wps","stapincode", &stapincode);
	
	switch (para) {
		case 0:   //disconnect
			if (radio2g_disable == 0)
			{
				XCMD(strcmdbuf, "iwpriv %s set WscConfStatus=1", devname2g);
				XCMD(strcmdbuf, "iwpriv %s set WscConfMode=0", devname2g);
				XCMD(strcmdbuf, "iwpriv %s set WscStop=1", devname2g);
			}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			if (radio5g_disable == 0)
			{
				XCMD(strcmdbuf, "iwpriv %s set WscConfStatus=1", devname5g);
				XCMD(strcmdbuf, "iwpriv %s set WscConfMode=0", devname5g);
				XCMD(strcmdbuf, "iwpriv %s set WscStop=1", devname5g);
			}
#endif
			break;
		case 1:
			/*initialize pincode*/
			huayi_uci_get_int(PKG_WPS,"wps","initpincodeflg", &initflag);
			if(0 == initflag) 
			{
				if (radio2g_disable == 0)
				{
					pincode = get_wsc_pincode(devname2g);
				}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
				else if (radio5g_disable == 0)
				{
					pincode = get_wsc_pincode(devname5g);
				}
				
				huayi_uci_set_int(PKG_WPS,"wps","initpincode5g", pincode);
				huayi_uci_set_int(PKG_WPS,"wps","own5gpincode", pincode);
#endif
				
				huayi_uci_set_int(PKG_WPS,"wps","initpincode2g", pincode);
				huayi_uci_set_int(PKG_WPS,"wps","own2gpincode", pincode);

				huayi_uci_set_int(PKG_WPS,"wps","initpincodeflg",1);
			} 
			else 
			{
				if (radio2g_disable == 0)
				{
					huayi_uci_get_int(PKG_WPS,"wps","initpincode2g", &pincode);
					XCMD(strcmdbuf, "iwpriv %s set WscVendorPinCode=%d", devname2g, pincode);
				}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
				if (radio5g_disable == 0)
				{
					huayi_uci_get_int(PKG_WPS,"wps","initpincode5g", &pincode);
					XCMD(strcmdbuf, "iwpriv %s set WscVendorPinCode=%d", devname5g, pincode);
				}
#endif
			}
			break;
		case 2:
			XCMD(strcmdbuf, "iwpriv %s set WscConfStatus=2", devname2g);
			XCMD(strcmdbuf, "iwpriv %s set WscConfMode=7", devname2g);
			XCMD(strcmdbuf, "iwpriv %s set WscMode=1", devname2g);
			XCMD(strcmdbuf, "iwpriv %s set WscGetConf=1", devname2g);
			XCMD(strcmdbuf, "iwpriv %s set WscPinCode=%d", devname2g, stapincode);
			huayi_core_delete_first_task_by_id(HUAYI_WPS_CONNECT_TASK);
			Agent->wps_timer_count = 0;
			strcpy(Agent->wps_interface, "ra0");
			huayi_wps_connect(arg);
			break;
		case 3:
			/*random pincode*/
			if (radio2g_disable == 0)
			{
				XCMD(strcmdbuf, "iwpriv %s set WscGenPinCode=1", devname2g);
			}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			else if (radio5g_disable == 0)
			{
				XCMD(strcmdbuf, "iwpriv %s set WscGenPinCode=1", devname5g);
			}
#endif
			
			usleep(5000
);

			if (radio2g_disable == 0)
			{
				pincode = get_wsc_pincode(devname2g);
			}
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			else if (radio5g_disable == 0)
			{
				pincode = get_wsc_pincode(devname5g);
			}

			if (radio2g_disable == 0 && radio5g_disable == 0)
			{
				XCMD(strcmdbuf, "iwpriv %s set WscVendorPinCode=%d", devname5g, pincode);
			}
			huayi_uci_set_int(PKG_WPS,"wps","own5gpincode", pincode);
#endif
			huayi_uci_set_int(PKG_WPS,"wps","own2gpincode", pincode);
			break;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
		case 5:
			XCMD(strcmdbuf, "iwpriv %s set WscConfStatus=2", devname5g);
			XCMD(strcmdbuf, "iwpriv %s set WscConfMode=7", devname5g);
			XCMD(strcmdbuf, "iwpriv %s set WscMode=1", devname5g);
			XCMD(strcmdbuf, "iwpriv %s set WscGetConf=1", devname5g);
			XCMD(strcmdbuf, "iwpriv %s set WscPinCode=%d", devname5g, stapincode);
			huayi_core_delete_first_task_by_id(HUAYI_WPS_CONNECT_TASK);
			Agent->wps_timer_count = 0;
			strcpy(Agent->wps_interface, "rai0");
			huayi_wps_connect(arg);
			break;
#endif
		case 6:
			huayi_uci_get_int(PKG_WPS,"wps","pincodeenable",&pincodeenable);
			if (radio2g_disable == 0)
			{
				if (pincodeenable == 1)
				{
					XCMD(strcmdbuf, "iwpriv %s set WscConfMode=7", devname2g);
					XCMD(strcmdbuf, "iwpriv %s set WscMode=1", devname2g);
				}
				else
				{
					XCMD(strcmdbuf, "iwpriv %s set WscConfMode=7", devname2g);
					XCMD(strcmdbuf, "iwpriv %s set WscMode=2", devname2g);
				}
			}
		
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
			if (radio5g_disable == 0)
			{
				if (pincodeenable == 1)
				{
					XCMD(strcmdbuf, "iwpriv %s set WscConfMode=7", devname5g);
					XCMD(strcmdbuf, "iwpriv %s set WscMode=1", devname5g);
				}
				else
				{
					XCMD(strcmdbuf, "iwpriv %s set WscConfMode=7", devname5g);
					XCMD(strcmdbuf, "iwpriv %s set WscMode=2", devname5g);
				}
			}
#endif
			break;
		default:
			break;
	}

	huayi_uci_commit(PKG_WPS);
	return VT_TRUE;
}

void string2hex(char *str, int *intVal)
{
	uint8_t i,cnt=0;
	char *p = str;             
	sscanf(str,"%X",intVal);
	
}
VTBool huayi_reload_wireless(void * arg,cJSON *input_root)
{	
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
#if defined(CONFIG_HUAYI_WIFI_MT7628_WLANLED)
	char resultbuf[OPTION_STR_LEN] = {0};
	int ret = 0;
	int regdata = 0;
	int disabled = 0;
#endif
	if(VT_TRUE == Agent->wifi_sche_down)
	{
		return VT_TRUE;
	}
#if defined(CONFIG_HUAYI_MT7628_EXTERNAL_PA)
    /*for MTK7628DA 2.4G ext-pa signal*/
	FILE *fp=NULL;
	char value[16]={0}, *tmpP=NULL, cmdLine[100]={0};
	unsigned int intVal=0, intRes=0;
	fp = popen("devmem 0x10000060","r");
	if(fp){
		if(fgets(value,sizeof(value),fp)){
			if(value[0]=='0' && value[1]=='x'){
				tmpP = &(value[2]);
				lv_print("tmpP=%s\n",tmpP);
				string2hex(tmpP,&intVal);
				intVal |= (0x3<<6); // I2S_MODE
				intVal |= (0x3<<24);// UART1_MODE
				sprintf(cmdLine,"devmem 0x10000060 32 0x%X",intVal);
				lv_print("%s",cmdLine);
				system(cmdLine);
			}
		}
		pclose(fp);
	}
#endif

#if defined(CONFIG_HUAYI_WIFI_MT7628_WLANLED)
	ret = get_cmd_result("devmem 0x10000064",resultbuf,sizeof(resultbuf));
	if(ret == HUAYI_TRUE)
	{
		regdata  = strtol(resultbuf,NULL,16);
		regdata &= 0xfffffffc;
		huayi_uci_get_int(PKG_WIRELESS,"radio0_0","disabled",&disabled);
		if(disabled == 0)
		{
			regdata |= 0x0;
		}
		else
		{
			regdata |= 0x1;
		}

		memset(resultbuf,0,sizeof(resultbuf));
		sprintf(resultbuf,"devmem 0x10000064 32 0x%x",regdata);
		system(resultbuf);

	}
#endif
	system("wifi reload");
	sleep(10);
#if defined(CONFIG_HUAYI_BUTTON_WIFI_SUPPORT)
	system("/etc/action.d/wifi_button_check");
#endif

	system("huayi-reset wifiguesfilter");

	return VT_TRUE;
}

VTBool huayi_reload_wireless_scan(void * arg,cJSON *input_root)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	int scan_status = 0;
	char scan_status_str[SMALL_STR_LEN] = {0};
	int para = 0;
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int wlan_5g_disabled = 0;
	int disabled = 0;
	huayi_uci_get_int(PKG_WIRELESS,"radio1_cli0","disabled",&wlan_5g_disabled);
	huayi_uci_get_int(PKG_WIRELESS,"radio1_0","disabled",&disabled);
#endif

	huayi_json_get_int(input_root, "parameter", &para);

	get_cmd_result("huayi_cmd get system scan_state",scan_status_str,SMALL_STR_LEN);
	scan_status = atoi(scan_status_str);
	if(scan_status == 1)
	{	
		//return VT_TRUE;
	}

	if((para & WIFI_24G_SCAN) == WIFI_24G_SCAN)
	{
		system("iwpriv apcli0 set SiteSurvey=");
		sleep(5);
	}
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if((para & WIFI_58G_SCAN) == WIFI_58G_SCAN)
	{
		if (Agent->repeater_status_58g == 0 && wlan_5g_disabled == 0 && disabled == 0)
		{
			system("ifconfig apclii0 down");
		}
		system("iwpriv rai1 set SiteSurvey=");
		if (Agent->repeater_status_58g == 0 && wlan_5g_disabled == 0 && disabled == 0)
		{
			system("ifconfig apclii0 up");
		}
		sleep(15);
	}
#endif
	if((para & WIFI_24G_SCAN) == WIFI_24G_SCAN)
	{
		if(scan_status == 1)
		{
			system("iwpriv apcli0 get_site_survey > /tmp/ScanList");
		}else if(scan_status == 3)
		{
			system("iwpriv apcli0 get_site_survey > /tmp/ChannelScanList");
		}
	}
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if((para & WIFI_58G_SCAN) == WIFI_58G_SCAN)
	{
		if(scan_status == 1)
		{
			if((para & WIFI_ALL_SCAN) == WIFI_ALL_SCAN)
			{
				system("iwpriv rai1 get_site_survey >> /tmp/ScanList");
			}else
			{
				system("iwpriv rai1 get_site_survey > /tmp/ScanList");
			}
		}else if(scan_status == 3)
		{
			if((para & WIFI_ALL_SCAN) == WIFI_ALL_SCAN)
			{
				system("iwpriv rai1 get_site_survey >> /tmp/ChannelScanList");
			}else
			{
				system("iwpriv rai1 get_site_survey > /tmp/ChannelScanList");
			}
		}
		
	}
#endif

	if(scan_status == 1)
	{
		system("huayi_cmd set system scan_state 2");
	}else if(scan_status == 3)
	{
		system("huayi_cmd set system scan_state 4");
	}
	//system("/etc/action.d/huayi_scan");
	
	return VT_TRUE;
}


VTBool huayi_reload_wifi_sche(void * arg,cJSON *input_root)
{
	huayi_core_delete_first_task_by_id(HUAYI_WIFI_SCHED_DOWN_TASK);
	huayi_core_wifi_schedule_down(arg);

	return VT_TRUE;
}

int huayi_change_channel(int wifi_type)
{
	char *ptr = NULL;
	FILE *fp = NULL;
	char buff[LIST_STR_LEN] = {0};

	char rep_interface[OPTION_STR_LEN] = {0};
	char bssid[RESULT_STR_LEN] = {0};
	char bssid_str[RESULT_STR_LEN] = {0};
	int channel = 0,channel_str = 0;
	int flag = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if(wifi_type == WIFI_58G_SCAN)
	{
		huayi_uci_get_int(PKG_WIRELESS,W58G_DEV,"channel",&channel);
		huayi_uci_get_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_bssid",bssid);
		strcpy(rep_interface,W58G_DEV);
	}else
#endif
	{
		huayi_uci_get_int(PKG_WIRELESS,W24G_DEV,"channel",&channel);
		huayi_uci_get_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_bssid",bssid);
		strcpy(rep_interface,W24G_DEV);
	}

	
	fp = fopen("/tmp/ChannelScanList", "r");	
	if (NULL == fp) 
	{		
		return -1;	
	}	

	while(fgets(buff, sizeof(buff), fp) != NULL)	
	{
		if ((ptr = strrstr(buff,"BSSID:")) != NULL)
		{
			ptr = ptr + strlen("BSSID:");
			strncpy(bssid_str, ptr, 17);
			if(!strcmp(bssid_str,bssid))
			{
				flag = 1;
				break;
			}	
		}
	}

	if(flag == 1)
	{
		if ((ptr = strstr(buff,"Channel:")) != NULL)
		{
			ptr = ptr + strlen("Channel:");
			channel_str = atoi(ptr);

			//if(channel_str != channel)
			{
				huayi_uci_set_int(PKG_WIRELESS,rep_interface,"channel",channel_str);
				huayi_uci_commit(PKG_WIRELESS);
				system("huayi-reset wireless");
			}
		}	
	}

	return 0;
}


void huayi_core_wifi_timer_scan(void* arg)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;
	int wifi_timer_scan = 0;
	int scan_status = 0;
	char scan_status_str[SMALL_STR_LEN] = {0};
	char huayi_cmd[OPTION_STR_LEN] = {0};
	int rep_status = 0;
	int disabled = 0;
	int flag = 0;
	int wifi_scan_time = 0;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int disabled_5g = 0;
	int rep_status_5g = 0;
#endif
	
	huayi_uci_get_int(PKG_STATUS_SYSTEM, "sys", "rep_24g_status",&rep_status);
	huayi_uci_get_int(PKG_WIRELESS,"radio0_cli0","disabled",&disabled);
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_int(PKG_WIRELESS,"radio1_cli0","disabled",&disabled_5g);
	huayi_uci_get_int(PKG_STATUS_SYSTEM, "sys", "rep_58g_status",&rep_status_5g);
	if(rep_status_5g != 0 || rep_status != 0)
	{
		flag = 0;
		Agent->wifi_scan_status = 1;
	}else if(disabled_5g == 0)
	{
		flag = 1;
		Agent->wifi_scan_type = WIFI_58G_SCAN;
		Agent->wifi_scan_status += 1;
	}else
#endif
	{
		if(rep_status == 0 && disabled == 0)
		{
			flag = 1;
			Agent->wifi_scan_type = WIFI_24G_SCAN;
			Agent->wifi_scan_status += 1;
		}else
		{
			flag = 0;
			Agent->wifi_scan_status = 1;
		}
	}

	if(flag == 1)
	{
		get_cmd_result("huayi_cmd get system scan_state",scan_status_str,SMALL_STR_LEN);
		scan_status = atoi(scan_status_str);

		if(scan_status == 4)
		{
			huayi_change_channel(Agent->wifi_scan_type);
			system("huayi_cmd set system scan_state 0");
		}

		if(scan_status == 0 && Agent->wps_downlink_status != WPS_DOING)
		{
			sprintf(huayi_cmd,"huayi-reset scan parameter %d",Agent->wifi_scan_type);
			system(huayi_cmd);
			system("huayi_cmd set system scan_state 3");
		}
	}

	wifi_scan_time += Agent->wifi_scan_status*30;

	if(Agent->wifi_scan_status > 5)
	{
		Agent->wifi_scan_status = 5;
	}
	huayi_core_add_task(wifi_scan_time,HUAYI_WIFI_SCAN_TASK,huayi_core_wifi_timer_scan,Agent);
}
