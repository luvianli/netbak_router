/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "WTPstate.h"
#include "WTPelement.h"
#include "WTPsocket.h"
#include "WTPtimer.h"
#include "WTPhandlereload.h"
#include<signal.h>   

void wtp_renew_ip()
{
	system("killall -SIGUSR1 udhcpc");
	
	return;
}

WTPBool APAssocACStatusSet(Capwap_AP_AC_CONNECT_State assoc_status)
{
	if(assoc_status == AP_ASSOC_AC)
	{
		wtp_status_to_proc(AP_ONLINE);//SYS_AP_UP_TIME_GET(&assoc_ac_time);
		current_uptime_to_proc();
	}
	else if(assoc_status == AP_DISASSOC_AC)
	{
		wtp_status_to_proc(AP_OFFLINE);
		
		system("huayi_cmd set system ac_assoc_time 0");

		system("huayi_cmd set system ac_ipaddr 0.0.0.0");
	}

	return WTP_TRUE;
}

void WTPDisconnectReason(DISCONNECT_REASON reason)
{
	switch(reason)
	{
		case DISCONNECT_REASON_LOGIN_FAIL:
			printf("disconnect reason login fail!\n");
			break;
		case DISCONNECT_REASON_DTLS_FAIL:
			printf("disconnect reason dtls fail!\n");
			break;
		case DISCONNECT_REASON_LINKDOWN:
			printf("disconnect reason linkdown!\n");
			break;
		case DISCONNECT_REASON_CAPBILITY:
			printf("disconnect reason capbility fail!\n");
			break;
		case DISCONNECT_REASON_INIT:
			printf("disconnect init thinap:disconnect_reason !\n");
		default:
			break;
	}
	return;
}

WTPBool WTPAssembleCommonResponse(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, unsigned char seqNum,CWProtocolCommonResponseValues *commonResponseVal)
{
	CWProtocolMessage	*msgElems= NULL;
	const int 		msgElemCount = 2;
	CWProtocolMessage 	*msgElemsBinding= NULL;
	const int 		msgElemBindingCount=0;
	int 			k = 0,i=0;
	int 			PMTU = CW_MTU;
	if(messagesPtr == NULL || fragmentsNumPtr == NULL)
	{
		return WTP_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount,return WTP_FALSE;);
	
	if(!(WTPAssemblePayloadMsgElemBaseMac(&(msgElems[k++]),Agent))||
		!(WTPAssembleMsgElemConfigActionType(&(msgElems[k++]),Agent,commonResponseVal->actionType)))
	{
		
		for(i = 0; i < k; i++)
		{ 
			CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
		}
		CW_FREE_OBJECT(msgElems);
		/* error will be handled by the caller */
		return  WTP_FALSE;
	}
	
	return CWAssembleMessage(messagesPtr,fragmentsNumPtr,PMTU,seqNum,
				commonResponseVal->rspMsgType, 
				msgElems,
				msgElemCount,
				msgElemsBinding,
				msgElemBindingCount,
#ifndef CW_DTLS
				CW_PACKET_PLAIN
#else
				CW_PACKET_CRYPT
#endif
				 );

}

WTPBool WTPProcessWlanConfigRequest(unsigned char *msg,int msgoffset,int len,CWProtocolCommonResponseValues *commonResponseVal,struct VcapwapAgent* Agent)
{
	CWControlHeaderValues	controlVal;
	CWProtocolMessage	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	commonResponseVal->rspMsgType = CW_MSG_TYPE_VALUE_WLAN_CONFIG_RESPONSE;

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) 
		{
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				if(!WTPParsePayloadMsgElem(&completeMsg,len,Agent)) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_CONFIG_UPDATE_ACTION_TYPE:
				if(!WTPParseConfigActionType(&completeMsg,len,&(commonResponseVal->actionType))) return WTP_FALSE;
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset - offsetTillMessages!= controlVal.msgElemsLen) 
	{ 
		return WTP_FALSE;
	}
	return WTP_TRUE;
}

WTPBool WTPProcessResetRequest(unsigned char *msg,int msgoffset,int len,CWProtocolCommonResponseValues *commonResponseVal,struct VcapwapAgent* Agent)
{
	CWControlHeaderValues	controlVal;
	CWProtocolMessage	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	commonResponseVal->rspMsgType = CW_MSG_TYPE_VALUE_RESET_RESPONSE;

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) 
		{
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				if(!WTPParsePayloadMsgElem(&completeMsg,len,Agent)) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_CONFIG_UPDATE_ACTION_TYPE:
				if(!WTPParseConfigActionType(&completeMsg,len,&(commonResponseVal->actionType))) return WTP_FALSE;
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset - offsetTillMessages!= controlVal.msgElemsLen) 
	{ 
		return WTP_FALSE;
	}
	return WTP_TRUE;
}

