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
	int ret = 0;
	char ifname[IPV4_STR_LEN] = {0};
	char gateway[RESULT_STR_LEN] = {0},ip_address[RESULT_STR_LEN] = {0};
	
	if(internet_do_ping_detect(INTERNET_DOMAIN, NULL) != 1)
	{
		NETCHECK_LOG("check ping error,check lsusb");
		ret = get_cmd_result("lsusb",resultbuf,sizeof(resultbuf));
		if(ret != 1)
		{
			NETCHECK_LOG("not found usb,restart usb");
			system("huayi_AT_cli AT+CFUN=0");
	        sleep(5);
			system("huayi_AT_cli AT+CFUN=1");
		}else	NETCHECK_LOG("check usb success,waite sim check");
		return 1;
	}else NETCHECK_LOG("check ping success");

	if(internet_do_ping_detect(serverip, NULL))
	{
		NETCHECK_LOG("ping server success,restart client");
		system("/etc/init.d/hy_cloud_client restart");
		return 1;
	}NETCHECK_LOG("ping server error");
	
    return 0;
}


