/*******************************************************************************************
 * Project:  huayi_4g_init                                                        *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20220215 add by rex  Version 1.0 
 * init lte 4g and 5g
 ******************************************************************************************/
#include "huayi_atcmd.h"
#include "huayi_uci_fun.h"
#include <signal.h>
#include <pthread.h>
#include <wait.h>

#define WAN_TYPE_4G		"mobile"

void huayi_4g_init_exit(int sig)
{
	(void) signal(SIGTERM, SIG_IGN);

	return;
}
void up_lte_modem_and_use_extsim(void)
{
	char cmd[CMD_STR_LEN] = {0};
	char resultbuf[OPTION_STR_LEN] = {0};
	int ret = 0, count = 0;
	int i_ready = 0;

	system("devmem 0x10000064 32 0x000555");
	
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", 41);
	system(cmd);
	while(1) {
		ret = get_cmd_result("lsusb",resultbuf,sizeof(resultbuf));
		if((ret == HUAYI_TRUE) && (strstr(resultbuf,"2c7c:") != NULL))
		{
			snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", 40);
			system(cmd);
			usleep(500);
			//system("huayi-reset lte4g parameter 1");
			if(count == 3) {
				system("huayi_AT_cli AT+CFUN=0");
				sleep(5);
				system("huayi_AT_cli AT+CFUN=1");
				sleep(8);
				system("kill -9 huayi_dial4gcontrol");
				system("/etc/init.d/huayi_dial4gcontrol restart");
				break;
			} else {
				sleep(1);
			}
			count += 1;

		}
		else
		{
			system("devmem 0x10000064 32 0x000555");
			snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", 41);
			system(cmd);
			sleep(1);
		}
	}
	while(1) {
		sleep(8);
		atcmd_get_cpin(&i_ready);
		if (i_ready != MODEM_SIN_READY)
		{
			snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", 40);
			system(cmd);
			usleep(500);
			system("huayi_AT_cli AT+CFUN=0");
			sleep(5);
			system("huayi_AT_cli AT+CFUN=1");
			sleep(8);
		}
		else
		{
			sleep(5);
			//break;
		}
	}
	return;
}

void huayi_to_ext_simcard()
{
	//gpio operator
	SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);
	char cmd[CMD_STR_LEN] = {0};
	int ret = 0, count = 0;
	char resultbuf[OPTION_STR_LEN] = {0};
	int i_ready = 0;
		
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", 2);
	system(cmd);
	sleep(1);
	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", 2);
	system(cmd);
	sleep(1);
	system("huayi-reset lte4g parameter 1");
	
	return;
}

void meige_module_init()
{
	char resultbuf[OPTION_STR_LEN] = {0};
	int ret = 0, count = 0;

    while (isProcessRunning("ated")) {
        printf("Process ated is running. Waiting for 3 seconds...\n");
        CON_LOG("Process ated is running. Waiting for 3 seconds...\n");
        sleep(3);
    }
    
#if defined(CONFIG_HUAYI_MODULE_SUPPORT)
		system("gpio l 1 4000 0 0 0 4000 && sleep 1 && gpio l 1 0 4000 0 0 4000");
        CON_LOG("gpio start");
      
#endif

}

int main(int argc, char *argv[])
{
	int err;
	void *tret;
	char 	wan_type[OPTION_STR_LEN] = {0};
	char simchange_en[OPTION_STR_LEN] = {0};
	int dev_intoext_flag = 0;
	char product_model[OPTION_STR_LEN] = {0};
	char resultbuf[OPTION_STR_LEN] = {0};
	int ret = 0, count = 0;
    int i = 0;
	(void) signal(SIGTERM, huayi_4g_init_exit);

	/*�������ͺŵĲ�Ʒ���⴦����Ӳ�����ģ���·ʱ��������Ҫ������ͨ��·*/
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",product_model);
	if(!strcmp(product_model, "4G1200-H002-HY06"))
	{
		up_lte_modem_and_use_extsim();
		return 0;
	}
	if(!strcmp(product_model, "4G300-H002-HY03"))
	{
		huayi_to_ext_simcard();
		return 0;
	}
     if(!strcmp(product_model, "WYX3C-1"))
    {
        meige_module_init();
    }


	while(1) 
	{
		ret = get_cmd_result("lsusb",resultbuf,sizeof(resultbuf));
		if(ret == HUAYI_TRUE)
		{
			if((strstr(resultbuf,"2c7c:") != NULL) ||
				(strstr(resultbuf,"05c6:") != NULL) ||
				(strstr(resultbuf,"2949:") != NULL) ||
				(strstr(resultbuf,"2dee:") !=NULL))
			{
				if(count == 3) {
                    CON_LOG("/etc/init.d/huayi_dial4gcontrol restart");
					system("/etc/init.d/huayi_dial4gcontrol restart");
                    if(!strcmp(product_model, "WYX3C-1W"))
					{
						system("switch_sim 0 &");
					} 
					break;
				} else {
					sleep(1);
				}
				count += 1;
			}
			else
			{
                
                CON_LOG("%d",i);
                if(i == 15){
					i = 0;
                    CON_LOG("LSUSB not recognized, power on the module again");
					system("gpio l 1 4000 0 0 0 4000 && sleep 1 && gpio l 1 0 4000 0 0 4000");
                }
                i++;
                
				sleep(2);
			}

		}
		else
		{
			sleep(1);
		}
	}

	return 0;
}


