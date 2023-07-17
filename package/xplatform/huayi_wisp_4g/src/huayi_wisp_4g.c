#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include <huayi_uci_fun.h>

#define PACKET_SIZE 64
#define MAX_PACKET_SIZE 1024

struct ping_packet {
    struct icmphdr header;
    char msg[PACKET_SIZE - sizeof(struct icmphdr)];
};


typedef enum{
	INTERNET_UNDEFINE = 0,
	INTERNET_4G,
	INTERNET_WIRED,
	
}INTERNET_MODE;
static int delaycount = 1;


void get_maxdelay(char *checksignalips, char *ifname)
{
	char cmd[TEMP_STR_LEN] = {0};
	char value[OPTION_STR_LEN] = { 0 };
	int i = 0, maxdelay = 0;
	int timevalue = 0;

	huayi_uci_get_int(PKG_SYSTEM,"mode","detect_time", &maxdelay);
	for(i = 0; i < 5; i++)
	{
		sprintf(cmd, "ping -c 1 -W 1 %s -I %s | grep \"time=\" | awk '{print $7}' > /tmp/pingtmp.txt", checksignalips, ifname);
		system(cmd);
		CON_LOG("cmd = %s\n", cmd);
		
		memset(cmd, 0, TEMP_STR_LEN);
		memset(value, 0, sizeof(value));
		strcpy(cmd, "cat /tmp/pingtmp.txt | awk -F \" \" '{print $1}'");
		get_cmd_result(cmd, value, sizeof(value));
		CON_LOG("ping time value  = %s\n", value);
		timevalue = atoi(value+strlen("time="));
		if(timevalue != 0)
		{
			CON_LOG("ping time value  = %d, maxdelay = %d\n", timevalue, maxdelay);
			if(timevalue > maxdelay)
			{
				delaycount += 1;
			}
		}
		else
		{
			CON_LOG("strlen(value) = %d , ping time value  = %s \n", strlen(value), value);
			delaycount += 1;
		}
	}

	return;
}


unsigned short icmp_checksum(unsigned short* buffer, int size) {
    unsigned long sum = 0;

    while (size > 1) {
        sum += *buffer++;
        size -= sizeof(unsigned short);
    }

    if (size == 1) {
        sum += *(unsigned char*)buffer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    return (unsigned short)(~sum);
}

int networkConnectTest(const char* ipAddress) {
    int i = 0;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = 0;
    if (inet_aton(ipAddress, &server.sin_addr) == 0) {
        WISP_NETWORK_LOG("inet_aton error");
        return -1;
    }

    int sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sd < 0) {
        WISP_NETWORK_LOG("socket error");
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        WISP_NETWORK_LOG("setsockopt error");
        close(sd);
        return -1;
    }

    struct ping_packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.header.type = ICMP_ECHO;
    packet.header.code = 0;
    packet.header.checksum = htons(~(ICMP_ECHO << 8));

    int seq = 0;
    int count = 4;  // 发送 ICMP 请求的次数
    int successCount = 0;

    for (i = 0; i < count; i++) {
        seq++;
        packet.header.un.echo.sequence = htons(seq);
        packet.header.checksum = 0;
        packet.header.checksum = htons(~icmp_checksum((unsigned short*)&packet, sizeof(packet)));

        if (sendto(sd, &packet, sizeof(packet), 0, (struct sockaddr*)&server, sizeof(server)) <= 0) {
            WISP_NETWORK_LOG("sendto error");
            close(sd);
            return -1;
        }

        struct sockaddr_in response;
        socklen_t responseLength = sizeof(response);
        unsigned char buffer[MAX_PACKET_SIZE];
        memset(buffer, 0, MAX_PACKET_SIZE);

        if (recvfrom(sd, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr*)&response, &responseLength) <= 0) {
            WISP_NETWORK_LOG("Request timed out.\n");
        } else {
            WISP_NETWORK_LOG("Received ICMP response from %s\n", inet_ntoa(response.sin_addr));
            successCount++;
        }
    }

    close(sd);

    if (successCount == count) {
        WISP_NETWORK_LOG("Network is reachable.\n");
        return 0;
    } else {
        WISP_NETWORK_LOG("Network is unreachable.\n");
        return -1;
    }
}

