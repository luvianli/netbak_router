#include "huayi_handle_reload.h"
#include "huayi_utimer_functions.h"

VTBool huayi_reload_network(void * arg,cJSON *input_root)
{
	char product_model[OPTION_STR_LEN] = {0};

#if defined(CONFIG_HUAYI_SPECIAL_LED_SUPPORT)
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	Agent->special_led_status = LED_RESET;
#endif

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	HuayiSystem("/etc/init.d/pptp stop", 0);
	HuayiSystem("/etc/init.d/xl2tp stop", 0);
#endif

	system("huayi_cmd set network wan_up 0");
	/*’Î∂‘’‚∏ˆ–Õ∫≈µƒ≤˙∆∑Ãÿ ‚¥¶¿Ì£¨”≤º˛…Ëº∆ƒ£øÈµÁ¬∑ ±£¨∆Ù∂Ø–Ë“™»Ìº˛¡¨Õ®µÁ¬∑*/
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	if(0 == strcmp(product_model, "4G1200-H002-HY06"))
	{
		system("/etc/init.d/huayi_dial4_init start");
	}

	//system("ifdown wan");
	system("/etc/init.d/network restart");

	return VT_TRUE;
}


VTBool huayi_reload_dhcps(void * arg,cJSON *input_root)
{
	system("/etc/init.d/dnsmasq restart");
	return VT_TRUE;
}

#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
VTBool huayi_reload_ddns(void * arg,cJSON *input_root)
{
	system("huayi_cmd set system ddns_state 0");
	system("huayi_cmd set system ddns_ip 0.0.0.0");
	system("/etc/init.d/yddns restart");
	system("/etc/init.d/orayddns restart");
	system("/etc/init.d/noip2 restart");

	return VT_TRUE;
}
#endif

VTBool huayi_reload_hwnat(void * arg,cJSON *input_root)
{
	system("/etc/init.d/hwnat restart");

	return VT_TRUE;
}

VTBool huayi_reload_static_route(void * arg,cJSON *input_root)
{
	char section[OPTION_STR_LEN] = {0};
	char target[IPV4_STR_LEN] = {0};
	char gateway[IPV4_STR_LEN] = {0};
	char netmask[IPV4_STR_LEN] = {0};
	char inter_face[OPTION_STR_LEN] = {0};
	int metric = 0;
	char cmd[CMD_STR_LEN] = {0};
	int i = 0;
	char wan_device[OPTION_STR_LEN];

	huayi_uci_get_str(PKG_NETWORK,"wan","ifname",wan_device);
	while(1)
	{
		snprintf(section,sizeof(section),"@route[%d]",i);
		if(!huayi_uci_get_str(PKG_NETWORK,section,"target",target))
		{
			break;
		}
		huayi_uci_get_str(PKG_NETWORK,section,"interface",inter_face);

		huayi_uci_get_str(PKG_NETWORK,section,"gateway",gateway);

		huayi_uci_get_str(PKG_NETWORK,section,"netmask",netmask);

		huayi_uci_get_int(PKG_NETWORK,section,"metric",&metric);


		snprintf(cmd,sizeof(cmd),"route del -net %s netmask %s gw %s metric %d",target,netmask,gateway,metric);
		system(cmd);

		if(strcmp(inter_face,"lan")==0)
		{
			snprintf(cmd,sizeof(cmd),"route add -net %s netmask %s gw %s metric %d dev br-lan",target,netmask,gateway,metric);
		}else if(strcmp(inter_face,"wan")==0)
		{
			snprintf(cmd,sizeof(cmd),"route add -net %s netmask %s gw %s metric %d dev %s",target,netmask,gateway,metric,wan_device);
		}else
		{
			i++;
			continue;
		}

		system(cmd);
		i++;
	}

	return VT_TRUE;
}


