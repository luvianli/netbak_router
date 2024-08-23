/*******************************************************************************************
 * Project:  check_sim                                                        *
 *                                                                                         *
 * Author :  rex									                                       *
 *  																					   *
 * Changelist: 																		       *
 * 20220215 add by rex  Version 1.0
 * check sim card
 ******************************************************************************************/
#include "huayi_atcmd.h"
#include "huayi_uci_fun.h"
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

#define serverip "device.10086group.com"

int main(int argc, char *argv[])
{ 
	char cmd[TEMP_STR_LEN] = {0},resultbuf[SHORT_STR_LEN] = {0};
	int ret = 0,i = 0,j = 0,k = 0;
	char ifname[IPV4_STR_LEN] = {0};
	char gateway[RESULT_STR_LEN] = {0},ip_address[RESULT_STR_LEN] = {0};
	int interval = 0,status = 0;
	
	while(1)
	{
		if(!isProcessRunning("hy_cloud_client"))
		{
			NETCHECK_LOG("client not found,restart");
			system("rm -fr /tmp/log/cloud_client_log");
			system("/etc/init.d/hy_cloud_client start");
		}else
		{
			huayi_uci_get_int(PKG_SYSTEM,"server","status", &status);
			if(status == 0 && k >= 3)
			{
				k = 0;
				NETCHECK_LOG("client error 3times,restart");
				system("/etc/init.d/hy_cloud_client restart");
			}else if(status == 0 && k < 3)
			{
				k++;
				huayi_uci_get_int(PKG_SYSTEM,"server","interval", &interval);
				usleep(interval);
				continue;
			}else
			{
				k = 0;
			}
		} 

		if(internet_do_ping_detect(INTERNET_DOMAIN, NULL) != 1)
		{
			NETCHECK_LOG("check ping error");
			if(j == 0)
			{
				ret = get_cmd_result("lsusb",resultbuf,sizeof(resultbuf));
				if(ret != 1 && i < 3)
				{
					NETCHECK_LOG("not found usb,restart usb");
					system("huayi_AT_cli AT+CFUN=0");
			        sleep(5);
					system("huayi_AT_cli AT+CFUN=1");
					i++;
					continue;
				}
				i++;
				if(i > 5)
				{
					j++;
					system("huayi-reset network");
				}
			}
		}else 
		{
			i = 0;
			j = 0;
			//NETCHECK_LOG("check ping success");
		}
		sleep(60);
	}
	
    return 0;
}


