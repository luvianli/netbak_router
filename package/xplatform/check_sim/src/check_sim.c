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
#include <signal.h>
#include <pthread.h>
#include <wait.h>


void save_siminfo(int i, int gpio)
{
	char cmd[OPTION_STR_LEN] = {0};
	int  netoperator = 0;
	char tmp[OPTION_STR_LEN] = {0};
	char checksignalip[OPTION_STR_LEN] = {0}, maxdelay[OPTION_STR_LEN] = {0};
	int ret = 0, count = 0;
	int i_ready = 0, i_signal = 0;
	char lte_value[OPTION_STR_LEN] = {0};
	int AsciiNum[22],j;
	sprintf(cmd, "sim%d", i);
	atcmd_get_cpin(&i_ready);
	while(i_ready != MODEM_SIN_READY)
	{
		sleep(1);
		atcmd_get_cpin(&i_ready);
		if(count == 3) {
			break;
		}
		count += 1;
	}
	count = 0;
retry_getccid:    //尽最大可能取到ICCID
	memset(lte_value, 0, OPTION_STR_LEN);
	ret = atcmd_get_iccid(lte_value);
	for(j=0;j<strlen(lte_value) ,AsciiNum[j]=lte_value[j];j++)		
		if(strlen(lte_value) < 5 ||!((AsciiNum[j]>=48 && AsciiNum[j]<=58) || (AsciiNum[j]>=65 && AsciiNum[j]<=90) || (AsciiNum[j]>=97 && AsciiNum[j]<=122)))
		{

			count += 1;
			if(40 == count)  //3.2ms
			{
				system("huayi_AT_cli AT+CFUN=0 ");
				sleep(5);
				system("huayi_AT_cli AT+CFUN=1 ");
			}
			strcpy(lte_value, "");
			usleep(800);
			if(count < 55)
				goto retry_getccid;
		}	
	count = 0;
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid", lte_value);
	sprintf(tmp, "%d_%s", gpio, lte_value);
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid_gpio", tmp);
	huayi_uci_set_int(PKG_M_SYSTEM, cmd, "card_slot", i);
	if(strlen(lte_value)  < 10)
	{
		return;     //no iccid, that mean no simcard
	}
retry_getlac:	  //尽最大可能取到LAC
	memset(lte_value, 0, OPTION_STR_LEN);
	atcmd_get_lac(lte_value);
	//printf("rex ====   %s, %d  ---lac = %s-\n", __FUNCTION__, __LINE__, lte_value);
	if(strlen(lte_value) < 2)
	{
		strcpy(lte_value, "");
		count+=1;
		if(count < 10)
			goto retry_getlac;
	}
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "lac", lte_value);
	count = 0;

retry_getci:	  //尽最大可能取到ci
	memset(lte_value, 0, OPTION_STR_LEN);
	atcmd_get_ci(lte_value);
	//printf("rex ====   %s, %d  --ci =%s--\n", __FUNCTION__, __LINE__, lte_value);
	if(strlen(lte_value) < 2)
	{
		strcpy(lte_value, "");
		count+=1;
		if(count < 10)
			goto retry_getci;
	}
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "ci", lte_value);
	count = 0;

retry_getoperator:	  //尽最大可能取到operator info
	memset(lte_value,0,OPTION_STR_LEN);
	atcmd_get_imsi(lte_value);
	//printf("rex ====   %s, %d  ---operator = %s-\n", __FUNCTION__, __LINE__, lte_value);
	if (strncmp(lte_value,"46000",5) == 0 || strncmp(lte_value,"46002",5) == 0 || strncmp(lte_value,"46004",5) == 0 ||
		strncmp(lte_value,"46007",5) == 0 || strncmp(lte_value,"46008",5) == 0 || strncmp(lte_value,"46013",5) == 0)
	{
		netoperator = 3;
	}
	else if (strncmp(lte_value,"46001",5) == 0 || strncmp(lte_value,"46006",5) == 0 || strncmp(lte_value,"46009",5) == 0 ||
		strncmp(lte_value,"46010",5) == 0 )
	{
		netoperator = 2;
	}
	else if (strncmp(lte_value,"46003",5) == 0 || strncmp(lte_value,"46005",5) == 0 || strncmp(lte_value,"46011",5) == 0 ||
		strncmp(lte_value,"460012",5) == 0 )
	{
		netoperator = 1;
	}
	else
	{
		netoperator = 0;
		count+=1;
		if(count < 10)
			goto retry_getoperator;
	}
	huayi_uci_set_int(PKG_M_SYSTEM, cmd, "operator", netoperator);
	count = 0;

	memset(lte_value, 0, OPTION_STR_LEN);
	memset(tmp, 0, OPTION_STR_LEN);
	atcmd_get_quectel_bandLte(lte_value);
	if (strstr(lte_value,"LTE") != NULL)
	{
		strcpy(tmp, "4G");
	}
	else if (strstr(lte_value,"WCDMA") != NULL)
	{
		strcpy(tmp, "3G");
	}
	else
	{
		strcpy(tmp, "2G");
	}
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "networktype", tmp);

	atcmd_get_signal_for_cloud(&i_signal);
	huayi_uci_set_int(PKG_M_SYSTEM, cmd, "signal", i_signal);

	huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "checksignalips", checksignalip);
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "checksignalips", checksignalip);

	huayi_uci_commit(PKG_M_SYSTEM);

	return;
}

