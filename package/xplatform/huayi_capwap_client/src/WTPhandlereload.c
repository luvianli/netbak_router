/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "WTPhandlereload.h"

extern WTPBool gRunAgent;

extern pthread_mutex_t config_mutex;
extern pthread_mutex_t reload_mutex;

WTP_CONFIG_RECEIVE	huayi_capwap_config;


void WTPExecuteNeedReloadWireless(void)
{

	//gNeedReloadWireless = CW_TRUE;
	pthread_mutex_unlock(&reload_mutex);
	return;
}


void WTPDoReset(struct VcapwapAgent* Agent)
{
	system("rm -rf /overlay/*");
	system("reboot -d 3");

	system("echo timer > /sys/class/leds/wifi-led/trigger");
	system("echo 50 > /sys/class/leds/wifi-led/delay_on");
	system("echo 50 > /sys/class/leds/wifi-led/delay_off");
	
	return;
}
void WTPDoReboot(struct VcapwapAgent* Agent)
{
	system("reboot -f");
	return;
}
void WTPDoLedOn(struct VcapwapAgent* Agent)
{
	char cmd[CMD_STR_SIZE]={0};
	system("echo timer > /sys/class/leds/wifi-led/trigger");
	system("echo 150 > /sys/class/leds/wifi-led/delay_on");
	system("echo 150 > /sys/class/leds/wifi-led/delay_off");
	system(cmd);
	return;
}
void WTPDoLedOff(struct VcapwapAgent* Agent)
{
	char cmd[CMD_STR_SIZE]={0};
	int wifi_button = 0, wifi_disable_5G = 0, wifi_disable_2G = 0;
	//sprintf(cmd, "/etc/action.d/led wifi_led");

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","wifi_button",&wifi_button);

	if(0 == wifi_button)
	{
		huayi_uci_get_int(PKG_WIRELESS,"radio1","disabled",&wifi_disable_5G);
		huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&wifi_disable_2G);

		if(0 == wifi_disable_5G || 0 == wifi_disable_2G)
		{
			system("echo timer > /sys/class/leds/wifi-led/trigger");
			system("echo 2000 > /sys/class/leds/wifi-led/delay_on");
			system("echo 1000 > /sys/class/leds/wifi-led/delay_off");
		}
		else
		{
			system("echo 0 >/sys/class/leds/wifi-led/brightness");
		}
	}
	else if(1 == wifi_button)
	{
		system("echo 0 >/sys/class/leds/wifi-led/brightness");
	}
	
	system(cmd);
	return;
}

WTPBool WtpFirmware_update(void * arg)
{
	struct VcapwapAgent *Agent=(struct VcapwapAgent *)arg;
	
	char sys_cmd[CMD_STR_SIZE];

	memset(sys_cmd, 0, CMD_STR_SIZE);
	snprintf(sys_cmd, sizeof(sys_cmd), "wget  http://%s/%s -O %s",Agent->ac_ip,Agent->firmware_url,FIRMWARE_NAME);
	printf("sys_cmd is %s\n",sys_cmd);
	system(sys_cmd);
	
	cut_upload_file(FIRMWARE_NAME,FIRMWARE_NAME_BIN);
	remove(FIRMWARE_NAME);

	if(0 == access(FIRMWARE_NAME_BIN,F_OK))
	{
		/*do upgrade*/
		printf("%s ready to upgrade\n",FIRMWARE_NAME_BIN);
		snprintf(sys_cmd, sizeof(sys_cmd), "sysupgrade -n %s",FIRMWARE_NAME_BIN);
		printf("sys_cmd is %s\n",sys_cmd);
		system(sys_cmd);
	}
	
	pthread_exit(NULL);
}


void start_firmware_update(struct VcapwapAgent* Agent)
{
	pthread_t aPFirmware;
	
	pthread_create(&aPFirmware, NULL, (void*)WtpFirmware_update, Agent);

	return ;

}
int  uci_set_vap_option(int index,char *option,char *value) {
	char buf[128] = {0};
	sprintf(buf,"@wifi-iface[%d]",index);
	return huayi_uci_set_str(PKG_WIRELESS,buf,option,value);
}

int  uci_set_vap_num_option(int index,char *option,int value) {
	char buf[128] = {0};
	sprintf(buf,"@wifi-iface[%d]",index);
	return huayi_uci_set_int(PKG_WIRELESS,buf,option,value);
}
int  uci_set_radio_option(int index,char *option,char *value) {
	char buf[128] = {0};
	sprintf(buf,"radio%d",index);
	return huayi_uci_set_str(PKG_WIRELESS,buf,option,value);
}

int  uci_set_radio_num_option(int index,char *option,int value) {
	char buf[128] = {0};
	sprintf(buf,"radio%d",index);
	return huayi_uci_set_int(PKG_WIRELESS,buf,option,value);
}

