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

#define DEFAULT_DELIMIT				';'
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
void usage(void)
{
	printf("Usage: huayi_msg <op> [paramter]\n");
	printf("huayi_msg simaction parameter_count;operator\r\n");
	return;
}


int hy_simaction( char **value)
{
	char tmpbuf[SHORT_STR_LEN]= { 0 };
	unsigned int i = 0, j = 0;
	char   *ptr = NULL;
	
	huayi_get_nth_value(0, *value, DEFAULT_DELIMIT, tmpbuf, sizeof(tmpbuf));
	
	ptr = strtok(*value, ";");
	if(ptr == NULL)
		return -1;

	while (ptr != NULL) 
    {
        ptr = strtok(NULL, ";");
		if(ptr != NULL) {
			if(strstr(ptr, "CMNET") != NULL || 
				strstr(ptr, "CTNET") != NULL ||
				strstr(ptr, "UNINET") != NULL)
			{
				system("huayi-reset lte4g parameter 0");
			}
		}
	}

	return 0;
}
#endif


int main(int argc,char **argv)
{
	char op[OPTION_STR_LEN];
	
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	if (argc < 3)
	{
		goto invald_cmd;
	}
	
	strcpy(op, argv[1]);
	
	if (!strcmp(op,"simaction"))
	{
		if (hy_simaction(argv+2) < 0)
		{
			goto cmd_fail;
		}
	}

	printf("huayi_msg excution success\n");
#endif

	return 0;

invald_cmd:
    usage();
	return 0;
	
cmd_fail:
	usage();
	printf("huayi_msg excution failed\n");
	return 0;
}

