/*******************************************************************************************
 * Project:  huayi_ext_sim                                                        *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20220215 add by rex  Version 1.0 
 * hardware use 2 sim, but soft only use ext simcard
 ******************************************************************************************/
#include "hy_ext_sim.h"
#include "huayi_atcmd.h"
#include <signal.h>
#include <pthread.h>
#include <wait.h>


void huayi_sim_change_exit(int sig)
{
	(void) signal(SIGTERM, SIG_IGN);

	return;
}

void do_gpio_to_extern(void)
{
	char cmd[CMD_STR_LEN] = {0};

	snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", 2);

	system(cmd);
	SIMCARD_CHANGE_LOG("%s, %d cmd = %s \n", __FUNCTION__, __LINE__, cmd);
	return;
}

void huayi_dev_to_ext_simcard()
{
	//gpio operator
	SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);
	do_gpio_to_extern();
	system("huayi-reset lte4g parameter 1");
	//huayi_uci_set_int(PKG_SYSTEM, "simcard","dev_in_to_ext_flag", 1);
	//huayi_uci_commit(PKG_SYSTEM);
	return;
}
void default_to_ext_simcard()
{
	do_gpio_to_extern();
	system("huayi-reset lte4g parameter 1");
	return;
}

void* sim_change_handle()
{
	char registStatus[LONG_BUFF_LEN]={0};
	static int timewait = 0, checkflag = 0, count = 0;
	int cloud_cmd_flag = 0, switchSim = 0;
	int dev_intoext_flag = 0;
	
	while(1) 
	{
		if( LTE_TRUE != atcmd_check()) 
		{
			goto goon;
		}
		SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);
		/*start# run only once, the device is powered on*/
		if( LTE_TRUE != atcmd_sim_is_exist() && checkflag != 2) 
		{
			SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);
			if(checkflag == 0)
			{
				default_to_ext_simcard();
				checkflag = 1;
			}
			if(15 > count)    /*in_simcard, prevent infinite loop*/
			{   /*wait 1 mins*/
				count += 1;
				goto goon;
			} 
		} 
		checkflag = 2;
		count = 0;
		/*end# run only once*/
		huayi_uci_get_int(PKG_SYSTEM, "simcard","switchSim", &switchSim);
		SIMCARD_CHANGE_LOG("%s, %d cloud_cmd_flag = %d\n", __FUNCTION__, __LINE__, cloud_cmd_flag);
		{   /* The cloud server does not issue the switchSim command; device is in factory mode;*/
			huayi_uci_get_int(PKG_SYSTEM, "simcard","dev_in_to_ext_flag", &dev_intoext_flag);
			SIMCARD_CHANGE_LOG("%s, %d, dev_intoext_flag = %d \n", __FUNCTION__, __LINE__,dev_intoext_flag );
			if(0 == dev_intoext_flag) 
			{
				SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);
				//todo sim card detect
				huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "registStatus", registStatus);
				if(strcmp(registStatus ,"connected") != 0) 
				{
					SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);
					if((10 == timewait) && (strcmp(registStatus ,"noSim") == 0)) 
					{   /*wait 40 sec*/
						timewait = 0;
						SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);
						/*sim card is not ready, to do sim change, to extern sim card*/
						huayi_dev_to_ext_simcard();
					}
					if(45 > timewait) 
					{   /*wait 3 mins*/
						timewait += 1;
						goto goon;
					}
					else 					
					{						
						timewait = 0;						
						SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);						
						/*sim card is not ready, to do sim change, to extern sim card*/			
						huayi_dev_to_ext_simcard();					
					}
				} 
				else    /* internal sim card is exist, sim card is ready; */
				{   
					timewait = 0;
					SIMCARD_CHANGE_LOG("%s, %d \n", __FUNCTION__, __LINE__);
					    /*check sim card connect internet*/
					if(simcard_internet_check(INTERNET_ABROAD_DOMAIN, NULL) != 1) 
					{
						/*sim card is not connect internet, to do sim change, to extern sim card*/
						huayi_dev_to_ext_simcard();
					}
				}
			} 
		}
goon:	
		CHECK_SIMCARD_CHANGE_LOG();
		sleep(4);
	}
	
	return (void*)0;
}


int main(int argc, char *argv[])
{
	pthread_t scTid;
	int err;
	void *tret;
	char 	wan_type[OPTION_STR_LEN] = {0};
	char simchange_en[OPTION_STR_LEN] = {0};
	int dev_intoext_flag = 0;
	
	(void) signal(SIGTERM, huayi_sim_change_exit);

	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);

	if (0 != strcmp(wan_type,WAN_TYPE_4G))
	{
		return 0;
	}
	
	/*first, use ext simcard*/
	huayi_uci_get_int(PKG_SYSTEM, "simcard","dev_in_to_ext_flag", &dev_intoext_flag);
	if(dev_intoext_flag == 0 )
	{
		SIMCARD_CHANGE_LOG("%s, %d , first use in simcard\n", __FUNCTION__, __LINE__);
		default_to_ext_simcard();
		sleep(100);
	}
	/************/
	pthread_create(&scTid, NULL, sim_change_handle, NULL);
	
	err = pthread_join(scTid,&tret);
    if(err!=0)  
    {   
        printf("can not join with sim card change:%s\n",strerror(err));
        exit(-1);
    }       
    printf("sim card change exit code %d\n",(int)tret);

	return 0;
}


