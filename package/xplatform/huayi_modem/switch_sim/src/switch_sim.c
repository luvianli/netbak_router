#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "huayi_uci_fun.h"
// Define enum for better readability
#ifdef CONFIG_HUAYI_SUPPORT_CARD_SUPPORT

enum SimSelection {

    SIM0_HIGH_HIGH,
#ifndef CONFIG_HUAYI_SUPPORT_MEIGE_SINGLE_CARD
    SIM1_HIGH_LOW,
#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_TRIPLE_CARD
    SIM2_LOW_HIGH,
#endif
#endif
	EXTERNAL_CARD_LOW_LOW,
    INVALID_SELECTION
};
#endif


int SlmDial(int sim)
{
    int count = 0, registered_count = 0, signal = 0, band = 0;
    char resultbuf[OPTION_STR_LEN] = {0};
    char cmd[TEMP_STR_LEN] = {0};
    char print_to[TEMP_STR_LEN] = {0};

    strcpy(print_to, (sim != EXTERNAL_CARD_LOW_LOW) ? "built-in" : "External");
    SWITCH_SIM_LOG("\n\ngpio sim%d",sim);
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
            int mode = 0;
            huayi_uci_get_int(PKG_M_SYSTEM, "system", "working_mode", &mode);
            if (mode == 1)
            {
	            huayi_uci_get_int(PKG_M_SYSTEM, "system", "band", &band);
				if( band ){
					memset(cmd,0,TEMP_STR_LEN);
					snprintf(cmd, TEMP_STR_LEN, "AT+SPLBAND=1,0,0,0,%d,0", band);
					system(cmd);
				    SWITCH_SIM_LOG("custom set band %d",band);
				}
                SWITCH_SIM_LOG("After completing the network injection, start checking for network injection status");

                while (registered_count < 60 && atcmd_get_slm_mobile_type() == HUAYI_FALSE)
                {
                    registered_count += 2;
                    atcmd_get_signal_for_cloud(&signal);

                    if (!(-121 <= signal && signal <= -51))
                    {
                        SWITCH_SIM_LOG("The signal of the card being switched is too poor, with a signal value of: %d. Waiting for %d (up to 60) seconds",
                                       signal, registered_count);
                    }
                    else
                    {
	                    registered_count += 8;
                        SWITCH_SIM_LOG("The card you want to switch to is not registered and has been waiting for %d (up to 20) seconds",
                                       (registered_count / 10));
                    }
					sleep(2);
                }

                if (registered_count < 60)
                {
                    SWITCH_SIM_LOG("Registered for network, starting dialing");

                    Atcmd("AT+CGDATA=\"M-ETHER\",1", strlen("AT+CGDATA=\"M-ETHER\",1"), 0);
                    Atcmd("AT^SPTETHER=\"gadget\",\"on\",1", strlen("AT^SPTETHER=\"gadget\",\"on\",1"), 0);
					huayi_uci_set_int(PKG_M_SYSTEM, "system", "at_card", sim);
					huayi_uci_commit (PKG_M_SYSTEM);
                    SWITCH_SIM_LOG("=== SWITCH SIM %d (%s) Dialing successful===\n\n", sim, print_to);
					
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


int switchSim(int simSelection) {
    switch (simSelection) {


        case SIM0_HIGH_HIGH:
            printf("Switching sim0; Setting High=3.3V & 2 Setting High=3.3V\n");
           // system("gpio l 0 0 4000 0 0 4000");
           // system("gpio l 2 0 4000 0 0 4000");
            system("gpio l 0 4000 0 0 0 4000");
            system("gpio l 2 0 4000 0 0 4000");
            break;
#ifndef CONFIG_HUAYI_SUPPORT_MEIGE_SINGLE_CARD
        case SIM1_HIGH_LOW:
            printf("Switching sim1; Setting High=3.3V & 2 Setting LOW=2.7V\n");
            system("gpio l 0 0 4000 0 0 4000");
            system("gpio l 2 4000 0 0 0 4000");
            break;
#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_TRIPLE_CARD
        case SIM2_LOW_HIGH:
            printf("Switching sim2; Setting LOW=2.7V & 2 Setting High=3.3V\n");
			system("gpio l 0 0 4000 0 0 4000");
            system("gpio l 2 0 4000 0 0 4000");
            break;
#endif
#endif

        case EXTERNAL_CARD_LOW_LOW:
            printf("Switching External card; Setting LOW=2.7V & 2 Setting LOW=2.7V\n");
            system("gpio l 0 4000 0 0 0 4000");
            system("gpio l 2 4000 0 0 0 4000");
            break;

        default:
            printf("\nSwitch error: Invalid selection\nUsage: %d [0|1|2|3]\n", simSelection);
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Missing parameters. Please check the command\n\n");
        return -1;
    }

    int iSel = atoi(argv[1]);
	int simnum = 0;
    huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);

    if (iSel >= 0 && iSel < simnum) {
        system("devmem 0x10000060 32 0x4110441");

        switchSim(iSel);

        system("gpio l 1 4000 0 0 0 4000");
        sleep(5);
        system("gpio l 1 0 4000 0 0 4000");
		sleep(5);

        // Assuming SlmDial function exists
         SlmDial(iSel);
    } else {
        printf("Invalid selection. Usage: %s [0|1|2|3]\n", argv[0]);
        return -1;
    }

    return 0;
}






