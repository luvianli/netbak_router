#ifndef _WTPSOCKET_HEADER_
#define _WTPSOCKET_HEADER_

#include "WTPcommon.h"
#include "WTPstate.h"






WTPBool wtp_socket_init(struct VcapwapAgent *Agent);
WTPBool wtp_socket_destory(struct VcapwapAgent *Agent);
void wtp_ipc_receive_msg(struct VcapwapAgent *Agent);
void wtp_master_receive_msg(struct VcapwapAgent *Agent);
void wtp_set_socket_acaddr(struct VcapwapAgent *Agent);
WTPBool wtp_send_msg_to_ac(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr,int fragmentsNum);
WTPBool wtp_events_init(struct VcapwapAgent *Agent);
void wtp_events_receive(struct VcapwapAgent *Agent);
void wtp_events_destory(struct VcapwapAgent *Agent);
WTPBool WTPUnicastcastDiscoverySocket(struct VcapwapAgent *Agent);



#endif
