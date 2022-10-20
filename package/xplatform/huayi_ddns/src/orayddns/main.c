#include "phupdate.h"
#include "log.h"
#include <signal.h>     /* for singal handle */
#ifndef WIN32
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netdb.h>
#include <unistd.h>     /* for close() */
#endif
#include <huayi-universal/huayi_uci_fun.h>

#define ORAY_CLIENTINFO	0x20063150
#define ORAY_CHALLENGEKEY 0x10101010

char clientIp[16]={0};



PHGlobal global;
static void my_handleSIG (int sig)
{
	char cmd[128] = {0};
	if (sig == SIGINT || sig == SIGQUIT)
	{
#ifndef WIN32
		remove("/var/run/phddns.pid");
#endif
		LOG ("signal = SIGINT\n");
	
		snprintf(cmd,128,"huayi_cmd set system ddns_state 2");
		system(cmd);
		snprintf(cmd,128,"huayi_cmd set system ddns_ip 0.0.0.0 ");
		system(cmd);
	
		phddns_stop(&global);
		exit(0);
	}
	if (sig == SIGTERM)
	{
#ifndef WIN32
		remove("/var/run/phddns.pid");
#endif
		LOG ("signal = SIGTERM\n");
		phddns_stop(&global);
	}
	signal (sig, my_handleSIG);
}

//状态更新回调
static void myOnStatusChanged(int status, long data)
{
	char cmd[128] = {0};
	LOG("[%s:%d] %s",__FUNCTION__,__LINE__,convert_status_code(status));
	if (status == okKeepAliveRecved)
	{
		struct in_addr t;
		t.s_addr = data;
		char tmpIp[16];
		strcpy(tmpIp,inet_ntoa(t));
		if(strcmp(clientIp,tmpIp)){
			strcpy(clientIp,tmpIp);
			snprintf(cmd,128,"huayi_cmd set system ddns_ip %s",clientIp);
			system(cmd);
		}
		LOG(", IP: %s", clientIp);
	}
	if (status == okDomainsRegistered)
	{
		LOG(", UserType: %d", data);
	}
	LOG("\n");
}

//域名注册回调
static void myOnDomainRegistered(char *domain)
{
	char cmd[128] = {0};
	LOG("[%s:%d] %s\n", __FUNCTION__,__LINE__,domain);
	
	snprintf(cmd,128,"huayi_cmd set system ddns_state 1");
	system(cmd);

	huayi_uci_set_str(PKG_DDNS,"wan","domain",domain);
	
	huayi_uci_commit(PKG_DDNS);
}

//用户信息XML数据回调
static void myOnUserInfo(char *userInfo, int len)
{
	LOG("[%s:%d] %s\n", __FUNCTION__,__LINE__,userInfo);
	char *ptrStart,*ptrEnd;
	char tmp[18],tmpIp[16],cmd[128];
	int ipLen=0;
	ptrStart=strstr(userInfo,"ClientIP");
	if(ptrStart){
		ptrEnd=strstr(ptrStart+9,"ClientIP");
		ipLen=(ptrEnd-2)-(ptrStart+9);
		strncpy(tmp,ptrStart+9,ipLen);

		struct in_addr t;
		t.s_addr = atoi(tmp);

		strcpy(tmpIp,inet_ntoa(t));
		if(strcmp(clientIp,tmpIp)){
			strcpy(clientIp,tmpIp);
			//snprintf(cmd,128,"ibms_cmd set status ddns_bind_ip %s",clientIp);
			
			snprintf(cmd,128,"huayi_cmd set system ddns_ip %s",clientIp);
			
			system(cmd);
		}
	}
	LOG("[%s:%d] ClientIP:%s\n", __FUNCTION__,__LINE__,clientIp);
}

//域名信息XML数据回调
static void myOnAccountDomainInfo(char *domainInfo, int len)
{
	LOG("myOnAccountDomainInfo %s\n", domainInfo);
}
static void initUserCfg()
{
	char cmd[128] = {0};
	init_global(&global);

	global.cbOnStatusChanged = myOnStatusChanged;
	global.cbOnDomainRegistered = myOnDomainRegistered;
	global.cbOnUserInfo = myOnUserInfo;
	global.cbOnAccountDomainInfo = myOnAccountDomainInfo;

	//set_default_callback(&global);
	
	global.clientinfo = ORAY_CLIENTINFO;		//这里填写刚才第二步算出的值
	global.challengekey = ORAY_CHALLENGEKEY;	//这里填写嵌入式认证码

	huayi_uci_get_str(PKG_DDNS,"wan","account",global.szUserID);
	huayi_uci_get_str(PKG_DDNS,"wan","password",global.szUserPWD);

	snprintf(cmd,128,"huayi_cmd set system ddns_state 2");
	system(cmd);
	snprintf(cmd,128,"huayi_cmd set system ddns_ip '' ");
	system(cmd);

}


int main(int argc, char *argv[])
{

	int next = 0,count = 0;
	char wan_ip[16] = {0},wan_ip_old[16] = {0};
	char command[128] = {0};
	
	void (*ohandler) (int);
#ifdef WIN32
	WORD VersionRequested;		// passed to WSAStartup
	WSADATA  WsaData;			// receives data from WSAStartup
	int error;

	VersionRequested = MAKEWORD(2, 0);

	//start Winsock 2
	error = WSAStartup(VersionRequested, &WsaData); 
	log_open("c:\\phclientlog.log", 1);	//empty file will cause we printf to stdout
#else
//	log_open("/var/log/phddns.log", 1);	//empty file will cause we printf to stdout
//	create_pidfile();
#endif

	printf("Usage: %s [-w wan idx] [-h]\n",argv[0]);
	

	ohandler = signal (SIGINT, my_handleSIG);
	if (ohandler != SIG_DFL) {
		LOG("previous signal handler for SIGINT is not a default handler\n");
		signal (SIGINT, ohandler);
	}
	(void) signal(SIGQUIT,my_handleSIG);

	initUserCfg();

	for (;;)
	{
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",wan_ip);
		if(strlen(wan_ip) == 0 || !strcmp(wan_ip,"0.0.0.0"))
		{
			snprintf(command,128,"huayi_cmd set system ddns_state 2");
			system(command);
			count = 0;
			sleep(10);
			continue;
		}
		
		if(strcmp(wan_ip,wan_ip_old)){
			global.bNeed_connect = TRUE;
			next = phddns_step(&global);
			count = 0;
			strcpy(wan_ip_old,wan_ip);
			printf("wanIP Change:%s\n",wan_ip);
		}else if(count>=next){
			next = phddns_step(&global);
			count = 0;
			printf("Time Out!\n");
		}
		count++;
		sleep(1);
	}
	phddns_stop(&global);
	return 0;
}
