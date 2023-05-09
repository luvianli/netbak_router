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

	(void) signal(SIGTERM, huayi_4g_init_exit);

	while(1) 
	{
		ret = get_cmd_result("lsusb",resultbuf,sizeof(resultbuf));
		if(ret == HUAYI_TRUE)
		{
			if((strstr(resultbuf,"2c7c:") != NULL) ||
				(strstr(resultbuf,"05c6:") != NULL) ||
				(strstr(resultbuf,"2949:") != NULL))
			{
				if(count == 3) {
					system("/etc/init.d/huayi_dial4gcontrol restart");
					break;
				} else {
					sleep(1);
				}
				count += 1;
			}
			else
			{
				sleep(1);
			}

		}
		else
		{
			sleep(1);
		}
	}

	return 0;
}