WTPBool WTPProcessConfigurationUpdateRequest(unsigned char *msg,int msgoffset,int len,CWProtocolCommonResponseValues *commonResponseVal,struct VcapwapAgent* Agent)
{
	CWControlHeaderValues	controlVal;
	CWProtocolMessage	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	commonResponseVal->rspMsgType = CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_RESPONSE;

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) 
		{
			case CW_MSG_ELEMENT_WTP_NAME_CW_TYPE:
				if(!(WTPParseWtpName(&completeMsg, len,Agent))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				if(!WTPParsePayloadMsgElem(&completeMsg,len,Agent)) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_CONFIG_UPDATE_ACTION_TYPE:
				if(!WTPParseConfigActionType(&completeMsg,len,&(commonResponseVal->actionType))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_IDLE_TIMEOUT_CW_TYPE:
				if(!(WTPParseIdleTimeOut(&completeMsg, len,Agent))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_CW_TIMERS_CW_TYPE:
				if(!(WTPParseEchoTimer(&completeMsg, len,Agent))) return WTP_FALSE;
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset - offsetTillMessages!= controlVal.msgElemsLen) 
	{ 
		return WTP_FALSE;
	}
	return WTP_TRUE;
}

WTPBool WTPProcessImageDataRequest(unsigned char *msg,int msgoffset,int len,CWProtocolCommonResponseValues *commonResponseVal,struct VcapwapAgent* Agent)
{
	CWControlHeaderValues	controlVal;
	CWProtocolMessage	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	commonResponseVal->rspMsgType = CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE;

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) 
		{
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				if(!WTPParsePayloadMsgElem(&completeMsg,len,Agent)) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_CONFIG_UPDATE_ACTION_TYPE:
				if(!WTPParseConfigActionType(&completeMsg,len,&(commonResponseVal->actionType))) return WTP_FALSE;
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset - offsetTillMessages!= controlVal.msgElemsLen) 
	{ 
		return WTP_FALSE;
	}
	return WTP_TRUE;
}


WTPBool WTPProcessRun(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize,unsigned int recvMsgType)
{
	CWProtocolMessage *messagesPtr = NULL;
	int fragmentsNum = 0;
	CWProtocolCommonResponseValues commonResponseVal;
	unsigned char	seqNum;

	commonResponseVal.rspMsgType = 0;
	
	switch(recvMsgType) {
		case CW_MSG_TYPE_VALUE_ECHO_RESPONSE :
			WTPClearStateVariables(&(Agent->stateVariables));
			break;
		case CW_MSG_TYPE_VALUE_WLAN_CONFIG_REQUEST:
			if(!(WTPProcessWlanConfigRequest(recvMsg.msg,recvMsg.offset,msgSize,&commonResponseVal,Agent)))
			{
				printf("WlanConfigRequest fail\n");
				APAssocACStatusSet(AP_DISASSOC_AC);
				WTPInitState(Agent);
			}
			break;
		case CW_MSG_TYPE_VALUE_RESET_REQUEST:
			if(!(WTPProcessResetRequest(recvMsg.msg,recvMsg.offset,msgSize,&commonResponseVal,Agent)))
			{
				printf("ResetRequest fail\n");
				APAssocACStatusSet(AP_DISASSOC_AC);
				WTPInitState(Agent);
			}
			break;
		case CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_REQUEST:
			if(!(WTPProcessConfigurationUpdateRequest(recvMsg.msg,recvMsg.offset,msgSize,&commonResponseVal,Agent)))
			{
				printf("ResetRequest fail\n");
				APAssocACStatusSet(AP_DISASSOC_AC);
				WTPInitState(Agent);
			}
			break;
		case CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST:
			if(!(WTPProcessImageDataRequest(recvMsg.msg,recvMsg.offset,msgSize,&commonResponseVal,Agent)))
			{
				printf("ImageDataRequest fail\n");
				APAssocACStatusSet(AP_DISASSOC_AC);
				WTPInitState(Agent);
			}
			break;
		default:
			break;
	}

	if(!commonResponseVal.rspMsgType)
	{
		return WTP_TRUE;
	}

	seqNum = WTPSetSeqNum(Agent,WTP_FALSE);
	
	if(!(WTPAssembleCommonResponse(Agent,&messagesPtr,&fragmentsNum,seqNum,&commonResponseVal)))
	{
		return WTP_FALSE;
	}
	
	wtp_send_msg_to_ac(Agent,&messagesPtr,fragmentsNum);

	switch(commonResponseVal.actionType)
	{
		case CW_ACTION_ADD_WLAN:
			printf("add wlan  here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			wtp_delete_first_task_by_id(WTP_JOB_RELOAD_WIRELESS);
			wtp_add_task(WTP_WAIT_RELOAD_WIRELESS,WTP_JOB_RELOAD_WIRELESS,(void *)WTPExecuteNeedReloadWireless,NULL,NULL);
			break;
		case CW_ACTION_UPDATE_WLAN:
			printf("update wlan  here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			wtp_delete_first_task_by_id(WTP_JOB_RELOAD_WIRELESS);
			wtp_add_task(WTP_WAIT_RELOAD_WIRELESS,WTP_JOB_RELOAD_WIRELESS,(void *)WTPExecuteNeedReloadWireless,NULL,NULL);
			break;
		case CW_ACTION_UPDATE_RADIO:
			printf("update RADIO  here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			wtp_delete_first_task_by_id(WTP_JOB_RELOAD_WIRELESS);
			wtp_add_task(WTP_WAIT_RELOAD_WIRELESS,WTP_JOB_RELOAD_WIRELESS,(void *)WTPExecuteNeedReloadWireless,NULL,NULL);
			break;
		case CW_ACTION_UPDATE_GLOABAL_ADVANCE:
			printf("update advance here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			wtp_delete_first_task_by_id(WTP_JOB_RELOAD_WIRELESS);
			wtp_add_task(WTP_WAIT_RELOAD_WIRELESS,WTP_JOB_RELOAD_WIRELESS,(void *)WTPExecuteNeedReloadWireless,NULL,NULL);
			break;
		case CW_ACTION_REBOOT:
			/*do reboot*/
			printf("we should reboot here\n");
			wtp_delete_first_task_by_id(WTP_JOB_HEART_BEAT_REQUEST);
			wtp_add_task(CW_COMMON_TIME_OFFSET,WTP_JOB_REBOOT,(void *)WTPDoReboot,NULL,Agent);
			break;
		case CW_ACTION_RESET:
			/*do reset*/
			printf("we should reset here\n");
			wtp_delete_first_task_by_id(WTP_JOB_HEART_BEAT_REQUEST);
			wtp_add_task(CW_COMMON_TIME_OFFSET,WTP_JOB_RESET,(void *)WTPDoReset,NULL,Agent);
			break;
		case CW_ACTION_UPGRADE:
			/*do sysupgrade*/
			printf("we should upgrade here\n");
			wtp_delete_first_task_by_id(WTP_JOB_HEART_BEAT_REQUEST);
			start_firmware_update(Agent);
			break;
		case CW_ACTION_LED_UP:
			printf("led up here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			WTPDoLedOn(Agent);
			break;
		case CW_ACTION_LED_OFF:
			printf("led off here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			WTPDoLedOff(Agent);
			break;
		default:
			printf("commonResponseVal.actionType is %d\n",commonResponseVal.actionType);
			break;
	}

	return WTP_TRUE;
}


WTPBool WTPAssembleEchoRequest(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, unsigned char seqNum)
{
	CWProtocolMessage	*msgElems= NULL;
	const int 		msgElemCount = 1;
	CWProtocolMessage 	*msgElemsBinding= NULL;
	const int 		msgElemBindingCount=0;
	int 			k = 0,i=0;
	
	if(messagesPtr == NULL || fragmentsNumPtr == NULL)
	{
		return WTP_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount,return WTP_FALSE;);
	
	if(!(WTPAssemblePayloadMsgElemBaseMac(&(msgElems[k++]),Agent)))
	{
		
		for(i = 0; i < k; i++)
		{ 
			CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
		}
		CW_FREE_OBJECT(msgElems);
		/* error will be handled by the caller */
		return  WTP_FALSE;
	}
	
	return CWAssembleMessage(messagesPtr,fragmentsNumPtr,PMTU,seqNum,
				CW_MSG_TYPE_VALUE_ECHO_REQUEST, 
				msgElems,
				msgElemCount,
				msgElemsBinding,
				msgElemBindingCount,
#ifndef CW_DTLS
				CW_PACKET_PLAIN
#else
				CW_PACKET_CRYPT
#endif
				 );

}



WTPBool WTPSendEchoRequest(struct VcapwapAgent* Agent)
{
	int PMTU;
	unsigned char seqNum;
	int fragmentsNum;
	CWProtocolMessage *messagesPtr = NULL;
	
	seqNum = WTPSetSeqNum(Agent,WTP_FALSE);
	
	PMTU = CW_MTU;
	
	WTPAssembleEchoRequest(Agent,&messagesPtr, &fragmentsNum, PMTU, seqNum);

	wtp_send_msg_to_ac(Agent,&messagesPtr,fragmentsNum);

	return WTP_TRUE;
}

WTPBool WTPExecuteEchoRequest(void* argument)
{
	struct VcapwapAgent* Agent = argument;
	WTPStateVariables* pAgentStateVariables = &(Agent->stateVariables);
	
	printf("Execute Echo_Request\n");
		
	if(Agent->currentState != CW_RUN)
	{
		printf("Not in Run State, Drop job\n");
		return WTP_TRUE;
	}

	printf("pAgentStateVariables->WTPRunStateVariables.numberOfSend is %d\n",pAgentStateVariables->WTPRunStateVariables.numberOfSend);
	printf("Agent->Agenttimer.idleTimeout is %d\n",Agent->Agenttimer.idleTimeout);
	if(pAgentStateVariables->WTPRunStateVariables.numberOfSend >= Agent->Agenttimer.idleTimeout) 
	{
		printf("Execute Echo_Request Retry Exceed\n");
		APAssocACStatusSet(AP_DISASSOC_AC);
		WTPInitState(Agent);
		return WTP_TRUE;
	}
	
	if(!WTPSendEchoRequest(Agent))
	{
		printf("Send Echo_Request FAILED\n");
		return WTP_FALSE;
	}
	
	pAgentStateVariables->WTPRunStateVariables.numberOfSend++;

	wtp_add_task(Agent->Agenttimer.echoRequestTimer, WTP_JOB_HEART_BEAT_REQUEST,(void *)WTPExecuteEchoRequest,NULL, Agent);
	
	return WTP_TRUE;
}


WTPBool WTPParseImageDataResponseMessage(unsigned char *msg, int msgoffset,int len,struct VcapwapAgent* Agent)
{

	CWControlHeaderValues	controlVal;
	CWProtocolMessage	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) {
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				if(!WTPParsePayloadMsgElem(&completeMsg, len, Agent)) return WTP_FALSE;
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset - offsetTillMessages!= len - 3) 
	{ 
		return WTP_FALSE;
	}
	return WTP_TRUE;
}



WTPBool WTPProcessImageDataResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize)
{

	struct WTPStateVariables* pAgentStateVariables = (struct WTPStateVariables* ) &(Agent->stateVariables);

	if(Agent->currentState != CW_UPGRADING) 
	{
		printf("Not in CW_UPGRADING State, Drop Msg\n");
		return WTP_TRUE;
	}
	if(!(WTPParseImageDataResponseMessage(recvMsg.msg, recvMsg.offset, msgSize,Agent)))
	{
		return WTP_FALSE;
	}

	WTPClearStateVariables(pAgentStateVariables);

	wtp_delete_first_task_by_id(WTP_JOB_DISCOVER_REQUEST);

	wtp_delete_first_task_by_id(WTP_JOB_JOIN_REQUEST);

	wtp_delete_first_task_by_id(WTP_JOB_IMAGE_DATA_REQUEST);
	
	start_firmware_update(Agent);
	
	return WTP_TRUE;
}



WTPBool WTPParseChangeStateResponseMessage(unsigned char *msg, int msgoffset,int len,CWProtocolChangeStateResponseValues *changestaterespvalue,struct VcapwapAgent* Agent)
{

	CWControlHeaderValues	controlVal;
	CWProtocolMessage	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) {
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				if(!(WTPParseResultCode(&completeMsg, len,&changestaterespvalue->result_code))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_IDLE_TIMEOUT_CW_TYPE:
				if(!(WTPParseIdleTimeOut(&completeMsg, len,Agent))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_CW_TIMERS_CW_TYPE:
				if(!(WTPParseEchoTimer(&completeMsg, len,Agent))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				if(!WTPParsePayloadMsgElem(&completeMsg, len, Agent)) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_WTP_NAME_CW_TYPE:
				if(!(WTPParseWtpName(&completeMsg, len,Agent))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_AC_NAME_CW_TYPE:
				if(!WTPParseACName(&completeMsg, len, Agent)) return WTP_FALSE;
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset - offsetTillMessages!= controlVal.msgElemsLen) 
	{ 
		return WTP_FALSE;
	}
	return WTP_TRUE;
}




WTPBool WTPSaveChangeStateResponseMessage(CWProtocolChangeStateResponseValues *changestaterespvalue,struct VcapwapAgent* Agent)
{
	return WTP_TRUE;
}


WTPBool WTPProcessChangeStateResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize)
{

	struct WTPStateVariables* pAgentStateVariables = (struct WTPStateVariables* ) &(Agent->stateVariables);
	CWProtocolChangeStateResponseValues changestaterespvalue ;
	
	printf("Current state: %s\n",STATE_ID_TO_STR(Agent->currentState));
	if(Agent->currentState != CW_DATA_CHECK) 
	{
		printf("Not in Discover State, Drop Msg\n");
		return WTP_TRUE;
	}
	if(!(WTPParseChangeStateResponseMessage(recvMsg.msg, recvMsg.offset, msgSize, &changestaterespvalue, Agent)) ||
		!(WTPSaveChangeStateResponseMessage(&changestaterespvalue,Agent)))
	{
		return WTP_FALSE;
	}

	wtp_delete_first_task_by_id(WTP_JOB_EVENT_CHANGE_REQUEST);

	WTPClearStateVariables(pAgentStateVariables);
	
	Agent->currentState = CW_RUN;

	wtp_name_to_uci(Agent->wtp_name);
	ac_ip_to_proc(Agent->ac_ip);
	APAssocACStatusSet(AP_ASSOC_AC);	

	wtp_add_task(Agent->Agenttimer.echoRequestTimer, WTP_JOB_HEART_BEAT_REQUEST,(void *)WTPExecuteEchoRequest,NULL, Agent);
	
	return WTP_TRUE;
}


WTPBool WTPAssembleChangeStateRequest(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, unsigned char seqNum)
{
	CWProtocolMessage	*msgElems= NULL;
	const int 		msgElemCount = 1;
	CWProtocolMessage 	*msgElemsBinding= NULL;
	const int 		msgElemBindingCount=0;
	int 			k = 0,i=0;
	
	if(messagesPtr == NULL || fragmentsNumPtr == NULL)
	{
		return WTP_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount,return WTP_FALSE;);
	
	if(!(WTPAssemblePayloadMsgElemBaseMac(&(msgElems[k++]),Agent)))
	{
		
		for(i = 0; i < k; i++)
		{ 
			CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
		}
		CW_FREE_OBJECT(msgElems);
		/* error will be handled by the caller */
		return  WTP_FALSE;
	}
	
	return CWAssembleMessage(messagesPtr,fragmentsNumPtr,PMTU,seqNum,
				CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_REQUEST, 
				msgElems,
				msgElemCount,
				msgElemsBinding,
				msgElemBindingCount,
#ifndef CW_DTLS
				CW_PACKET_PLAIN
#else
				CW_PACKET_CRYPT
#endif
				 );

}



WTPBool WTPSendChangeStateRequest(struct VcapwapAgent* Agent, WTPBool  is_retry)
{
	int PMTU;
	unsigned char seqNum;
	int fragmentsNum;
	CWProtocolMessage *messagesPtr = NULL;
	
	seqNum = WTPSetSeqNum(Agent,is_retry);
	
	PMTU = CW_MTU;
	
	WTPAssembleChangeStateRequest(Agent,&messagesPtr, &fragmentsNum, PMTU, seqNum);

	wtp_send_msg_to_ac(Agent,&messagesPtr,fragmentsNum);

	return WTP_TRUE;
	
}


WTPBool WTPExecuteChangeStateRequest(void* argument)
{
	struct VcapwapAgent* Agent = argument;
	WTPStateVariables* pAgentStateVariables = &(Agent->stateVariables);
	WTPBool is_retry = WTP_FALSE;
	
	printf("Execute Change_State_Request current state %s send %d\n",STATE_ID_TO_STR(Agent->currentState),pAgentStateVariables->WTPDataCheckStateVariables.numberOfSend);
		
	if(Agent->currentState != CW_DATA_CHECK)
	{
		printf("Not in Data Check State, Drop job\n");
		return WTP_TRUE;
	}

	if(pAgentStateVariables->WTPDataCheckStateVariables.numberOfSend > CW_RETRY_LIMIT) 
	{
		printf("Execute Change_State_Request Retry Exceed\n");
		APAssocACStatusSet(AP_DISASSOC_AC);
		WTPInitState(Agent);
		return WTP_TRUE;
	}
	
	if(pAgentStateVariables->WTPDataCheckStateVariables.numberOfSend > 0)
	{
		is_retry = WTP_TRUE ;
	}
	
	if(!WTPSendChangeStateRequest(Agent,is_retry))
	{
		printf("Send CONFIG_REQUEST FAILED\n");
		return WTP_FALSE;
	}
	
	pAgentStateVariables->WTPDataCheckStateVariables.numberOfSend++;

	wtp_add_task(CW_COMMON_TIME_OFFSET, WTP_JOB_EVENT_CHANGE_REQUEST,(void *)WTPExecuteChangeStateRequest,NULL, Agent);
	
	return WTP_TRUE;
}

WTPBool WTPParseConfigResponseMessage(unsigned char *msg, int msgoffset,int len,CWProtocolConfigResponseValues *configrespvalue,struct VcapwapAgent* Agent)
{

	CWControlHeaderValues	controlVal;
	CWProtocolMessage	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) {
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				if(!(WTPParseResultCode(&completeMsg, len,&configrespvalue->result_code))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				if(!WTPParsePayloadMsgElem(&completeMsg, len, Agent)) return WTP_FALSE;
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset - offsetTillMessages !=  controlVal.msgElemsLen) 
	{ 
		return WTP_FALSE;
	}
	return WTP_TRUE;
}



WTPBool WTPSaveConfigResponseMessage(CWProtocolConfigResponseValues *configrespvalue,struct VcapwapAgent* Agent)
{
	return WTP_TRUE;
}

WTPBool WTPProcessConfigResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize)
{

	struct WTPStateVariables* pAgentStateVariables = (struct WTPStateVariables* ) &(Agent->stateVariables);
	CWProtocolConfigResponseValues configrespvalue ;


	printf("Current state: %s\n",STATE_ID_TO_STR(Agent->currentState));
	if(Agent->currentState != CW_CONFIGURE) 
	{
		printf("Not in Discover State, Drop Msg\n");
		return WTP_TRUE;
	}
	if(!(WTPParseConfigResponseMessage(recvMsg.msg, recvMsg.offset, msgSize, &configrespvalue, Agent)) ||
		!(WTPSaveConfigResponseMessage(&configrespvalue,Agent)))
	{
		return WTP_FALSE;
	}

	wtp_delete_first_task_by_id(WTP_JOB_CONFIG_REQUEST);

	WTPClearStateVariables(pAgentStateVariables);
	
	Agent->currentState = CW_DATA_CHECK;
	
	if(!(WTPExecuteChangeStateRequest(Agent))){
		return WTP_FALSE;
	}
	
	return WTP_TRUE;
}

WTPBool WTPAssembleConfigRequest(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, unsigned char seqNum)
{
	CWProtocolMessage	*msgElems= NULL;
	const int 		msgElemCount = 1;
	CWProtocolMessage 	*msgElemsBinding= NULL;
	const int 		msgElemBindingCount=0;
	int 			k = 0,i=0;
	
	if(messagesPtr == NULL || fragmentsNumPtr == NULL)
	{
		return WTP_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount,return WTP_FALSE;);
	
	if((!(WTPAssemblePayloadMsgElemBaseMac(&(msgElems[k++]),Agent))))
	{
		
		for(i = 0; i < k; i++)
		{ 
			CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
		}
		CW_FREE_OBJECT(msgElems);
		/* error will be handled by the caller */
		return  WTP_FALSE;
	}
	
	return CWAssembleMessage(messagesPtr,fragmentsNumPtr,PMTU,seqNum,
				CW_MSG_TYPE_VALUE_CONFIGURE_REQUEST, 
				msgElems,
				msgElemCount,
				msgElemsBinding,
				msgElemBindingCount,
#ifndef CW_DTLS
				CW_PACKET_PLAIN
#else
				CW_PACKET_CRYPT
#endif
				 );

}

WTPBool WTPAssembleImageDataRequest(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, unsigned char seqNum)
{
	CWProtocolMessage	*msgElems= NULL;
	const int 		msgElemCount = 4;
	CWProtocolMessage 	*msgElemsBinding= NULL;
	const int 		msgElemBindingCount=0;
	int 			k = 0,i=0;
	
	if(messagesPtr == NULL || fragmentsNumPtr == NULL)
	{
		return WTP_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount,return WTP_FALSE;);
	
	if((!(WTPAssemblePayloadMsgElemBaseMac(&(msgElems[k++]),Agent)))||
 	 	(!(WTPAssemblePayloadMsgElemWtpModel(&(msgElems[k++]),Agent)))||
 		(!(WTPAssemblePayloadMsgElemSoftwareVersion(&(msgElems[k++]),Agent)))||
 		(!(WTPAssemblePayloadMsgElemBootVersion(&(msgElems[k++]),Agent))))
	{
		
		for(i = 0; i < k; i++)
		{ 
			CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
		}
		CW_FREE_OBJECT(msgElems);
		/* error will be handled by the caller */
		return  WTP_FALSE;
	}
	
	return CWAssembleMessage(messagesPtr,fragmentsNumPtr,PMTU,seqNum,
				CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST, 
				msgElems,
				msgElemCount,
				msgElemsBinding,
				msgElemBindingCount,
#ifndef CW_DTLS
				CW_PACKET_PLAIN
#else
				CW_PACKET_CRYPT
#endif
				 );

}


WTPBool WTPSendConfigRequest(struct VcapwapAgent* Agent, WTPBool  is_retry)
{
	int PMTU;
	unsigned char seqNum;
	int fragmentsNum;
	CWProtocolMessage *messagesPtr = NULL;
	
	seqNum = WTPSetSeqNum(Agent,is_retry);
	
	PMTU = CW_MTU;
	
	WTPAssembleConfigRequest(Agent,&messagesPtr, &fragmentsNum, PMTU, seqNum);

	wtp_send_msg_to_ac(Agent,&messagesPtr,fragmentsNum);

	return WTP_TRUE;
	
}

WTPBool WTPExecuteConfigRequest(void* argument)
{
	struct VcapwapAgent* Agent = argument;
	WTPStateVariables* pAgentStateVariables = &(Agent->stateVariables);
	WTPBool is_retry = WTP_FALSE;
	
	printf("Execute CONFIG_RESQUEST current state %s send %d\n",STATE_ID_TO_STR(Agent->currentState),pAgentStateVariables->WTPConfigStateVariables.numberOfSend);
		
	if(Agent->currentState != CW_CONFIGURE)
	{
		printf("Not in CONFIG State, Drop job\n");
		return WTP_TRUE;
	}

	if(pAgentStateVariables->WTPConfigStateVariables.numberOfSend > CW_RETRY_LIMIT) 
	{
		printf("Execute CONFIG_RESQUEST Retry Exceed\n");
		APAssocACStatusSet(AP_DISASSOC_AC);
		WTPInitState(Agent);
		return WTP_TRUE;
	}
	
	if(pAgentStateVariables->WTPConfigStateVariables.numberOfSend > 0)
	{
		is_retry = WTP_TRUE ;
	}
	
	if(!WTPSendConfigRequest(Agent,is_retry))
	{
		printf("Send CONFIG_REQUEST FAILED\n");
		return WTP_FALSE;
	}
	
	pAgentStateVariables->WTPConfigStateVariables.numberOfSend++;

	wtp_add_task(CW_COMMON_TIME_OFFSET, WTP_JOB_CONFIG_REQUEST,(void *)WTPExecuteConfigRequest,NULL, Agent);
	
	return WTP_TRUE;
}

WTPBool WTPSendImageDataRequest(struct VcapwapAgent* Agent, WTPBool  is_retry)
{
	int PMTU;
	unsigned char seqNum;
	int fragmentsNum;
	CWProtocolMessage *messagesPtr = NULL;
	
	seqNum = WTPSetSeqNum(Agent,is_retry);
	
	PMTU = CW_MTU;
	
	WTPAssembleImageDataRequest(Agent,&messagesPtr, &fragmentsNum, PMTU, seqNum);

	wtp_send_msg_to_ac(Agent,&messagesPtr,fragmentsNum);

	return WTP_TRUE;
	
}


WTPBool WTPExecuteImageDataRequest(void* argument)
{
	struct VcapwapAgent* Agent = argument;
	WTPStateVariables* pAgentStateVariables = &(Agent->stateVariables);
	WTPBool is_retry = WTP_FALSE;
	
	printf("Execute ImageDataRequest current state %s send %d\n",STATE_ID_TO_STR(Agent->currentState),pAgentStateVariables->WTPImageDataVariables.numberOfSend);
		
	if(Agent->currentState != CW_UPGRADING)
	{
		printf("Not in CW_UPGRADING State, Drop job\n");
		return WTP_TRUE;
	}

	if(pAgentStateVariables->WTPImageDataVariables.numberOfSend > CW_RETRY_LIMIT) 
	{
		printf("Execute CW_UPGRADING Retry Exceed\n");
		APAssocACStatusSet(AP_DISASSOC_AC);
		WTPInitState(Agent);
		return WTP_TRUE;
	}
	
	if(pAgentStateVariables->WTPImageDataVariables.numberOfSend > 0)
	{
		is_retry = WTP_TRUE ;
	}
	
	if(!WTPSendImageDataRequest(Agent,is_retry))
	{
		printf("Send CW_UPGRADING FAILED\n");
		return WTP_FALSE;
	}
	
	pAgentStateVariables->WTPConfigStateVariables.numberOfSend++;

	wtp_add_task(CW_COMMON_TIME_OFFSET, WTP_JOB_IMAGE_DATA_REQUEST,(void *)WTPExecuteImageDataRequest,NULL, Agent);
	
	return WTP_TRUE;
}



WTPBool WTPParseJoinResponseMessage(unsigned char *msg, int msgoffset,int len,CWProtocolJoinResponseValues *joinrespvalue,struct VcapwapAgent* Agent)
{
	CWControlHeaderValues 	controlVal;
	CWProtocolMessage 	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) {
			case CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE:
				if(!(WTPParseResultCode(&completeMsg, len,&joinrespvalue->result_code))) 
				{
					return WTP_FALSE;
				}
				break;
			case CW_MSG_ELEMENT_AC_NAME_CW_TYPE:
				if(len>CW_OPTION_STR_LEN)
				{
					return WTP_FALSE;
				}
				if(!(WTPParseACName(&completeMsg, len,Agent))) 
				{
					return WTP_FALSE;
				}
				break;
			case CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE:
				if(!(WTPParseImageIdentifier(&completeMsg, len,&Agent->imageidentifier)))
				{
					return WTP_FALSE;
				}
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset  -  offsetTillMessages!= controlVal.msgElemsLen) 
	{ 
		return WTP_FALSE;
	}
	
	return WTP_TRUE;
}


WTPBool WTPSaveJoinResponseMessage(CWProtocolJoinResponseValues *joinrespvalue,struct VcapwapAgent* Agent)
{
	//Agent->imageidentifier = joinrespvalue->imageidentifier;
	//strcpy(Agent->ac_name,(char *)joinrespvalue->ac_name);
	
	return WTP_TRUE;
}

WTPBool WTPProcessJoinResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize)
{

	struct WTPStateVariables* pAgentStateVariables = (struct WTPStateVariables* ) &(Agent->stateVariables);
	CWProtocolJoinResponseValues joinrespvalue ;
	
	if(Agent->currentState != CW_JOIN) 
	{
		printf("Not in Join State, Drop Msg\n");
		return WTP_TRUE;
	}
	if(!(WTPParseJoinResponseMessage(recvMsg.msg, recvMsg.offset, msgSize, &joinrespvalue, Agent)) ||
		!(WTPSaveJoinResponseMessage(&joinrespvalue,Agent)))
	{
		return WTP_FALSE;
	}

	wtp_delete_first_task_by_id(WTP_JOB_JOIN_REQUEST);

	WTPClearStateVariables(pAgentStateVariables);

	if(Agent->imageidentifier != INIT_IDENTIIER)
	{
		Agent->currentState = CW_UPGRADING;
		
		if(!(WTPExecuteImageDataRequest(Agent))){
			return WTP_FALSE;
		}
		printf("Agent->currentState %d\n",Agent->currentState);
		return WTP_TRUE;
	}
	
	Agent->currentState = CW_CONFIGURE;
	
	if(!(WTPExecuteConfigRequest(Agent))){
		return WTP_FALSE;
	}
	
	return WTP_TRUE;
}


WTPBool WTPAssembleJoinRequest(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, unsigned char seqNum)
{
	CWProtocolMessage	*msgElems= NULL;
	const int 		msgElemCount = 9;
	CWProtocolMessage 	*msgElemsBinding= NULL;
	const int 		msgElemBindingCount=0;
	int 			k = 0,i=0;
	
	if(messagesPtr == NULL || fragmentsNumPtr == NULL)
	{
		return WTP_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount,return WTP_FALSE;);
	
	if ( (!(CWAssembleMsgElemWTPIPv4Address(&(msgElems[k++]),Agent)))||
 		  (!(CWAssembleMsgElemWTPLocationData(&(msgElems[k++]),Agent)))||
 		  (!(WTPAssemblePayloadMsgElemUptime(&(msgElems[k++]))))||
 		   (!(WTPAssemblePayloadMsgElemBaseMac(&(msgElems[k++]),Agent)))||
 		    (!(WTPAssemblePayloadMsgElemWtpModel(&(msgElems[k++]),Agent)))||
 		     (!(WTPAssemblePayloadMsgElemSoftwareVersion(&(msgElems[k++]),Agent)))||
 		      (!(WTPAssemblePayloadMsgElemBootVersion(&(msgElems[k++]),Agent)))||
 		       (!(WTPAssemblePayloadMsgElemRadioType(&(msgElems[k++]),Agent))) ||
 		       (!(CWAssembleMsgElemWTPName(&(msgElems[k++]),Agent)))
			)
	{
		
		for(i = 0; i < k; i++)
		{ 
			CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
		}
		CW_FREE_OBJECT(msgElems);
		/* error will be handled by the caller */
		return  WTP_FALSE;
	}
	
	return CWAssembleMessage(messagesPtr,fragmentsNumPtr,PMTU,seqNum,
				CW_MSG_TYPE_VALUE_JOIN_REQUEST, 
				msgElems,
				msgElemCount,
				msgElemsBinding,
				msgElemBindingCount,
#ifndef CW_DTLS
				CW_PACKET_PLAIN
#else
				CW_PACKET_CRYPT
#endif
				 );

}


WTPBool WTPSendJoinRequest(struct VcapwapAgent* Agent, WTPBool  is_retry)
{
	int PMTU;
	unsigned char seqNum;
	int fragmentsNum;
	CWProtocolMessage *messagesPtr = NULL;
	
	seqNum = WTPSetSeqNum(Agent,is_retry);
	
	PMTU = CW_MTU;
	
	WTPAssembleJoinRequest(Agent,&messagesPtr, &fragmentsNum, PMTU, seqNum);

	wtp_send_msg_to_ac(Agent,&messagesPtr,fragmentsNum);

	return WTP_TRUE;
	
}

WTPBool WTPExecuteJoinRequest(void* argument)
{
	struct VcapwapAgent* Agent = argument;
	WTPStateVariables* pAgentStateVariables = &(Agent->stateVariables);
	WTPBool is_retry = WTP_FALSE;
	
	printf("Execute JOIN_REQUEST current state %s send %d\n",STATE_ID_TO_STR(Agent->currentState),pAgentStateVariables->WTPJoinStateVariables.numberOfSend);
		
	if(Agent->currentState != CW_JOIN)
	{
		printf("Not in Join State, Drop job\n");
		return WTP_TRUE;
	}

	if(pAgentStateVariables->WTPJoinStateVariables.numberOfSend > CW_RETRY_LIMIT) 
	{
		printf("Execute JOIN_REQUEST Retry Exceed\n");
		APAssocACStatusSet(AP_DISASSOC_AC);
		WTPInitState(Agent);
        return WTP_TRUE;
	}
	
	if(pAgentStateVariables->WTPJoinStateVariables.numberOfSend > 0)
	{
		is_retry = WTP_TRUE ;
	}
	
	if(!WTPSendJoinRequest(Agent,is_retry))
	{
		printf("Send JOIN_REQUEST FAILED\n");
		return WTP_FALSE;
	}
	
	pAgentStateVariables->WTPJoinStateVariables.numberOfSend++;
	
	wtp_add_task(CW_COMMON_TIME_OFFSET, WTP_JOB_JOIN_REQUEST,(void *)WTPExecuteJoinRequest,NULL, Agent);
	
	return WTP_TRUE;
}


WTPBool WTPParseDiscoverResponseMessage(unsigned char *msg, int msgoffset,int len,struct VcapwapAgent* Agent)
{

	CWControlHeaderValues 	controlVal;
	CWProtocolMessage 	completeMsg;
	int 			offsetTillMessages;
	
	if (msg == NULL) 
	{
		return WTP_FALSE;
	}

	completeMsg.msg = msg;
	completeMsg.offset = msgoffset;
	
	
	/* skip timestamp */
	controlVal.msgElemsLen =len - CW_TRANSPORT_HEADER_RESERVED_LEN;
	
	offsetTillMessages = completeMsg.offset;

	while((completeMsg.offset-offsetTillMessages) < controlVal.msgElemsLen) 
	{
		unsigned short int type=0;
		unsigned short int len=0;
		
		CWParseFormatMsgElem(&completeMsg,&type,&len);
		switch(type) {
			case CW_MSG_ELEMENT_AC_NAME_CW_TYPE:
				if(!(WTPParseACName(&completeMsg, len,Agent))) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE:
				if(!WTPParsePayloadMsgElem(&completeMsg, len, Agent)) return WTP_FALSE;
				break;
			case CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE:
				if(!WTPParseControlIPv4Addresses(&completeMsg, len, Agent)) return WTP_FALSE;
				break;
			default:
				(completeMsg.offset) += len;
				break;
		}
	}
	
	if(completeMsg.offset - offsetTillMessages != controlVal.msgElemsLen) 
	{ 
		return WTP_FALSE;
	}
	return WTP_TRUE;
}

WTPBool WTPProcessDiscoverResponse(struct VcapwapAgent* Agent,CWProtocolMessage recvMsg, unsigned int msgSize)
{
	struct WTPStateVariables* pAgentStateVariables = (struct WTPStateVariables* ) &(Agent->stateVariables);
	 
	printf("Current state: %s\n",STATE_ID_TO_STR(Agent->currentState));
	if(Agent->currentState != CW_DISCOVER) 
	{
		printf("Not in Discover State, Drop Msg\n");
		return WTP_TRUE;
	}

	
	if(!(WTPParseDiscoverResponseMessage(recvMsg.msg, recvMsg.offset, msgSize, Agent)))
	{
		return WTP_FALSE;
	}
	
	wtp_delete_first_task_by_id(WTP_JOB_DISCOVER_REQUEST);

	WTPClearStateVariables(pAgentStateVariables);

	Agent->currentState = CW_JOIN;
	
	if(!(WTPExecuteJoinRequest(Agent))){
		return WTP_FALSE;
	}
	
	return WTP_TRUE;
}


WTPBool WTPAssembleDiscoveryRequest(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, unsigned char seqNum)
{
	CWProtocolMessage	*msgElems= NULL;
	const int 		msgElemCount = 6;
	CWProtocolMessage 	*msgElemsBinding= NULL;
	const int 		msgElemBindingCount=0;
	int 			k = 0,i=0;
	
	if(messagesPtr == NULL || fragmentsNumPtr == NULL)
	{
		return WTP_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems, msgElemCount,return WTP_FALSE;);
	
	if ((!(CWAssembleMsgElemDiscoveryType(&(msgElems[k++]))))||
 		 (!(CWAssembleMsgElemWTPFrameTunnelMode(&(msgElems[k++]))))||
 		 (!(CWAssembleMsgElemWTPMACType(&(msgElems[k++]))))||
 		 (!(WTPAssemblePayloadMsgElemBaseMac(&(msgElems[k++]),Agent)))||
 		 (!(CWAssembleMsgElemWTPIPv4Address(&(msgElems[k++]),Agent)))||
 		 (!(WTPAssemblePayloadMsgElemTrustId(&(msgElems[k++]),Agent))))
	{
		
		for(i = 0; i < k; i++)
		{ 
			CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);
		}
		CW_FREE_OBJECT(msgElems);
		/* error will be handled by the caller */
		return  WTP_FALSE;
	}
	
	return CWAssembleMessage(messagesPtr,fragmentsNumPtr,PMTU,seqNum,
				CW_MSG_TYPE_VALUE_DISCOVERY_REQUEST, 
				msgElems,
				msgElemCount,
				msgElemsBinding,
				msgElemBindingCount,
#ifndef CW_DTLS
				CW_PACKET_PLAIN
#else
				CW_PACKET_CRYPT
#endif
				 );

}



WTPBool WTPSendDiscoverRequest(struct VcapwapAgent *Agent)
{
	int PMTU;
	unsigned char seqNum;
	int fragmentsNum;
	CWProtocolMessage *messagesPtr = NULL;
	
	seqNum = WTPSetSeqNum(Agent,WTP_FALSE);
	
	PMTU = CW_MTU;
	
	WTPAssembleDiscoveryRequest(Agent,&messagesPtr, &fragmentsNum, PMTU, seqNum);

	wtp_send_msg_to_ac(Agent,&messagesPtr,fragmentsNum);

	return WTP_TRUE;
}

WTPBool WTPBroadcastDiscoverySocket(struct VcapwapAgent *Agent)
{
	int nReuseFlag = 1;

	if(setsockopt(Agent->master_socket.socket, SOL_SOCKET, SO_BROADCAST, &nReuseFlag, sizeof(int)) < 0)
	{ 
		printf("WTPBroadcastDiscovery:Set socket option	failed\n "); 
		close(Agent->master_socket.socket); 
		return WTP_FALSE; 
	}

	inet_aton("255.255.255.255", &(Agent->master_socket.address.sin_addr));

	return WTP_TRUE;
}

WTPBool WTPUnicastcastDiscoverySocket(struct VcapwapAgent *Agent)
{
	int nReuseFlag = 1;

	if(setsockopt(Agent->master_socket.socket, SOL_SOCKET, SO_REUSEADDR, &nReuseFlag, sizeof(int)) < 0)
	{ 
		printf("WTPBroadcastDiscovery:Set socket option	failed\n "); 
		close(Agent->master_socket.socket); 
		return WTP_FALSE; 
	}

	inet_aton(Agent->ac_ip, &(Agent->master_socket.address.sin_addr));

	return WTP_TRUE;
}


WTPBool WTPExcuteDiscoveryRequest(struct VcapwapAgent *Agent)
{
	WTPStateVariables* pAgentStateVariables = &(Agent->stateVariables);
	
	printf("Execute DISCOVER_REQUEST current state %s send %d\n",STATE_ID_TO_STR(Agent->currentState),pAgentStateVariables->WTPDiscoverStateVariables.numberOfSend);

	if(Agent->currentState != CW_DISCOVER) 
	{
		printf("Not in Discover State, Drop job\n");
		return WTP_TRUE;
	}
	
	if(pAgentStateVariables->WTPDiscoverStateVariables.numberOfSend > CW_RETRY_LIMIT) 
	{
		printf("Execute DISCOVER_REQUEST Retry Exceed\n");
		WTPDisconnectReason(DISCONNECT_REASON_LINKDOWN);
		pAgentStateVariables->WTPDiscoverStateVariables.numberOfSend = 0;
		Agent->need_broadcast = WTP_TRUE;
	}

	if(Agent->need_broadcast == WTP_TRUE)
	{
		WTPBroadcastDiscoverySocket(Agent);
	}

	if(!WTPSendDiscoverRequest(Agent))
	{
		printf("Send DISCOVER_REQUEST FAILED\n");
		return WTP_FALSE;
	}

	if(Agent->need_broadcast == WTP_TRUE)
	{
		Agent->need_broadcast = WTP_FALSE;
		WTPUnicastcastDiscoverySocket(Agent);
		Agent->broadcast_count++;
	}

	if(Agent->broadcast_count > CW_RETRY_LIMIT)
	{
		Agent->broadcast_count = 0;
		wtp_renew_ip();
	}
	
	pAgentStateVariables->WTPDiscoverStateVariables.numberOfSend++;
	
	wtp_add_task(Agent->Agenttimer.discoveryTimer, WTP_JOB_DISCOVER_REQUEST,(void *)WTPExcuteDiscoveryRequest,NULL, Agent);

	return WTP_TRUE;
}

#if 0
WTPBool WTPAliveCheck(struct VcapwapAgent *Agent)
{
	char  wtp_ip[CW_IP_STR_LEN] = {0};
	
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",wtp_ip);
	
	if(strlen(wtp_ip)== 0 || !strcmp(Agent->wtp_ip,"0.0.0.0")||strcmp(wtp_ip,Agent->wtp_ip))
	{
		system("/sbin/reboot");
		return WTP_TRUE;
	}

	wtp_add_task(600, WTP_JOB_ALIVE_CHECK,(void *)WTPAliveCheck,NULL, Agent);

	return WTP_TRUE;
}
#endif

WTPBool WTPInitState(struct VcapwapAgent* Agent)
{

	wtp_task_queen_destory();

	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",Agent->wtp_ip);
	
	printf("WTPInitState Go to Discover State\n");

	//wtp_add_task(600, WTP_JOB_ALIVE_CHECK,(void *)WTPAliveCheck,NULL, Agent);
	
	Agent->currentState = CW_DISCOVER;

	WTPExcuteDiscoveryRequest(Agent);

	return WTP_TRUE;
}

