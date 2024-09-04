#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "huayi_uci_fun.h"
// Define enum for better readability
#ifdef CONFIG_HUAYI_SUPPORT_CARD_SUPPORT

enum SimSelection {

    SIM0_HIGH_LOW,
    SIM1_HIGH_HIGH,
    SIM2_LOW_HIGH,
	EXTERNAL_CARD_LOW_LOW,
    INVALID_SELECTION
};
#endif


int sim = 0;
void set_wanIp(){
	char cmd[TEMP_STR_LEN] = {0};
	char cmd_result[CMD_STR_LEN];
	int is_ipv4 = 0,flag = 0;
	int i = 0;
	memset(cmd_result, 0, CMD_STR_LEN);
	get_cmd_result("echo $(huayi_AT_cli AT+CGDCONT?) | grep -oE 'IPV4:[0-9.]*' | awk -F: '{print $2}'", cmd_result, CMD_STR_LEN);

	if (strchr(cmd_result, '.') != NULL)flag =1;
	sleep(2);
	if(flag){
			CON_LOG("wan ip :%s", cmd_result);
			memset(cmd,0,TEMP_STR_LEN);
			sprintf(cmd,"huayi_cmd set network wan_ipv4 %s",cmd_result);
			system(cmd);
		}else CON_LOG ("error##wan ip :%s", cmd_result);


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

        if (mode == 1)
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
				//set_wanIp();
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
			printf("Switching 0; 0 Setting LOW=0V & 2 Setting High=3.3V\n");
			system("gpio l 11 4000 0 0 0 4000");
            system("gpio l 5 0 4000 0 0 4000");
            break;
        case 1:
            printf("Switching 1; 0 Setting High=3.3V & 2 Setting LOW=0V\n");
            system("gpio l 11 0 4000 0 0 4000");
            system("gpio l 5 4000 0 0 0 4000");
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
	int simnum = 0;
    huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);

    if (sim >= 0 && sim < simnum) {
        system("devmem 0x10000060 32 0x4110441");	
        switchSim();
		sleep(2);
		system("gpio l 2 0 4000 0 0 4000");
        sleep(5);
		system("gpio l 2 4000 0 0 0 4000");
		sleep(2);
		system("/etc/init.d/huayi_dial4g restart");
        // Assuming SlmDial function exists
        SlmDial();
    } else 
		printf("\nSwitch error %d\n Usage: %s [0-%d]\n", sim,argv[0],simnum);

    return 0;
}