int networkCheck(const char* interface, const char* ipAddress) {
    if (networkConnectTest(ipAddress) == 0) {
        WISP_NETWORK_LOG("Network is reachable.\n");
        return 0;
    }

    char command[100];
    snprintf(command, sizeof(command), "ping -c 1 -W 1 %s -I %s > /dev/null 2>&1", ipAddress, interface);

    int status = system(command);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        WISP_NETWORK_LOG("Network is reachable.\n");
        return 0;
    } else {
        WISP_NETWORK_LOG("Network is unreachable.\n");
        return -1;
    }
}



int wisp4g_internet_do_ping(const char *host, char *ifname, char *del_gateway)
{
	char cmd[CMD_STR_LEN] = {0};
	char wan_gateway[IPV4_STR_LEN] = {0};
	char resultbuf[SHORT_STR_LEN] = {0};
	int i = 0;

	if ((NULL == host) || (strlen(host)<7))
		goto fail;

	
	if(strncmp(del_gateway,"0.0.0.0",strlen("0.0.0.0")))
	{
		sprintf(cmd,"route del default gw %s",del_gateway);
		system(cmd);
		//printf("cmd_str = %s\n",cmd_str);
		if(strcmp(ifname, "apcli0") == 0)
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "gateway", wan_gateway);
		}
		else
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", wan_gateway);
		}
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"route -n | grep %s | grep UG | grep %s |wc -l", ifname, wan_gateway);
		get_cmd_result(cmd, resultbuf, sizeof(resultbuf));

		if(atoi(resultbuf) == 0 && strncmp(wan_gateway,"0.0.0.0",strlen("0.0.0.0")))
		{
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"route add default gw %s dev %s",wan_gateway,ifname);
			system(cmd);
		}
	}else{
		goto fail;
	}
    if(networkCheck(ifname,host) == 0){
        return 1;
    }else{
        goto fail;
    }
fail:
	return 0;
}


