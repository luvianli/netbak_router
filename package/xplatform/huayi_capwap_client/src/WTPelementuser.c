/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "WTPelementuser.h"

void CWWTPGetVendorIdentifier(int *vendorid)
{
	*vendorid = VENDOR_ID;
	return;
}

void CWWTPGetProductName(char * ModelNum)
{
	sprintf(ModelNum,"v2c");
	
	return;
}

void CWWTPGetBOARDID(char * BoardID)
{	
	sprintf(BoardID,"v2c");
	
	return;
}

void CWWTPGetBOARDREVISION(char * BoardRevision)
{	
	sprintf(BoardRevision,"1.0");
	
	return;
}

void CWWTPGetBASEMACADDRESS(unsigned char * MacAddress)
{
	char mac[CW_MAC_LEN] ={0x00,0x19,0xbe,0x20,0x03,0xef};
	
	CW_COPY_MEMORY(MacAddress,mac,CW_MAC_LEN);
	
	return;
}


void CWWTPGetHWVersion(char * HWVersion)
{	
	sprintf(HWVersion,"1.0");
	return;
}

void CWWTPGetSWVersion(char * SWVersion)
{	
	unsigned long software = htonl(20181118);
	/*get software num*/
	memcpy(SWVersion,&software,sizeof(unsigned long));
	
	return;
}

void CWWTPGetBootVersion(char * BootVersion)
{	
	unsigned long boot_code = htonl(20181118);
	memcpy(BootVersion,&boot_code,sizeof(unsigned long));
	return;
}

unsigned char CWWTPGetMaxRadios(void) 
{
	int num = 0;
	num = 2;
	return num;
}

unsigned char CWWTPGetRadiosInUseNum(void) 
{
	int num = 0;
	num =2;	
	return num;
}

int CWWTPGetEncCapabilities(void) 
{
	return 0x01010000;
}


WTPBool CWWTPGetBoardData(CWWTPVendorInfos *valPtr, struct VcapwapAgent* Agent) 
{	
	if(valPtr == NULL) return WTP_FALSE;
	
	valPtr->vendorInfosCount = BOARDDATAINFOCOUTE; // we fill 5 information (just the required ones)

	(valPtr->vendorInfos)[0].type = CW_WTP_MODEL_NUMBER;
	memset(((valPtr->vendorInfos)[0]).valuePtr,0,sizeof(((valPtr->vendorInfos)[0]).valuePtr));
	CWWTPGetVendorIdentifier(&((valPtr->vendorInfos)[0].vendorIdentifier));
	CWWTPGetProductName((char *)((valPtr->vendorInfos)[0]).valuePtr); //ap MODEL NUMBER
	(valPtr->vendorInfos)[0].length =strlen((char *)(valPtr->vendorInfos)[0].valuePtr);

	(valPtr->vendorInfos)[1].type = CW_WTP_SERIAL_NUMBER;
	memset(((valPtr->vendorInfos)[1]).valuePtr,0,sizeof(((valPtr->vendorInfos)[1]).valuePtr));
	CWWTPGetVendorIdentifier(&((valPtr->vendorInfos)[1].vendorIdentifier));
    memcpy(((valPtr->vendorInfos)[1]).valuePtr,Agent->SerialNumber,strlen(Agent->SerialNumber));
	(valPtr->vendorInfos)[1].length =strlen((char *)(valPtr->vendorInfos)[1].valuePtr);

	(valPtr->vendorInfos)[2].type = CW_BOARD_ID;
	memset(((valPtr->vendorInfos)[2]).valuePtr,0,sizeof(((valPtr->vendorInfos)[2]).valuePtr));
	CWWTPGetVendorIdentifier(&((valPtr->vendorInfos)[2].vendorIdentifier));
	CWWTPGetBOARDID((char *)((valPtr->vendorInfos)[2]).valuePtr);	/*ap BOARD ID*/
	(valPtr->vendorInfos)[2].length =strlen((char *)(valPtr->vendorInfos)[2].valuePtr);

	(valPtr->vendorInfos)[3].type = CW_BOARD_REVISION;
	memset(((valPtr->vendorInfos)[3]).valuePtr,0,sizeof(((valPtr->vendorInfos)[3]).valuePtr));
	CWWTPGetVendorIdentifier(&((valPtr->vendorInfos)[3].vendorIdentifier));
	CWWTPGetBOARDREVISION((char *)((valPtr->vendorInfos)[3]).valuePtr);	/*ap BOARD REVISION*/
	(valPtr->vendorInfos)[3].length =strlen((char *)(valPtr->vendorInfos)[3].valuePtr);

	(valPtr->vendorInfos)[4].type = CW_BASE_MAC_ADDRESS;
	CWWTPGetVendorIdentifier(&((valPtr->vendorInfos)[4].vendorIdentifier));
	(valPtr->vendorInfos)[4].length = CW_MAC_LEN;
	CWWTPGetBASEMACADDRESS(((valPtr->vendorInfos)[4]).valuePtr);	/*ap BASE MAC ADDRESS*/
	
	return WTP_TRUE;
}

WTPBool CWWTPGetVendorInfos(CWWTPVendorInfos *valPtr, struct VcapwapAgent* Agent) 
{
	
	
	
	if(valPtr == NULL) return WTP_FALSE;
	
	valPtr->vendorInfosCount = WTPDISCRIBERCOUTE ;

	(valPtr->vendorInfos)[0].type = CW_WTP_HARDWARE_VERSION;
	memset(((valPtr->vendorInfos)[0]).valuePtr,0,sizeof(((valPtr->vendorInfos)[0]).valuePtr));
	(valPtr->vendorInfos)[0].vendorIdentifier =0 ;
	CWWTPGetHWVersion((char *)((valPtr->vendorInfos)[0]).valuePtr); /*ap hardware version*/
	(valPtr->vendorInfos)[0].length = strlen((char *)((valPtr->vendorInfos)[0] ).valuePtr);


	((valPtr->vendorInfos)[1]).type = CW_WTP_SOFTWARE_VERSION;
	memset(((valPtr->vendorInfos)[1]).valuePtr,0,sizeof(((valPtr->vendorInfos)[1]).valuePtr));
	(valPtr->vendorInfos)[1].vendorIdentifier =0 ;
	CWWTPGetSWVersion((char *)((valPtr->vendorInfos)[1]).valuePtr); /*ap software version*/
	(valPtr->vendorInfos)[1].length = sizeof(unsigned long);
	
	(valPtr->vendorInfos)[2].type = CW_BOOT_VERSION;
	memset(((valPtr->vendorInfos)[2]).valuePtr,0,sizeof(((valPtr->vendorInfos)[2]).valuePtr));
	(valPtr->vendorInfos)[2].vendorIdentifier =0 ;
	CWWTPGetBootVersion((char *)((valPtr->vendorInfos)[2]).valuePtr); /*Boot version*/
	(valPtr->vendorInfos)[2].length = sizeof(unsigned long);
	
	return WTP_TRUE;
}
