#include <stdio.h>
#include <unistd.h>
#include <huayi_uci_fun.h>		
#include <string.h>






/*网络不通
	国内的4G模块多加一个检测：1、查询ICCID能不能读到，不能读到记录日志。
						2、如果能读到ICCID，查看dns是否正确(/tmp/resolv.auto，里面的dns有没有0.0.0.0或者8.8.8.8)，
						如果有，则重启一下4G功能(huayi-reset lte4g parameter 1)
*/
int checkDNS() {
    FILE *file;
    char line[256];
    int found = 0;

    file = fopen("tmp/resolv.conf.auto", "r");
    if (file == NULL) {
        return -1;
    }
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "0.0.0.0") || strstr(line, "8.8.8.8")) {
            found = 1;
            break;
        }
    }
    fclose(file);
    return found;
}	

int DetectConfigurationOfSimAndDns()
{
	char lte_value[LONG_BUFF_LEN] = { 0 };
	char cmd[CMD_STR_LEN] = {0};
	int ret=0, AsciiNum[22], j;
	memset(lte_value, 0, OPTION_STR_LEN);
	atcmd_get_iccid(lte_value);
	for(j=0;j<strlen(lte_value) ,AsciiNum[j]=lte_value[j];j++){
    	if(strlen(lte_value) < 5 ||!((AsciiNum[j]>=48 && AsciiNum[j]<=58) || (AsciiNum[j]>=65 && AsciiNum[j]<=90) || (AsciiNum[j]>=97 && AsciiNum[j]<=122))){
    		DETECTION_NETWORK_LOG("SIM card cannot read");
    		return 0;
    	}
	}
    int result = checkDNS();
    if (result == -1) {
        DETECTION_NETWORK_LOG("Failed to check DNS\n");
    } else if (result == 1) {
        DETECTION_NETWORK_LOG("huayi-reset lte4g parameter 1");
        system("huayi-reset lte4g parameter 1");
        return 1;
    } else {
        DETECTION_NETWORK_LOG("The specified IP address was not found\n");
    }
	return 0;
}


int parse_pingmsg(char *ping , char *file)
{
	char cmd[CMD_STR_LEN] = {0};
	char iccid[OPTION_STR_LEN] = { 0 };
	char value[OPTION_STR_LEN] = {0};
	int ret=0;
	sprintf(cmd, "%s > %s", ping,file);
	system(cmd);
	sleep(1);
	
	memset(cmd, 0 ,sizeof(cmd));
	sprintf(cmd, "cat %s | awk -F \" \" '{print $1}'", file);
	get_cmd_result(cmd, value, sizeof(value));
	if(strlen(value) >= 1)
	{
		DETECTION_NETWORK_LOG("value =  %s\n", value);
		unlink(file);
		return 1;
	}
	else{
        DETECTION_NETWORK_LOG("value =  %s. network is not connect.\n",value);
		ret=DetectConfigurationOfSimAndDns();
	}
	return ret;
}

int simcard_internet_check(int count,char *host[], char *ifname)
{	
	char ping_addr[CMD_STR_LEN] = {0};
	int i=1;

	if ((NULL == host) || (strlen(host)<7))
		return 0;
	
	while(i<count)
	{
		if (ifname == NULL)
			sprintf(ping_addr,"ping -c 1 -W 1 %s | grep \"time=\" | awk -F \"=\" '{print $NF}' ", host[i]);
		else
			sprintf(ping_addr,"ping -c 1 -W 1 %s -I %s | grep \"time=\" | awk -F \"=\" '{print $NF}' ", host[i],ifname);
		if(parse_pingmsg(ping_addr, PING_FILE)==1){
            unlink(PING_FILE);
			return 1;
        }
		i++;
	}
	unlink(PING_FILE);
	return 0;


}

int main(int argc,char *argv[])
{
	int i=0;
	if(argc>1)
		simcard_internet_check(argc,argv,NULL);
	return 0;

}
