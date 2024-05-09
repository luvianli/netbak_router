/*******************************************************************************************
 * Project:  check_sim                                                        *
 *                                                                                         *
 * Author :  rex									                                       *
 *  																					   *
 * Changelist: 																		       *
 * 20220215 add by rex  Version 1.0
 * check sim card
 ******************************************************************************************/
#include "huayi_atcmd.h"
#include "huayi_uci_fun.h"
#include <signal.h>
#include <pthread.h>
#include <wait.h>
void switch_sim(int i, int gpio);

int isProductionMode();
void save_siminfo(int i, int gpio);


void save_siminfo(int i, int gpio)
{
    char cmd[OPTION_STR_LEN] = {0};
    int  netoperator = 0;
    char tmp[OPTION_STR_LEN] = {0};
    char checksignalip[OPTION_STR_LEN] = {0}, maxdelay[OPTION_STR_LEN] = {0};
    int ret = 0, count = 0;
    int i_ready = 0, i_signal = 0;
    char lte_value[OPTION_STR_LEN] = {0};

    sprintf(cmd, "sim%d", i);
    atcmd_get_cpin(&i_ready);
    while (i_ready != MODEM_SIN_READY)
    {
        sleep(1);
        atcmd_get_cpin(&i_ready);
        if (count == 3)
        {
            break;
        }
        count += 1;
    }
    count = 0;
retry_getccid:    //¾¡×î´ó¿ÉÄÜÈ¡µ½ICCID
    memset(lte_value, 0, OPTION_STR_LEN);
    ret = atcmd_get_iccid(lte_value);
    if (strlen(lte_value) < 5 || (strstr(lte_value, "ERR") != NULL) || (0 == ret) || isAlphanumeric(lte_value) == 0)
    {

        count += 1;
        if (40 == count) //3.2ms
        {
            system("huayi_AT_cli AT+CFUN=0 ");
            sleep(5);
            system("huayi_AT_cli AT+CFUN=1 ");
        }
        strcpy(lte_value, "");
        usleep(800);
        if (count < 55)
            goto retry_getccid;
    }
    count = 0;
    huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid", lte_value);
    sprintf(tmp, "%d_%s", gpio, lte_value);
    huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid_gpio", tmp);
    huayi_uci_set_int(PKG_M_SYSTEM, cmd, "card_slot", i);
    if (strlen(lte_value)  < 10)
    {
        return;     //no iccid, that mean no simcard
    }
retry_getlac:	  //¾¡×î´ó¿ÉÄÜÈ¡µ½LAC
    memset(lte_value, 0, OPTION_STR_LEN);
    atcmd_get_lac(lte_value);
    //printf("rex ====   %s, %d  ---lac = %s-\n", __FUNCTION__, __LINE__, lte_value);
    if (strlen(lte_value) < 2)
    {
        strcpy(lte_value, "");
        count += 1;
        if (count < 10)
            goto retry_getlac;
    }
    huayi_uci_set_str(PKG_M_SYSTEM, cmd, "lac", lte_value);
    count = 0;

retry_getci:	  //¾¡×î´ó¿ÉÄÜÈ¡µ½ci
    memset(lte_value, 0, OPTION_STR_LEN);
    atcmd_get_ci(lte_value);
    //printf("rex ====   %s, %d  --ci =%s--\n", __FUNCTION__, __LINE__, lte_value);
    if (strlen(lte_value) < 2)
    {
        strcpy(lte_value, "");
        count += 1;
        if (count < 10)
            goto retry_getci;
    }
    huayi_uci_set_str(PKG_M_SYSTEM, cmd, "ci", lte_value);
    count = 0;

retry_getoperator:	  //¾¡×î´ó¿ÉÄÜÈ¡µ½operator info
    memset(lte_value, 0, OPTION_STR_LEN);
    atcmd_get_imsi(lte_value);
    //printf("rex ====   %s, %d  ---operator = %s-\n", __FUNCTION__, __LINE__, lte_value);
    if (strncmp(lte_value, "46000", 5) == 0 || strncmp(lte_value, "46002", 5) == 0 || strncmp(lte_value, "46004", 5) == 0 ||
            strncmp(lte_value, "46007", 5) == 0 || strncmp(lte_value, "46008", 5) == 0 || strncmp(lte_value, "46013", 5) == 0)
    {
        netoperator = 3;
    }
    else if (strncmp(lte_value, "46001", 5) == 0 || strncmp(lte_value, "46006", 5) == 0 ||
             strncmp(lte_value, "46009", 5) == 0 ||
             strncmp(lte_value, "46010", 5) == 0)
    {
        netoperator = 2;
    }
    else if (strncmp(lte_value, "46003", 5) == 0 || strncmp(lte_value, "46005", 5) == 0 ||
             strncmp(lte_value, "46011", 5) == 0 ||
             strncmp(lte_value, "460012", 5) == 0)
    {
        netoperator = 1;
    }
    else
    {
        netoperator = 0;
        count += 1;
        if (count < 10)
            goto retry_getoperator;
    }
    huayi_uci_set_int(PKG_M_SYSTEM, cmd, "operator", netoperator);
    count = 0;

    memset(lte_value, 0, OPTION_STR_LEN);
    memset(tmp, 0, OPTION_STR_LEN);
    atcmd_get_quectel_bandLte(lte_value);
    if (strstr(lte_value, "LTE") != NULL)
    {
        strcpy(tmp, "4G");
    }
    else if (strstr(lte_value, "WCDMA") != NULL)
    {
        strcpy(tmp, "3G");
    }
    else
    {
        strcpy(tmp, "2G");
    }
    huayi_uci_set_str(PKG_M_SYSTEM, cmd, "networktype", tmp);

    atcmd_get_signal_for_cloud(&i_signal);
    huayi_uci_set_int(PKG_M_SYSTEM, cmd, "signal", i_signal);

    huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "checksignalips", checksignalip);
    huayi_uci_set_str(PKG_M_SYSTEM, cmd, "checksignalips", checksignalip);

    huayi_uci_commit(PKG_M_SYSTEM);

    return;
}


