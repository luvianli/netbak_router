#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "huayi_uci_fun.h"
// Define enum for better readability
#ifdef CONFIG_HUAYI_SUPPORT_CARD_SUPPORT

enum SimSelection {

    SIM0_HIGH_LOW,
#ifndef CONFIG_HUAYI_SUPPORT_MEIGE_SINGLE_CARD
    SIM1_HIGH_HIGH,
#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_TRIPLE_CARD
    SIM2_LOW_HIGH,
#endif
#endif
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
    int count = 0, registered_count = 0, signal = 0,mode = 0;
    char resultbuf[OPTION_STR_LEN] = {0};
    char cmd[TEMP_STR_LEN] = {0};
    char print_to[TEMP_STR_LEN] = {0};
	char value[TEMP_STR_LEN] = {0};
	int j,simnum;

    strcpy(print_to, (sim != EXTERNAL_CARD_LOW_LOW) ? "built-in" : "External");
    SWITCH_SIM_LOG("\n\ngpio sim%d",sim);
	huayi_uci_set_int(PKG_M_SYSTEM, "system", "at_card", sim);
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);
    while (count < 60)
    {
        get_cmd_result("lsusb", resultbuf, sizeof(resultbuf));

        if (strstr(resultbuf, "2dee:") != NULL && rc4_SLM770U())
        {
            SWITCH_SIM_LOG("=== SWITCH SIM %d (%s) OK ===", sim, print_to);

            Atcmd("AT+SFUN=4", strlen("AT+SFUN=4"), 0);

            snprintf(cmd, TEMP_STR_LEN, "AT+CGDCONT=1,\"IPV4V6\",\"%s\",\"\",0,0", "cmnet");

			if (Atcmd(cmd, strlen(cmd), 0) == HUAYI_FALSE)
            {
                return 0;
            }
			
			huayi_uci_get_int(PKG_M_SYSTEM, "system", "working_mode", &mode);

            if (mode == 1)
            {
                SWITCH_SIM_LOG("After completing the network injection, start checking for network injection status");
				for (j = 0; j < 3 && !atcmd_get_imsi(value); j++) {
					sleep(2);
				}
				if(j >= 3&&sim == simnum-1)
				{
					SWITCH_SIM_LOG("not found sim card\n");
					return 0;
				}
				/*if(set_operator_band(value))
					system("huayi-reset band && killall switch_sim");*/

                while (registered_count < 60 && atcmd_get_slm_mobile_type(value) == HUAYI_FALSE)
                {
                    registered_count += 2;
                    atcmd_get_signal_for_cloud(&signal);

                    if (!(-121 <= signal && signal <= -51))
                    {
                        SWITCH_SIM_LOG("##waitting##signal value of: %d .Waiting for %d (up to 60) seconds",signal, registered_count);
                    }
                    else
                    {
	                    //registered_count += 8;
                        SWITCH_SIM_LOG("##waitting##card value :%s .waiting for %d (up to 60) seconds",value,(registered_count));
                    }
					sleep(2);
					if(strlen(value)<5)
					{
						atcmd_get_imsi(value);
						SWITCH_SIM_LOG("imsi:%s",value);
					}
                }

                if (registered_count < 60)
                {
					SWITCH_SIM_LOG("Registered for network, starting dialing");
                    Atcmd("AT+CGDATA=\"M-ETHER\",1", strlen("AT+CGDATA=\"M-ETHER\",1"), 0);
                    Atcmd("AT^SPTETHER=\"gadget\",\"on\",1", strlen("AT^SPTETHER=\"gadget\",\"on\",1"), 0);
                    SWITCH_SIM_LOG("=== SWITCH SIM %d (%s) Dialing successful===\n\n", sim, print_to);
					set_wanIp();
                    return 1;
                }
                else
                {
                    SWITCH_SIM_LOG("Dialing failed, waiting for timeout to exit card switching");
                    return 0;
                }
            }
            else
            {
                SWITCH_SIM_LOG("Production testing mode skip dialing");
                return 1;
            }
        }
        count += 3;
        sleep(3);

        if (strstr(resultbuf, "2dee:") == NULL)
        {
            SWITCH_SIM_LOG("Module not found, retrying, taking (up to 60)");
        }
        else
        {
            SWITCH_SIM_LOG("Module encryption failed, retrying, taking (up to 60)");
        }
        //SWITCH_SIM_LOG("%s",(strstr(resultbuf, "2dee:")) ? "Module not found, retrying, taking (up to 60)" : "Module encryption failed, retrying, taking (up to 60)");
    }

    return 0;
}


int switchSim() {
    switch (sim) {


        case SIM0_HIGH_LOW:
			printf("Switching 0; 0 Setting High=3.3V & 2 Setting LOW=2.7V\n");
            system("gpio l 0 0 4000 0 0 4000");
            system("gpio l 2 4000 0 0 0 4000");
            break;
#ifndef CONFIG_HUAYI_SUPPORT_MEIGE_SINGLE_CARD
        case SIM1_HIGH_HIGH:
            printf("Switching 1; 0 Setting High=3.3V & 2 Setting High=3.3V\n");
			system("gpio l 0 0 4000 0 0 4000");
            system("gpio l 2 0 4000 0 0 4000");
            break;
#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_TRIPLE_CARD
        case SIM2_LOW_HIGH:
            printf("Switching 2; 0 Setting LOW=2.7V & 2 Setting High=3.3V\n");
            system("gpio l 0 4000 0 0 0 4000");
            system("gpio l 2 0 4000 0 0 4000");
            break;
#endif
#endif

        case EXTERNAL_CARD_LOW_LOW:
            printf("Switching External card; 0 Setting LOW=2.7V & 2 Setting LOW=2.7V\n");
            system("gpio l 0 4000 0 0 0 4000");
            system("gpio l 2 4000 0 0 0 4000");
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
        system("gpio l 1 4000 0 0 0 4000");
        sleep(5);
        system("gpio l 1 0 4000 0 0 4000");
		sleep(5);

        // Assuming SlmDial function exists
        SlmDial();
    } else 
		printf("\nSwitch error %d\n Usage: %s [0-%d]\n", sim,argv[0],simnum);

    return 0;
}






