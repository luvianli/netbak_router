#include <stdio.h>
#include "dial4g.h"



int debug = 0;

int net_usb_link(struct SMSAgent *Agent)
{
    int link_up = 0;
	int wan_link = 0; 
	char wan_type[OPTION_STR_LEN] = {0};
	
	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&link_up);
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&wan_link);
	
	if (strcmp(wan_type,"wired") == 0)
	{
		if (link_up == 0 && wan_link == 1)
		{
			Agent->is_wired_mode = 1;
		}
		else
		{
			Agent->is_wired_mode = 0;
		}
		link_up = 1;
	}
	else
	{
		Agent->is_wired_mode = 0;
	}
	
    return link_up;
}

int net_3g_link(void)
{
    int link_up = 0; 
	
    return link_up;
}


int get_common_4gmode(void)
{
	int i_ret = 0;
	char *ptr = NULL, *pKey = NULL;
	char sysinfo[16] = {0}, sys_mode[2] = {0};
	int i_mode = 0;

	if ( Atcmd("AT^SYSINFO", strlen("AT^SYSINFO"), 0) == LTE_FALSE )
	{
		i_ret = -1;
		goto atcmd_get_sysmode_err;
	}

	ptr = ATret;
	//^SYSINFO: 2,3,0,X,1
    if ((pKey = strstr(ptr, "^SYSINFO:")) != NULL )
    {
        at_value_get(sysinfo, pKey, sizeof(sysinfo), '\n');
		if((huayi_get_nth_value(3, sysinfo, ',', sys_mode, sizeof(sys_mode)) == -1)){
			goto atcmd_get_sysmode_err;
		}
		
		i_mode = atoi(sys_mode);
		if ( 9 == i_mode || 17 == i_mode )
			i_mode = CT_4G;
		else if ( 4 == i_mode || 5 == i_mode || 8 == i_mode || 11 == i_mode || 15 == i_mode )
			i_mode = CT_23G;
		else if ( 2 == i_mode || 3 == i_mode )
			i_mode = CT_23G;
		else if ( 0 == i_mode )
			i_mode = CT_NO_SERVICE;
		else
			i_mode = CT_NO_SERVICE;
    }
	
atcmd_get_sysmode_err:	
	return i_mode;
}

int get_common_imei(void)
{
	int i_ret = 0;
	char *ptr = NULL, *pKey = NULL;
	char sysinfo[16] = {0};
	char cmd[CMD_STR_LEN] = {0};

	if ( Atcmd("AT+CGSN", strlen("AT+CGSN"), 0) == LTE_FALSE )
	{
		i_ret = -1;
		goto get_common_imei_err;
	}
	ptr = ATret;

	if ( strstr(ptr, "OK") == NULL )
	{
		i_ret = -1;
		goto get_common_imei_err;
	}
	
    if ((pKey = strstr(ptr, "+CGSN")) != NULL )
    {
        at_value_get(sysinfo, pKey, sizeof(sysinfo), '\n');
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem imei %s",sysinfo);
		system(cmd);
    }
	
get_common_imei_err:	
	return i_ret;
}

int get_common_imsi(void)
{
	int i_ret = 0;
	char *ptr = NULL, *pKey = NULL;
	char sysinfo[16] = {0};
	char cmd[CMD_STR_LEN] = {0};

	if ( Atcmd("AT+CIMI", strlen("AT+CIMI"), 0) == LTE_FALSE )
	{
		i_ret = -1;
		goto get_common_imsi_err;
	}
	ptr = ATret;

	if ( strstr(ptr, "OK") == NULL )
	{
		i_ret = -1;
		goto get_common_imsi_err;
	}

	//^SYSINFO: 2,3,0,X,1
    if ((pKey = strstr(ptr, "+CIMI")) != NULL )
    {
        at_value_get(sysinfo, pKey, sizeof(sysinfo), '\n');
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set status modem %s",sysinfo);
		system(cmd);
    }
	
get_common_imsi_err:	
	return i_ret;
}

int get_CLM920_imei(void)
{
	int i_ret = 0;
	char *ptr = NULL, *pKey = NULL;
	char imei[16] = {0};
	char cmd[CMD_STR_LEN] = {0};

	if ( Atcmd("ATI", strlen("ATI"), 0) == LTE_FALSE )
	{
		i_ret = -1;
		goto get_common_imei_err;
	}
	ptr = ATret;

	if ( strstr(ptr, "OK") == NULL )
	{
		i_ret = -1;
		goto get_common_imei_err;
	}
	
    if ((pKey = strstr(ptr, "IMEI")) != NULL )
    {
        at_value_get(imei, pKey, sizeof(imei), '\n');
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem imei %s",imei);
		system(cmd);
    }
	
get_common_imei_err:	
	return i_ret;
}


int net_ttyusb_sysinfo(void)
{
    int i_ret = 0;
    char *ptr = NULL;
    char dialFlag[2] = {0}, temp_s[2] = {0};

     char *pKey = NULL;

    if ( Atcmd("AT^SYSINFO", strlen("AT^SYSINFO"), 0) == LTE_FALSE )
    {
        goto yuga_get_sysinfo_err;
    }
    ptr = ATret;

    if ((pKey = strstr(ptr, "^SYSINFO:")) != NULL )
    {
       at_value_get(dialFlag, pKey, sizeof(dialFlag), ',');
       huayi_get_nth_value(1, pKey, ',', temp_s, sizeof(temp_s));
       yprintf("dialFlag=%s, temp_s=%s.\n", dialFlag, temp_s);
    }

    if ( !strcmp(dialFlag, "2") && (!strcmp(temp_s, "2") || !strcmp(temp_s, "3")))
        i_ret = 1;

yuga_get_sysinfo_err:
    return i_ret;
}

