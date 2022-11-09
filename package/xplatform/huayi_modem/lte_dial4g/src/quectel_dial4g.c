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

int get_quectel_status(char *mobile_type, int *dial_link)
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
	
	if ( strlen(buf) > 1 )
	{
		huayi_get_nth_value(2, buf, ',', psBuf, sizeof(psBuf));
		huayi_get_nth_value(3, buf, ',', dataCapBuf, sizeof(dataCapBuf));
		
		if ( strstr(psBuf, "Attached")!=NULL )
			*dial_link=1;
		else
			*dial_link=0;

		if ( strstr(dataCapBuf, "LTE")!=NULL )
		{
			i_mode = CT_4G;
			strcpy(mobile_type,"4G");
		}
		else
		{
			i_mode = CT_23G;
			strcpy(mobile_type,"3G");
		}
	}

	return i_mode;
}

int init_check_lte_at(void)
{
	int dialing = 0;
	int dialing_count = 0;
	
	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}
	
	while (atcmd_get_usbnet(&dialing) == LTE_FALSE && dialing_count < 10 )
	{
		sleep(1);
		dialing_count++;
	}
	
	if (dialing == ECM)
	{
		/*配置网卡接口类型为RmNet*/
		if ( Atcmd("at+qcfg=\"usbnet\",0", strlen("at+qcfg=\"usbnet\",0"), 0) == LTE_FALSE )
		{
			system("huayi-reset lte4g parameter 1");
			return -1;
		}

		//system("huayi_cmd set modem lteMode 1");
		system("huayi-reset lte4g parameter 1");    
		return -1;
	}
	
	return 0;
}

int ECM_init_check_lte_at(void)
{
	int dialing = 0;
	int dialing_count = 0;
	int qsimdet = 0;
	int qsimstat = 0;
	char cmd[CMD_STR_LEN] = { 0 };
	char simhotplug_en[OPTION_STR_LEN] = {0};
	int ret = 0;
	
	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}
	
	/*if(LTE_FALSE ==atcmd_set_nat())
	{
		system("huayi_AT_cli AT+QCFG='nat',1");
	}
	*/
	/*for 聚合模块*/
	ret = AtcmdSendDelay("AT+QCFG='devmode,0'",strlen("AT+QCFG='devmode,0'"),0);
	if(ret == LTE_FALSE)
	{
		system("huayi_AT_cli AT+QCFG='devmode,0'");
	}
	/*end*/
	while (atcmd_get_usbnet(&dialing) == LTE_FALSE && dialing_count < 10 )
	{
		sleep(1);
		dialing_count++;
	}

	if (dialing != 3)
	{
		strcpy(cmd, "at+qcfg=\"usbnet\",3");   //use rndis
		AtcmdSendDelay(cmd,sizeof(cmd),0);
		sleep(2);
	}
	
	huayi_uci_get_str(PKG_SYSTEM,"mode", "simhotplug", simhotplug_en);
	if (0 == strcmp(simhotplug_en, "1"))
	{
		atcmd_get_qsimdet(&qsimdet);
		if (qsimdet != 1)//支持热插拔
		{
			strcpy(cmd, "at+qsimdet=1,1");
			AtcmdSendDelay(cmd,sizeof(cmd),0);
			sleep(2);
		}
		
		atcmd_get_qsimstat(&qsimstat);
		if (qsimstat != 1)//热插拔状态查询
		{
			strcpy(cmd, "at+qsimstat=1");
			AtcmdSendDelay(cmd,sizeof(cmd),0);
			sleep(2);
		}
		/*1 is ECM; 3 is rndis;*/
		if (dialing != 3 || qsimdet != 1 || qsimstat != 1)//支持热插拔和热插拔状态查询
		{
			system("huayi-reset lte4g parameter 1");
			return -1;
		}
	} 
	else
	{
		/*1 is ECM; 3 is rndis;*/
		if (dialing != 3)
		{
			system("huayi-reset lte4g parameter 1");
			return -1;
		}
	}
	
	return 0;
}