void switch_sim(int i, int gpio)
{
    char tmp[OPTION_STR_LEN];

    isProductionMode();

    snprintf(tmp, OPTION_STR_LEN, "switch_sim %d", i);
    system(tmp);

    save_siminfo(i, gpio);
}

int isIccidDuplicate(const char iccidArray[][OPTION_STR_LEN], int currentIndex) {
	int i;
	for (i = 0; i < currentIndex; i++) {
        if (strcmp(iccidArray[i], iccidArray[currentIndex]) == 0 && strcmp(iccidArray[i],"")) {
			SWITCH_SIM_LOG ("iccid%d:%s == iccid%d:%s ,reset sim info",i ,iccidArray[i],currentIndex, iccidArray[currentIndex]);
            return 1;
        }
    }
    return 0;
}

void clearAllIccids(int simnum) {
    char cmd[OPTION_STR_LEN];
	int i;
    for (i = 0; i < simnum; i++) {
        sprintf(cmd, "sim%d", i);
        huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid", "");
    }
    huayi_uci_commit(PKG_M_SYSTEM);
}

void set_siminfo()
{
    int i, simnum, usegpio, gpio, def_gpio, card_slot;
    char cmd[OPTION_STR_LEN] = {0};
    char lte_value[OPTION_STR_LEN] = {0};

    // Wait until atcmd is true
    while (LTE_TRUE != atcmd_check())
    {
        sleep(1);
    }

    huayi_uci_get_int(PKG_M_SYSTEM, "usesiminfo", "usegpio", &usegpio);
    atcmd_get_EC20_imei(lte_value);

    if (strlen(lte_value) < 5)
    {
        strcpy(lte_value, "");
    }

    huayi_uci_set_str(PKG_M_SYSTEM, "system", "imei", lte_value);
    huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);

	
	char iccid[simnum][OPTION_STR_LEN];
    huayi_uci_get_int(PKG_M_SYSTEM, "usesiminfo", "card_slot", &card_slot);

againt:

    for (i = 0; i < simnum -1; i++)
    {
        sprintf(cmd, "sim%d", i);
        huayi_uci_get_str(PKG_M_SYSTEM, cmd, "iccid", iccid[i]);	
        huayi_uci_get_int(PKG_M_SYSTEM, cmd, "gpio", &gpio);


        if (isIccidDuplicate(iccid, i)) {
            clearAllIccids(simnum);
			goto againt;
        }


        if (strlen(iccid[i]) < 5 && card_slot != i)
        {
            SWITCH_SIM_LOG("\n\nsim%d start switch card", i);
            switch_sim(i, gpio);
        }
        else if (strlen(iccid[i]) > 5)
        {
            SWITCH_SIM_LOG("Sim%d information already exists");
        }
        else if(card_slot == i)
        {
            def_gpio = gpio;
        }
    }

    switch_sim(card_slot, def_gpio);
}