#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
VTBool huayi_reload_lte4g(void * arg,cJSON *input_root)
{
	char pmodel[RESULT_STR_LEN]={0};
	char cmd[256]={0};
	char modems_status[16] = "idle";
	int  use_apn = 0;
	int para = 0;
	char product_model[OPTION_STR_LEN] = {0};

	huayi_uci_get_int(PKG_MODEM,"status","mode",&use_apn);
	huayi_json_get_int(input_root, "parameter", &para);
	if(1 == use_apn || 1 == para)
	{
		huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
		if(!strcmp(product_model, "4G300-H002-HY03"))
		{
			sleep(20);
			system("/etc/init.d/huayi_dial4gcontrol restart");
		}
		else
		{
			//sleep(3);
			system("killall -9 huayi_atcmd_server");
			system("killall -9 huayi_dial4g");
			system("/etc/init.d/huayi_dial4gcontrol stop");
		}

		system("huayi_cmd set network wan_up 0");
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem signal %d", 0);
		system(cmd);
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem registStatus %s",modems_status);
		system(cmd);
		sleep(1);
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
		if(strncmp("EC200T",pmodel, 6) == 0 || strncmp("EC200A",pmodel, 6) == 0)
		{
			//system("huayi_AT_cli AT+QPOWD=1");
			system("huayi_AT_cli AT+CFUN=1,1");
		}
		else if(strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
		{
			system("huayi_AT_cli AT+QPOWD=1");
		}
		else if(strncmp("N720V5_V1",pmodel, 9) == 0)
		{
			system("huayi_AT_cli AT+CFUN=1,1");
		}
        else if(strncmp("N720V5_V1",pmodel, 9) == 0)
		{
			system("huayi_AT_cli AT+CFUN=1,1");
		}
        else if(strncmp("SLM770U",pmodel, 7) == 0)
		{
			system("huayi_AT_cli AT+CFUN=1,1");
		}
		else
		{
			printf("Unidentified modem module!!!\n");
		}

		sleep(20);
		system("/etc/init.d/huayi_dial4gcontrol restart");
	}
	else
	{
		/*’Î∂‘’‚∏ˆ–Õ∫≈µƒ≤˙∆∑Ãÿ ‚¥¶¿Ì£¨”≤º˛…Ëº∆ƒ£øÈµÁ¬∑ ±£¨∆Ù∂Ø–Ë“™»Ìº˛¡¨Õ®µÁ¬∑*/
		huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
		if(!strcmp(product_model, "4G1200-H002-HY06"))
		{
			system("/etc/init.d/huayi_dial4_init start");
		}
		else
		{
			system("/etc/init.d/huayi_dial4gcontrol restart");
		}
	}

	if(!access("/etc/init.d/hy_cloud_client", 0))
		system("/etc/init.d/hy_cloud_client restart");

	//system("huayi-reset network");

	return VT_TRUE;
}

VTBool huayi_reload_small4g_func(void * arg,cJSON *input_root)
{
	int  use_apn = 0;

	huayi_uci_get_int(PKG_MODEM,"status","mode",&use_apn);
	if(0 == use_apn)
	{
		//system("ifup wan4g");
		system("huayi_AT_cli at+cfun=1,1");
	}
	else
	{
		printf("Unidentified modem module!!!\n");
	}

	if(!access("/etc/init.d/hy_cloud_client", 0))
		system("/etc/init.d/hy_cloud_client restart");

	return VT_TRUE;
}

#endif


#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
VTBool huayi_reload_save_database(void * arg,cJSON *input_root)
{
	system("rm -rf /etc/config/data");

 	system("cp -rf /tmp/data /etc/config/");

 	return VT_TRUE;
}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
VTBool huayi_reload_save_database(void * arg,cJSON *input_root)
{
	system("rm -rf /etc/config/data");

 	system("cp -rf /tmp/data /etc/config/");

 	return VT_TRUE;
}
#endif

VTBool huayi_reload_ftpd(void * arg,cJSON *input_root)
{
	system("/etc/init.d/vsftpd restart");
	return VT_TRUE;
}

VTBool huayi_reload_printer(void * arg,cJSON *input_root)
{
	system("/etc/init.d/p910nd restart");
	return VT_TRUE;
}

VTBool huayi_reload_mount(void * arg,cJSON *input_root)
{
	struct VcoreAgent *Agent =(struct VcoreAgent*)arg;

	huayi_core_delete_first_task_by_id(HUAYI_RELOAD_MOUNT_TASK);
	huayi_core_mount(Agent);
	//huayi_core_mount(Agent);
	return VT_TRUE;
}

VTBool huayi_reload_udhcpd(void * arg,cJSON *input_root)
{
	system("/etc/init.d/huayi_udhcpd restart");

	return VT_TRUE;
}

#if defined(CONFIG_HUAYI_SUPPORT_CONTROL_TELNET_OPTION)
VTBool huayi_reload_telnet(void * arg,cJSON *input_root)
{
	system("/etc/init.d/telnet restart");

	return VT_TRUE;
}

#endif

VTBool huayi_switch_card(void *arg,cJSON *input_root)
{
    char cmd[LONG_BUFF_LEN]={0};
    huayi_uci_get_str(PKG_SYSTEM,"mysocket","send",cmd);
    
    CustomCommunicationClient(cmd,NULL);
    
    huayi_uci_set_str(PKG_SYSTEM,"mysocket","send","");
    huayi_uci_commit(PKG_SYSTEM);
	return VT_TRUE;
}

VTBool huayi_check_internet_state(void *arg,cJSON *input_root){

    CLIENT_SOCKET_LOG("==");

    DetectInternetFaults();
    return VT_TRUE;
}


VTBool huayi_meige_module_decryption(void *arg,cJSON *input_root){

    CLIENT_SOCKET_LOG("==");

    rc4_SLM770U();
    return VT_TRUE;
}

VTBool huayi_meige_set_band(void *arg,cJSON *input_root){

    SWITCH_SIM_LOG("==huayi-reset band==");
	int band = 0,i = 1,j = 0,get_band = 0, flag = 0, at_card = 5,count =0;
	char cmd[TEMP_STR_LEN],result[TEMP_STR_LEN];
	char value[TEMP_STR_LEN] = {0};
agint:
	i = 1;
	for (j = 0; j < 15 && !atcmd_get_imsi(value); j++) {
        sleep(2);
    }

	huayi_uci_get_int(PKG_M_SYSTEM, "system", "band", &band);
	if(band != 0)
		set_operator_band(value);

	huayi_uci_get_int(PKG_M_SYSTEM, "system", "band", &band);
	if (band) {
		SWITCH_SIM_LOG("custom set band %d", band);
		
		if (band >= 1 && band <= 80) {
			while (band > 1 && band != 17 && band != 33 && band != 49 && band != 65) i *= 2, band--;
			snprintf(cmd, TEMP_STR_LEN, "AT+SPLBAND=1,%d,%d,%d,%d,%d", (band == 49 ? i : 0), (band == 33 ? i : 0), (band == 17 ? i : 0), (band == 1 ? i : 0), (band == 65 ? i : 0));
			SWITCH_SIM_LOG("%s", cmd);
			Atcmd(cmd, strlen(cmd), 0);
		} else if (band == 10008) Atcmd("AT+SPLBAND=1,0,0,0,149,0", strlen("AT+SPLBAND=1,0,0,0,149,0"), 0);
		else if (band == 34041) Atcmd("AT+SPLBAND=1,0,482,0,0,0", strlen("AT+SPLBAND=1,0,482,0,0,0"), 0);
		else if (band == 999) Atcmd("AT+SPLBAND=1,0,482,2056,69,0", strlen("AT+SPLBAND=1,0,482,2056,69,0"), 0);//Á¶ÅÁî®5Âíå8
	} else {Atcmd("AT+SPLBAND=1,0,482,2056,213,0", strlen("AT+SPLBAND=1,0,482,2056,213,0"), 0); flag =1;}
	Atcmd("AT+CFUN=0", strlen("AT+CFUN=0"), 0);
	
	Atcmd("AT+CFUN=1", strlen("AT+CFUN=1"), 0);
    /*for (i = 0; i < 15 && !atcmd_get_slm_mobile_type(value); i++) {
        sleep(2);
    }
	if(i == 15){SWITCH_SIM_LOG ("cops:%s error ->> Switch default band",value);goto FALSE;}

	for(i = 0;i<15;i++){
		get_cmd_result("echo $(huayi_AT_cli at+cced=0,1) | awk -F ',' '{print $6}'", result, sizeof(result));
		if(get_band == atoi(result)){
			SWITCH_SIM_LOG ("##set band succeed####config band:%d##AT_CCED ->:%d",get_band,atoi(result));
			goto TRUE;
		}
		sleep (2);
		if(flag && strlen(result)){SWITCH_SIM_LOG ("auto band"); goto TRUE;}
	}
	SWITCH_SIM_LOG ("##error##band:%d===get cced band:%d -->goto agint:%d\n",get_band,atoi(result),count);
	if(count<5){
		goto agint;
		count ++;
	}
	SWITCH_SIM_LOG("error ---> start reboot");
	system("huayi-reset reboot");*/

	
TRUE:
	SWITCH_SIM_LOG ("Switching frequency band successful, enable redialing");
FALSE:
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &at_card);
	sprintf(cmd,"switch_sim %d",at_card);
	system(cmd);
    return VT_TRUE;
}


VTBool huayi_meige_clean_cardinfo(void *arg,cJSON *input_root){
	int i;
	char cmd[8] = {0};
	for (i = 0; i < 4; i++)
	{
		sprintf(cmd, "sim%d", i);
		huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid", "");
		huayi_uci_set_str(PKG_M_SYSTEM, cmd, "operator", 0);
	}
	huayi_uci_set_int(PKG_M_SYSTEM, "system", "all_info", 1);
	huayi_uci_commit(PKG_M_SYSTEM);
	return VT_TRUE;
}