int init_set_modem_status(void)
{
	char lte_value[OPTION_STR_LEN] = {0};
	char mobile_network[OPTION_STR_LEN] = {0};
	char operator_str[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};
	
	memset(lte_value,0,OPTION_STR_LEN);
	atcmd_get_EC20_imei(lte_value);
	if(strlen(lte_value) < 5) 
	{
		strcpy(lte_value, "");
	}
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem imei %s",lte_value);
	system(cmd);

	memset(lte_value,0,OPTION_STR_LEN);
	atcmd_get_imsi(lte_value);
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem imsi %s",lte_value);
	system(cmd);

	if (strncmp(lte_value,"46000",5) == 0 || strncmp(lte_value,"46002",5) == 0 || strncmp(lte_value,"46004",5) == 0 ||
		strncmp(lte_value,"46007",5) == 0 || strncmp(lte_value,"46008",5) == 0 || strncmp(lte_value,"46013",5) == 0)
	{
		strcpy(mobile_network,"CHINA MOBILE CMCC");
	}
	else if (strncmp(lte_value,"46001",5) == 0 || strncmp(lte_value,"46006",5) == 0 || strncmp(lte_value,"46009",5) == 0 ||
		strncmp(lte_value,"46010",5) == 0 )
	{
		strcpy(mobile_network,"CHN-UNICOM");
	}
	else if (strncmp(lte_value,"46003",5) == 0 || strncmp(lte_value,"46005",5) == 0 || strncmp(lte_value,"46011",5) == 0 ||
		strncmp(lte_value,"460012",5) == 0 )
	{
		strcpy(mobile_network,"CHN-CT");
	}
	else
	{
		strcpy(lte_value, "");
		strcpy(mobile_network, "");
	}

	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem network \"%s\"",mobile_network);
	system(cmd);
	
	memcpy(operator_str, lte_value, 5);
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem operator %s", operator_str);
	system(cmd);

	memset(lte_value,0,OPTION_STR_LEN);
	atcmd_get_EC20_phoneNumber(lte_value);
	if(strlen(lte_value) < 5) 
	{
		strcpy(lte_value, "");
	}
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem phoneNumber +%s",lte_value);
	system(cmd);

	memset(lte_value,0,OPTION_STR_LEN);
	atcmd_get_quectel_bandLte(lte_value);
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem bandLte %s",lte_value);
	system(cmd);

	if (strstr(lte_value,"LTE") != NULL)	
	{
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem mobile_type 4G");
	}
	else
	{
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem mobile_type 3G");
	}
	system(cmd);

	
	memset(lte_value,0,OPTION_STR_LEN);
	atcmd_get_center_number(lte_value);
	if(strlen(lte_value) < 5) 
	{
		strcpy(lte_value, "");
	}
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem center_number %s",lte_value);
	system(cmd);

	memset(lte_value, 0, OPTION_STR_LEN);
	atcmd_get_iccid(lte_value);
	if(strlen(lte_value) < 5) 
	{
		strcpy(lte_value, "");
	}
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem iccid %s",lte_value);
	system(cmd);

	memset(lte_value, 0, OPTION_STR_LEN);
	atcmd_get_lac(lte_value);
	if(strlen(lte_value) < 2) 
	{
		strcpy(lte_value, "");
	}
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem lac %s",lte_value);
	system(cmd);

	memset(lte_value, 0, OPTION_STR_LEN);
	atcmd_get_ci(lte_value);
	if(strlen(lte_value) < 2) 
	{
		strcpy(lte_value, "");
	}
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem ci %s",lte_value);
	system(cmd);
	
	return 0;
}

int check_modem_status(int retry)
{
	int i_dail_check = 0;
	int i_sysinfo = 0;
	char lte_value[OPTION_STR_LEN] = {0};

	if(retry >=3)
	{
		if(LTE_TRUE ==	restart_4gradio())
		{
			return 0;
		}else
		{
			return retry;
		}
		
	}
	
	get_quectel_status(lte_value,&i_dail_check);
    if ( 1 == i_dail_check )
    {
        return 0;
    }
	
	i_sysinfo = net_ttyusb_sysinfo();
	if(i_sysinfo==0)
	{
		retry++;
		return retry;
	}

	i_sysinfo = net_ttyusb_cgreg();
	if(i_sysinfo==0)
	{
		retry++;
		return retry;
	}

	return 0;
}

