/*==============================================================================
//
// Project:
//		userapp-ddns module
//
//------------------------------------------------------------------------------
// Description:
//
//           main of the ddns part
==============================================================================*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include "ddns.h"
#include <syslog.h>
 
#include <huayi-universal/huayi_uci_fun.h>

int SYS_wan_ip;

extern int ping_test(char *srv,int count,char *ipaddr);

static void my_handleSIG (int sig)
{
	printf("yddns is eixt:%d\n",sig);
	char cmd[128] = {0};
	
	snprintf(cmd,128,"huayi_cmd set system ddns_link 2");
	system(cmd);
	snprintf(cmd,128,"huayi_cmd set system ddns_bind_ip '' ");
	system(cmd);
	
	exit(0);
}

static int updateDdns(struct userInfo_cxy * info)
{
	struct in_addr t;
	int ret = 1,netx = 0;
    char provider[32]={0},domain[32]={0},command[128] = {0};
	char wan_ip[32] = {0},ipaddr[16]={0};
	printf("updateDdns start....\n");
	
	ret = info->status = info->service->update_entry(info);

	printf("updateDdns end....%d\n",ret);
	t.s_addr = info->ip;
	strcpy(wan_ip,inet_ntoa(t));

	if (!ret){

		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",wan_ip);
		
		snprintf(command,128,"huayi_cmd set system ddns_state 1");
		system(command);
		snprintf(command,128,"huayi_cmd set system ddns_ip %s ",wan_ip);
		system(command);
		
		//netx = SUCCESS_INTERVAL;
		netx = 30*60;
	}else{
		
		snprintf(command,128,"huayi_cmd set system ddns_state 2");
		system(command);
		snprintf(command,128,"huayi_cmd set system ddns_ip '' ");
		system(command);
		
		//netx = FAIL_INTERVAL
		netx = 5*60;
	}
	return netx;
}


int main(int argc,char *argv[]) 
{
	struct userInfo_cxy info;

	char wan_ip[16]={0},wan_ip_old[16]={0},ipaddr[16]={0},provider_type[32] = {0};
    char provider[64]={0},domain[64]={0},account[64]={0},password[64]={0};
	char macaddr[32] = {0},command[128] = {0};	

	int wan_link = 0;
	int next = 0,count = 0,failCount = 0;

	(void) signal(SIGQUIT,my_handleSIG);

	snprintf(command,128,"huayi_cmd set system ddns_state 2");
	system(command);
	snprintf(command,128,"huayi_cmd set system ddns_ip '' ");
	system(command);
	
	
	//dyndns.org
	//yddns -s dyndns Domain,UserName,Password,wanIp
	//3322.org
	//yddns -s qdns Domain,UserName,Password,wanIp
	//tzo.org
	//yddns -s tzo Domain,UserName,Password,wanIp
	/*just suppport wan*/
	huayi_uci_get_str(PKG_DDNS,"wan","provider",provider);
	huayi_uci_get_str(PKG_DDNS,"wan","domain",domain);
	huayi_uci_get_str(PKG_DDNS,"wan","account",account);
	huayi_uci_get_str(PKG_DDNS,"wan","password",password);

	if(!strcmp(provider,"dyndns.org")){
		snprintf(provider_type,32,"dyndns");
	}
	else if(!strcmp(provider, "3322.org")){
		snprintf(provider_type,32,"qdns");
	}
	else if(!strcmp(provider, "tzo.org")){
		snprintf(provider_type,32,"tzo");
	}
	else{
		printf("[%s:%d]unSupport yddns provider: %s\n",__FUNCTION__,__LINE__,provider);
		return -1;
	}

	info.service = (struct service_cxy *)find_service(provider_type);

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",wan_ip);
	printf("wan_ip:%s\n",wan_ip);
	
	strcpy(info.host, domain);
	strcpy(info.usrname, account);
	strcpy(info.usrpwd, password);

	info.ip=inet_addr(wan_ip);
	SYS_wan_ip=inet_addr(wan_ip);
	
	strcpy(info.mx, "0");
	strcpy(info.backmx,"0");
	info.updated_time=0;
	info.trytime=0;

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","mac_address",macaddr);
	strcpy(info.devmac, macaddr);

	if(info.ip)
		info.status = UPDATERES_OK;
	else
		info.status = UPDATERES_ERROR;
	
	while(1)
	{
		printf("count:%d\n",count);
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
			info.ip=inet_addr(wan_ip);
			SYS_wan_ip=inet_addr(wan_ip);
			next = updateDdns(&info);
			strcpy(wan_ip_old,wan_ip);
			count = 0;
			printf("next:%d\n",next);
		}else if(count>=next){
			next = updateDdns(&info);
			count = 0;
			printf("next:%d\n",next);
			printf("time out\n");
		}
		count++;
		sleep(1);
	}

	return 0;
}

