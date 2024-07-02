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

void huayi_get_sys_info(char *soft_ver,char *product_name,int *uptime)
{
	int time = 0;
	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version", soft_ver);
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_name",product_name);
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&time);
	*uptime = time;

	return ;
}

void huayi_get_sche_reboot_config(int *sch_reboot,int *rectime,int *week,int *hour,int *minute,int *rec_time)
{
	int mode = 0,rectimee = 0,weeks = 0,hourr = 0,minutee = 0,timee = 0;
	struct tm *p = NULL;
	time_t timep;
	unsigned long time_of_day = 0;
	int rec_uptime = 0,uptime = 0,timed = 0;
	int k = 0,i = 0,cur_time = 0,next_time = 0,interval = 0;
	bool this_week = 0;
	
	huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","sche_reboot_mode",&mode);
	huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","rectime",&rectimee);
	huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","weeks",&weeks);
	huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","hour",&hourr);
	huayi_uci_get_int(PKG_SYSTEM,"sche_reboot","minute",&minutee);
	*sch_reboot = mode;
	*rectime = rectimee;
	*week = weeks;
	*hour = hourr;
	*minute = minutee;

	time(&timep);		
	p=localtime(&timep);
	time_of_day = timep + p->tm_gmtoff - 28800;

	switch(mode)
	{
		case 1:
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);
			huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","rec_uptime",&rec_uptime);
			if(rec_uptime == 0)
			{
				*rec_time = rectimee;
			}else
			{
				*rec_time = rectimee - (uptime - rec_uptime);
				timed = rectimee - (uptime - rec_uptime);
			 	if(timed < 0)
			 	{
			 		*rec_time = 0;
			 	}
			}
			break;
		case 2:
			i = p->tm_wday;
			cur_time = (p->tm_wday *1440 + p->tm_hour*60 + p->tm_min);
			weeks = (weeks == 255 ? 127 : weeks);
			for(k=i;k<7;k++)
			{
				if(MyGetField32(weeks,k))
				{
					next_time = ( k * 1440 +hourr *60+minutee);
					if(next_time >= cur_time)
					{
						/*find a day this week*/
						this_week = 1 ;
						break;
					}
				}
			}

			if(this_week)
			{
				interval= (next_time - cur_time);
			}else
			{	
				/*get first day next week*/
				for(k=0;k<=i;k++)
				{
					if(MyGetField32(weeks,k))
					{
						/*first day need reboot in next  week*/
						next_time = ( k * 1440 +hourr *60+minutee);
						interval = next_time + ( (1440 * 7 ) - cur_time);
						break;
					}
				}
			}

			if (interval > 0)
				*rec_time = (interval * 60) - p->tm_sec;
			else
				*rec_time = 0;
			break;
		case 0:
		default:
			*rec_time = 0;
			break;
	}

	return;
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

void huayi_set_sche_reboot_config(cJSON *response_root)
{
	int sche_reboot_mode = 0;
	int rectime=0,weeks = 0,hour = 0,minute = 0;
	
	if(NULL == response_root) 
	{
		return ;
	}


	if(!huayi_json_get_int(response_root,"sch_reboot",&sche_reboot_mode))
	{
		return ;
	}
	
	if(sche_reboot_mode < 0 ||sche_reboot_mode > 2)
	{
		return ;
	}	

	if(sche_reboot_mode == 1)
	{
		if(!huayi_json_get_int(response_root,"rectime",&rectime))
		{
			return ;
		}else if(rectime < 10)
		{
			CLOUD_CLIENT_LOG("rectime at least 10 minute\n");
			return ;
		}
	}else if(sche_reboot_mode == 2)
	{
		if(!huayi_json_get_int(response_root,"week",&weeks)||
			!huayi_json_get_int(response_root,"hour",&hour) ||
			!huayi_json_get_int(response_root,"minute",&minute))
		{
			return ;
		}
		if(hour < 0 || hour > 23||minute < 0||minute > 59||weeks < 0||weeks > 127)
		{
			CLOUD_CLIENT_LOG("time restart parameter error\n");
			return ;
		}
	}

	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","sche_reboot_mode",sche_reboot_mode);
	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","rectime",rectime);
	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","weeks",weeks);
	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","hour",hour);
	huayi_uci_set_int(PKG_SYSTEM,"sche_reboot","minute",minute);

	system("huayi_cmd set system rec_uptime 0");

	huayi_uci_commit(PKG_SYSTEM);

	system("huayi-reset sche_reboot");
	
	return ;
}

