#ifndef _CWMESSAGE_HEADER_
#define _CWMESSAGE_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>


/*this header file used by capwap server and fastcgi*/
/*internal msg type defined in this file*/

#define CW_DOMAIN  										"/var/run/cwserver.domain"

#define FASTCGI_DOMAIN  								"/var/run/fastcgi.domain"

#define MAX_IPC_MSG_SIZE								1024

#define CW_MAC_LEN										6

#define CW_MAC_STR_LEN									18

#define CW_IP_STR_LEN									18

#define CW_MODEL_STR_LEN								16

#define CW_APNAME_STR_LEN								32

#define	CW_SSID_STR_LEN									32

#define CW_PASSPHRASE_LEN								64

#define CW_OPTION_STR_LEN								32

#define CW_LONG_OPTION_LEN								64

#define TEMP_BUF_SIZE									128

#define CMD_STR_SIZE									256

#define CW_FILE_URL_LEN									512

#define MAXVENDORCOUTE	 								8

#define WTPDISCRIBERCOUTE	 						    3

#define BOARDDATAINFOCOUTE	 						    5

#define	CW_ZERO_MEMORY(mess,size)		memset(mess,0,size)

#define	CW_COPY_MEMORY(dst, src, len)	memcpy(dst, src, len)

#define VENDOR_ID										4 /*use unix*/

#define CW_MAX_AP_NUM									512

#define CW_LOADBALANCE_SOCKET_NUM						8

#define CW_TOTAL_SOCKET_NUM								(CW_LOADBALANCE_SOCKET_NUM+4)  /*IPC+MASTER+TIMER+EVENTS*/

#define CW_MASTER_UDP_PORT								5246

#define CW_SERVICE_UDP_PORT_START						5800

#define CW_SERVICE_UDP_PORT_END							(CW_SERVICE_UDP_PORT_START+CW_LOADBALANCE_SOCKET_NUM-1)

#define MAX_UDP_MSG_SIZE 								1472

#define CW_DISCOVER_TIME_OFFSET 						5

#define CW_COMMON_TIME_OFFSET							3

#define WTP_WAIT_RELOAD_WIRELESS 						5

#define CW_RETRY_LIMIT									3

#define MAX_ECHO_RETRY									6

#define MIN_ECHO_RETRY									3

#define MAX_ECHO_TIMER									200

#define MIN_ECHO_TIMER									30

#define CW_MTU											1400

#define CW_APFIRMWARE_PATH								"/ap_firmware/"

typedef enum {
	LIB_FALSE = 0,
	LIB_TRUE = 1
} LIBBool;

typedef enum {
	CW_SULKING 		=0,
	CW_DISCOVER 	=1,
	CW_DTLS			=2,
	CW_JOIN			=3,
	CW_CONFIGURE	=4,
	CW_DATA_CHECK	=5,
	CW_RUN			=6,
	CW_UPGRADING	=7,
	CW_RESETING		=8,
	CW_REBOOTING	=9,
} CWStateTransitions;


#define CW_RADIO_2G  0x01
#define CW_RADIO_5G  0x02
#define CW_MAX_RADIO		2

#define WIRELESSMODE_11B	1
#define WIRELESSMODE_11A	2
#define WIRELESSMODE_11G	4
#define WIRELESSMODE_11N	6
#define WIRELESSMODE_11AN	8
#define WIRELESSMODE_11BGN	9
#define WIRELESSMODE_11AC	14

#define MODE_TO_STR(mode) \
	((mode == WIRELESSMODE_11B) ? "11b" : \
  (mode == WIRELESSMODE_11A)? "11a" : \
  (mode == WIRELESSMODE_11G)? "11g" : \
  (mode == WIRELESSMODE_11N)? "11ng" : \
  (mode == WIRELESSMODE_11AN)? "11na" : \
  (mode == WIRELESSMODE_11BGN)? "11ng": \
  (mode == WIRELESSMODE_11AC)? "11ac" : \
	 "AUTO")

#define HTMODE_20MHZ		0
#define HTMODE_40MHZ		1
#define HTMODE_80MHZ		2

#define CHANNEL_AUTO		0
#define MAX_2G_CHANNEL		13

