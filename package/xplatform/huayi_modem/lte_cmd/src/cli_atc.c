#include "huayi_atcmd.h"

void show_usage(void)
{
    printf("cli_atc ATCMD\n");
	printf("example:\n");
	printf("  cli_atc [AT|AT+CSQ|AT$CPIN?]\n");
	printf("\n\n");
}

int main(int argc, char **argv)
{
	int ret = 0;
	char cmd[128]={0};
 
	if(argc<2)
	{
		show_usage();
		return -1;
	}

	sprintf(cmd, "%s", argv[1]);
	ret = Atcmd(cmd, strlen(cmd), ATCMD_GET);
	if ( ret == LTE_FALSE )
		printf("AT CMD [%s] ERROR !!! \n", cmd);
	else
		printf("%s", ATret);

    return 0;
}

