/**
 * @file huayi_sn.c
 *
 * Created commands to change huayi config
 *
 * This tool is responsible to change the configuration 
 * of the huayi system.
 * 
 * =========================================================
 * Details about the command mode.
 * huayi_cmd <op>  <cmd> <interface><value>
 * @op: set or get
 * @cmd: suppoted cmd listed in cmd_help.txt
 * @interface: interface listed in cmd_help.txt, as sector/wifi. If empty, means system
 * @value: If op is get, option huayi will not be used. If op is set, the value's type
 * defined in cmd_help.txt(type), supported number of values defined in cmd_help.txt(num)
 *
 * Example:
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <ctype.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include "huayi_uci_fun.h"
#include "huayi_config_common.h"
#include "huayi_atcmd.h"

#define HUAYI_UBOOTCONFIG_MTD_NUM  2  
extern LTE_BOOL atcmd_get_signal(int *value);

char mtd_ubootcfg[OPTION_STR_LEN]={0};

void usage(void)
{
	printf("Usage: huayi_sn <op> [paramter]\n");
	printf("huayi_sn write [ XXXXXXXXXXX ]; str 'XXXXXXXXXXX' length must be <=32 \r\n");
	printf("huayi_sn read sn \r\n");
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	printf("huayi_sn read info \n");
	printf("read info:  sn;imei;iccid;4Gmodel;mac;ssid;wifikey\r\n");
#if defined(CONFIG_HUAYI_SIM_CHANGE_SUPPORT)
	printf("huayi_sn change extsim \n");
	printf("change to extern simcard \r\n");
	printf("huayi_sn change insim \n");
	printf("change to internal simcard \r\n");
#endif
	printf("huayi_sn read 4ginfo \n");
	printf("read 4ginfo:  simcard_ready;4g_signal\r\n");
	printf("huayi_sn product_test\r\n");
#endif
	printf("huayi_sn read info \n");
	printf("read info:  sn;mac;ssid;wifikey\r\n");
	printf("huayi_sn read port \n");
	printf("read port:  port0=1;port1=1....\r\n");
	return;
}

int hy_mtd_read(char **value)
{
	unsigned char sn[OPTION_STR_LEN]={ 0 };
	char ssid[OPTION_STR_LEN] = { 0 }, key[TEMP_STR_LEN] = { 0 };
	char mac[OPTION_STR_LEN] = { 0 };
	int portstatus[OPTION_STR_LEN] = { 0 };
	int i = 0, port_link = 0;
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	char pmodel[SHORT_STR_LEN]={ 0 }, imei[OPTION_STR_LEN] = { 0 };
	char iccid[OPTION_STR_LEN] = { 0 };
	int signal = 0;
	int sim_state = 0;
#endif

	if ( !strncmp(*value, "sn", 2) )	
	{	
		if (get_dev_sn(sn) < 0) 
		{
			strcpy(sn, " ");
		}
		if(strncmp(sn, "FFFFFFFFFFFF", strlen(sn)) == 0)
		{
			strcpy(sn, " ");
		}
		printf("sn oristr : ");
		printf("%s; \n", sn);	
	}	
	else if ( !strncmp(*value, "info", 4) )	
	{		
		system("iwconfig ra0 > /tmp/iwconfigra0");
		huayi_uci_get_str(PKG_WIRELESS, "radio0_0", "ssid", ssid);
		huayi_uci_get_str(PKG_WIRELESS, "radio0_0", "key", key);
		if (get_dev_sn(sn) < 0) 
		{
			strcpy(sn, " ");
		}	
		if(strncmp(sn, "FFFFFFFFFFF", strlen(sn)) == 0)
		{
			strcpy(sn, " ");
		}
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", imei);
		huayi_uci_get_str(PKG_STATUS_MODEM,"modem", "iccid", iccid);
		get_cmd_result("cat /tmp/iwconfigra0 | grep 'Access Point' | awk '{print $5}'", mac, sizeof(mac));
		printf("%s;%s;%s;%s;%s;%s;%s;\n", sn, imei, iccid, pmodel, mac, ssid, key);
#else
		get_cmd_result("cat /tmp/iwconfigra0 | grep 'Access Point' | awk '{print $5}'", mac, sizeof(mac));
		printf("%s;%s;%s;%s;\n", sn, mac, ssid, key);
#endif
	}	
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	else if ( !strncmp(*value, "4ginfo", 6) )	
	{		
	
		atcmd_get_cpin(&sim_state);
		/*sim_state == 1 is ok; sim card ready*/
		atcmd_get_iccid(iccid);
		atcmd_get_signal(&signal);
		printf("%d;%d;%s;\n", sim_state, signal, iccid);
		
	}
#endif
	else if (!strncmp(*value, "port", 4))
	{	
		for( i = 0; i < 4; i ++) {
			port_link = is_phyconnected(i);
			portstatus[i] = port_link;
		}
		
		for( i = 0; i < 4; i ++) {
			printf("port%d=0x%d;", i, portstatus[i]);
		}	
		printf("\n");
	}
	else if (!strncmp(*value, "wpsbutton", 9))
	{	
		usleep(500);
		if(!access("/tmp/wps_test", 0))
		{
			printf("1;\n");
			usleep(500);
			system("rm -fr /tmp/wps_test");
		}
		else
		{
			printf("0;\n");
		}
	}
	else if (!strncmp(*value, "resetbutton", 11))
	{	
		sleep(1);
		if(!access("/tmp/reset_test", 0))
		{
			printf("1;\n");
			usleep(500);
			system("rm -fr /tmp/reset_test");
		}
		else
		{
			printf("0;\n");
		}
	}
	else	
	{		
		return -1;	
	}
	
	
	return 0;
}