int main(int argc,char *argv[])
{
	char wired_device[SHORT_STR_LEN] = {0};
	char cpe_device[SHORT_STR_LEN] = {0};
	char wan_ifname[SHORT_STR_LEN] = {0};
	char wan_ipv4[IPV4_STR_LEN] = {0};
	char wisp_ipv4[IPV4_STR_LEN] = {0};
	char wan4g_ipv4[IPV4_STR_LEN] = {0};
	char wan_netmask[IPV4_STR_LEN] = {0};
	char wan_gateway[IPV4_STR_LEN] = {0};
	char cmd_str[CMD_STR_LEN] = {0};
	char temp_dns[TEMP_STR_LEN] = {0};
	char dns[IPV4_STR_LEN] = {0};
	char section[SHORT_STR_LEN] = {0};
	char del_gateway[IPV4_STR_LEN] = {0};
	char resultbuf[SHORT_STR_LEN] = {0};
	char wan_type[OPTION_STR_LEN] = {0};
	char net_ipv4[IPV4_STR_LEN] = {0};
	int wired_status = 0;
	int cpe_status = 0;
	int wired_link = 0;
	int cpe_link = 0;
	int  i = 0;
	char detectip[OPTION_STR_LEN] = {0};
    int simReady = 0;
	delaycount = 1;
	while(1)
	{
		sleep(10);
		memset(wan_type,0,sizeof(wan_type));
		huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);

		if(strcmp(wan_type,"rep4g") != 0)
		{
			continue;
		}
		huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
		memset(wired_device,0,sizeof(wired_device));
		memset(cpe_device,0,sizeof(cpe_device));
		memset(wisp_ipv4, 0, sizeof(wisp_ipv4));
		memset(wan4g_ipv4, 0, sizeof(wan4g_ipv4));
		wired_link = 0;
		cpe_link = 0;
		wired_status = 0;
		cpe_status = 0;
		huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "device", wired_device);
		huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "device", cpe_device);
		huayi_uci_get_int(PKG_STATUS_BLEND, "wisp", "link", &wired_link);
		huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &cpe_link);
		huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "ipv4", wisp_ipv4);
		huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "ipv4", wan4g_ipv4);
		if(wired_link == 0 && cpe_link == 0)
			continue;

    	atcmd_get_cpin(&simReady);
    	if(simReady == 1)
    	{
			memset(cmd_str,0,sizeof(cmd_str));
			memset(resultbuf, 0, sizeof(resultbuf));
			strcpy(cmd_str,"ifconfig wwan0 | grep inet | awk {'print $1'}");
			get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));
			if(!strstr(resultbuf, "inet"))
			{
				system("quectel-CM &");
                if(cpe_link == 0){
                    system("huayi_cmd set blend cpe_link 1");
                }
			}
    	}
        else{
            if(cpe_link == 1){
                system("huayi_cmd set blend cpe_link 0");
                system("ifup wan");
            }
        }
		
		if(strncmp(wisp_ipv4, "0.0.0.0", strlen("0.0.0.0")) == 0
			&& strncmp(wan4g_ipv4, "0.0.0.0", strlen("0.0.0.0")) == 0)
			continue;
		memset(wisp_ipv4, 0, sizeof(wisp_ipv4));
		memset(wan4g_ipv4, 0, sizeof(wan4g_ipv4));
		if(wired_link == 1 && cpe_link == 1)//同时在线时，使用wisp优先策略
		{
			memset(del_gateway, 0, sizeof(del_gateway));
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", del_gateway);
			wired_status = wisp4g_internet_do_ping(detectip, wired_device, del_gateway);
			if(wired_status == 0)
			{
                system(". /lib/netifd/hy_del_wisp.sh del");
				memset(del_gateway, 0, sizeof(del_gateway));
				huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "gateway", del_gateway);
				cpe_status = wisp4g_internet_do_ping(detectip, cpe_device, del_gateway);
				if(cpe_status == 0)//使用wisp上网方式
				{
                    system("ifup wan");
					sprintf(section,"wisp");
                    
				}else//使用4G上网方式
				{
					sprintf(section,"cpe");
				}
			}
			else
			{
				get_maxdelay(detectip, wired_device);
				if(delaycount >= 5)
				{
					//使用4G上网方式
					sprintf(section,"cpe");
				}
				else  //使用wisp上网方式
				{
					sprintf(section,"wisp");
				}
				delaycount = 1;
			}
		}
		else if(cpe_link == 1)//使用4G上网方式
		{	
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "ipv4", wan4g_ipv4);
			if(strncmp(wan4g_ipv4, "0.0.0.0", strlen("0.0.0.0")) == 0)
				continue;
			sprintf(section,"cpe");
		}
		else//使用wisp上网方式
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "ipv4", wisp_ipv4);
			if(strncmp(wisp_ipv4, "0.0.0.0", strlen("0.0.0.0")) == 0)
				continue;
			sprintf(section,"wisp");
		}

		memset(cmd_str,0,sizeof(cmd_str));
		memset(del_gateway,0,sizeof(del_gateway));
		if(!strncmp(section,"wisp",strlen("wisp")))//使用wisp上网方式，删除4G默认路由
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", del_gateway);
		}
		else//4G上网方式，删除有线默认路由
		{
			huayi_uci_get_str(PKG_STATUS_BLEND, "wisp", "gateway", del_gateway);
		}
		//printf("section = %s\n",section);
		memset(wan_ifname,0,sizeof(wan_ifname));
		memset(wan_ipv4,0,sizeof(wan_ipv4));
		memset(wan_netmask,0,sizeof(wan_netmask));
		memset(wan_gateway,0,sizeof(wan_gateway));
		huayi_uci_get_str(PKG_STATUS_BLEND, section, "device", wan_ifname);
		huayi_uci_get_str(PKG_STATUS_BLEND, section, "ipv4", wan_ipv4);
		huayi_uci_get_str(PKG_STATUS_BLEND, section, "netmask", wan_netmask);
		huayi_uci_get_str(PKG_STATUS_BLEND, section, "gateway", wan_gateway);
		huayi_uci_get_str(PKG_STATUS_BLEND, section, "dns_server",temp_dns);
		//printf("del_gateway = %s\n",del_gateway);
		if(strncmp(del_gateway,"0.0.0.0",strlen("0.0.0.0")))
		{
			sprintf(cmd_str,"route del default gw %s",del_gateway);
			system(cmd_str);
			//printf("cmd_str = %s\n",cmd_str);			
		}
		else
		{
			/*判断当前使用的默认路由是否为0.0.0.0，如果为0.0.0.0，则删除*/
			memset(cmd_str,0,sizeof(cmd_str));
			memset(resultbuf, 0, sizeof(resultbuf));
			strcpy(cmd_str,"route | grep \"default\" | awk {'print $2'}");
			get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));
			if(strstr(resultbuf, "0.0.0.0"))
			{
				system("route del default");
				sleep(1);
				system("route del default");
			}
		}

		memset(cmd_str,0,sizeof(cmd_str));
		sprintf(cmd_str,"route -n | grep %s | grep UG | grep %s |wc -l",wan_ifname,wan_gateway);
		get_cmd_result(cmd_str, resultbuf, sizeof(resultbuf));

		if(atoi(resultbuf) == 0 && strncmp(wan_gateway,"0.0.0.0",strlen("0.0.0.0")))
		{
			memset(cmd_str,0,sizeof(cmd_str));
			sprintf(cmd_str,"route add default gw %s dev %s",wan_gateway,wan_ifname);
			system(cmd_str);
			//printf("add route cmd_str = %s\n",cmd_str);
		}

		huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", net_ipv4);
		//printf("net_ipv4 = %s, wan_ipv4 = %s\n",net_ipv4, wan_ipv4);
		//if(strcmp(wan_ipv4,net_ipv4))  //更新proc下的网络状态信息
		{	
			memset(cmd_str,0,sizeof(cmd_str));
			snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
			system(cmd_str);

			memset(cmd_str,0,sizeof(cmd_str));
			snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_ipv4 %s",wan_ipv4);
			system(cmd_str);

			memset(cmd_str,0,sizeof(cmd_str));
			snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_netmask %s",wan_netmask);
			system(cmd_str);

			memset(cmd_str,0,sizeof(cmd_str));
			snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_gateway %s",wan_gateway);
			system(cmd_str);

			memset(dns,0,IPV4_STR_LEN);
			i = 0;
			//printf("temp_dns = %s\n",temp_dns);
			while(huayi_get_nth_value(i++, temp_dns, " ", dns, sizeof(dns)))
			{
				//printf("dns = %s\n",dns);
				memset(cmd_str,0,sizeof(cmd_str));
				snprintf(cmd_str,CMD_STR_LEN,"huayi_cmd set network wan_dns_server %s",dns);
				//printf("cmd_str = %s\n",cmd_str);
				system(cmd_str);
			}

			//if(now_int_mode == INTERNET_4G)
			//{
				//system("ifup wan4g");
			//	system("rm -fr /tmp/resolv.conf.auto");
			//	system("echo # Interface wan4g >> /tmp/resolv.conf.auto");
			//	memset(cmd_str,0,sizeof(cmd_str));
			//	snprintf(cmd_str, CMD_STR_LEN, "echo nameserver %s >> /tmp/resolv.conf.auto", wan_gateway);
			//	system(cmd_str);
			//	system("echo search lan >> /tmp/resolv.conf.auto");
			//}else if(now_int_mode == INTERNET_WIRED)
			//{
				//system("ifup wan");
			//}
		}
	}

	return 0;

}
