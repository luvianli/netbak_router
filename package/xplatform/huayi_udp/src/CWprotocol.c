/*******************************************************************************************
 * Project:  huayi_capwap_server                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "CWprotocol.h"

int CWGetFragmentID() {
	static int fragID = 0;
	return fragID++;
}

LIBBool CWParseFormatMsgElem(CWProtocolMessage *completeMsg,unsigned short int *type,unsigned short int *len)
{
	*type = CWProtocolRetrieve16(completeMsg);
	*len = CWProtocolRetrieve16(completeMsg);
	return LIB_TRUE;
}

LIBBool CWParseCWHeader(CWProtocolMessage *msgPtr, CWControlHeaderValues *valPtr)
{
	/*TO DO: Return Msg Response Type 
			 Fill in msg size*/
	/*CWResponseMsg respondMsgType = CW_ERROR_RSP;*/
	unsigned char flags=0;
	
	if(msgPtr == NULL|| valPtr == NULL) 
	{
		return LIB_FALSE;
	}
	
	(msgPtr->offset) += 8;

	valPtr->messageTypeValue = CWProtocolRetrieve32(msgPtr);

	valPtr->seqNum = CWProtocolRetrieve8(msgPtr);
	
	valPtr->msgElemsLen = CWProtocolRetrieve16(msgPtr);

	flags=CWProtocolRetrieve8(msgPtr);

	return LIB_TRUE;
}

LIBBool CWParseNoAction(CWProtocolMessage *msgPtr,int len)
{
	CWParseMessageElementStartWithoutvalPtr();
	
	msgPtr->offset += len- CW_IEEE80211_PAYLOAD_HEAD_SIZE ;
	
	CWParseMessageElementEnd();
}


// Assembles the Control Header
LIBBool CWAssembleControlHeader(CWProtocolMessage *controlHdrPtr, CWControlHeaderValues *valPtr) {
	if(controlHdrPtr == NULL || valPtr == NULL) return LIB_FALSE;
	
	CW_CREATE_PROTOCOL_MESSAGE(*controlHdrPtr, 8,	 // meaningful bytes of the header
						return LIB_FALSE;);
	
/*	CWProtocolStore32(controlHdrPtr, gCWIANATimes256 + valPtr->messageTypeValue); // = IANA Enterprise Number * 256 + Message Type Value*/
	CWProtocolStore32(controlHdrPtr, valPtr->messageTypeValue); 
	CWProtocolStore8(controlHdrPtr, valPtr->seqNum);
	CWProtocolStore16(controlHdrPtr, (CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS+(valPtr->msgElemsLen))); // 7 is for the next 8+32+16 bits (= 7 bytes), MessageElementsLength+flags + timestamp 
	CWProtocolStore8(controlHdrPtr, 0); // flags
	//CWProtocolStore32(controlHdrPtr, ((unsigned int)(time(NULL))) ); // timestamp
	
	return LIB_TRUE;
}

