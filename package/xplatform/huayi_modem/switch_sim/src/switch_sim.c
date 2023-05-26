#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>
#include "huayi_uci_fun.h"
int main(int argc, char* argv[])
{ 
	int iRet;
	if(argv[1])
	{
		int iSel = atoi(argv[1]);
		char cmd[CMD_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_SWITCH_SIM_SUPPORT)
		switch(iSel)
		{ 
			case 1 :
	   	    	 	snprintf(cmd, CMD_STR_LEN,"gpio l %d 0 4000 0 0 4000",CONFIG_HUAYI_SIM_GPIO1_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO2_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO3_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 0 4000 0 0 4000",CONFIG_HUAYI_SIM_GPIO4_NUM);
					system(cmd);
					break;
			case 2 :
	   	    	 	snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO1_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 0 4000 0 0 4000",CONFIG_HUAYI_SIM_GPIO2_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO3_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 0 4000 0 0 4000",CONFIG_HUAYI_SIM_GPIO4_NUM);
					system(cmd);
				    break;
		   case 3 :
	   	    	 	snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO1_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO2_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 0 4000 0 0 4000",CONFIG_HUAYI_SIM_GPIO3_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 0 4000 0 0 4000",CONFIG_HUAYI_SIM_GPIO4_NUM);
					system(cmd);
			 	    break;
		   case 4 :
	   	    	 	snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO1_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO2_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO3_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO4_NUM);
					system(cmd);
			 	    break;			
			default :
	   	    	 	snprintf(cmd, CMD_STR_LEN,"gpio l %d 0 4000 0 0 4000",CONFIG_HUAYI_SIM_GPIO1_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO2_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 4000 0 0 0 4000",CONFIG_HUAYI_SIM_GPIO3_NUM);
					system(cmd);
					snprintf(cmd, CMD_STR_LEN,"gpio l %d 0 4000 0 0 4000",CONFIG_HUAYI_SIM_GPIO4_NUM);
					system(cmd);
					break;

		}


		system("huayi_AT_cli AT+CFUN=0");
		sleep(5);
		system("huayi_AT_cli AT+CFUN=1");
		
#endif
	}
	else
	{
		printf("========Select a sim card==========");
	}
 	
    return  0;
}