void check_network()
{
    int totalsimnum = 0, simn = 0, usegpio = 0;
    char cmd[OPTION_STR_LEN] = {0};
    int  netoperator = 0;
    char tmp[OPTION_STR_LEN] = {0};
    char check_ip[OPTION_STR_LEN] = {0};
    int ret = 0, count = 0, i = 0;
    int i_ready = 0, i_signal = 0;
    char lte_value[OPTION_STR_LEN] = {0};
    char iccid[OPTION_STR_LEN] = { 0 };
    int hbq = 0, this_bool = 0;
    int enable = 0;



    isProductionMode();

    //External card

#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_AFTER_SALES
    int simnum;
    huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);
	simn = simnum - 1;
	SWITCH_SIM_LOG ("CONFIG_HUAYI_SUPPORT_MEIGE_AFTER_SALES START");

#else
	huayi_uci_get_int(PKG_M_SYSTEM, "usesiminfo", "card_slot", &simn);
	SWITCH_SIM_LOG ("HUAYI_SUPPORT_MEIGE_TRIPLE_CARD START");


#endif
    sprintf(cmd, "switch_sim %d", simn);
    system(cmd);
    sleep(25);

#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_AFTER_SALES
	ret = atcmd_get_iccid(iccid);
	if(ret == 1 && strlen(iccid)>5)
	{
		save_useing_siminfo(simn);
		return ;
	}else goto check_net;
#endif

    //huayi_uci_get_int(PKG_M_SYSTEM, "usesiminfo", "card_slot", &simn);
cretry1:
    memset(check_ip, 0, OPTION_STR_LEN);
    huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "checksignalips", check_ip);
    if (networkCheck(check_ip))
    {
        save_useing_siminfo(simn);
        CON_LOG("sim %d network ok \n", simn);
        return;
    }
    else
    {
        count += 1;
        sleep(6);
        if (count <= 10)
            goto cretry1;
    }
    count = 0;

    CON_LOG("sim%d ping bu tong %s", simn, check_ip);
check_net:
    huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &totalsimnum);
    for (i = 0; i < totalsimnum; i++)
    {
        if (i == simn)
            continue;
        //å¤–ç½®å¡
#ifndef CONFIG_HUAYI_SUPPORT_MEIGE_AFTER_SALES
        if (i == totalsimnum - 1)
        {
            huayi_uci_get_int(PKG_M_SYSTEM, "system", "external_card_enable", &enable);
            if (!enable)
            {
                CON_LOG("external_card_enable     0");
                //system("reboot");
                break;
            }
        }
#endif
        CON_LOG("network error ,switch sim%d", i);
        isProductionMode();
        memset(tmp, 0, OPTION_STR_LEN);
        sprintf(tmp, "switch_sim %d", i);
        system(tmp);
        sleep(30);
cretry2:
        memset(tmp, 0, OPTION_STR_LEN);
        huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "default_checkip", tmp);
        if (networkCheck(tmp))
        {
            sleep(3);
            save_useing_siminfo(i);
            CON_LOG("sim %d network ok \n", i);
            this_bool = 1;
            return;
        }
        else
        {
            count += 1;
            sleep(6);
            if (count <= 10)
                goto cretry2;

        }
        count = 0;
        sleep(1);
    }
	SWITCH_SIM_LOG("All cards network error -> exe reboot");
    sleep(15);
    system("reboot");

    return;
}


void init_cloud_client()
{
    CON_LOG("Starting communication with cloud platform\n\n");
    system("hy_cloud_client &");
    return;
}

void at_init()
{
    int ret = 0;

    ret = AtcmdSendDelay("AT+QCFG=\"devmode,0\"", strlen("AT+QCFG=\"devmode,0\""), 0);
    //usleep(500);
    //system("huayi_AT_cli AT+CFUN=0 ");
    //sleep(5);
    //system("huayi_AT_cli AT+CFUN=1 ");

    return;
}

void gpio_init()
{
    system("switch_sim 100");
    sleep(1);

    return;
}


void TcpServerStart()
{


#if defined(CONFIG_HUAYI_TCP_SOCKET)
    system("huayi_multi_tcp_server &");
    CON_LOG("start huayi_multi_tcp_server");
#endif
}



void production_test()
{

    char iccid[OPTION_STR_LEN] = { 0 };
    memset(iccid, 0, OPTION_STR_LEN);
    huayi_uci_get_str(PKG_M_SYSTEM, "sim1", "iccid", iccid);
    if (strlen(iccid) > 10)
    {
        system("huayi_production_tcp_server &");
        CON_LOG("start huayi_production_tcp_server");
    }
}