unsigned int calc_tx_power_by_percent(unsigned char wirelessmode,unsigned char percentage)
{
	int max_tx = 0;
	int min_tx = 0;

	switch(wirelessmode)
	{
		case WIRELESSMODE_11B:
			huayi_uci_get_int(PKG_CAPABILITY,"11b","max_txpower",&max_tx);
			huayi_uci_get_int(PKG_CAPABILITY,"11b","min_txpower",&min_tx);
			break;
		case WIRELESSMODE_11A:
			huayi_uci_get_int(PKG_CAPABILITY,"11a","max_txpower",&max_tx);
			huayi_uci_get_int(PKG_CAPABILITY,"11a","min_txpower",&min_tx);
			break;
		case WIRELESSMODE_11G:
			huayi_uci_get_int(PKG_CAPABILITY,"11g","max_txpower",&max_tx);
			huayi_uci_get_int(PKG_CAPABILITY,"11g","min_txpower",&min_tx);
			break;
		case WIRELESSMODE_11N:
		case WIRELESSMODE_11BGN:
			huayi_uci_get_int(PKG_CAPABILITY,"11bgn","max_txpower",&max_tx);
			huayi_uci_get_int(PKG_CAPABILITY,"11bgn","min_txpower",&min_tx);
			break;
		case WIRELESSMODE_11AN:
			huayi_uci_get_int(PKG_CAPABILITY,"11an","max_txpower",&max_tx);
			huayi_uci_get_int(PKG_CAPABILITY,"11an","min_txpower",&min_tx);
			break;
		case WIRELESSMODE_11AC:
			huayi_uci_get_int(PKG_CAPABILITY,"11ac","max_txpower",&max_tx);
			huayi_uci_get_int(PKG_CAPABILITY,"11ac","min_txpower",&min_tx);
			break;
		default:
			max_tx = 20;
			break;
	}

	printf("mode:%d max_tx:%d\n",wirelessmode,max_tx);
	
	switch(percentage)
	{
		case 12:
			return (max_tx - 9);
		case 25:
			return (max_tx - 6);
		case 50:
			return (max_tx - 3);
		case 75:
			return (max_tx - 1);
		case 0:
			return min_tx;
		case 100:
		default:
			return max_tx;
	}
}

int set_radio_htmode_hwmode_option(int index, unsigned char int_band,unsigned char int_bw)
{
	char device_name[32] = {0};
	
	sprintf(device_name,"radio%d",index);
	
	if(WIRELESSMODE_11B == int_band)
	{
		huayi_uci_set_str(PKG_WIRELESS,device_name,"hwmode","11b");
		huayi_uci_delete_option(PKG_WIRELESS,device_name,"htmode");
		huayi_uci_delete_option(PKG_WIRELESS,device_name,"ht_coex");
	}
	else if(WIRELESSMODE_11G == int_band)
	{
		huayi_uci_set_str(PKG_WIRELESS,device_name,"hwmode","11g");
		huayi_uci_delete_option(PKG_WIRELESS,device_name,"htmode");
		huayi_uci_delete_option(PKG_WIRELESS,device_name,"ht_coex");
	}
	else if((WIRELESSMODE_11BGN == int_band) ||(WIRELESSMODE_11N == int_band))
	{
		huayi_uci_set_str(PKG_WIRELESS,device_name,"hwmode","11g");
		
		if(HTMODE_20MHZ == int_bw)
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","HT20");
			huayi_uci_set_str(PKG_WIRELESS,device_name,"ht_coex","0");
		}
		else if(HTMODE_40MHZ == int_bw)
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","HT40");
			huayi_uci_set_str(PKG_WIRELESS,device_name,"ht_coex","0");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","HT40");
			huayi_uci_set_str(PKG_WIRELESS,device_name,"ht_coex","1");
		}
	}
	else if(WIRELESSMODE_11A == int_band)
	{
		huayi_uci_set_str(PKG_WIRELESS,device_name,"hwmode","11a");
		huayi_uci_delete_option(PKG_WIRELESS,device_name,"htmode");
		huayi_uci_delete_option(PKG_WIRELESS,device_name,"ht_coex");
	}
	else if(WIRELESSMODE_11AN == int_band)
	{
		huayi_uci_set_str(PKG_WIRELESS,device_name,"hwmode","11a");
		
		if(HTMODE_20MHZ == int_bw)
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","HT20");
			huayi_uci_set_str(PKG_WIRELESS,device_name,"ht_coex","0");
		}
		else if(HTMODE_40MHZ == int_bw)
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","HT40");
			huayi_uci_set_str(PKG_WIRELESS,device_name,"ht_coex","0");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","HT40");
			huayi_uci_set_str(PKG_WIRELESS,device_name,"ht_coex","1");
		}
	}
	else if(WIRELESSMODE_11AC == int_band)
	{
		huayi_uci_set_str(PKG_WIRELESS,device_name,"hwmode","11a");
		huayi_uci_delete_option(PKG_WIRELESS,device_name,"ht_coex");
		if(HTMODE_20MHZ == int_bw)
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","VHT20");
		}
		else if(HTMODE_40MHZ == int_bw)
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","VHT40");
		}
		else if(HTMODE_80MHZ == int_bw)
		{
			huayi_uci_set_str(PKG_WIRELESS,device_name,"htmode","VHT80");
		}
	}
	
	return 0;
}

