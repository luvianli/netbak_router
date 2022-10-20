#ifndef _HUAYI_RELOAD_HEADER_
#define _HUAYI_RELOAD_HEADER_
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "huayi_core_internal.h"
#include "huayi_uci_fun.h"


typedef enum {
	VR_FALSE = 0,
	VR_TRUE,
} VRBool;

struct ReloadSendSocket
{
	int reload_ipc_fd;
	struct sockaddr_un address; 
};

typedef struct ReloadCmd
{
	char 	CmdStr[OPTION_STR_LEN];
	MsgType msg;
	int 	agrc;
	char 	HelpInfo[TEMP_STR_LEN];
}ReloadCmdStructure;

typedef struct ReloadAgent
{
	struct ReloadSendSocket sendSocket;
	cJSON *input_root;
}ReloadAgentStructure;


#endif