int quectel_gobinet_connect(void)
{
	char cmd[256] = { 0 };
	char mode[RESULT_STR_LEN]={0};
	char user4g[RESULT_STR_LEN]={0},pass4g[RESULT_STR_LEN]={0};
	char apn[RESULT_STR_LEN]={0},validation[RESULT_STR_LEN]={0};
	char dialnum[RESULT_STR_LEN]={0},pincode[RESULT_STR_LEN]={0};
	
	huayi_uci_get_str(PKG_MODEM, "status", "mode", mode);
	if(atoi(mode)==0)
	{
		system("quectel-CM &");
	}
	else if(atoi(mode)==1)	
	{
		huayi_uci_get_str(PKG_MODEM, "status", "user4g", user4g);
		huayi_uci_get_str(PKG_MODEM, "status", "pass4g", pass4g);
		huayi_uci_get_str(PKG_MODEM, "status", "apn", apn);
		huayi_uci_get_str(PKG_MODEM, "status", "dialNum", dialnum);
		huayi_uci_get_str(PKG_MODEM, "status", "pinCode", pincode);
		huayi_uci_get_str(PKG_MODEM, "status", "validation", validation);
		if ( strlen(pincode) > 0 )
			snprintf(cmd,sizeof(cmd),"quectel-CM -s %s %s %s %s -p %s &",apn,user4g,pass4g,validation,pincode);
		else
			snprintf(cmd,sizeof(cmd),"quectel-CM -s %s %s %s %s &",apn,user4g,pass4g,validation);
		system(cmd);
	}
	
	return 1;
}

int quectel_ECxx_model_handler(struct SMSAgent *Agent)
{
    int i_ready = 0, i_dail_check = 0;
	int i_link = 0;
	int quectel_CM_start = 0;
	int retry = 0, signal_count=0,cpin_count = 0;
	int ret = 0;
	char lte_value[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};

	ret = init_check_lte_at();
	while (ret < 0)
	{
		sleep(1);
	}

	init_set_modem_status();
	
    while (VS_TRUE == gRunAgent )
    {
    	modem_led_control(Agent);
        sleep(GET_SMS_INTERVAL);
		
        if ( !huayi_file_exists(DEFAULT_DEV_ATC) )
        {
        	Agent->lte_led_status = MODEM_NO_MODEM;
        	goto out;
        }
		
        atcmd_get_cpin(&i_ready);
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem sim_ready %d",i_ready);
		system(cmd);
		
		if (i_ready != MODEM_SIN_READY)
		{
			if (i_ready == MODEM_SIN_PIN)
				Agent->lte_led_status = MODEM_SIN_LOCK;
			else if (i_ready == MODEM_SIN_PUK)
				Agent->lte_led_status = MODEM_PUK_LOCK;
			else
				Agent->lte_led_status = MODEM_NO_SIM;
			
			continue;
		}
		else
		{
			cpin_count++;
		}
		
		init_set_modem_status();
		
        i_link = net_usb_link(Agent);
		signal_count=0;
        if ( i_link > 0 )
        {
        retry_signal:
			cpin_count = 0;
			atcmd_get_signal(&Agent->i_signal);
			if ( Agent->i_signal > 67 )
				Agent->lte_led_status = MODEM_CONN_SIG_H;
			else  if ( Agent->i_signal > 34 )
				Agent->lte_led_status = MODEM_CONN_SIG_M;
			else  if ( Agent->i_signal > 0 )
				Agent->lte_led_status = MODEM_CONN_SIG_L;
			else
			{
				if ( 0 == signal_count )
				{
					signal_count++;
					sleep(1);
					goto retry_signal;
				}
				
				Agent->lte_led_status = MODEM_NO_CONN;
			}

			huayi_sms_agent_loop(&Agent);

			get_quectel_status(lte_value,&i_dail_check);	
			
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem mobile_type %s",lte_value);
			system(cmd);
			
            continue;
		}
		else
			Agent->lte_led_status = MODEM_NO_CONN;
		
        atcmd_get_signal(&Agent->i_signal);
        if ( Agent->i_signal < 3 ) //0~100
            continue;
		
		/*if (cpin_count > 3)
		{
			system("huayi-reset lte4g");
			cpin_count = 0;
		}*/
		
		if (quectel_CM_start == 0)
		{
			quectel_gobinet_connect();
			if (Agent->init_modem_status == 0)
			{
				init_set_modem_status();
				Agent->init_modem_status = 1;
			}
			quectel_CM_start = 1;
			sleep(10);
			system("ifup wan4g");
		}
		
		sleep(1);
		retry = check_modem_status(retry);		
    }

out:	
    printf("[Debug]Exit dial4g!!!\n");
    return 0;
}

