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

int slm_init_set_modem_status(void)
{
	char lte_value[OPTION_STR_LEN] = {0};
	char mobile_network[OPTION_STR_LEN] = {0};
	char operator_str[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};
	int signal = 0;

	memset(lte_value,0,OPTION_STR_LEN);
	atcmd_get_L509_imei(lte_value);
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
	lynq_get_4gmode(lte_value);
	if (strstr(lte_value, "LTE")!=NULL)
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
	lynq_get_iccid(lte_value);
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

	atcmd_get_signal(&signal);
	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set modem signal %d",signal);
	system(cmd);
	return 0;
}

int lynq_model_handler(struct SMSAgent *Agent)
{
    int i_ready = 0, i_dail_check = 0;
	int i_link = 0;
	int signal_count=0;
	char cmd[CMD_STR_LEN] = {0};

	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}

	while ( 1 )
    {
		slm_init_set_modem_status();
		modem_led_control(Agent);
        sleep(10);
		
       if ( !huayi_file_exists(DEFAULT_DEV_ATC) )
        {
        	Agent->lte_led_status = MODEM_NO_MODEM;
        	goto out;//will auto reload module by cs_init
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
			neoway_sms_agent_loop(&Agent);
            continue;
		}
		else
			Agent->lte_led_status = MODEM_NO_CONN;
	
    }
out:	
	printf("[Debug]Exit dial4g!!!\n");
	return 0;
}