void huayi_real_config_to_uci(unsigned char radioid)
{

	unsigned int txpower = 0;

	if(radioid > CW_MAX_RADIO)
	{
		return;
	}
	
	uci_set_radio_num_option(radioid,"disabled",!(huayi_capwap_config.radio_config[radioid].if_admin));

	if((0 == huayi_capwap_config.radio_config[radioid].channel) || (1 == huayi_capwap_config.sys_config.gloabal_auto_channel))
	{
		uci_set_radio_option(radioid,"channel","auto");
	}else
	{
		uci_set_radio_num_option(radioid,"channel",huayi_capwap_config.radio_config[radioid].channel);
	}

	
	uci_set_radio_num_option(radioid,"set_min_snr_en",huayi_capwap_config.sys_config.gloabal_roaming);

	if(huayi_capwap_config.sys_config.gloabal_roaming)
	{
		if(radioid == 0)
			uci_set_radio_num_option(radioid,"set_min_snr",huayi_capwap_config.sys_config.roaming_threshold_2g);
		else if(radioid == 1)
			uci_set_radio_num_option(radioid,"set_min_snr",huayi_capwap_config.sys_config.roaming_threshold_5g);
	}else
	{
		uci_set_radio_num_option(radioid,"set_min_snr",0);
	}
	
	uci_set_radio_option(radioid,"country",(char *)huayi_capwap_config.radio_config[radioid].country);

	uci_set_radio_num_option(radioid,"wifimode",huayi_capwap_config.radio_config[radioid].wirelessmode);
	uci_set_radio_num_option(radioid,"bw",huayi_capwap_config.radio_config[radioid].htmode);

	uci_set_vap_num_option(radioid,"maxstanum",huayi_capwap_config.ssid_config[radioid].maxsta);

	//vap
	if(WIRELESSMODE_11N == huayi_capwap_config.radio_config[radioid].wirelessmode){
		uci_set_vap_num_option(radioid,"puren",1);
	}else
	{
		uci_set_vap_num_option(radioid,"puren",0);
	}

	//txpower = calc_tx_power_by_percent(huayi_capwap_config.radio_config[radioid].wirelessmode,huayi_capwap_config.radio_config[radioid].txpower);
	txpower = huayi_capwap_config.radio_config[radioid].txpower;

	printf("radioid %d txpower is %d\n",radioid,txpower);
	
	uci_set_radio_num_option(radioid,"txpower",txpower);
	
	uci_set_radio_num_option(radioid,"beacon",huayi_capwap_config.radio_config[radioid].beacon);
	
	uci_set_vap_option(radioid,"ssid",(char *)huayi_capwap_config.ssid_config[radioid].ssid);
	uci_set_vap_num_option(radioid,"hidden",huayi_capwap_config.ssid_config[radioid].hide);
	uci_set_vap_num_option(radioid,"noforward",huayi_capwap_config.ssid_config[radioid].isolate);
	
	uci_set_vap_num_option(radioid,"vlan",huayi_capwap_config.ssid_config[radioid].vlanid);

	if(0==huayi_capwap_config.ssid_config[radioid].encry){//不加密
		uci_set_vap_option(radioid,"encryption","none");
	}else{//加密 默认wpa wap2 混合
		uci_set_vap_option(radioid,"encryption","psk-mixed+tkip+ccmp");
		uci_set_vap_option(radioid,"key",(char *)huayi_capwap_config.ssid_config[radioid].passphrase);
	}

	huayi_uci_commit(PKG_WIRELESS);
	return;
}

void wirless_config_to_uci(struct VcapwapAgent *Agent)
{
	unsigned char radioid;

	if(Agent->radiotype & CW_RADIO_2G)
	{
		radioid = 0;
		huayi_real_config_to_uci(radioid);
	}
	
	if(Agent->radiotype & CW_RADIO_5G)
	{
		radioid = 1;
		huayi_real_config_to_uci(radioid);
	}
	
	return;
}
void CWExecuteReload(void * arg)
{
	printf("Thread CWExecuteReload: Started!\n");
	
	struct VcapwapAgent *Agent=(struct VcapwapAgent *)arg;
	memset(&huayi_capwap_config,0,sizeof(WTP_CONFIG_RECEIVE));
	
	while(gRunAgent == WTP_TRUE)
	{
		
		pthread_mutex_lock(&reload_mutex); 

		pthread_mutex_lock(&config_mutex);
		// do real config
		wirless_config_to_uci(Agent);
			
		pthread_mutex_unlock(&config_mutex);
		system("huayi-reset wireless");
	}
	pthread_exit(NULL);
	printf("Thread CWExecuteReload: Exit!\n");
	return;

}