#define STATE_ID_TO_STR(id) \
 ((id == CW_SULKING) ? "CW_SULKING" : \
  (id == CW_DISCOVER)? "CW_DISCOVER" : \
  (id == CW_DTLS)? "CW_DTLS" : \
  (id == CW_JOIN)? "CW_JOIN" : \
  (id == CW_CONFIGURE)? "CW_CONFIGURE" : \
  (id == CW_DATA_CHECK)? "CW_DATA_CHECK": \
  (id == CW_RUN)? "CW_RUN" : \
  (id == CW_UPGRADING)? "CW_UPGRADING" : \
	 "Unknown State")


typedef union{
	struct{
		unsigned char numberOfSend; 
	}WTPDiscoverStateVariables;

	struct{
		unsigned char numberOfSend; 
	}WTPJoinStateVariables;

	struct{
		unsigned char numberOfSend; 
	}WTPConfigStateVariables;

	struct{
		unsigned char numberOfSend; 
	}WTPDataCheckStateVariables;

	struct{
		unsigned char numberOfSend;
	}WTPRunStateVariables;

	struct{
		unsigned char numberOfSend; 
	}WTPImageDataVariables;
}WTPStateVariables;

typedef struct {
	unsigned char *msg;
	int offset;
	int data_msgType; 
} CWProtocolMessage;

#define	CW_CREATE_OBJECT_ERR(obj_name, obj_type, on_err)	{obj_name = (obj_type*) (malloc(sizeof(obj_type))); if(!(obj_name)) {on_err}}

#define	CW_CREATE_ARRAY_ERR(ar_name, ar_size, ar_type, on_err)	{ar_name = (ar_type*) (malloc(sizeof(ar_type) * (ar_size))); if(!(ar_name)) {on_err}}

#define	CW_CREATE_OBJECT_SIZE_ERR(obj_name, obj_size,on_err)	{obj_name = (malloc(obj_size)); if(!(obj_name)) {on_err}}


#define		CW_CREATE_PROTOCOL_MESSAGE(mess, size, err)		CW_CREATE_OBJECT_SIZE_ERR(((mess).msg), (size), err);		\
									CW_ZERO_MEMORY(((mess).msg), (size));						\
									(mess).offset = 0;

#define 	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(ar_name, ar_size, on_err) 	{\
											int i;\
											CW_CREATE_ARRAY_ERR(ar_name, ar_size, CWProtocolMessage, on_err)\
											for(i=0;i<(ar_size); i++) {\
												(ar_name)[i].msg = NULL;\
												(ar_name)[i].offset = 0; \
											}\
										}






#define	CW_FREE_OBJECT(obj_name)		{if(obj_name){free((obj_name)); (obj_name) = NULL;}}


#define		CW_FREE_PROTOCOL_MESSAGE(mess)				CW_FREE_OBJECT(((mess).msg));								\
									(mess).offset = 0;

void print_mem (unsigned char *p, int k);
void print_addr(struct sockaddr_in *addr);

unsigned char CWProtocolRetrieve8(CWProtocolMessage *msgPtr);
unsigned short CWProtocolRetrieve16(CWProtocolMessage *msgPtr);
unsigned int CWProtocolRetrieve32(CWProtocolMessage *msgPtr);
unsigned long CWProtocolRetrieve64(CWProtocolMessage *msgPtr);
void CWProtocolRetrieveStr(CWProtocolMessage *msgPtr, int len, unsigned char *str);
void CWProtocolRetrieveMac(CWProtocolMessage *msgPtr,unsigned char *mac);
void CWProtocolRetrieveRawBytes(CWProtocolMessage *msgPtr, int len, char *bytes) ;

void CWProtocolStore8(CWProtocolMessage *msgPtr, unsigned char val);
void CWProtocolStore16(CWProtocolMessage *msgPtr, unsigned short val);
void CWProtocolStore32(CWProtocolMessage *msgPtr, unsigned int val);
void CWProtocolStore64(CWProtocolMessage *msgPtr, unsigned long long int val);
void CWProtocolStore128(CWProtocolMessage *msgPtr, unsigned long long int val);
void CWProtocolStoreStr(CWProtocolMessage *msgPtr,char *str);
void CWProtocolStoreMAC(CWProtocolMessage *msgPtr,char *str);
void CWProtocolStoreMessage(CWProtocolMessage *msgPtr, CWProtocolMessage *msgToStorePtr);
void CWProtocolStoreRawBytes(CWProtocolMessage *msgPtr, char *bytes, int len);

LIBBool CWAssembleMsgElem(CWProtocolMessage *msgPtr, unsigned int type);


#endif
