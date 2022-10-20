#include     <stdio.h>      
#include     <stdlib.h>     
#include     <unistd.h>     
#include     <sys/types.h>  
#include     <sys/stat.h>   
#include     <fcntl.h>  
#include     <sys/ioctl.h>
#include     <net/if.h>
#include     <sys/socket.h>
#include     <string.h>
#include	 <arpa/inet.h>

#include     "dial4g.h"

int yuga_get_qnwinfo(void)
{
	char *ptr = NULL, *pKey = NULL;
	char sysinfo[32] = {0};
	int i_mode = CT_4G;
	char netmode[8]={0};
	
	if ( Atcmd("AT+QNWINFO", strlen("AT+QNWINFO"), 0) == LTE_FALSE )
	{
		goto yuga_get_qnwinfo_err;
	}

	ptr = ATret;
	//+QNWINFO: WCDMA,X,X,X
	/* EC25 & EC21 Support SRLTE
	*  AT+QNWINFO
	   +QNWINFO: "CDMA1X","46003","CDMA BC0",283
	   +QNWINFO: "FDD LTE","46011","LTE BAND 1",75
	   OK
	*/

	if ((pKey = strstr(ptr, "+QNWINFO:")) != NULL )
	{
		yprintf( "pKey:[%s].\n\n",pKey) ;
	#if 0
        at_value_get(sysinfo, pKey, 32, ',');
		if ( strstr(sysinfo, "LTE") )
			strcpy(netmode, "4G");
		else if ( strncasecmp(sysinfo, "HDR", 3) == 0 
			|| strstr(sysinfo, "WCDMA") 
			|| strstr(sysinfo, "HS") )
			strcpy(netmode, "3G");
		else if ( strstr(sysinfo, "CDMA")
			|| strncasecmp(sysinfo, "GSM", 3) == 0
			|| strncasecmp(sysinfo, "GPRS", 4) == 0 
			|| strncasecmp(sysinfo, "EDGE", 4) == 0 )
			strcpy(netmode, "2G");
		else
			strcpy(netmode, "ERROR");
	#else
		if ( strstr(pKey, "LTE") )
		{
			strcpy(netmode, "4G");
			i_mode=CT_4G;
		}
		else if ( strstr(pKey, "HDR")
			|| strstr(pKey, "WCDMA") 
			|| strstr(pKey, "HS") )
		{
			strcpy(netmode, "3G");
			i_mode=CT_23G;
		}
		else if ( strstr(pKey, "CDMA")
			|| strstr(pKey, "GSM")
			|| strstr(pKey, "GPRS")
			|| strstr(pKey, "EDGE") )
		{
			strcpy(netmode, "2G");
			i_mode=CT_23G;
		}
		else
			strcpy(netmode, "ERROR");
	#endif
	}
	yprintf( "netmode Information:[%s].\n\n",netmode) ;
	
yuga_get_qnwinfo_err:	
	return i_mode;
}


int yuga_dial_check(void)
{
    int i_ret = 0;
    char *ptr = NULL;
    char dialFlag[2] = {0}, ipType[4] = {0};

     char *pKey = NULL;

    if ( Atcmd("AT$QCRMCALL?", strlen("AT$QCRMCALL?"), 0) == LTE_FALSE )
    {
        goto dial_check_err;
    }
    ptr = ATret;

    if ((pKey = strstr(ptr, "$QCRMCALL:")) != NULL )
    {
       at_value_get(dialFlag, pKey, sizeof(dialFlag), ',');
       huayi_get_nth_value(1, pKey, ',', ipType, sizeof(ipType));
       yprintf("dialFlag=%s, ipType=%s.\n", dialFlag, ipType);
    }

    //ipType=' V4'
    if ( !strcmp(dialFlag, "1") && !strncmp(ipType, " V4", 3) )
        i_ret = 1;

dial_check_err:
    return i_ret;
}