void set_siminfo()
{
	int simnum = 0, usegpio = 0, gpio = 0;
	char cmd[OPTION_STR_LEN] = {0};
	char tmp[OPTION_STR_LEN] = {0};
	int i = 0, flag = 0, ret;
	char lte_value[OPTION_STR_LEN] = {0};
	int i_ready = 0, count = 0;

	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}
	//printf("rex ====   %s, %d  ----\n", __FUNCTION__, __LINE__);
	huayi_uci_get_int(PKG_M_SYSTEM, "usesiminfo", "usegpio", &usegpio);
	memset(lte_value,0,OPTION_STR_LEN);
	atcmd_get_EC20_imei(lte_value);
	if(strlen(lte_value) < 5)
	{
		strcpy(lte_value, "");
	}
	//printf("rex ====   %s, %d  ----\n", __FUNCTION__, __LINE__);
	huayi_uci_set_str(PKG_M_SYSTEM, "system", "imei", lte_value);
	/*轮询收集所有卡片的信息*/
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);
	//printf("rex ====   %s, %d  --simnum = %d--\n", __FUNCTION__, __LINE__, simnum);
	for(i = 1; i < simnum+1; i++ )
	{
		sprintf(cmd, "sim%d", i);
		huayi_uci_get_int(PKG_M_SYSTEM, cmd, "gpio", &gpio);
		printf("rex ====   %s, %d  --gpio =%d--, usegpio = %d\n", __FUNCTION__, __LINE__, gpio, usegpio);
		if(gpio == usegpio)
		{
			flag = i;
			continue;
		}
		memset(tmp, 0, OPTION_STR_LEN);
		sprintf(tmp, "switch_sim %d", i);
		system(tmp);
		save_siminfo(i, gpio);
		//FASTCGI_LOG("sim");//test
	}
	/*switch use sim card，切换到配置文件要求使用的卡*/
	memset(tmp, 0, OPTION_STR_LEN);
	sprintf(tmp, "switch_sim %d", flag);
	system(tmp);
	save_siminfo(flag, usegpio);
	
	return;
}

void check_network()
{
	int totalsimnum = 0, simn = 0, usegpio =0;
	char cmd[OPTION_STR_LEN] = {0};
	int  netoperator = 0;
	char tmp[OPTION_STR_LEN] = {0};
	int ret = 0, count = 0, i = 0;
	int i_ready = 0, i_signal = 0;
	char lte_value[OPTION_STR_LEN] = {0};
	char iccid[OPTION_STR_LEN] = { 0 };

	/*check use sim is connect network  检测上一次使用的卡是否可以上网(出厂时上一次使用的卡为默认卡)*/
	system("ifup wan4g");
	sleep(2);
	huayi_uci_get_int(PKG_M_SYSTEM, "usesiminfo", "usegpio", &usegpio);
	memset(tmp, 0, OPTION_STR_LEN);
	sprintf(tmp, "s%d", usegpio);
	huayi_uci_get_int(PKG_M_SYSTEM, tmp, "simn", &simn);
CON_LOG("enter\n");//test

cretry1:
	memset(tmp, 0, OPTION_STR_LEN);
	huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "checksignalips", tmp);
	ret = simcard_internet_check(tmp, NULL);
	CON_LOG("ret = %d\n", ret);
	if(ret == 1)
	{
		save_useing_siminfo(simn);
		CON_LOG("check_network\n");//test
		return;
	}
	else
	{
		count += 1;
		sleep(3);
		if(count <= 20)
			goto cretry1;
	}
	count = 0;
	CON_LOG("sim%d ping bu tong %s",simn,tmp);
check_net:
	huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &totalsimnum);
	for(i = 1; i < totalsimnum+1; i++ )
	{
		sprintf(cmd, "sim%d", i);
		huayi_uci_get_str(PKG_M_SYSTEM, cmd, "iccid", iccid);
		if(strlen(iccid) < 5)
			continue;
		//huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "default_checkip", tmp);
		memset(tmp, 0, OPTION_STR_LEN);
		sprintf(tmp, "switch_sim %d", i);
		system(tmp);
		sleep(6);
		system("ifup wan4g");
		sleep(2);
	cretry2:
		memset(tmp, 0, OPTION_STR_LEN);
		huayi_uci_get_str(PKG_M_SYSTEM, "usesiminfo", "default_checkip", tmp);
		ret = simcard_internet_check(tmp, NULL);
		if(ret == 1)
		{
			//sleep(3);
			save_useing_siminfo(i);
			CON_LOG("agant check_network\n");//test
			return;
		}
		else
		{
			count += 1;
			sleep(3);
			if(count <= 20)
				goto cretry2;
		}
		count = 0;
	}
	CON_LOG("sim%d ping bu tong %s",i,tmp);
	sleep(1);
	goto check_net;
	FASTCGI_LOG("return check_network");//test
	return;
}

void init_cloud_client()
{
	CON_LOG("====first start hy_cloud_client====\n\n");
	system("hy_cloud_client &");
	return;
}

void gpio_init()
{
	system("switch_sim 100");
	sleep(1);

	return;
}


int main(int argc, char *argv[])
{
	system("huayi_atcmd_server /dev/ttyUSB1 &");
	set_siminfo();
	check_network();
	system("/etc/init.d/huayi_dial4gcontrol restart");

	return 0;
}