int quectel_ECM_connect(void)
{
	int ret = 0;
	char cmd[CMD_STR_LEN] = { 0 };
	char mode[RESULT_STR_LEN]={0};
	char user4g[RESULT_STR_LEN]={0},pass4g[RESULT_STR_LEN]={0};
	char apn[RESULT_STR_LEN]={0},validation[RESULT_STR_LEN]={0};
	
	huayi_uci_get_str(PKG_MODEM, "status", "mode", mode);
	
	if(atoi(mode)==0)
	{
		//CTNET UNINET CMNET
		//snprintf(cmd,sizeof(cmd),"at+qicsgp=1,1,\"CTNET\"");
		snprintf(cmd,sizeof(cmd),"at+qicsgp=1,1");
		ret = AtcmdSendDelay(cmd,sizeof(cmd),0);
		//if(ret != LTE_TRUE)
		//	return ret;
	}
	else
	{
		huayi_uci_get_str(PKG_MODEM, "status", "user4g", user4g);		
		huayi_uci_get_str(PKG_MODEM, "status", "pass4g", pass4g);		
		huayi_uci_get_str(PKG_MODEM, "status", "apn", apn);	
		huayi_uci_get_str(PKG_MODEM, "status", "validation", validation);
		snprintf(cmd,sizeof(cmd),"at+qicsgp=1,1,\"%s\",\"%s\",\"%s\",%s",apn,user4g,pass4g,validation);
		ret = AtcmdSendDelay(cmd,sizeof(cmd),0);
		if(ret != LTE_TRUE)
			return ret;
	}
	AtcmdSendDelay("at+cgact=1,1",strlen("at+cgact=1,1"),0);
	ret = AtcmdSendDelay("at+qnetdevctl=3,1,1",strlen("at+qnetdevctl=3,1,1"),0);

	return ret;
}

