#include <stdio.h>
#include <unistd.h>
#include <huayi_uci_fun.h>		






/*网络不通
	国内的4G模块多加一个检测：1、查询ICCID能不能读到，不能读到记录日志。
						2、如果能读到ICCID，查看dns是否正确(/tmp/resolv.auto，里面的dns有没有0.0.0.0或者8.8.8.8)，如果有，则重启一下4G功能(huayi-reset lte4g parameter 1)
*/
	

int DetectConfigurationOfSimAndDns()
{
	char lte_value[OPTION_STR_LEN] = { 0 },state[OPTION_STR_LEN] = { 0 },route_state[OPTION_STR_LEN] = { 0 },gateway[OPTION_STR_LEN] = { 0 };
	char cmd[CMD_STR_LEN] = {0};
	int ret=0, AsciiNum[22], j,i_ready = 0;
	memset(lte_value, 0, OPTION_STR_LEN);
	atcmd_get_iccid(lte_value);
	for(j=0;j<strlen(lte_value) ,AsciiNum[j]=lte_value[j];j++)		
	if(strlen(lte_value) < 5 ||!((AsciiNum[j]>=48 && AsciiNum[j]<=58) || (AsciiNum[j]>=65 && AsciiNum[j]<=90) || (AsciiNum[j]>=97 && AsciiNum[j]<=122)))
	{
		DETECTION_NETWORK_LOG("SIM card cannot read");
		return 0;
	}
    memset(cmd, 0, sizeof(cmd));
    memset(lte_value, 0 ,sizeof(lte_value));

    atcmd_get_cpin(&i_ready);
    sprintf(cmd, "ifconfig eth1 | grep -w \"inet\" | awk '{print $2}' | wc -l", state);
    get_cmd_result(cmd, lte_value, sizeof(lte_value));
    memset(cmd, 0 ,sizeof(cmd));
    sprintf(cmd, "route -n | grep eth1 | grep UG | grep %s | wc -l",gateway);
    get_cmd_result(cmd, route_state, sizeof(route_state));
    DETECTION_NETWORK_LOG("cpin:%d,ifconfig eth1 ient state:%d,route internet:%d",i_ready,atoi(lte_value),atoi(route_state));
    if(i_ready == 1 && (atoi(lte_value) == 0 || atoi(route_state) == 1)){
        
        DETECTION_NETWORK_LOG("huayi_AT_cli AT+QPOWD=1");
        system("huayi_AT_cli AT+QPOWD=1");
    }
	memset(cmd, 0 ,sizeof(cmd));
	memset(lte_value, 0 ,sizeof(lte_value));
	strcpy(cmd, "grep '0.0.0.0\|8.8.8.8' /tmp/resolv.conf.auto  | sort");
	get_cmd_result(cmd, lte_value, sizeof(lte_value));
	if(strlen(lte_value) < 1)
		return 0;
	system("huayi-reset lte4g parameter 1");
	DETECTION_NETWORK_LOG("huayi-reset lte4g parameter 1");
	return 1;
	


}


int simcard_internet_check(int count,char *host[], char *ifname)
{	
	char ping_addr[CMD_STR_LEN] = {0};
	int i=1,max_delay = 0;

	if ((NULL == host) || (strlen(host)<7))
		return 0;
	while(i<count)
	{
        max_delay = pingCheck(host[i]);
		if (max_delay != -1){
            DETECTION_NETWORK_LOG("max_delay =  %d. network is connect.\n",max_delay);
			return 1;					
		}
        sleep(5);
		i++;
	}
    DetectConfigurationOfSimAndDns();
	DETECTION_NETWORK_LOG("network is not error");
    
	return 0;


}

int main(int argc,char *argv[])
{
	int i=0;
	if(argc>1)
		simcard_internet_check(argc,argv,NULL);
	return 0;

}
