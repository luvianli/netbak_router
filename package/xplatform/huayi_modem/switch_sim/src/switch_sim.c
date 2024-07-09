#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "huayi_uci_fun.h"

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


int switchSim() {
    switch (sim) {

		case 0:
			printf("Switching CHN-CT\n");
			system("gpio l 0 0 4000 0 0 4000");
			system("gpio l 36 0 4000 0 0 4000");
		break;
		
		case 1:
			printf("Switching CHINA MOBILE CMCC\n");
			system("gpio l 0 0 4000 0 0 4000");
			system("gpio l 36 4000 0 0 0 4000");
		break;
		
		case 2:
			printf("Switching 2\n");
			system("gpio l 0 4000 0 0 0 4000");
			system("gpio l 0 4000 0 0 0 4000");
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
		
        switchSim();
		sleep(3);
		system("huayi_AT_cli AT+CFUN=0");
        sleep(2);
		system("huayi_AT_cli AT+CFUN=1");
    } else 
		printf("\nSwitch error %d\n Usage: %s [0-3]\n", sim,argv[0]);

    return 0;
}






