#ifndef _WTPELEMENT_HEADER_
#define _WTPELEMENT_HEADER_

#include "WTPcommon.h"
#include "CWprotocol.h"
#include "WTPstate.h"


WTPBool CWAssembleMsgElemDiscoveryType(CWProtocolMessage *msgPtr);
WTPBool CWAssembleMsgElemWTPBoardData(CWProtocolMessage *msgPtr, struct VcapwapAgent* Agent);
WTPBool CWAssembleMsgElemWTPDescriptor(CWProtocolMessage *msgPtr, struct VcapwapAgent* Agent);
WTPBool CWAssembleMsgElemWTPFrameTunnelMode(CWProtocolMessage *msgPtr);
WTPBool CWAssembleMsgElemWTPMACType(CWProtocolMessage *msgPtr);
WTPBool CWAssembleMsgElemWTPRadioInfo(CWProtocolMessage *msgPtr, struct VcapwapAgent* Agent);
WTPBool CWAssembleMsgElemWTPIPv4Address(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);
WTPBool CWAssembleMsgElemWTPLocationData(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);
WTPBool CWAssembleMsgElemWTPName(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);
WTPBool WTPAssemblePayloadMsgElemUptime(CWProtocolMessage *msgPtr);
WTPBool WTPAssemblePayloadMsgElemBaseMac(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);
WTPBool WTPAssemblePayloadMsgElemWtpModel(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);
WTPBool WTPAssemblePayloadMsgElemSoftwareVersion(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);
WTPBool WTPAssemblePayloadMsgElemBootVersion(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);
WTPBool WTPAssemblePayloadMsgElemRadioType(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);
WTPBool WTPAssembleMsgElemConfigActionType(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent,CW_WTP_VENDOR_ACTION actionType);
WTPBool WTPAssemblePayloadMsgElemTrustId(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent);


WTPBool WTPParseResultCode(CWProtocolMessage *msgPtr, int len, unsigned int *valPtr);
WTPBool WTPParseIdleTimeOut(CWProtocolMessage *msgPtr, int len,struct VcapwapAgent* Agent);
WTPBool WTPParseEchoTimer(CWProtocolMessage *msgPtr, int len,struct VcapwapAgent* Agent);
WTPBool WTPParseWtpName(CWProtocolMessage *msgPtr, int len, struct VcapwapAgent* Agent);
WTPBool WTPParseACName(CWProtocolMessage *msgPtr, int len, struct VcapwapAgent* Agent);
WTPBool WTPParseImageIdentifier(CWProtocolMessage *msgPtr, int len, IMAGE_IDENTIFIER_CODE *valPtr);

WTPBool WTPParseControlIPv4Addresses(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent);
WTPBool WTPParsePayloadMsgElem(CWProtocolMessage *msgPtr, unsigned short int len, struct VcapwapAgent* Agent);
WTPBool WTPParseConfigActionType(CWProtocolMessage *msgPtr, int len, CW_WTP_VENDOR_ACTION *actionType);



#endif
