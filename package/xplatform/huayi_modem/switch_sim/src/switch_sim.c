#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "huayi_uci_fun.h"

int sim = 0;

void save_siminfo()
{
    char lte_value[OPTION_STR_LEN] = {0},iccid[OPTION_STR_LEN] = {0};
	int	 ret = 0,count = 0;
	int  netoperator = 0;

	ret = atcmd_get_iccid(lte_value);
	huayi_uci_get_str(PKG_M_SYSTEM, "sim2", "iccid", iccid);
	if (ret == 1 && strcmp(lte_value,iccid))
	{	
		SWITCH_SIM_LOG ("sim2 change");
		huayi_uci_set_str(PKG_M_SYSTEM, "sim2", "iccid", lte_value);
retry_getoperator:
	    memset(lte_value, 0, OPTION_STR_LEN);
	    atcmd_get_imsi(lte_value);
	    if (strncmp(lte_value, "46000", 5) == 0 || strncmp(lte_value, "46002", 5) == 0 || strncmp(lte_value, "46004", 5) == 0 ||
	            strncmp(lte_value, "46007", 5) == 0 || strncmp(lte_value, "46008", 5) == 0 || strncmp(lte_value, "46013", 5) == 0)
	    {
	        netoperator = 3;
	    }
	    else if (strncmp(lte_value, "46001", 5) == 0 || strncmp(lte_value, "46006", 5) == 0 ||
	             strncmp(lte_value, "46009", 5) == 0 ||
	             strncmp(lte_value, "46010", 5) == 0)
	    {
	        netoperator = 2;
	    }
	    else if (strncmp(lte_value, "46003", 5) == 0 || strncmp(lte_value, "46005", 5) == 0 ||
	             strncmp(lte_value, "46011", 5) == 0 ||
	             strncmp(lte_value, "460012", 5) == 0)
	    {
	        netoperator = 1;
	    }
	    else
	    {
	        netoperator = 0;
	        count += 1;
	        if (count < 3)
	            goto retry_getoperator;
	    }
	    huayi_uci_set_int(PKG_M_SYSTEM, "sim2", "operator", netoperator);	
		huayi_uci_commit(PKG_M_SYSTEM);
	}

    return;

}

void set_wanIp(){
	char cmd[TEMP_STR_LEN] = {0};
	char cmd_result[CMD_STR_LEN];
	int is_ipv4 = 0,flag = 0;
	int i = 0;
	
	for(i;i<2;i++)
	{	
		memset(cmd_result, 0, CMD_STR_LEN);
		flag = atcmd_get_cgpaddr(cmd_result);
		if(flag){
				SWITCH_SIM_LOG("wan ip :%s", cmd_result);
				system("ifup wan4g");
				memset(cmd,0,TEMP_STR_LEN);
				sprintf(cmd,"huayi_cmd set network wan_ipv4 %s",cmd_result);
				system(cmd);
				return ;
		}else SWITCH_SIM_LOG ("get wanip error %dtime,try again", i+1);
		sleep(6);
	}SWITCH_SIM_LOG ("get wanip error");

}

int SlmDial()
{
    int count = 0, registered_count = 0, signal = 0,mode = 0,allinfo = 0;
    char resultbuf[OPTION_STR_LEN] = {0};
    char cmd[TEMP_STR_LEN] = {0};
    char print_to[TEMP_STR_LEN] = {0};
	char value[TEMP_STR_LEN] = {0};
	int j,simnum,band;

    while (count < 60)
    {
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "working_mode", &mode);
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "all_info", &allinfo);
		huayi_uci_set_int(PKG_M_SYSTEM, "system", "at_card", sim);

        if (mode == 1 && allinfo == 0)
        {				
            while (registered_count < 60 && atcmd_get_slm_mobile_type(value) == HUAYI_FALSE)
            {
            	registered_count += 2;
				SWITCH_SIM_LOG("##waitting##card value :%s .waiting for %d (up to 60) seconds",value,(registered_count));
				sleep(2);
            }

            if (registered_count < 70)
            {
                SWITCH_SIM_LOG("=== SWITCH SIM %d (%s) Dialing successful===\n\n", sim, print_to);
				set_wanIp();
                return 1;
            }
            else
            {
                SWITCH_SIM_LOG("Dialing failed, waiting for timeout to exit card switching\n");
                return 0;
            }
    	}
        else
        {
            SWITCH_SIM_LOG("Production testing mode skip dialing");
            return 1;
        }
    	count += 3;
        sleep(3);
	}

    return 0;
}

int switchSim() {
    switch (sim) {

		case 0:
			SWITCH_SIM_LOG("Switching CHN-CT\n");
			system("gpio l 0 0 4000 0 0 4000");
			system("gpio l 36 0 4000 0 0 4000");
		break;
		
		case 1:
			SWITCH_SIM_LOG("Switching CHINA MOBILE CMCC\n");
			system("gpio l 0 0 4000 0 0 4000");
			system("gpio l 36 4000 0 0 0 4000");
		break;
		
		case 2:
			SWITCH_SIM_LOG("Switching waicha\n");
			system("gpio l 0 4000 0 0 0 4000");
			system("gpio l 36 4000 0 0 0 4000");
		break;

        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Missing parameters. Please check the command\n\n");
        return -1;
    }

    sim = atoi(argv[1]);

    if (sim >= 0 && sim < 3) {
        //system("devmem 0x10000060 32 0x4110441");
		SWITCH_SIM_LOG("switching sim%d", sim);	
        switchSim();
		sleep(2);
		system("huayi_AT_cli AT+CFUN=0");
        sleep(5);
		system("huayi_AT_cli AT+CFUN=1");
		sleep(2);
		SlmDial();
    } else 
		printf("\nSwitch error %d\n Usage: %s [0-3]\n", sim,argv[0]);

    return 0;
}