// Assembles the Transport Header
LIBBool CWAssembleTransportHeader(CWProtocolMessage *transportHdrPtr, CWProtocolTransportHeaderValues *valuesPtr) {
	unsigned int val = 0;
	if(transportHdrPtr == NULL || valuesPtr == NULL) return LIB_FALSE;
	
	if(valuesPtr->bindingValuesPtr != NULL)
		{CW_CREATE_PROTOCOL_MESSAGE(*transportHdrPtr,MAXCAPWAPHEADERSIZEBINDING, return LIB_FALSE;);}
	else {CW_CREATE_PROTOCOL_MESSAGE(*transportHdrPtr,8 , return LIB_FALSE;);}	 // meaningful bytes of the header (no wirless header and MAC address)
	CWSetField32(val, 
		     CW_TRANSPORT_HEADER_VERSION_START,
		     CW_TRANSPORT_HEADER_VERSION_LEN,
		     CW_PROTOCOL_VERSION); // current version of CAPWAP

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_TYPE_START,
		     CW_TRANSPORT_HEADER_TYPE_LEN,
		     (valuesPtr->payloadType == CW_PACKET_PLAIN) ? 0 : 1);
	
	if(valuesPtr->bindingValuesPtr != NULL)
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_HLEN_START,
			     CW_TRANSPORT_HEADER_HLEN_LEN,
			     CW_BINDING_HLEN);
	else 
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_HLEN_START,
			     CW_TRANSPORT_HEADER_HLEN_LEN,
			     2);

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RID_START,
		     CW_TRANSPORT_HEADER_RID_LEN,
		     1); // only one radio per WTP?
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_WBID_START,
		     CW_TRANSPORT_HEADER_WBID_LEN,
		     1); // Wireless Binding ID
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_T_START,
		     CW_TRANSPORT_HEADER_T_LEN,
		     0);

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_F_START,
		     CW_TRANSPORT_HEADER_F_LEN,
		     0); // is fragment
		     //valuesPtr->isFragment); // is fragment

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_L_START,
		     CW_TRANSPORT_HEADER_L_LEN,
		     valuesPtr->last); // last fragment
	
	if(valuesPtr->bindingValuesPtr != NULL)
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_W_START,
			     CW_TRANSPORT_HEADER_W_LEN,
			     1); //wireless header
	else 
		CWSetField32(val,
			     CW_TRANSPORT_HEADER_W_START,
			     CW_TRANSPORT_HEADER_W_LEN,
			     0);

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_M_START,
		     CW_TRANSPORT_HEADER_M_LEN,
		     0); // no radio MAC address

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_K_START,
		     CW_TRANSPORT_HEADER_K_LEN,
		     0); // Keep alive flag

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FLAGS_START,
		     CW_TRANSPORT_HEADER_FLAGS_LEN,
		     0); // required

	CWProtocolStore32(transportHdrPtr, val);
	// end of first 32 bits
	
	val = 0;

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN,
		     valuesPtr->fragmentID); // fragment ID
	
	CWSetField32(val,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START,
		     CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN,
		     valuesPtr->fragmentOffset); // fragment offset

	CWSetField32(val,
		     CW_TRANSPORT_HEADER_RESERVED_START,
		     CW_TRANSPORT_HEADER_RESERVED_LEN,
		     0); // required

	CWProtocolStore32(transportHdrPtr, val);
	// end of second 32 bits
	return LIB_TRUE;
}