void SLM_init()
{
    char lte_value[OPTION_STR_LEN] = {0}, lod_sn[OPTION_STR_LEN] = {0}, iccid_value[OPTION_STR_LEN] = {0};
    char def_ssid[OPTION_STR_LEN] = {0};
    char imei_6[RESULT_STR_LEN] = {0};
    char cmd[OPTION_STR_LEN] = {0};
    int totalsimnum = 0, simn = 0, usegpio = 0, gpio = 0;

    while (LTE_TRUE != atcmd_check())
    {
        sleep(1);
    }
    memset(lte_value, 0, OPTION_STR_LEN);
    atcmd_get_EC20_imei(lte_value);
    huayi_uci_get_str(PKG_M_SYSTEM, "system", "imei", lod_sn);
    huayi_uci_set_str(PKG_M_SYSTEM, "system", "imei", lte_value);
    huayi_uci_commit(PKG_M_SYSTEM);

    //rc4_SLM770U();

    if (!huayi_uci_get_str(PKG_WIRELESS, "radio0_0", "def_ssid", def_ssid) || (strlen(lod_sn) > 8 &&
            strncmp(lod_sn, lte_value, strlen(lod_sn))))
    {
        memset(cmd, 0, OPTION_STR_LEN);
        strcpy(cmd, "uci -q -c /etc/huayi/brand_info get capability.cap.def_ssid_prefix");
        get_cmd_result(cmd, iccid_value, sizeof(iccid_value));
        strcpy(imei_6, lte_value + strlen(lte_value) - 6);
        strncat(iccid_value, imei_6, sizeof(iccid_value) - strlen(iccid_value) - 1);

        huayi_uci_set_str(PKG_WIRELESS, W24G_IFACE, "ssid", iccid_value);
        huayi_uci_set_str(PKG_WIRELESS, W24G_IFACE, "def_ssid", iccid_value);
        sprintf(cmd, "echo %s >> /tmp/dat2uci.log", imei_6);
        system(cmd);
        huayi_uci_commit(PKG_WIRELESS);
        system("huayi-reset wireless");
        sleep(12);
        CON_LOG("------> %s", iccid_value);
    }
}

int isProductionMode()
{
    int mode = 0 ;

    huayi_uci_get_int(PKG_M_SYSTEM, "system", "working_mode", &mode);
    if (mode == 0)
    {
        while (isProcessRunning("ated"))
        {
            CON_LOG("Process ated is running. Waiting for 3 seconds...\n");
            sleep(3);
        }
    }
    CON_LOG("ated not start");
    return 1;

}



void Production_testing_mode()
{
    int mode = 0;
    huayi_uci_get_int(PKG_M_SYSTEM, "system", "working_mode", &mode);
    if (mode == 0)
    {
        CON_LOG("Start production testing");
        set_siminfo();
        CON_LOG("Information collection completed, waiting for production testing");
        do
        {
            huayi_uci_get_int(PKG_M_SYSTEM, "system", "working_mode", &mode);
            sleep(2);
        }
        while (mode == 0);
        CON_LOG("End production testing");
    }
    else
    {
        CON_LOG("Start working mode");
    }

}

void get_siminfo()
{
#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_SINGLE_CARD
    return ;
#else
    int mode = 0,simnum,gpio;
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);

	char iccid[simnum][OPTION_STR_LEN];
    huayi_uci_get_int(PKG_M_SYSTEM, "system", "working_mode", &mode);
	huayi_uci_get_str(PKG_M_SYSTEM, "sim0", "iccid", iccid[0]);
	huayi_uci_get_str(PKG_M_SYSTEM, "sim1", "iccid", iccid[1]);

    while(mode == 1&& (strlen(iccid[0])< 5||strlen(iccid[1])< 5))
    {	    
    	if(strlen(iccid[0])< 5)
    	{	
    		huayi_uci_get_int(PKG_M_SYSTEM, "sim0", "gpio", &gpio);
			switch_sim(0, gpio);
			huayi_uci_get_str(PKG_M_SYSTEM, "sim0", "iccid", iccid[0]);
		}
    	if(strlen(iccid[1])< 5)
    	{	
    		huayi_uci_get_int(PKG_M_SYSTEM, "sim1", "gpio", &gpio);
			switch_sim(1, gpio);
			huayi_uci_get_str(PKG_M_SYSTEM, "sim1", "iccid", iccid[1]);
		}		
		sleep(2);
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "working_mode", &mode);
    }
	sleep(2);
#endif
	return ;
}

int main(int argc, char *argv[])
{

    SLM_init();

    Production_testing_mode();

    check_network();
    init_cloud_client();

    return 0;
}


