#ifndef _HUAYI_CLIENT_HEADER_
#define _HUAYI_CLIENT_HEADER_
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/file.h>
//#include <huayi_mysql.h>
#include "huayi_uci_fun.h"
#include "huayi_atcmd.h"


#define SMS_GET_TIMEOUT		10

#define HUAYI_SERVER_DOMAIN		 					"/var/run/x55_server.sock"

#define HUAYI_SMS_DOMAIN 							"/var/run/huayi_sms.sock"


typedef enum {
	VS_FALSE = 0,
	VS_TRUE,
} VSBool;

struct ReceiveSmsSocket
{
	int sms_ipc_fd;
	struct sockaddr_un address; 
};



typedef struct SMSAgent
{
	struct ReceiveSmsSocket sms_sock;
	int  lte_led_status;
	int  i_signal;
	char init_modem_status;
	char is_wired_mode;
	char msg[MAX_SMS_MSG_LEN];
}SMSAgentStructure;

typedef struct sms_message{
	unsigned char center_len;
	unsigned char center[MAX_SMS_VALUE_LEN];
	unsigned char code;
	unsigned char src_code_len;
	unsigned char src_code[MAX_SMS_VALUE_LEN];
	unsigned char protocol_type;
	unsigned char encode_type;
	unsigned char date[MAX_SMS_VALUE_LEN];
	unsigned char time_zone;
	unsigned char msg_len;
	unsigned char msg[MAX_SMS_MSG_LEN];
}SMS_STRUCTURE;

typedef struct huayi_sms_msg{
	unsigned char fragment_num;
	unsigned char is_used;
	unsigned char delay_sec;
	unsigned char pdu_len;
	unsigned char sender[MAX_SMS_VALUE_LEN];
	unsigned char msg_id[MAX_SMS_VALUE_LEN];
	unsigned char msg[MAX_SMS_MSG_LEN];
}HUAYI_SMS_MSG;


extern VSBool gRunAgent;

#endif