// Assemble a CAPWAP Control Packet, with the given Message Elements, Sequence Number and Message Type. Create Transport and Control Headers.
// completeMsgPtr is an array of fragments (can be of size 1 if the packet doesn't need fragmentation
LIBBool CWAssembleMessage(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr, int PMTU, unsigned char seqNum, int msgTypeValue, CWProtocolMessage *msgElems, const int msgElemNum, CWProtocolMessage *msgElemsBinding, const int msgElemBindingNum,int is_crypted)
{
	CWProtocolMessage transportHdr, controlHdr, msg;
	int msgElemsLen = 0;
	int i;
	
	CWProtocolTransportHeaderValues transportVal;
	CWControlHeaderValues controlVal;
	
	if(completeMsgPtr == NULL || fragmentsNumPtr == NULL || (msgElems == NULL && msgElemNum > 0) || (msgElemsBinding == NULL && msgElemBindingNum > 0)) return LIB_FALSE;
	
	//Calculate the whole size of the Msg Elements	
	for(i = 0; i < msgElemNum; i++) msgElemsLen += msgElems[i].offset;
	for(i = 0; i < msgElemBindingNum; i++) msgElemsLen += msgElemsBinding[i].offset;

	//Assemble Control Header
	controlVal.messageTypeValue = msgTypeValue;
	controlVal.msgElemsLen = msgElemsLen;
	controlVal.seqNum = seqNum;
	
	if(!(CWAssembleControlHeader(&controlHdr, &controlVal))) {
		CW_FREE_PROTOCOL_MESSAGE(controlHdr);
		for(i = 0; i < msgElemNum; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
		CW_FREE_OBJECT(msgElems);
		for(i = 0; i < msgElemBindingNum; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElemsBinding[i]);}
		CW_FREE_OBJECT(msgElemsBinding);
		return LIB_FALSE; // will be handled by the caller
	}
	
	// assemble the message putting all the data consecutively
	CW_CREATE_PROTOCOL_MESSAGE(msg, controlHdr.offset + msgElemsLen, return LIB_FALSE;);
	
	CWProtocolStoreMessage(&msg, &controlHdr);
	for(i = 0; i < msgElemNum; i++) { // store in the request all the Message Elements
		CWProtocolStoreMessage(&msg, &(msgElems[i]));
	}
	for(i = 0; i < msgElemBindingNum; i++) { // store in the request all the Message Elements
		CWProtocolStoreMessage(&msg, &(msgElemsBinding[i]));
	}

	//Free memory not needed anymore
	CW_FREE_PROTOCOL_MESSAGE(controlHdr);
	for(i = 0; i < msgElemNum; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElems[i]);}
	CW_FREE_OBJECT(msgElems);
	for(i = 0; i < msgElemBindingNum; i++) { CW_FREE_PROTOCOL_MESSAGE(msgElemsBinding[i]);}
	CW_FREE_OBJECT(msgElemsBinding);
	
	// handle fragmentation
	PMTU = PMTU - MAX_DTLS_HEADER_SIZE - MAX_CAPWAP_HEADER_SIZE;
	
	if(PMTU > 0) {
		PMTU = (PMTU/8)*8; // CAPWAP fragments are made of groups of 8 bytes
		if(PMTU == 0) goto cw_dont_fragment;
		
		*fragmentsNumPtr = msg.offset / PMTU;
		if((msg.offset % PMTU) != 0) (*fragmentsNumPtr)++;
	} else {
	cw_dont_fragment:
		*fragmentsNumPtr = 1;
	}
	
	transportVal.bindingValuesPtr = NULL;
		
	if(*fragmentsNumPtr == 1) {

		CW_CREATE_OBJECT_ERR(*completeMsgPtr, CWProtocolMessage, return LIB_FALSE;);
		
		transportVal.isFragment = transportVal.last = transportVal.fragmentOffset = transportVal.fragmentID = 0;
		transportVal.payloadType = is_crypted;
//		transportVal.last = 1;

		// Assemble Message Elements
		if	(!(CWAssembleTransportHeader(&transportHdr, &transportVal))) {
			CW_FREE_PROTOCOL_MESSAGE(msg);
			CW_FREE_PROTOCOL_MESSAGE(transportHdr);
			CW_FREE_OBJECT(*completeMsgPtr);
			return LIB_FALSE; // will be handled by the caller
		} 
	
		// assemble the message putting all the data consecutively
		CW_CREATE_PROTOCOL_MESSAGE(((*completeMsgPtr)[0]), transportHdr.offset + msg.offset, return LIB_FALSE;);
		
		CWProtocolStoreMessage(&((*completeMsgPtr)[0]), &transportHdr);
		CWProtocolStoreMessage(&((*completeMsgPtr)[0]), &msg);
		
		CW_FREE_PROTOCOL_MESSAGE(transportHdr);
		CW_FREE_PROTOCOL_MESSAGE(msg);
	} else {
		int totalSize;
		int fragID = CWGetFragmentID();
		totalSize = msg.offset;
		
		CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(*completeMsgPtr, *fragmentsNumPtr, return LIB_FALSE;);
		msg.offset = 0;
	
		for(i = 0; i < *fragmentsNumPtr; i++) { // for each fragment to assemble
			int fragSize;
			
			transportVal.isFragment = 1;
			transportVal.fragmentOffset = msg.offset / 8;
			transportVal.fragmentID = fragID;
			transportVal.payloadType = is_crypted;

			if(i < ((*fragmentsNumPtr)-1)) { // not last fragment
				fragSize = PMTU;
				transportVal.last = 0;
			} else { // last fragment
				fragSize = totalSize - (((*fragmentsNumPtr)-1) * PMTU);
				transportVal.last = 1;
			}
			
			// Assemble Transport Header for this fragment
			if(!(CWAssembleTransportHeader(&transportHdr, &transportVal))) {
				CW_FREE_PROTOCOL_MESSAGE(msg);
				CW_FREE_PROTOCOL_MESSAGE(transportHdr);
				CW_FREE_OBJECT(*completeMsgPtr);
				return LIB_FALSE; // will be handled by the caller
			}

			CW_CREATE_PROTOCOL_MESSAGE(((*completeMsgPtr)[i]), transportHdr.offset + fragSize, return LIB_FALSE;);
			
			CWProtocolStoreMessage(&((*completeMsgPtr)[i]), &transportHdr);
			CWProtocolStoreRawBytes(&((*completeMsgPtr)[i]),(char *)&((msg.msg)[msg.offset]), fragSize);
			msg.offset += fragSize;
			
			CW_FREE_PROTOCOL_MESSAGE(transportHdr);
		}
		CW_FREE_PROTOCOL_MESSAGE(msg);
	}
	
	return LIB_TRUE;
}

