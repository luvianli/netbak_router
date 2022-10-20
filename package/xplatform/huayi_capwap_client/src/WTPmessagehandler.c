/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "WTPmessagehandler.h"
#include "WTPstate.h"

extern WTPBool gRunAgent;
void wtp_events_process(struct VcapwapAgent *Agent,struct huayi_notification *event)
{
	char ip_str[CW_IP_STR_LEN] = {0};
		
	if (event->time > 0) 
	{
		switch(event->notification_id)
		{
			case CNOTE_USB_STORAGE_ADD:
			case CNOTE_USB_STORAGE_REMOVE:
			case CNOTE_HUAYI_BUTTON_PRESSED:
			case CNOTE_HUAYI_BUTTON_RELEASED:
			case CNOTE_HUAYI_SCHE_REBOOT_UPDATE:
				break;
			case CNOTE_CHECK_IP_ADDR_CHG:
				huayi_uci_get_str(PKG_STATUS_NETWORK,"device_port0","ip_address",ip_str);
				if(strcmp(Agent->wtp_ip,ip_str) != 0)
				{
					gRunAgent = WTP_FALSE;
					return;
				}
				break;
			default:
				printf("wtp_events_process:Unknown Event:%d\n", event->notification_id);
		}
	}

	return;
}

void wtp_process_master_msg(struct VcapwapAgent *Agent,unsigned char *recv_array, unsigned int recv_byte)
{
	CWProtocolMessage RecvMsg;
	CWControlHeaderValues controlVal;
	unsigned int 	RecvMsgSize;
	unsigned int	RecvMsgType;
	
	RecvMsg.msg = recv_array;
    RecvMsg.offset = 0;
	CWParseCWHeader(&RecvMsg, &controlVal);
	RecvMsgSize = (unsigned int)controlVal.msgElemsLen;
	RecvMsgType = controlVal.messageTypeValue;
	Agent->seqNum = controlVal.seqNum;
	#ifdef CAPWAP_SEQNUM_CHECK
	 /*do seqnum check here*/
	#endif

	//printf("RecvMsgType is %d\n",RecvMsgType);
	//printf("RecvMsgSize is %d\n",RecvMsgSize);

	switch(RecvMsgType) {
			case CW_MSG_TYPE_VALUE_DISCOVERY_RESPONSE:
				printf("WTPProcessReceiveMsg CW_DISCOVER_RSP\n");
				if(!(WTPProcessDiscoverResponse(Agent,RecvMsg, RecvMsgSize))){
					printf("CW_DISCOVER_RSP fail\n");
					APAssocACStatusSet(AP_DISASSOC_AC);
					WTPInitState(Agent);
				}
				break;
			case CW_MSG_TYPE_VALUE_DTLS_HANDSHAKE_RSP:
				printf("WTPProcessReceiveMsg DTLS Handshake complete!\n");
				//if(!(WTPProcessDtlsHandshakeResponse(RecvMsg, RecvMsgSize, Agent))){
				//	CWInitState(Agent);
				//}
				break;
			case CW_MSG_TYPE_VALUE_JOIN_RESPONSE:
				printf("WTPProcessReceiveMsg CW_JOIN_RSP\n");
				if(!(WTPProcessJoinResponse(Agent,RecvMsg, RecvMsgSize))){
					printf("CW_JOIN_RSP fail\n");
					APAssocACStatusSet(AP_DISASSOC_AC);
					WTPInitState(Agent);
				}
				break;
			case CW_MSG_TYPE_VALUE_CONFIGURE_RESPONSE:
				printf("WTPProcessReceiveMsg CW_CONFIGURE_RSP\n");
				if(!(WTPProcessConfigResponse(Agent,RecvMsg, RecvMsgSize))){
					printf("CW_CONFIGURE_RSP fail\n");
					APAssocACStatusSet(AP_DISASSOC_AC);
					WTPInitState(Agent);
				}
				break;
			case CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_RESPONSE:
				printf("WTPProcessReceiveMsg CW_EVENT_CHANGE_RSP\n");
				if(!(WTPProcessChangeStateResponse(Agent,RecvMsg, RecvMsgSize))){
					printf("CW_EVENT_CHANGE_RSP fail\n");
					APAssocACStatusSet(AP_DISASSOC_AC);
					WTPInitState(Agent);
				}
				break;
			case CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE:
				printf("WTPProcessReceiveMsg IMAGE_DATA_RESPONSE\n");
				if(!(WTPProcessImageDataResponse(Agent,RecvMsg, RecvMsgSize))){
					printf("IMAGE_DATA_RESPONSE fail\n");
					APAssocACStatusSet(AP_DISASSOC_AC);
					WTPInitState(Agent);
				}
				break;
			case CW_MSG_TYPE_VALUE_WTP_EVENT_RESPONSE :
				printf("WTPProcessReceiveMsg CW_MSG_TYPE_VALUE_WTP_EVENT_RSP\n");
				//if(!(WTPProcessWTPEventResponse(RecvMsg, RecvMsgSize,RecvMsgseqNum, Agent))){
				//	APAssocACStatusSet(AP_DISASSOC_AC);
				//	CWInitState(Agent);
				//}
				break;
			case CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_REQUEST :
			case CW_MSG_TYPE_VALUE_ECHO_RESPONSE :
			case CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST:
			case CW_MSG_TYPE_VALUE_RESET_REQUEST :
			case CW_MSG_TYPE_VALUE_CLEAR_CONFIGURATION_REQUEST :
			case CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_REQUEST:
			case CW_MSG_TYPE_VALUE_WLAN_CONFIG_REQUEST:
			case CW_MSG_TYPE_VALUE_STA_INFO_RSP:
			case CW_MSG_TYPE_VALUE_STA_NOTIFICATION_RSP:		 	
				printf("WTPProcessReceiveMsg CW_RUN_RSP:%d\n",RecvMsgType);
				if(!(WTPProcessRun(Agent,RecvMsg, RecvMsgSize,RecvMsgType))){
					printf("CW_RUN_RSP fail\n");
					printf("WTPProcessReceiveMsg return Error!\n");
				}
				break;
			default:
				printf("WTPProcessReceiveMsg:Unknown Msgtype %d\n",RecvMsgType);
				return;
		}
	return;
}

