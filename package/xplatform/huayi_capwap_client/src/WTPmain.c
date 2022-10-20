/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/

#include "WTPmain.h"
#include "WTPsocket.h"
#include "WTPepoll.h"
#include "WTPtimer.h"
#include "WTPhandlereload.h"
#include <signal.h>



WTPBool gRunAgent;
pthread_mutex_t reload_mutex;
pthread_mutex_t config_mutex;

void huayi_capwap_client_delay_bootup(void)
{
	int a = 0,b = 0,c = 0;
	int delay_time = 0;
	
	srand((unsigned)time(NULL));

	a = rand() % 10;

	b = rand() % 10;

	c = rand() % 10;

	delay_time = a*1000 + b*100 + c*10;
	
	usleep(delay_time * 1000);

	return;
}

void WTPSetfirstseqNum(struct VcapwapAgent *Agent)
{
	srand((unsigned)time(NULL)); 
	Agent->seqNum = rand() % 255;
	return;

}

WTPBool wtp_main_init(struct VcapwapAgent *Agent)
{
	memset(Agent,0,sizeof(struct VcapwapAgent));

	gRunAgent = WTP_TRUE;

	Agent->currentState = CW_SULKING;
	
	WTPSetfirstseqNum(Agent);
	
	return WTP_TRUE;
}

void WTPMainGracefulExit(int sig)
{
	gRunAgent = WTP_FALSE;

	(void) signal(SIGTERM, SIG_IGN);

	return;
}


WTPBool wtp_load_config(struct VcapwapAgent *Agent)
{	
	char mac_str[CW_MAC_STR_LEN] = {0};
	int radiotype=0,echoRequestTimer=0,discoveryTimer=0,version=0;
	int trustid = 0;


	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","mac_address",mac_str);
	
	snprintf(Agent->SerialNumber,TEMP_BUF_SIZE,"N/A");

	huayi_uci_get_str(PKG_THINAP,"global","wtp_name",Agent->wtp_name);
	huayi_uci_get_int(PKG_THINAP,"global","radiotype",&radiotype);
	huayi_uci_get_int(PKG_THINAP,"global","heartbeat_interval",&echoRequestTimer);
	huayi_uci_get_int(PKG_THINAP,"global","discovery_interval",&discoveryTimer);
	huayi_uci_get_int(PKG_ABOUT,"about_software","fw_version",&version);
	huayi_uci_get_str(PKG_ABOUT,"about_system","product_model",Agent->wtp_model);
	huayi_uci_get_int(PKG_THINAP,"global","trustid",&trustid);	

	if(trustid == 0)
	{
		printf("wtp trustid invaild,stop!\n");
		return WTP_FALSE;
	}

	Agent->trustid = trustid;

	snprintf(Agent->wtp_location,CW_OPTION_STR_LEN,"N/A");

	MAC17_TO_MAC6((unsigned char *)mac_str,Agent->baseMac);

	Agent->softVersion = version;

	Agent->bootVersion = 106;

	Agent->radiotype = radiotype; /*1 2.4G 2 5.8G 3 2.4G+5.8G*/
	
	Agent->manage_port = CW_MASTER_UDP_PORT;

	Agent->Agenttimer.echoRequestTimer = echoRequestTimer;
	
	Agent->Agenttimer.discoveryTimer = discoveryTimer;
	
	return WTP_TRUE;
}

void wtp_ip_address_check(struct VcapwapAgent *Agent)
{

check_ac:
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","gateway",Agent->ac_ip);
	if(strlen(Agent->ac_ip)==0|| !strcmp(Agent->ac_ip,"0.0.0.0"))
	{
		sleep(5);
		goto check_ac;
	}
	
check_ap:
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",Agent->wtp_ip);
	if(strlen(Agent->wtp_ip)==0|| !strcmp(Agent->wtp_ip,"0.0.0.0"))
	{
		sleep(5);
		goto check_ap;
	}

	return;
}

int main(int argc, char *argv[])
{
	pthread_t wtpreload;
	
	struct VcapwapAgent Agent;

	pthread_mutex_init(&reload_mutex,NULL);
	pthread_mutex_init(&config_mutex,NULL);

	huayi_capwap_client_delay_bootup();

	if(!wtp_main_init(&Agent))
	{
		printf("WTP Init failed,service stop!!!\n");
		return WTP_FALSE;
	}
	
	(void) signal(SIGTERM,WTPMainGracefulExit);
	
	if(!wtp_load_config(&Agent))
	{
		printf("WTP load config failed,service stop!!!\n");
		return WTP_FALSE;
	}

	wtp_ip_address_check(&Agent);
	
	if(!wtp_eloop_init())
	{
		printf("WTP eloop init failed,service stop!!!\n");
		return WTP_FALSE;
	}

	if(!wtp_timer_init(&Agent))
	{
		wtp_eloop_destory();
		printf("WTP timerfd init failed,service stop!!!\n");
		return WTP_FALSE;
	}

	if(!wtp_events_init(&Agent))
	{
		printf("huayi events init failed,service stop!!!\n");
		wtp_eloop_destory();
		wtp_timer_destory(&Agent);
		return WTP_FALSE;
	}

	if(!wtp_socket_init(&Agent))
	{
		printf("WTP socket init failed,service stop!!!\n");
		wtp_eloop_destory();
		wtp_events_destory(&Agent);
		wtp_timer_destory(&Agent);
		return WTP_FALSE;
	}

	pthread_mutex_lock(&reload_mutex);
	
	pthread_create(&wtpreload, NULL, (void*)CWExecuteReload, &Agent);

	if(!wtp_eloop_register_socket(Agent.master_socket.socket,(void*)wtp_master_receive_msg,&Agent))
	{
		printf("WTP eloop master socket register failed,service stop!!!\n");
		goto service_stop;
	}
	
	if(!wtp_eloop_register_socket(Agent.wtp_timer_fd,(void*)wtp_timer_process,&Agent))
	{
		printf("WTP eloop timer socket register failed,service stop!!!\n");
		goto service_stop;
	}

	if(!wtp_eloop_register_socket(Agent.wtp_event_fd,(void*)wtp_events_receive,&Agent))
	{
		printf("WTP eloop timer socket register failed,service stop!!!\n");
		goto service_stop;
	}
	

	WTPInitState(&Agent);
	
	while(WTP_TRUE == gRunAgent)
	{
		wtp_eloop_execution();
	}

service_stop:
	APAssocACStatusSet(AP_DISASSOC_AC);
	
	pthread_mutex_destroy(&config_mutex);
	
	pthread_mutex_unlock(&reload_mutex);
	
	pthread_mutex_destroy(&reload_mutex);

	wtp_eloop_remove_socket(Agent.wtp_timer_fd);
	
	wtp_eloop_remove_socket(Agent.master_socket.socket);

	wtp_eloop_remove_socket(Agent.wtp_event_fd);

	wtp_events_destory(&Agent);

	wtp_socket_destory(&Agent);

	wtp_timer_destory(&Agent);
	
	wtp_eloop_destory();

	exit(0);
}


