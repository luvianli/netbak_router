#include <stdio.h>
#include <unistd.h>
#include <huayi_uci_fun.h>
#include <netdb.h>

typedef enum{
	INTERNET_UNDEFINE = 0,
	INTERNET_4G,
	INTERNET_WIRED,
	
}INTERNET_MODE;

#define serverip "device.10086group.com"
#define check_ip "223.5.5.5;223.5.5.5;114.114.114.114"

int check_dns(const char *hostname) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4或IPv6
    hints.ai_socktype = SOCK_STREAM;

    // 尝试解析主机名
    int status = getaddrinfo(hostname, NULL, &hints, &res);
    if (status != 0) {
        NETCHECK_LOG("DNS解析失败: %s\n", gai_strerror(status));
        return 0; // DNS不正常
    }

    freeaddrinfo(res); // 释放内存
    return 1; // DNS正常
}

int main(int argc,char *argv[])
{
	char wired_device[SHORT_STR_LEN] = {0};
	char cpe_device[SHORT_STR_LEN] = {0};
	char wan_ifname[SHORT_STR_LEN] = {0};
	char wan_ipv4[IPV4_STR_LEN] = {0};
	char wan_netmask[IPV4_STR_LEN] = {0};
	char wan_gateway[IPV4_STR_LEN] = {0};
	char cmd_str[CMD_STR_LEN] = {0};
	char temp_dns[TEMP_STR_LEN] = {0};
	char dns[IPV4_STR_LEN] = {0};
	char section[SHORT_STR_LEN] = {0};
	char del_gateway[IPV4_STR_LEN] = {0};
	char resultbuf[SHORT_STR_LEN] = {0};

	char iccid[OPTION_STR_LEN] = {0};
	char net_ipv4[IPV4_STR_LEN] = {0};

	int wired_status = 0;
	int cpe_status = 0;
	int wired_link = 0;
	int cpe_link = 0;
	int old_int_mode = INTERNET_UNDEFINE;
	int now_int_mode = INTERNET_UNDEFINE;
	int  i = 0,j = 0,k = 0;
	char detectip[OPTION_STR_LEN] = {0};

	while(1)
	{
		sleep(60);
		memset(iccid, 0, OPTION_STR_LEN);
		wired_status = atcmd_get_iccid(iccid);
		if(wired_status ==0 ||strlen(iccid) < 5)
		{
			if(i < 6)
			{
				i++;
				sleep(2);
				continue ;
			}
			else
			{
				NETCHECK_LOG("get sim card error,reboot");
				system("reboot");
			}	
		}

		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","gateway",wan_gateway);
		if(!internet_do_ping_detect(wan_gateway, NULL))
		{
			NETCHECK_LOG("ping gateway error,reboot");
			system("reboot");
			continue ;
		}

		i = 0;
checknet:
		if(!networkCheck(check_ip))
		{	
			if(i < 3)
			{
				i++;
				sleep(10);
				goto checknet;
			}
			if(k == 0)
			{
				system("huayi_AT_cli at+CFUN=0");
				sleep(2);
				system("huayi_AT_cli at+CFUN=1");
				sleep(2);
				system("/etc/init.d/huayi_dial4gcontrol restart");
				NETCHECK_LOG("network error,reset modem");
				k == 1;
				sleep(10);
				goto checknet;
			}
			NETCHECK_LOG("network error,reboot");
			system("reboot");
			continue ;
		}

		k = 0;
		
		huayi_uci_get_int(PKG_SYSTEM,"server","success", &cpe_status);
		if(cpe_status == 0)
		{		
			if(internet_do_ping_detect(serverip, NULL))
			{
				NETCHECK_LOG("ping server success,restart client");
				system("/etc/init.d/hy_cloud_client restart");
				continue ;
			}

			huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "network", detectip);
			if(strcmp(detectip,"CHINA MOBILE CMCC") == 0)
			{	
				strcpy(temp_dns,"10086.cn");
				if (!check_dns(temp_dns)) {
					NETCHECK_LOG("dns error,reboot");
					system("reboot");
				} else {
					NETCHECK_LOG("dns success");
				}
			}else if(strcmp(detectip,"CHN-UNICOM") == 0)
			{	
				strcpy(temp_dns,"chinaunicom.com.cn");
				if (!check_dns(temp_dns)) {
					NETCHECK_LOG("dns error,reboot");
					system("reboot");
				} else {
					NETCHECK_LOG("dns success");
				}
			}else if(strcmp(detectip,"CHN-CT") == 0)
			{	
				strcpy(temp_dns,"chinatelecom.com.cn");
				if (!check_dns(temp_dns)) {
					NETCHECK_LOG("dns error,reboot");
					system("reboot");
				} else {
					NETCHECK_LOG("dns success");
				}
			}

			if(j < 1)
			{
				NETCHECK_LOG("ping server error,reset network");	
				system("huayi-reset network");
				goto checknet;
			}
			else
			{				
				NETCHECK_LOG("ping server error,reboot");
				system("reboot");
			}
			j++;
		}
	}

	return 0;

}
