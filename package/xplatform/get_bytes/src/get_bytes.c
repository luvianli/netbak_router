#include "huayi_atcmd.h"
#include "huayi_uci_fun.h"
#include <signal.h>
#include <pthread.h>
#include <wait.h>

void get_bytes()
{
	int simnum,at_card,i=0;
	int rx=0,tx=0;
	int rx_bytes=0,tx_bytes=0;
	int rx_old=0,tx_old=0;
	char cmd[OPTION_STR_LEN] = {0},cmd2[OPTION_STR_LEN] = {0},time[OPTION_STR_LEN] = {0};
	char registStatus[LONG_BUFF_LEN]={0};

	huayi_uci_set_int(PKG_SYSTEM, "network", "rx_bytes", 0);
	huayi_uci_set_int(PKG_SYSTEM, "network", "tx_bytes", 0);
	huayi_uci_set_str(PKG_SYSTEM, "network", "time", "");
	huayi_uci_commit(PKG_SYSTEM);

	while(1)
	{
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "registStatus", registStatus);
		if (strcmp(registStatus ,"connected") != 0) {
			CON_LOG("modem is not connected; registStatus is : %d\n",registStatus);
			sleep(40);
			continue;
		}

		get_cmd_result("ifconfig eth1 | grep 'RX bytes' | awk '{print $2}'",cmd, CMD_STR_LEN);
		strcpy(cmd2, cmd + 6);
		CON_LOG("rx bytes:%s,%s",cmd,cmd2);
		rx=atoi(cmd2)/1024;
		memset(cmd, 0, CMD_STR_LEN);
		get_cmd_result("ifconfig eth1 | grep 'TX bytes' | awk '{print $6}'",cmd, CMD_STR_LEN);
		strcpy(cmd2, cmd+6);
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
#ifdef CONFIG_HUAYI_SUPPORT_MEIGE_AFTER_SALES
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "at_card", &at_card);
		CON_LOG("sim:%d,card:%d",simnum,at_card);
		if(simnum-1 == at_card)//外插卡
		{		
			sleep(60);
			continue ;
		}
#endif
		huayi_uci_get_int(PKG_SYSTEM, "network", "rx_bytes", &rx_bytes);
		huayi_uci_get_int(PKG_SYSTEM, "network", "tx_bytes", &tx_bytes);

		rx_bytes = rx_bytes + rx;
		tx_bytes = tx_bytes + tx;
		huayi_uci_set_int(PKG_SYSTEM, "network", "rx_bytes", rx_bytes);
		huayi_uci_set_int(PKG_SYSTEM, "network", "tx_bytes", tx_bytes);
		get_current_time(time);
		huayi_uci_set_str(PKG_SYSTEM, "network", "time", time);
		huayi_uci_commit(PKG_SYSTEM);
		sleep(60);
		continue ;
	}
	return ;
}

void get_current_time(char *tmpbuf)
{
	FILE *fp = NULL;
	char *p = NULL;
	char buf[128]={0};
	
	fp = popen("date +\"%Y%m%d%H%M%S\"", "r");
	if(fp == NULL) 
	{
		return;
	}
	
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		if((p=strstr(buf, "\n")))
		{
			p[0]='\0';
		}
	}
	
	pclose(fp);
	
	strcpy(tmpbuf,buf);
	
	return;
}

int main()
{
	get_bytes();
	return ;
}

