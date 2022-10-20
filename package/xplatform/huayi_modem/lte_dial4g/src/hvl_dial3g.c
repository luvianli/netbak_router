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
#include     "hvl_ndis/ndis_qmi_service.h"





int linux_MF210_handler(void)
{
	int i_ready, i_dail_check, i_dail, i_link,i_mode;
	int lte_led_status = MODEM_IDLE;
	int retry = 0, signal_count=0;
	int i_signal = 0;
	int i_connType = CT_4G;
	char lte_value[OPTION_STR_LEN] = {0};
	char lte_mode[OPTION_STR_LEN]={0};
	char cmd[CMD_STR_LEN]={0};
	yprintf("[Debug]Enter dial4g!!!\n");

	//wait for modem ready
	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}
	
	atcmd_get_EC20_imei(lte_value);
	atcmd_get_imsi(lte_value);
	atcmd_get_EC20_phoneNumber(lte_value);

	while ( 1 )
	{
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem signal %d",i_signal);
		system(cmd);
		modem_led_control(lte_led_status,i_signal);
		
		sleep(10);
		
	    if ( !huayi_file_exists(DEFAULT_MF210_DEV_ATC) )
		{
			lte_led_status = MODEM_NO_MODEM;
		   goto out;//will auto reload module by cs_init
		}
		
		//wan is usb0
		i_link = net_3g_link();
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

			i_connType = CT_23G;
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
		{
			retry++;
			continue;
		}

		//system("ifup wan4g");
	}

out:	
	printf("[Debug]Exit dial4g!!!\n");
	return 0;
}


int hvl_EM770W_handler(void)
{
	int i_ready, i_dail_check, i_dail, i_link,i_mode;
	int lte_led_status = MODEM_IDLE;
	int retry = 0, signal_count=0;
	int i_signal = 0;
	int i_connType = CT_4G;
	char lte_value[OPTION_STR_LEN] = {0};
	char lte_mode[OPTION_STR_LEN]={0};
	char cmd[CMD_STR_LEN]={0};
	yprintf("[Debug]Enter dial4g!!!\n");

	//wait for modem ready
	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}
	
	atcmd_get_EC20_imei(lte_value);
	atcmd_get_imsi(lte_value);
	atcmd_get_EC20_phoneNumber(lte_value);

	while ( 1 )
	{
		modem_led_control(lte_led_status,i_signal);
		
		sleep(10);
		
	    if ( !huayi_file_exists(DEFAULT_MF210_DEV_ATC) )
		{
			lte_led_status = MODEM_NO_MODEM;
		   goto out;//will auto reload module by cs_init
		}
		
		//wan is usb0
		i_link = net_3g_link();
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

			i_connType = CT_23G;
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
		{
			retry++;
			continue;
		}

	}

out:	
	printf("[Debug]Exit dial4g!!!\n");
	return 0;
}