int quectel_ECM_handler(struct SMSAgent *Agent)
{
    int i_ready = 0, i_dail_check = 0;
	int i_link = 0;
	int signal_count=0, ip_count = 0;
	int ret = 0;
	char cmd[CMD_STR_LEN] = {0};
	int conntype = 0, netcardstatus = 0, netstate = 0, actstate = 0;
	char cgpipaddr[IPV4_STR_LEN] = { 0 }, wan_ipaddr[IPV4_STR_LEN] = { 0 };
	char qeng_state[SHORT_STR_LEN] = { 0 };
	int dialresetcount = 0;
	char wired_ipaddr[IPV4_STR_LEN] = {0};
	char wan_type[OPTION_STR_LEN] = {0};


	ret = ECM_init_check_lte_at();
	while (ret < 0)
	{
		sleep(1);
	}
	
	init_set_modem_status();

	while(1) 
	{
		i_dail_check = quectel_ECM_connect();
		if(i_dail_check == LTE_TRUE)
			break;
		else
			sleep(1);
	}
	
	
    while (VS_TRUE == gRunAgent )
    {
    	modem_led_control(Agent);
        sleep(GET_SMS_INTERVAL);
		
        if ( !huayi_file_exists(DEFAULT_DEV_ATC) )
        {
        	Agent->lte_led_status = MODEM_NO_MODEM;
        	goto out;//will auto reload module
        }
		
        atcmd_get_cpin(&i_ready);
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem sim_ready %d",i_ready);
		system(cmd);

		//if(i_ready == MODEM_SIN_READY && Agent->sim_status != MODEM_SIN_READY)
		{
			init_set_modem_status();
		}
		Agent->sim_status = i_ready;
		if (i_ready != MODEM_SIN_READY)
		{
			if (i_ready == MODEM_SIN_PIN)
				Agent->lte_led_status = MODEM_SIN_LOCK;
			else if (i_ready == MODEM_SIN_PUK)
				Agent->lte_led_status = MODEM_PUK_LOCK;
			else
				Agent->lte_led_status = MODEM_NO_SIM;
			continue;
		}
		
        i_link = net_usb_link(Agent);
		signal_count=0;
        if ( i_link > 0 )
        {
retry_signal:
			atcmd_get_signal(&Agent->i_signal);
			if ( Agent->i_signal > 67 )
				Agent->lte_led_status = MODEM_CONN_SIG_H;
			else  if ( Agent->i_signal > 34 )
				Agent->lte_led_status = MODEM_CONN_SIG_M;
			else  if ( Agent->i_signal > 0 )
				Agent->lte_led_status = MODEM_CONN_SIG_L;
			else
			{
				if ( 0 == signal_count )
				{
					signal_count++;
					sleep(1);
					goto retry_signal;
				}
				
				Agent->lte_led_status = MODEM_NO_CONN;
			}

			huayi_sms_agent_loop(&Agent);
            //continue;
		}
		else
		{
			Agent->lte_led_status = MODEM_NO_CONN;	
		}
		atcmd_get_signal(&Agent->i_signal);
		if ( Agent->i_signal <= 10 )
			continue;
		atcmd_get_qeng(qeng_state);
		if(NULL != strstr(qeng_state, "LIMSRV") || NULL != strstr(qeng_state, "SEARCH"))
		{
			huayi_uci_get_int(PKG_STATUS_MODEM,"modem","dialresetcount", &dialresetcount);
			if(Agent->i_signal > 10 && dialresetcount < 2 ) {   // reset 2 count
				snprintf(cmd, CMD_STR_LEN,"huayi_cmd set modem dialresetcount %d",dialresetcount+1);	
				system(cmd);
				system("huayi-reset lte4g parameter 1");
			}
		}
		/*atcmd_get_cgact(&actstate);
		if(0 == actstate)
		{
			AtcmdSendDelay("at+cgact=1,1", strlen("at+cgact=1,1"), 0);
			sleep(1);
			continue;
		}
		atcmd_get_qnetdevctl(&conntype, &netcardstatus, &netstate);
		if(0 == netcardstatus ||0 ==  netstate)
		{
			AtcmdSendDelay("at+qnetdevctl=3,1,1",strlen("at+qnetdevctl=3,1,1"),0);
			sleep(1);
			continue;
		}*/
		atcmd_get_cgpaddr(cgpipaddr);
		if(strlen(cgpipaddr) > 7)
		{
			huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", wan_ipaddr);

#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
			huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);

			if (!strcmp(wan_type,"prior"))
			{
				huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "ipv4", wired_ipaddr);
				snprintf(cmd, CMD_STR_LEN,"echo 'cgpipaddr =  %s   cpe_ipaddr = %s' >> /tmp/test.txt", cgpipaddr, wired_ipaddr);	
				system(cmd);
				if(strlen(wired_ipaddr) > 7 && strstr(cgpipaddr, wired_ipaddr) == NULL)
				{
					if(2 == ip_count)
					{
						system("huayi-reset small4gfunc");
						ip_count = 0;
					}
					ip_count += 1;
				}
			}else
#endif
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
			huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);

			if (!strcmp(wan_type,"rep4g"))
			{
				huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "ipv4", wired_ipaddr);
				snprintf(cmd, CMD_STR_LEN,"echo 'cgpipaddr =  %s   cpe_ipaddr = %s' >> /tmp/test.txt", cgpipaddr, wired_ipaddr);	
				system(cmd);
				if(strlen(wired_ipaddr) > 7 && strstr(cgpipaddr, wired_ipaddr) == NULL)
				{
					if(2 == ip_count)
					{
						system("huayi-reset small4gfunc");
						ip_count = 0;
					}
					ip_count += 1;
				}
			}else
#endif

			{	
				if(strlen(wan_ipaddr) > 7 && NULL == strstr(cgpipaddr, wan_ipaddr))
					system("huayi-reset small4gfunc");
			}
		}
    }

out:	
    printf("[Debug]Exit dial4g!!!\n");
    return 0;
}

