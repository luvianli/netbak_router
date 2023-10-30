#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "huayi_uci_fun.h"



int main(int argc, char* argv[])
{
	int iRet;
    int lock = 0;
    int force = 1;
    char resultbuf[OPTION_STR_LEN] = {0};
    pthread_t ptd;
    int iSel = atoi(argv[1]);
    /*
    if (iSel == 4){ 
        if(argc > 2 && strcmp(argv[2], "ZYCPEadmin1235") == 0) {
                // 处理密码正确的情况
                printf("password correct , start switch——sim4\n");
                CON_LOG("password correct , start switch——sim4\n");
        } else {
                printf("Switch card 4, please enter password; Format switch_Sim 4 [password]\n");
                CON_LOG("Switch card 4, please enter password; Format switch_Sim 4 [password]");
                return;
        }
    }
    */
        system("devmem 0x10000060 32 0x4110441");
    if (argc >= 2 && iSel < 5) {
    	switch(iSel)
    	{
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
              /*
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
    		    printf("switch sim1;0 Set High=3.3V & 2 Set LOW=2.7V\n" );
    			system("gpio l 0 0 4000 0 0 4000");
                system("gpio l 2 4000 0 0 0 4000");
    		  break;
    	}
        system("gpio l 1 4000 0 0 0 4000");
        sleep(5);
        system("gpio l 1 0 4000 0 0 4000");
    	sleep(10);
        int count = 0;
        while(count < 10){
        get_cmd_result("lsusb",resultbuf,sizeof(resultbuf));
        if(strstr(resultbuf,"2dee:") !=NULL){
            if(rc4_SLM770U()){
                const char* command = "huayi_AT_cli AT+SFUN=4;huayi_AT_cli AT+CGDCONT=1,\\\"IPV4V6\\\",\\\"cmnet\\\",\\\"\\\",0,0;huayi_AT_cli AT+CGDATA=\\\"M-ETHER\\\",1;huayi_AT_cli AT^SPTETHER=\\\"gadget\\\",\\\"on\\\",1";
                system(command);
                CON_LOG("===switch sim%s   OK===\n\n",argv[1]);
              //  huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "card_slot",iSel);
               // huayi_uci_commit(PKG_M_SYSTEM);
                return 0;
            }
            CON_LOG("Card cutting activation failed");
        }
     
        count += 1;
		sleep(3);
    }
        
    }else 
        printf("Missing or missing parameters. Please check the command\n\n");

    return 0;
}





