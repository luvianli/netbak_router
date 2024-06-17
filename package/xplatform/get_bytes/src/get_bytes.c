#include "huayi_atcmd.h"
#include "huayi_uci_fun.h"
#include <signal.h>
#include <pthread.h>
#include <wait.h>

void get_bytes()
{
	int simnum,at_card,i=0;
	long rx=0,tx=0;
	long rx_bytes=0,tx_bytes=0;
	long rx_old=0,tx_old=0;
	char cmd[OPTION_STR_LEN] = {0},cmd2[OPTION_STR_LEN] = {0},result[OPTION_STR_LEN] = {0};
	char registStatus[LONG_BUFF_LEN]={0};
	char wan_type[OPTION_STR_LEN] = {0};
	char section[SHORT_STR_LEN] = {0};

	while(1)
	{
		huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
		if (!strcmp(wan_type,"mobile"))
		{
			sprintf(section,"eth1");
		}else
			sprintf(section,"eth0.2");
		
		sprintf(cmd,"ifconfig %s | grep 'RX bytes' | awk '{print $2}'",section);
		get_cmd_result(cmd,result, CMD_STR_LEN);
		strcpy(cmd2, result + 6);
		CON_LOG("rx bytes:%s,%s",cmd,cmd2);
		rx=atoi(cmd2)/1024;
		//memset(cmd, 0, CMD_STR_LEN);
		sprintf(cmd,"ifconfig %s | grep 'TX bytes' | awk '{print $6}'",section);
		get_cmd_result(cmd,result, CMD_STR_LEN);
		strcpy(cmd2, result + 6);
		CON_LOG("tx bytes:%s,%s",cmd,cmd2);
		tx=atoi(cmd2)/1024;
		CON_LOG("rold:%d,told:%d,rx:%d,tx:%d",rx_old,tx_old,rx,tx);

		if(rx_old < rx)
		{
			i = rx ;
			rx = rx - rx_old;
			rx_old = i;
		}else rx_old = rx;
		if(tx_old < tx)
		{			
			i = tx ;
			tx = tx - tx_old;
			tx_old = i;
		}else tx_old = tx;
		CON_LOG("rold:%d,told:%d,rx:%d,tx:%d",rx_old,tx_old,rx,tx);

		huayi_uci_get_long(PKG_SYSTEM, "network", "rx_bytes", &rx_bytes);
		huayi_uci_get_long(PKG_SYSTEM, "network", "tx_bytes", &tx_bytes);

		rx_bytes = rx_bytes + rx;
		tx_bytes = tx_bytes + tx;
		huayi_uci_set_long(PKG_SYSTEM, "network", "rx_bytes", rx_bytes);
		huayi_uci_set_long(PKG_SYSTEM, "network", "tx_bytes", tx_bytes);
		huayi_uci_commit(PKG_SYSTEM);
		sleep(45);
		continue ;
	}
	return ;
}

int main()
{
	char bytes[OPTION_STR_LEN] = {0};
	huayi_uci_get_str(PKG_SYSTEM,"network","amound_end",bytes);
	huayi_uci_set_str(PKG_SYSTEM,"network","all",bytes);
	huayi_uci_set_long(PKG_SYSTEM, "network", "rx_bytes", 0);
	huayi_uci_set_long(PKG_SYSTEM, "network", "tx_bytes", 0);
	huayi_uci_commit(PKG_SYSTEM);

	get_bytes();
	return ;
}

