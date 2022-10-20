#ifndef _WTPMESSAGEHANDLER_HEADER_
#define _WTPMESSAGEHANDLER_HEADER_

#include "WTPcommon.h"
#include "CWprotocol.h"


void wtp_process_master_msg(struct VcapwapAgent *Agent,unsigned char *recv_array, unsigned int recv_byte);
void wtp_events_process(struct VcapwapAgent *Agent,struct huayi_notification *event);
void wtp_process_service_msg(void);








#endif
