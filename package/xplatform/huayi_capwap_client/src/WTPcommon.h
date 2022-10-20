#ifndef _WTPCOMMON_HEADER_
#define _WTPCOMMON_HEADER_

#include "CWmessage.h"
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h> 
#include <fcntl.h>
#include <sys/timerfd.h>
#include <pthread.h>
#include "CWprotocol.h"
#include "huayi-universal/huayi_uci_fun.h"
#include "huayi_alarm.h"
#include "huayi_events.h"


#define WTP_DOMAIN										"/var/run/wtpclient.domain"										

#define MEM_SIZE       1024
#define MEM_HALT       512

typedef enum
{
  AP_DISASSOC_AC = 0,
  AP_ASSOC_AC
}Capwap_AP_AC_CONNECT_State;



typedef enum {
	WTP_JOB_DISCOVER_REQUEST,
	WTP_JOB_JOIN_REQUEST,
	WTP_JOB_IMAGE_DATA_REQUEST,
	WTP_JOB_CONFIG_REQUEST,
	WTP_JOB_EVENT_CHANGE_REQUEST,
	WTP_JOB_HEART_BEAT_REQUEST,
	WTP_JOB_RELOAD_WIRELESS,
	WTP_JOB_DATA_KEEP_ALIVE,
	WTP_JOB_EVENT_STATE_REQUEST,
	WTP_JOB_RESET,
	WTP_JOB_REBOOT,
	WTP_JOB_ALIVE_CHECK,
}WTPTASK;

typedef enum {
	WTP_FALSE = LIB_FALSE,
	WTP_TRUE = LIB_TRUE
} WTPBool;

struct WTPManageSocket
{
	int socket;
	struct sockaddr_in address;
};

struct WTPAgenttimer{
	unsigned int  discoveryTimer;
	unsigned int  echoRequestTimer;
	unsigned int  idleTimeout;
	unsigned int  statisticsTimer;
	unsigned int  warningRetransInterval;
	unsigned int  sampleInterval;
	unsigned int  statPeriod;
};

typedef struct VcapwapAgent
{
	unsigned char baseMac[CW_MAC_LEN];
	struct WTPManageSocket master_socket;
	int 			wtp_timer_fd;
	unsigned char	seqNum;
	char  ac_ip[CW_IP_STR_LEN];
	char  wtp_ip[CW_IP_STR_LEN];
	char  wtp_model[CW_MODEL_STR_LEN];
	CWStateTransitions currentState;
	WTPStateVariables	stateVariables;
	char SerialNumber[TEMP_BUF_SIZE];
	char ac_name[CW_OPTION_STR_LEN];
	unsigned short manage_port;
	IMAGE_IDENTIFIER_CODE imageidentifier;
	struct WTPAgenttimer Agenttimer;
	unsigned char firmware_url[CW_FILE_URL_LEN];
	char wtp_location[CW_OPTION_STR_LEN];
	char wtp_name[CW_APNAME_STR_LEN];
	unsigned int softVersion;
	unsigned int bootVersion;
	unsigned char radiotype;
	WTPBool need_broadcast;
	unsigned int trustid;
	int wtp_event_fd;
	int broadcast_count;
}VcapwapAgentStructure;

typedef struct wireless_radio_cfg
{
	unsigned char if_admin;
	unsigned char wirelessmode;
	unsigned char htmode;
	unsigned char channel;
	unsigned char txpower;
	unsigned char country_len;
	unsigned short beacon;
	unsigned char country[CW_OPTION_STR_LEN];
}WIRELESS_RADIO_CONFIG;

typedef struct wireless_ssid_cfg
{
	unsigned char	ssid[CW_SSID_STR_LEN] ;
	unsigned char	passphrase[CW_PASSPHRASE_LEN];
	unsigned char	radioid,hide,isolate,encry,maxsta;
	unsigned short	vlanid;
	unsigned char	ssid_len,passphrase_len;
}WIRELESS_SSID_CONFIG;

typedef struct system_cfg
{
	unsigned char	gloabal_roaming;
	unsigned char	gloabal_auto_channel;
	unsigned char   roaming_threshold_2g;
	unsigned char   roaming_threshold_5g;
}WTP_SYSTEM_CONFIG;


typedef struct
{
	struct wireless_radio_cfg radio_config[CW_MAX_RADIO];
	struct wireless_ssid_cfg ssid_config[CW_MAX_RADIO];
	struct system_cfg sys_config;
	
}WTP_CONFIG_RECEIVE;

typedef enum
{
	AP_OFFLINE = 0,
	AP_ONLINE = 1,
}AC_STATE;

unsigned char WTPSetSeqNum(struct VcapwapAgent *Agent,WTPBool is_retry);
int ac_ip_to_proc(char      *ac_ip);
int wtp_status_to_proc(AC_STATE state);
int wtp_name_to_uci(char *wtp_name);
int current_uptime_to_proc(void);
int findStrInFile(char *filename, int offset,char *str, int str_len);
void *getMemInFile(char *filename, int offset, int len);
int cut_upload_file(char *filename,char *realfile);

#endif
