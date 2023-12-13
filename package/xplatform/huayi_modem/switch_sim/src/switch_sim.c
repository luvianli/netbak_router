#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "huayi_uci_fun.h"

int SlmDial(int sim) {
    int count = 0, registered_count = 0, signal = 0;
    char resultbuf[OPTION_STR_LEN] = {0};
    char cmd[TEMP_STR_LEN] = {0};
    char print_to[TEMP_STR_LEN] = {0};
    strcpy(print_to,(sim < 2) ? "built-in" : "External");
    SWITCH_SIM_LOG("gpio ok");
    while (count < 60) {
        get_cmd_result("lsusb", resultbuf, sizeof(resultbuf));

        if (strstr(resultbuf, "2dee:") != NULL && rc4_SLM770U()) {
            SWITCH_SIM_LOG("=== SWITCH SIM %d (%s) OK ===", sim,print_to);

            Atcmd("AT+SFUN=4", strlen("AT+SFUN=4"), 0);
            
            snprintf(cmd, TEMP_STR_LEN, "AT+CGDCONT=1,\"IPV4V6\",\"%s\",\"\",0,0", "cmnet");
            if (Atcmd(cmd, strlen(cmd), 0) == HUAYI_FALSE) {
                return 0;
            }

            SWITCH_SIM_LOG("After completing the network injection, start checking for network injection status");

            while (registered_count < 60 && atcmd_get_slm_mobile_type() == HUAYI_FALSE) {
                sleep(2);
                registered_count += 2;
                atcmd_get_signal_for_cloud(&signal);

               if(!(-121 <= signal && signal <= -51)){
                    SWITCH_SIM_LOG("The signal of the card being switched is too poor, with a signal value of: %d. Waiting for %d (up to 60) seconds", signal, registered_count);
                }
                else{
                    SWITCH_SIM_LOG("The card you want to switch to is not registered and has been waiting for %d (up to 60) seconds", registered_count);
                }
            }

            if (registered_count < 60) {
                SWITCH_SIM_LOG("Registered for network, starting dialing");

                Atcmd("AT+CGDATA=\"M-ETHER\",1", strlen("AT+CGDATA=\"M-ETHER\",1"), 0);
                Atcmd("AT^SPTETHER=\"gadget\",\"on\",1", strlen("AT^SPTETHER=\"gadget\",\"on\",1"), 0);

                SWITCH_SIM_LOG("=== SWITCH SIM %d (%s) Dialing successful===\n\n", sim,print_to);
                return 1;
            }else{
                SWITCH_SIM_LOG("Dialing failed, waiting for timeout to exit card switching");
                return 0;
            }
        }
        count += 3;
        sleep(3);

        if (strstr(resultbuf, "2dee:")){
            SWITCH_SIM_LOG("Module not found, retrying, taking (up to 60)");
        }else{
            SWITCH_SIM_LOG("Module encryption failed, retrying, taking (up to 60)");
        }
        //SWITCH_SIM_LOG("%s",(strstr(resultbuf, "2dee:")) ? "Module not found, retrying, taking (up to 60)" : "Module encryption failed, retrying, taking (up to 60)");
    }

    return 0;
}

int main(int argc, char* argv[])
{
	int iRet;
    int lock = 0;
    int force = 1;
    pthread_t ptd;
    int iSel = atoi(argv[1]);
        system("devmem 0x10000060 32 0x4110441");
    if (argc >= 2 && iSel < 5) {
    	switch(iSel)
    	{   
            /*
    	   case 0 :
                 printf("switch sim2;0 Set LOW=2.7V & 2 Set High=3.3V" );
                 system("gpio l 0 0 4000 0 0 4000");
                 system("gpio l 2 0 4000 0 0 4000");
    		  break;
    	   case 1 :
                printf("switch sim1;0 Set High=3.3V & 2 Set LOW=2.7V\n" );
    			system("gpio l 0 0 4000 0 0 4000");
                system("gpio l 2 4000 0 0 0 4000");
    		  break;
           
    	   case 2 :
    		    printf("switch sim3;0 Set High=3.3V & 2 Set High=3.3V" );
                system("gpio l 0 0 4000 0 0 4000");
                system("gpio l 2 0 4000 0 0 4000");

    		  break;
    		  */
           case 2 :
                printf("switch sim4;0 Set LOW=2.7V & 2 Set LOW=2.7V" );
                system("gpio l 0 4000 0 0 0 4000");
                system("gpio l 2 4000 0 0 0 4000");

              break;
    		default :
                system("gpio l 0 4000 0 0 0 4000");
                system("gpio l 2 4000 0 0 0 4000");
                /*
    		    printf("switch sim1;0 Set High=3.3V & 2 Set LOW=2.7V\n" );
    			system("gpio l 0 0 4000 0 0 4000");
                system("gpio l 2 4000 0 0 0 4000");
                */
    		  break;
    	}
        system("gpio l 1 4000 0 0 0 4000");
        sleep(5);
        system("gpio l 1 0 4000 0 0 4000");
        SlmDial(iSel);
    }else 
        SWITCH_SIM_LOG("Missing or missing parameters. Please check the command\n\n");

    return 0;
}