int net_ttyusb_cgreg(void)
{
    int i_ret = 0;
    char *ptr = NULL;
    char dialFlag[2] = {0}, temp_s[2] = {0};

     char *pKey = NULL;

    if ( Atcmd("AT+CGREG?", strlen("AT+CGREG?"), 0) == LTE_FALSE )
    {
        goto yuga_get_cgreg_err;
    }
    ptr = ATret;

    if ((pKey = strstr(ptr, "+CGREG:")) != NULL )
    {
       at_value_get(dialFlag, pKey, sizeof(dialFlag), ',');
       huayi_get_nth_value(1, pKey, ',', temp_s, sizeof(temp_s));
       yprintf("dialFlag=%s, temp_s=%s.\n", dialFlag, temp_s);
    }

    if ( !strcmp(dialFlag, "0") && (!strcmp(temp_s, "1") || !strcmp(temp_s, "5")))
        i_ret = 1;

yuga_get_cgreg_err:
    return i_ret;
}


int hw_gpio_init(int hw_type)
{
	/*
	I2S - GPIO# 0~3 - 7:6 - 0x1
	I2C - GPIO# 4~5 - 21:20 - 0x1
	UART - GPIO# 45~46 - 25:24 - -0x1
	0x55144450 - 1010101000101000100010001010000
	*/
	//system("reg s 0");
	//system("reg w 60 54144450");

	//mtk_set_gpio_pin(IP04383_SMS_LED, GPIO_OFF);
	//mtk_set_gpio_pin(IP04383_23G_LED, GPIO_OFF);
	//mtk_set_gpio_pin(IP04383_4G_LED, GPIO_OFF);
	//mtk_set_gpio_pin(IP04383_SIGNAL1_LED, GPIO_OFF);
	//mtk_set_gpio_pin(IP04383_SIGNAL2_LED, GPIO_OFF);
	//mtk_set_gpio_pin(IP04383_SIGNAL3_LED, GPIO_OFF);
	//mtk_set_gpio_pin(IP04383_SIGNAL4_LED, GPIO_OFF);

	return 0;
}

char modem_msg[MODEM_STATUS_MAX][16] = {"Idle", "No Modem", "No Connected", "No SIM", "PIN Lock", "PUK Lock", "Signal LOW", "Signal Middle", "Signal High"};

int lte_led_status_old = MODEM_NO_MODEM;
int modem_led_control(struct SMSAgent *Agent)
{
	char cmd[256]={0};
	char modems_status[16] = "idle";
	yprintf("led status_old=%s, status=%s.\n", modem_msg[lte_led_status_old], modem_msg[Agent->lte_led_status]);
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem signal %d",Agent->i_signal);
	system(cmd);
	
	if ( lte_led_status_old == Agent->lte_led_status )
		return -1;

	switch(Agent->lte_led_status)
	{
		case MODEM_IDLE:
			break;
		case MODEM_NO_SIM: 
			strcpy(modems_status, "noSim");
			break;
		case MODEM_SIN_LOCK:
			strcpy(modems_status, "simPinLock");
			break;
		case MODEM_PUK_LOCK:
			strcpy(modems_status, "simPukLock");
			break;
		case MODEM_CONN_SIG_L:
		case MODEM_CONN_SIG_M:
		case MODEM_CONN_SIG_H:
			strcpy(modems_status, "connected");
			break;	
		case MODEM_NO_CONN:
			strcpy(modems_status, "searching");
			break;
		case MODEM_NO_MODEM: 
		default:
			strcpy(modems_status, "noModem");
			break;
	}

	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem registStatus %s",modems_status);
	system(cmd);

	if (Agent->is_wired_mode == 1)
		system("ifup wan");
	
	switch(Agent->lte_led_status)
	{
		case MODEM_CONN_SIG_L:
		case MODEM_CONN_SIG_M:
		case MODEM_CONN_SIG_H:
			break;	
		case MODEM_NO_CONN:
			break;
		case MODEM_NO_MODEM: 
		case MODEM_NO_SIM:
		case MODEM_SIN_LOCK:
		case MODEM_PUK_LOCK:
		default:
			yprintf("MODEM_NO_CONN : LED off !!!\n");	
			break;
	}
	
	lte_led_status_old = Agent->lte_led_status;
	return 0;
}

int main(int argc, char *argv[])
{
	char pmodel[16]={0};
	char hardModel[16]={0};
	struct SMSAgent Agent;
	
	if (argc > 1 && strcmp(argv[1], "-d") == 0)
		debug = 1;

	huayi_sms_init(&Agent);

	if(!huayi_sms_sqlite3_init())
	{
		printf("huayi_mysql_init mysql failed,service stop!!!\n");
		return -1;
	}

	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);	
	printf("[%s:%d] detect model [%s]\n", __FUNCTION__, __LINE__, pmodel);
		
	if(strncmp("EC200T",pmodel, 6) == 0 || strncmp("EC200A",pmodel, 6) == 0)
	{
		quectel_ECM_handler(&Agent);
	}
	else if(strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
	{
		quectel_ECxx_model_handler(&Agent);
	}
	else if(strncmp("N720V5_V1",pmodel, 9) == 0)
	{
		neoway_RNDIS_handler(&Agent);
	}
    else if(strncmp("SLM770U",pmodel, strlen("SLM770U")) == 0||strncmp("ML307",pmodel, strlen("ML307")) == 0||strncmp("U9300C_DGN",pmodel, 10) == 0)
    {
        slm_ECM_handler(&Agent);
    }
	else
	{
		printf("Unidentified modem module!!!\n");
    	return 0;
	}

	huayi_sms_exit(&Agent);
}