int hy_mtd_write( char **value)
{
	char *hexstrbuf=NULL;
	unsigned int srclen = 0, hexstrlen = 0;
	int fd = open(mtd_ubootcfg, O_RDWR | O_SYNC);

	

	srclen = strlen(*value);
	if(32 < srclen || srclen <= 0)
	{
		fprintf(stderr, "source str length must be <= 32 !\n");
		return -1;
	}
	if (fd < 0) 
	{
		fprintf(stderr, "Could not open mtd device: %s\n", mtd_ubootcfg);
		return -1;
	}
	
	hexstrbuf = (char *)malloc(OPTION_STR_LEN);
	if (NULL == hexstrbuf)
	{
		printf("Allocate memory for 64 failed.\n");
		close(fd);
		return -1;		  
	}
	system("mtd erase Config");

	printf("source str = %s, srclen = %d\n", *value, srclen);
	Str2Hex(*value, hexstrbuf, srclen);
	hexstrlen = strlen(hexstrbuf);
	printf("hex str = %s, hexstrlen = %d\n", hexstrbuf, hexstrlen);
	lseek(fd, 0, SEEK_SET);
	if (write(fd, hexstrbuf, hexstrlen) != hexstrlen) 
	{
		fprintf(stderr, "write() %s failed\n", mtd_ubootcfg);
		goto write_fail;
	}
	
	close(fd);
	free(hexstrbuf);
	
	return 0;
	
write_fail:
	close(fd);
	free(hexstrbuf);
	
	return -1;
}

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
#if defined(CONFIG_HUAYI_SIM_CHANGE_SUPPORT)
int hy_change_simcard( char **value)
{
	unsigned char sn[OPTION_STR_LEN]={ 0 };
	char ssid[OPTION_STR_LEN] = { 0 }, key[TEMP_STR_LEN] = { 0 };
	char pmodel[SHORT_STR_LEN]={ 0 }, imei[OPTION_STR_LEN] = { 0 };
	char iccid[OPTION_STR_LEN] = { 0 }, mac[OPTION_STR_LEN] = { 0 };
	int signal = 0;
	char cmd[CMD_STR_LEN] = {0};
	
	if ( !strncmp(*value, "extsim", 6) )	
	{	
		system("/etc/init.d/hy_sc stop 2>/dev/null");
		//system("/etc/init.d/huayi_dial4g stop 2>/dev/null");
        //system("killall -9 check_sim");
        //system("devmem 0x10000060 32 0x4110441");
        //system("gpio l 0 0 4000 0 0 4000");
        //system("gpio l 2 0 4000 0 0 4000");
		system("switch_sim 1");
		//snprintf(cmd, CMD_STR_LEN, "gpio l %d 0 4000 0 0 4000", CONFIG_HUAYI_SIM_CHANGE_GPIO_NUM);
		//system(cmd);
		//system("huayi_AT_cli at+cfun=1,1");	
	}	
	else if ( !strncmp(*value, "insim", 5) )	
	{		
		system("/etc/init.d/hy_sc stop 2>/dev/null");
		//system("/etc/init.d/huayi_dial4g stop 2>/dev/null");
        //system("killall -9 check_sim");
        system("switch_sim 0");
        //system("devmem 0x10000060 32 0x4110441");

        //system("gpio l 0 0 4000 0 0 4000");
        //system("gpio l 2 4000 0 0 0 4000");
		//snprintf(cmd, CMD_STR_LEN, "gpio l %d 4000 0 0 0 4000", CONFIG_HUAYI_SIM_CHANGE_GPIO_NUM);
		//system(cmd);
		//system("huayi_AT_cli at+cfun=1,1");
	}		
	else	
	{		
		return -1;	
	}
	
	return 0;
}
#endif
#endif

int hy_product_test()
{
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	system("/etc/init.d/hy_sc stop 2>/dev/null");
	sleep(1);
	system("/etc/init.d/huayi_dial4g stop 2>/dev/null");
	usleep(500);
	system("/etc/init.d/hy_cloud_client stop 2>/dev/null");
#endif
	return 0;
}



int main(int argc,char **argv)
{
	char op[OPTION_STR_LEN];

	strcpy(op, argv[1]);
	
	if(0 != strcmp(op,"product_test")) {
		if (argc < 3)
		{
			goto invald_cmd;
		}
	}
	
	snprintf(mtd_ubootcfg, OPTION_STR_LEN, "/dev/mtdblock%d", HUAYI_UBOOTCONFIG_MTD_NUM);
	
	if (!strcmp(op,"read"))
	{
		if (hy_mtd_read(argv+2) < 0)
		{
			goto cmd_fail;
		}
	}
	else if (!strcmp(op,"write"))
	{
		if (hy_mtd_write(argv+2) < 0)
		{
			goto cmd_fail;
		}
	}
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
#if defined(CONFIG_HUAYI_SIM_CHANGE_SUPPORT)
	else if (!strcmp(op,"change"))
	{
		if (hy_change_simcard(argv+2) < 0)
		{
			goto cmd_fail;
		}
	}
#endif
#endif

	else if (!strcmp(op,"product_test"))
	{
		if (hy_product_test() < 0)
		{
			goto cmd_fail;
		}
	}

	printf("huayi_sn excution success\n");
	return 0;

invald_cmd:
    usage();
	return 0;
	
cmd_fail:
	usage();
	printf("huayi_sn excution failed\n");
	return 0;
}

