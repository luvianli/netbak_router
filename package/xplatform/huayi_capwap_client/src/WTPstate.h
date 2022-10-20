#ifndef _WTPSTATE_HEADER_
#define _WTPSTATE_HEADER_

#include "WTPcommon.h"
#include "CWprotocol.h"


#define WTPClearStateVariables(pAgentStateVariables) \
     do {\
         memset(pAgentStateVariables, 0, sizeof(WTPStateVariables));\
         }while(0)


typedef enum
{
	DISCONNECT_REASON_INIT = 0,
	DISCONNECT_REASON_LOGIN_FAIL = 1,
	DISCONNECT_REASON_DTLS_FAIL = 2,
	DISCONNECT_REASON_LINKDOWN = 3,
	DISCONNECT_REASON_CAPBILITY =4
}DISCONNECT_REASON;

WTPBool APAssocACStatusSet(Capwap_AP_AC_CONNECT_State assoc_status);
WTPBool WTPInitState(struct VcapwapAgent* Agent);

WTPBool WTPProcessDiscoverResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize);
WTPBool WTPProcessJoinResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize);
WTPBool WTPProcessConfigResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize);
WTPBool WTPProcessImageDataResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize);
WTPBool WTPProcessChangeStateResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize);
WTPBool WTPProcessRun(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize,unsigned int recvMsgType);


#endif
