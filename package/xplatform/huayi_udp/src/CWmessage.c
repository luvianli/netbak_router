
#include "CWmessage.h"

void print_mem (unsigned char *p, int k)
{
	int i;
	for (i = 0; i < k; i++)
	{
		printf("%02x ", *p++);

		if ((i + 1) % 16 == 0) printf("\n");
	}

	printf("\n");

	return;
}


void print_addr(struct sockaddr_in *addr)
{
	printf("addr:%s:%d\n",inet_ntoa(addr->sin_addr),ntohs(addr->sin_port));
	return;
}

// retrieves 8 bits from the message, increments the current offset in bytes.
unsigned char CWProtocolRetrieve8(CWProtocolMessage *msgPtr) {
	unsigned char val;
	
	CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 1);
	(msgPtr->offset) += 1;
	
	return val;
}

// retrieves 16 bits from the message, increments the current offset in bytes.
unsigned short CWProtocolRetrieve16(CWProtocolMessage *msgPtr) {
	unsigned short val;
	CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 2);
	(msgPtr->offset) += 2;
	
	return ntohs(val);
}

// retrieves 32 bits from the message, increments the current offset in bytes.
unsigned int CWProtocolRetrieve32(CWProtocolMessage *msgPtr) {
	unsigned int val;
	
	CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 4);
	(msgPtr->offset) += 4;
	
	return ntohl(val);
}

// retrieves 64 bits from the message, increments the current offset in bytes.
unsigned long CWProtocolRetrieve64(CWProtocolMessage *msgPtr) {
	unsigned long val;
	
	CW_COPY_MEMORY(&val, &((msgPtr->msg)[(msgPtr->offset)]), 8);
	(msgPtr->offset) += 8;
	
	return ntohl(val);
}

// retrieves a string (not null-terminated) from the message, increments the current offset in bytes.
// Adds the '\0' char at the end of the string which is returned
void CWProtocolRetrieveStr(CWProtocolMessage *msgPtr, int len, unsigned char *str) {
	CW_COPY_MEMORY(str, &((msgPtr->msg)[(msgPtr->offset)]), len);
	str[len] = '\0';
	(msgPtr->offset) += len;
	
	return;
}
void CWProtocolRetrieveMac(CWProtocolMessage *msgPtr,unsigned char *mac) {
	char len = CW_MAC_LEN;
	CW_COPY_MEMORY(mac, &((msgPtr->msg)[(msgPtr->offset)]), len);
	(msgPtr->offset) += len;
	
	return;
}


// retrieves len bytes from the message, increments the current offset in bytes.
void CWProtocolRetrieveRawBytes(CWProtocolMessage *msgPtr, int len, char *bytes) {

	CW_COPY_MEMORY(bytes, &((msgPtr->msg)[(msgPtr->offset)]), len);
	(msgPtr->offset) += len;
	return;
}

// stores 8 bits in the message, increments the current offset in bytes
void CWProtocolStore8(CWProtocolMessage *msgPtr, unsigned char val) {
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 1);
	(msgPtr->offset) += 1;
	return;
}

// stores 16 bits in the message, increments the current offset in bytes
void CWProtocolStore16(CWProtocolMessage *msgPtr, unsigned short val) {
	val = htons(val);
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 2);
	(msgPtr->offset) += 2;
	return;
}

// stores 32 bits in the message, increments the current offset in bytes
void CWProtocolStore32(CWProtocolMessage *msgPtr, unsigned int val) {
	val = htonl(val);
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 4);
	(msgPtr->offset) += 4;
	return;
}

// stores 32 bits in the message, increments the current offset in bytes
void CWProtocolStore64(CWProtocolMessage *msgPtr, unsigned long long int val) {
	val = htonl(val);
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 8);
	(msgPtr->offset) += 8;
	return;
}
void CWProtocolStore128(CWProtocolMessage *msgPtr, unsigned long long int val) {
	val = htonl(val);
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), &(val), 16);
	(msgPtr->offset) += 16;
	return;
}


// stores a string in the message, increments the current offset in bytes. Doesn't store
// the '\0' final character.
void CWProtocolStoreStr(CWProtocolMessage *msgPtr,char *str) {
	int len = strlen(str);
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), str, len);
	(msgPtr->offset) += len;
	return;
}

void CWProtocolStoreBSSID(CWProtocolMessage *msgPtr, unsigned char *str) {
	int len = CW_MAC_LEN;
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), str, len);
	(msgPtr->offset) += len;
	return;
}

void CWProtocolStoreMAC(CWProtocolMessage *msgPtr,char *str) {
	int len = CW_MAC_LEN;
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), str, len);
	(msgPtr->offset) += len;
	return;
}

// stores another message in the message, increments the current offset in bytes.
void CWProtocolStoreMessage(CWProtocolMessage *msgPtr, CWProtocolMessage *msgToStorePtr) {
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), msgToStorePtr->msg, msgToStorePtr->offset);
	(msgPtr->offset) += msgToStorePtr->offset;
	return;
}

// stores len bytes in the message, increments the current offset in bytes.
void CWProtocolStoreRawBytes(CWProtocolMessage *msgPtr, char *bytes, int len) {
	CW_COPY_MEMORY(&((msgPtr->msg)[(msgPtr->offset)]), bytes, len);
	(msgPtr->offset) += len;
	return;
}


LIBBool CWAssembleMsgElem(CWProtocolMessage *msgPtr, unsigned int type) {
	CWProtocolMessage completeMsg;
	if(msgPtr == NULL) 
	{
		return LIB_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MESSAGE(completeMsg, 6+(msgPtr->offset), return LIB_FALSE;);
	
	// store header
	CWProtocolStore16(&completeMsg, type);
	CWProtocolStore16(&completeMsg, msgPtr->offset); // size of the body
	
	// store body
	CWProtocolStoreMessage(&completeMsg, msgPtr);
	
	CW_FREE_PROTOCOL_MESSAGE(*msgPtr);
	msgPtr->msg = completeMsg.msg;
	msgPtr->offset = completeMsg.offset;
	return LIB_TRUE;
}


