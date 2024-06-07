/*******************************************************************************************
 * Project:  huayi_clound_config                                                           *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20210114 add by rex  Version 0.0 													   *
 ******************************************************************************************/

#include "huayi_cloud_config.h"
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <ctype.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define HUAYI_UBOOTCFG_MTD_NUM  2  


char mtdubootcfg[OPTION_STR_LEN]={0};

int huayi_sn_read(char *sn)
{
	int i = 0, fd = 0;;
	unsigned char all_addr[OPTION_STR_LEN]={0};
	unsigned char oristr[OPTION_STR_LEN]={0};
	
	snprintf(mtdubootcfg, OPTION_STR_LEN, "/dev/mtdblock%d", HUAYI_UBOOTCFG_MTD_NUM);
	fd = open(mtdubootcfg, O_RDONLY);
		
	if (fd < 0) 
	{
		printf("Could not open mtd device: %s\n", mtdubootcfg);
		return -1;
	}	

	lseek(fd, 0, SEEK_SET);
	if(read(fd, all_addr, OPTION_STR_LEN) != OPTION_STR_LEN)
	{
		printf("read() failed\n");
		close(fd);
		return -1;
	}
	all_addr[OPTION_STR_LEN+1]='\0';
	for(i = 0; all_addr[i] != 0; i++) {
    	if(isxdigit(all_addr[i]) == 0)
			all_addr[i]='\0';
	}
	HexStrToStr(all_addr, oristr, strlen(all_addr));
	memcpy(sn, oristr, strlen(oristr));
	close(fd);
	return 0;
}

void huayi_get_sys_info(char *sn, char *soft_ver)
{
	
	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version", soft_ver);
	huayi_sn_read(sn);
	//huayi_uci_get_str(PKG_SYSTEM,"server","sn", sn);

	return ;
}

int huayi_isreboot_config(cJSON *response_root)
{
	char vap_section[OPTION_STR_LEN] = {0}, radio_section[OPTION_STR_LEN] = {0};	
	char ssid[OPTION_STR_LEN] = {0}, key[TEMP_STR_LEN] = {0};
	int isreboot = -1;


	if(NULL == response_root) 
	{
		return 0;
	}

	if(!huayi_json_get_int(response_root,"rebot",&isreboot)) {
		printf("reboot param is not respon !\n");
		return 0;
	} else {
		if(1 == isreboot) {
			printf("%s, %d, isreboot = %d !\n", __FUNCTION__, __LINE__, isreboot);
			return 1;
		}
	}
	//printf("%s, %d, isreboot = %d !\n", __FUNCTION__, __LINE__, isreboot);	
	return 0;
}

void set_next_ip_port(cJSON *response_root)
{
	int flag = 0;
	char oldurl[OPTION_STR_LEN] = { 0 },oldip[OPTION_STR_LEN] = { 0 };
	char ip[OPTION_STR_LEN] = { 0 };
	int oldport,port;

	huayi_uci_get_str(PKG_SYSTEM,"server","serverurl", oldurl);
	huayi_uci_get_int(PKG_SYSTEM,"server","port",&oldport);
	huayi_uci_get_str(PKG_SYSTEM,"server","ip",oldip);

	if(huayi_json_get_str(response_root,"server",ip,TEMP_STR_LEN)) 
	{
		if(0 != strlen(ip) && (0 != strcmp(ip, oldurl)) && (0 != strcmp(ip, oldip)))
		{
			CLOUD_CLIENT_LOG("server ip change !\n");
			if(is_ipv4_addr(ip) == 0)//参数为ip
			{
				huayi_uci_set_str(PKG_SYSTEM,"server","serverurl", "null");
				huayi_uci_set_str(PKG_SYSTEM,"server","ip",ip);
			}else {//参数为域名
				huayi_uci_set_str(PKG_SYSTEM,"server","serverurl", ip);
				huayi_uci_set_str(PKG_SYSTEM,"server","ip","null");
			}
			flag = 1;
		}
	}

	if(huayi_json_get_int(response_root,"port",&port))
	{
		if((port > 0) && (port != oldport)) 
		{
			CLOUD_CLIENT_LOG("server port change !\n");
			huayi_uci_set_int(PKG_SYSTEM,"server","port",port);
			flag = 1;
		}
	}

	if(1 == flag ) {
		huayi_uci_commit(PKG_SYSTEM);
	}

	return ;
}


int is_ipv4_addr(char *ip)//检验是否为ip地址
{
	int ip_num[4] = {-1, -1, -1, -1};
	char ip_s[4][4];
	int i = 0,count = 0;

	if (ip == NULL || ip[0] == '0' || ip[0] == '\0') {
		return -1;
	}

	for (i = 0, count = 0; i < strlen(ip); i++) {
		if ((ip[i] != '.') && (ip[i] < '0' || ip[i] > '9')) {
			return -1;
		}
		if (ip[i] == '.') {
			count++;
			if (count > 3) {
				return -1;
			}
		}
	}

	memset(ip_s, 0, sizeof(char[4]) * 4);

	sscanf(ip, "%[^.].%[^.].%[^.].%[^ ]", ip_s[0], ip_s[1], ip_s[2], ip_s[3]);
	sscanf(ip_s[0], "%d", &ip_num[0]);
	sscanf(ip_s[1], "%d", &ip_num[1]);
	sscanf(ip_s[2], "%d", &ip_num[2]);
	sscanf(ip_s[3], "%d", &ip_num[3]);

	for (i = 0; i < 4; i++) {
		if (strlen(ip_s[i]) == 0 || (ip_s[i][0] == '0' && ip_s[i][1] != '\0') || ip_num[i] < 0 || ip_num[i] > 255) {
			return -1;
		}
	}

	return 0;
}

