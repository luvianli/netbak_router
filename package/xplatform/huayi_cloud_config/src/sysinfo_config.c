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


