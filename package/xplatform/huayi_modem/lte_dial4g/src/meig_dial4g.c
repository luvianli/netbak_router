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

#include "dial4g.h"

get_meig_status(int *dial_link)
{
	char buf[128] = {0};
	int i_mode = CT_4G;
	char dataCapBuf[32]={0};
	char psBuf[32]={0};
	FILE *fp = NULL;

/*
MCC: 0, MNC: 0, PS: Detached, DataCap: UNKNOW
MCC: 460, MNC: 0, PS: Detached, DataCap: UNKNOW
MCC: 460, MNC: 0, PS: Attached, DataCap: LTE
*/

	get_cmd_result("cat /tmp/lte_status", buf, 128);
	yprintf( "buf[%s].\n\n", buf);
	if ( strlen(buf) > 1 )
	{
		huayi_get_nth_value(2, buf, ',', psBuf, sizeof(psBuf));
		huayi_get_nth_value(3, buf, ',', dataCapBuf, sizeof(dataCapBuf));
		yprintf( "dataCap[%s], ps[%s].\n\n", dataCapBuf, psBuf);
		if ( strstr(psBuf, "Attached")!=NULL )
			*dial_link=1;
		else
			*dial_link=0;

		if ( strstr(dataCapBuf, "LTE")!=NULL )
			i_mode = CT_4G;
		else
			i_mode = CT_23G;
	}
	else
		yprintf( "not yet update the lte status.\n\n");

	return i_mode;
}


int get_SLM750_bandLte(void)
{
	int i_ret = 0;
	char *ptr = NULL, *pKey = NULL;
	char bandLte[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};

	if ( Atcmd("AT+BANDQRY?", strlen("AT+BANDQRY?"), 0) == LTE_FALSE )
	{
		i_ret = -1;
		goto get_SLM750_bandLte_err;
	}

	ptr = ATret;
    if ((pKey = strstr(ptr, "+BANDQRY")) != NULL )
    {
        at_value_get(bandLte, pKey, sizeof(bandLte), '\n');
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem bandLte %s",bandLte);
		system(cmd);
    }
	
get_SLM750_bandLte_err:	
	return i_ret;
}


int meig_model_handler(void)
{
    int i_ready, i_dail_check, i_dail, i_link,i_sysinfo,i_status;
	int lte_led_status = MODEM_IDLE;
	int retry = 0, signal_count=0,cpin_count = 0;
	int MeiG_CM_start = 0;
	int i_signal = 0;
	int i_connType = CT_4G;
	char lte_value[OPTION_STR_LEN] = {0};
	char mode[RESULT_STR_LEN]={0},user4g[RESULT_STR_LEN]={0},pass4g[RESULT_STR_LEN]={0},apn[RESULT_STR_LEN]={0},pincode[RESULT_STR_LEN]={0},validation[RESULT_STR_LEN]={0},dialnum[RESULT_STR_LEN]={0};
	
	//wait for modem ready
	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}

	atcmd_get_EC20_imei(lte_value);
	atcmd_get_imsi(lte_value);
	atcmd_get_EC20_phoneNumber(lte_value);
	get_SLM750_bandLte();

	yprintf("[Debug]Enter dial4g!!!\n");
    while ( 1 )
    {
    	modem_led_control(lte_led_status,i_signal);
        sleep(10);
		if(retry >=3)
		{
			if(LTE_TRUE ==	restart_4gradio())
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
			cpin_count = 0;
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

			i_connType = get_meig_status(&i_dail_check);
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
		else
		{
			cpin_count++;
		}
		
        atcmd_get_signal(&i_signal);
        yprintf("[Debug]atcmd_get_signal=%d\n", i_signal);
        if ( i_signal < 3 ) //0~100
            continue;
		
		if (cpin_count > 3)
		{
			system("huayi-reset network");
			cpin_count = 0;
		}
		
		huayi_uci_get_str(PKG_MODEM, "status", "mode", mode);
		if(atoi(mode)==0 && MeiG_CM_start == 0)
		{
			system("MeiG-CM &");
			MeiG_CM_start = 1;
		}
		else if(atoi(mode)==1 && MeiG_CM_start == 0)	
		{
			char cmd[CMD_STR_LEN]={0};
			huayi_uci_get_str(PKG_MODEM, "status", "user4g", user4g);
			huayi_uci_get_str(PKG_MODEM, "status", "pass4g", pass4g);
			huayi_uci_get_str(PKG_MODEM, "status", "apn", apn);
			huayi_uci_get_str(PKG_MODEM, "status", "dialNum", dialnum);
			huayi_uci_get_str(PKG_MODEM, "status", "pinCode", pincode);
			huayi_uci_get_str(PKG_MODEM, "status", "validation", validation);
			if ( strlen(pincode) > 0 )
				snprintf(cmd,sizeof(cmd),"MeiG-CM -s %s %s %s %s -p %s &",apn,user4g,pass4g,validation,pincode);
			else
				snprintf(cmd,sizeof(cmd),"MeiG-CM -s %s %s %s %s &",apn,user4g,pass4g,validation);
			system(cmd);
			MeiG_CM_start = 1;
		}
		
		sleep(1);
		i_connType = get_meig_status(&i_dail_check);
        yprintf("[Debug]i_dail_check=%d\n", i_dail_check);
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
    }

out:	
    printf("[Debug]Exit dial4g!!!\n");
    return 0;
}
