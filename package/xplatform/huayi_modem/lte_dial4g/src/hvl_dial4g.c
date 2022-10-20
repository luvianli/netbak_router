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

#define cprintf  yprintf
#define CONNECT_TIME_OUT  30    //15 seconds

int lm101a_connect(void)
{
	char version[64] = {0};
	int ret = -1;
	ndis_ipinfo pipinfo;
	int connecting_timeout = CONNECT_TIME_OUT;
	int fd = ndis_open();	
	
	if(fd == -1)
	{
		cprintf("ndis_open failed.\n");
		return -1;
	}
	
	//if get version failed, indicated initliaze error.
	ret = ndis_get_lib_version(fd,version,sizeof(version));
	if(ret!=0||strlen(version)<3)
	{
		cprintf("ndis_get_lib_version failed,ret=%d.\n",ret);	
        ndis_close(fd);
		return -1;
	}else
	{
		cprintf("ndis_get_lib_version success,version=%s.\n",version);		
	}

	cprintf("ndis connect test! \n");
	// connect to internet
	{
		char apn[128] = "UNINET";
		char usrname[128] = "";
		char pwd[128] = "";
		char authtype[10] = "0";
	#if 0 //FIXME: Auto detect according to IMEI
		FILE* m_fd = fopen("/usr/sbin/profile.ini","r");
		if(m_fd)
		{
			fclose(m_fd);
			read_ini_config("/usr/sbin/profile.ini","profile","apn",apn);
			read_ini_config("/usr/sbin/profile.ini","profile","usrname",usrname);
			read_ini_config("/usr/sbin/profile.ini","profile","pwd",pwd);
			read_ini_config("/usr/sbin/profile.ini","profile","auth",authtype);								
		}
		cprintf("connection use apn:%s,usrname:%s,pwd:%s,auth:%s.\n",apn,usrname,pwd,authtype);	
		ret = ndis_connect(fd,apn,usrname,pwd, atoi(authtype));			
		if(ret!=0)
		{
			cprintf("ndis_connect failed,ret=%d.\n",ret);	
			goto EXIT_PROGRAM;
		}else
		{
			cprintf("ndis_connect success.\n");
		}	
	#endif	
	}
	
	while(connecting_timeout--)
	{
		ret = ndis_get_status(fd,&pipinfo);
		if(ret!=0)
		{
			cprintf("ndis_get_status failed,ret=%d.\n",ret);					
		}else
		{
			cprintf("ndis_get_status success.\n");
			if(NDIS_CONNECTED==pipinfo.i32status){
			    cprintf("connected to internet success, .\n");
				break;
			}
		}
		usleep(500000);
	}

	if(NDIS_CONNECTING==pipinfo.i32status)
	{
		cprintf("connecting to internet time out,disconnect from internet.\n");	
		ret = ndis_disconnect(fd);
		if(ret!=0)
		{
			cprintf("ndis_disconnect failed,ret=%d.\n",ret);	
            goto EXIT_PROGRAM;		
		}else
		{
			cprintf("ndis_disconnect success.\n");
		}	
		ret = ndis_get_status(fd,&pipinfo);
	}
EXIT_PROGRAM:	
	ndis_close(fd);
	return pipinfo.i32status;
}

int get_ML7820_bandLte(void)
{
	int i_ret = 0;
	char *ptr = NULL, *pKey = NULL;
	char bandLte[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};

	if ( Atcmd("AT+ZBAND?", strlen("AT+ZBAND?"), 0) == LTE_FALSE )
	{
		i_ret = -1;
		goto get_SLM750_bandLte_err;
	}

	ptr = ATret;
    if ((pKey = strstr(ptr, "+ZBAND")) != NULL )
    {
        at_value_get(bandLte, pKey, sizeof(bandLte), '\n');
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem bandLte %s",bandLte);
		system(cmd);
    }
	
get_SLM750_bandLte_err:	
	return i_ret;
}


int hvl_LM101A_handler(void)
{
	int i_ready, i_dail_check, i_dail, i_link,i_mode;
	int lte_led_status = MODEM_IDLE;
	int retry = 0, signal_count=0;
	int i_signal = 0;
	int i_connType = CT_4G;
	char lte_mode[16]={0};

	yprintf("[Debug]Enter dial4g!!!\n");
	//wait for modem ready
	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}

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
		
		//wan is usb0
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

			i_connType = get_common_4gmode();	
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

		yprintf("[Debug]net_ttyusb_dial_check!!!\n");
		i_dail_check = lm101a_connect();
		if ( NDIS_CONNECTED != i_dail_check )
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


int ECM_connect(void)
{
	int ret = 0;
	char cmd[256] = { 0 };
	char mode[RESULT_STR_LEN]={0},user4g[RESULT_STR_LEN]={0},pass4g[RESULT_STR_LEN]={0},apn[RESULT_STR_LEN]={0},pincode[RESULT_STR_LEN]={0},validation[RESULT_STR_LEN]={0},dialnum[RESULT_STR_LEN]={0};
	
	huayi_uci_get_str(PKG_MODEM, "status", "mode", mode);
	yprintf("[Debug]Enter dial4g!!!\n");
	
dial:
	if(atoi(mode)==0)
	{
		snprintf(cmd,sizeof(cmd),"at+cgdcont=1,\\\"IP\\\"");
		Atcmd(cmd,sizeof(cmd),0);
	}
	else if(atoi(mode)==1)
	{
		huayi_uci_get_str(PKG_MODEM, "status", "user4g", user4g);
		huayi_uci_get_str(PKG_MODEM, "status", "pass4g", pass4g);
		huayi_uci_get_str(PKG_MODEM, "status", "apn", apn);
		huayi_uci_get_str(PKG_MODEM, "status", "dialNum", dialnum);
		huayi_uci_get_str(PKG_MODEM, "status", "pinCode", pincode);
		huayi_uci_get_str(PKG_MODEM, "status", "validation", validation);
		snprintf(cmd,sizeof(cmd),"at+zgpcoauth=1,%s,%s,%s",user4g,pass4g,validation);
		Atcmd(cmd,sizeof(cmd),0);
		memset(cmd,0,sizeof(cmd));
		snprintf(cmd,sizeof(cmd),"at+cgdcont=1,\\\"IP\\\",%s",apn);
		Atcmd(cmd,sizeof(cmd),0);
	}
	while(1)
	{	
		Atcmd("AT+CGACT=1,1",strlen("AT+CGACT=1,1"),0);
		if(strstr(ATret,"OK"))
		{
			printf("[Debug]AT+CGACT=1,1 OK!\n%s\n", ATret);
			break;
		}
	}

	Atcmd("AT+ZGACT=1,1",strlen("AT+ZGACT=1,1"),0);

	if(strstr(ATret,"+ZCONSTAT: 1,1"))
	{
		yprintf("[Debug]AT+ZGACT=1,1 OK!\n%s\n", ATret);
	}
	else
	{
		sleep(2);
		goto dial;
	}

	return 1;
}


int linux_ECM_handler(void)
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
	get_common_imei();
	get_common_imsi();
	atcmd_get_EC20_phoneNumber(lte_value);
	get_ML7820_bandLte();

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
		
		//wan is usb0
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

			i_connType = get_common_4gmode();
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
		
		yprintf("[Debug]net_ttyusb_dial_check!!!\n");
		i_dail_check = ECM_connect();
		if ( 1 == i_dail_check )
			continue;
			
	}

out:	
	printf("[Debug]Exit dial4g!!!\n");
	return 0;
}