int yuga_dial(void)
{	
	int i_ret = 0;
	char *ptr = NULL;
    char dialFlag[2] = {0}, ipType[4] = {0};
	char cmd[256] = { 0 };

	char *pKey = NULL;
	char mode[RESULT_STR_LEN]={0},apn[RESULT_STR_LEN]={0};
	 
	huayi_uci_get_str(PKG_MODEM, "status", "mode", mode);
	yprintf("[Debug]Enter dial4g!!!\n");
	 
	if(atoi(mode)==1)
	{	
		huayi_uci_get_str(PKG_MODEM, "status", "apn", apn);
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd,sizeof(cmd),"AT+CGDCONT=1,\\\"IP\\\",\\\"%s \\\"",apn);
		Atcmd(cmd,sizeof(cmd),0);
	}

    if ( Atcmd("AT$QCRMCALL=1,1,1,2,1", strlen("AT$QCRMCALL=1,1,1,2,1"), 0) == LTE_FALSE )
    {
        goto dial_err;
    }
    ptr = ATret;

    if ((pKey = strstr(ptr, "$QCRMCALL:")) != NULL )
    {
       at_value_get(dialFlag, pKey, sizeof(dialFlag), ',');
       huayi_get_nth_value(1, pKey, ',', ipType, sizeof(ipType));
       yprintf("dialFlag=%s, ipType=%s.\n", dialFlag, ipType);
    }

    if ( !strcmp(dialFlag, "1") && !strncmp(ipType, " V4", 3) )
        i_ret = 1;

dial_err:
    return i_ret;
}


int yuga_model_handler(void)
{
	
	int i_ready, i_dail_check, i_dail, i_link,i_sysinfo,i_status;
	int lte_led_status = MODEM_IDLE;
	int retry = 0, signal_count=0;
	int i_signal = 0;
	int i_connType = CT_4G;
	char lte_value[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};

	yprintf("[Debug]Enter dial4g!!!\n");

	//wait for modem ready
	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}
	get_CLM920_imei();
	atcmd_get_imsi(lte_value);
	atcmd_get_EC20_phoneNumber(lte_value);

	while ( 1 )
	{
		modem_led_control(lte_led_status,i_signal);
		sleep(10);
		if(retry >=3)
		{
			if(LTE_TRUE == restart_4gradio())
			{
				retry =0;
			}else
			{
				yprintf("[Debug]yuga_model_handler restart fail!!!\n");
				continue;
			}
			
		}
		
	   if ( !huayi_file_exists(DEFAULT_DEV_ATC) )
		{
			lte_led_status = MODEM_NO_MODEM;
		   goto out;//will auto reload module by cs_init
		}

		i_link = net_usb_link();
		yprintf("[Debug]net_usb_link[%d]!!!\n", i_link);
		signal_count=0;
		if ( i_link > 0 )
		{
		retry_signal:
			atcmd_get_signal(&i_signal);
			if ( i_signal > 67 )
				lte_led_status = MODEM_CONN_SIG_H;
			else  if ( i_signal > 34 )
				lte_led_status = MODEM_CONN_SIG_M;
			else  if ( i_signal > 0 )
				lte_led_status = MODEM_CONN_SIG_L;
			else
			{
				if ( 0 == signal_count )
				{
					signal_count++;
					sleep(1);
					goto retry_signal;
				}
				
				lte_led_status = MODEM_NO_CONN;
			}

			i_connType = yuga_get_qnwinfo();
			yprintf("connType=%s, signal=%d !!!\n", 
					CT_4G == i_connType?"4G":"2/3G", i_signal); 

			continue;
		}
		else
			lte_led_status = MODEM_NO_CONN;
		
		yprintf("[Debug]atcmd_get_cpin!!!\n");
		atcmd_get_cpin(&i_ready);
		if (i_ready != MODEM_SIN_READY)
		{
			if (i_ready == MODEM_SIN_PIN)
				lte_led_status = MODEM_SIN_LOCK;
			else if (i_ready == MODEM_SIN_PUK)
				lte_led_status = MODEM_PUK_LOCK;
			else
				lte_led_status = MODEM_NO_SIM;
			
			continue;
		}
		
		yprintf("[Debug]atcmd_get_signal!!!\n");
		atcmd_get_signal(&i_signal);
		if ( i_signal < 3 ) //0~100
			continue;
		
		sleep(1);
        yprintf("[Debug]net_ttyusb_dial_check!!!\n");
        i_dail_check = yuga_dial_check();
        if ( 1 == i_dail_check )
            continue;

		
		i_sysinfo = net_ttyusb_sysinfo();
		if(i_sysinfo==0)
		{
			retry++;
			continue;
		}

		i_sysinfo = net_ttyusb_cgreg();
		if(i_sysinfo==0)
		{
			retry++;
			continue;
		}
		retry=0;		
		
        yprintf("[Debug]net_ttyusb_dial!!!\n");
        i_dail = yuga_dial();
        if ( 1 == i_dail )
            continue;		
	}

out:	
	printf("[Debug]Exit dial4g!!!\n");
	return 0;
}

