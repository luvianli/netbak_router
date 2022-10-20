/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "WTPelement.h"
#include "WTPelementuser.h"
#include "WTPsocket.h"

/*MSG elemnet type 20:Discovery Type*/
extern pthread_mutex_t config_mutex;
extern WTP_CONFIG_RECEIVE	huayi_capwap_config;


WTPBool CWAssembleMsgElemDiscoveryType(CWProtocolMessage *msgPtr) 
{
	if(msgPtr == NULL) return WTP_FALSE;

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return WTP_FALSE;);
	
	CWProtocolStore8(msgPtr,CW_STATIC_CONFIGURE_DISCOVERY);//capwap statics ip
	
	return CWAssembleMsgElem(msgPtr,CW_MSG_ELEMENT_DISCOVERY_TYPE_CW_TYPE);
}



WTPBool CWAssembleMsgElemWTPBoardData(CWProtocolMessage *msgPtr, struct VcapwapAgent* Agent) 
{
	const int VENDOR_ID_LENGTH = 4; 	//Vendor Identifier is 4 bytes long
	const int TLV_HEADER_LENGTH = 4; 	//Type and Length of a TLV field is 4 byte long 
	CWWTPVendorInfos infos;
	int i, size = 0;

	if(msgPtr == NULL) return WTP_FALSE;

	 // get infos
	 memset(&infos, 0, sizeof(CWWTPVendorInfos));
	if(!CWWTPGetBoardData(&infos, Agent)) {
		return  WTP_FALSE;
	}
	
	//Calculate msg elem size
	size = VENDOR_ID_LENGTH;
	for(i = 0; i < infos.vendorInfosCount; i++) 
		{size += (TLV_HEADER_LENGTH + ((infos.vendorInfos)[i]).length);}
	
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return WTP_FALSE;);
	CWProtocolStore32(msgPtr, ((infos.vendorInfos)[0].vendorIdentifier));
	for(i = 0; i < infos.vendorInfosCount; i++) 
	{
		CWProtocolStore16(msgPtr, ((infos.vendorInfos)[i].type));
		CWProtocolStore16(msgPtr, ((infos.vendorInfos)[i].length));
		if((infos.vendorInfos)[i].length == 4) {
			*((infos.vendorInfos)[i].valuePtr) = htonl(*((infos.vendorInfos)[i].valuePtr));
		}
		CWProtocolStoreRawBytes(msgPtr, (char*) ((infos.vendorInfos)[i].valuePtr), (infos.vendorInfos)[i].length);
	}

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE);
}

WTPBool CWAssembleMsgElemWTPDescriptor(CWProtocolMessage *msgPtr, struct VcapwapAgent* Agent) 
{
	const int GENERIC_RADIO_INFO_LENGTH = 6;//First 4 bytes for Max Radios, Radios In Use and Encryption Capability 
	const int VENDOR_ID_LENGTH = 4; 	//Vendor Identifier is 4 bytes long
	const int TLV_HEADER_LENGTH = 4; 	//Type and Length of a TLV field is 4 byte long 
	CWWTPVendorInfos infos;
	int i, size = 0;

	if(msgPtr == NULL) return WTP_FALSE;

	// get infos
	memset(&infos, 0, sizeof(CWWTPVendorInfos));
	if(!CWWTPGetVendorInfos(&infos,Agent)) { 
		return  WTP_FALSE;
	}
	
	//Calculate msg elem size
	size = GENERIC_RADIO_INFO_LENGTH;
	for(i = 0; i < infos.vendorInfosCount; i++) 
	{
		size += (VENDOR_ID_LENGTH + TLV_HEADER_LENGTH + ((infos.vendorInfos)[i]).length);
	}
		
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return WTP_FALSE;);
	CWProtocolStore8(msgPtr, CWWTPGetMaxRadios()); // number of radios supported by the WTP
	CWProtocolStore8(msgPtr, CWWTPGetRadiosInUseNum()); // number of radios present in the WTP
	CWProtocolStore32(msgPtr, CWWTPGetEncCapabilities()); // encryption capabilities

	for(i = 0; i < infos.vendorInfosCount; i++) {
		CWProtocolStore32(msgPtr, ((infos.vendorInfos)[i].vendorIdentifier));
		CWProtocolStore16(msgPtr, ((infos.vendorInfos)[i].type));
		CWProtocolStore16(msgPtr, ((infos.vendorInfos)[i].length));
		
		if((infos.vendorInfos)[i].length == 8) {
			*((infos.vendorInfos)[i].valuePtr) = htonl(*((infos.vendorInfos)[i].valuePtr));
		}
	
		CWProtocolStoreRawBytes(msgPtr, (char*) ((infos.vendorInfos)[i].valuePtr), (infos.vendorInfos)[i].length);
	}
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE);
}


/*MSG elemnet type 41:WTP Frame Tunnel Mode*/
WTPBool CWAssembleMsgElemWTPFrameTunnelMode(CWProtocolMessage *msgPtr) 
{
	unsigned char tunnelmode;
	
	if(msgPtr == NULL) return WTP_FALSE;

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return WTP_FALSE;);
	

	tunnelmode = CW_LOCAL_BRIDGING;

	CWProtocolStore8(msgPtr, tunnelmode); // frame encryption

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE);
}

/*MSG elemnet type 44:WTP MAC Type*/
WTPBool CWAssembleMsgElemWTPMACType(CWProtocolMessage *msgPtr) 
{
	unsigned char mactype;
	
	if(msgPtr == NULL) return WTP_FALSE;
	
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 1, return WTP_FALSE;);
	
	mactype = CW_LOCAL_MAC;
	
	CWProtocolStore8(msgPtr, mactype); // mode of operation of the WTP (local, split, ...)

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE);
}

WTPBool CWAssembleMsgElemWTPIPv4Address(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent) {
	unsigned int ipv4addr;
	
	if(msgPtr == NULL) return WTP_FALSE;

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, 4, return WTP_FALSE;);

	ipv4addr = htonl(inet_addr(Agent->wtp_ip));
	
    CWProtocolStore32(msgPtr,ipv4addr);

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_IPV4_ADDRESS_CW_TYPE);
}


WTPBool CWAssembleMsgElemWTPLocationData(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent) {
	
	if(msgPtr == NULL) return WTP_FALSE;

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(Agent->wtp_location), return WTP_FALSE;);
	
    CWProtocolStoreStr(msgPtr,Agent->wtp_location);

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_LOCATION_DATA_CW_TYPE);
}

WTPBool CWAssembleMsgElemWTPName(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent) {
	
	if(msgPtr == NULL) return WTP_FALSE;

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, strlen(Agent->wtp_name), return WTP_FALSE;);
	
    CWProtocolStoreStr(msgPtr,Agent->wtp_name);

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_NAME_CW_TYPE);
}




WTPBool WTPAssemblePayloadMsgElemUptime(CWProtocolMessage *msgPtr)
{
	int uptime = 0;

	if(msgPtr == NULL) 
	{
	  return WTP_FALSE;
	}

	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr,10, return WTP_FALSE;);

	CWProtocolStore32(msgPtr, VENDOR_ID);
	
	CWProtocolStore16(msgPtr, CW_ME_TYPE_VENDOR_UPTIME);
	
	CWProtocolStore32(msgPtr,(unsigned int)uptime);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}


WTPBool WTPAssemblePayloadMsgElemBaseMac(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent)
{
	if(msgPtr == NULL) 
	{
	  return WTP_FALSE;
	}

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr,12, return WTP_FALSE;);

	CWProtocolStore32(msgPtr, VENDOR_ID);
	
	CWProtocolStore16(msgPtr, CW_ME_TYPE_VENDOR_BASE_MAC);
	
	CWProtocolStoreMAC(msgPtr,(char *)(Agent->baseMac));
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

WTPBool WTPAssemblePayloadMsgElemWtpModel(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent)
{
	unsigned int len = strlen(Agent->wtp_model);
	if(msgPtr == NULL) 
	{
	  return WTP_FALSE;
	}

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr,len+ CW_IEEE80211_PAYLOAD_HEAD_SIZE, return WTP_FALSE;);

	CWProtocolStore32(msgPtr, VENDOR_ID);
	
	CWProtocolStore16(msgPtr, CW_ME_TYPE_VENDOR_WTP_MODEL);
	
	CWProtocolStoreStr(msgPtr, Agent->wtp_model);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

WTPBool WTPAssemblePayloadMsgElemSoftwareVersion(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent)
{
	if(msgPtr == NULL) 
	{
	  return WTP_FALSE;
	}

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr,10, return WTP_FALSE;);

	CWProtocolStore32(msgPtr, VENDOR_ID);
	
	CWProtocolStore16(msgPtr, CW_ME_TYPE_VENDOR_WTP_SOFTWARE_VERSION);
	
	CWProtocolStore32(msgPtr, Agent->softVersion);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}


WTPBool WTPAssemblePayloadMsgElemBootVersion(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent)
{
	if(msgPtr == NULL) 
	{
	  return WTP_FALSE;
	}

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr,10, return WTP_FALSE;);

	CWProtocolStore32(msgPtr, VENDOR_ID);
	
	CWProtocolStore16(msgPtr, CW_ME_TYPE_VENDOR_WTP_BOOT_VERSION);
	
	CWProtocolStore32(msgPtr,Agent->bootVersion);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

WTPBool WTPAssemblePayloadMsgElemRadioType(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent)
{
	if(msgPtr == NULL) 
	{
	  return WTP_FALSE;
	}

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr,7, return WTP_FALSE;);

	CWProtocolStore32(msgPtr, VENDOR_ID);
	
	CWProtocolStore16(msgPtr, CW_ME_TYPE_VENDOR_RADIO_TYPE);
	
	CWProtocolStore8(msgPtr, Agent->radiotype);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

WTPBool WTPAssembleMsgElemConfigActionType(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent,CW_WTP_VENDOR_ACTION actionType)
{
	if(msgPtr == NULL) 
	{
	  return WTP_FALSE;
	}

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr,1, return WTP_FALSE;);

	CWProtocolStore8(msgPtr,actionType);

	return CWAssembleMsgElem(msgPtr,CW_MSG_ELEMENT_CONFIG_UPDATE_ACTION_TYPE);
}

WTPBool WTPAssemblePayloadMsgElemTrustId(CWProtocolMessage *msgPtr,struct VcapwapAgent *Agent)
{
	if(msgPtr == NULL) 
	{
	  return WTP_FALSE;
	}

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr,10, return WTP_FALSE;);

	CWProtocolStore32(msgPtr, VENDOR_ID);
	
	CWProtocolStore16(msgPtr, CW_ME_TYPE_VENDOR_TRUST_ID);
	
	CWProtocolStore32(msgPtr,Agent->trustid);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}


WTPBool WTPParseResultCode(CWProtocolMessage *msgPtr, int len, unsigned int *valPtr) 
{	
	CWParseMessageElementStart();
	
	*valPtr = CWProtocolRetrieve32(msgPtr);

	if(valPtr == NULL) return WTP_FALSE;
	
	CWParseMessageElementEnd();
}

WTPBool WTPParseIdleTimeOut(CWProtocolMessage *msgPtr, int len,struct VcapwapAgent* Agent) 
{	
	CWParseMessageElementStartWithoutvalPtr();
	
	Agent->Agenttimer.idleTimeout = CWProtocolRetrieve32(msgPtr);
	
	if((Agent->Agenttimer.idleTimeout > MAX_ECHO_RETRY) || (Agent->Agenttimer.idleTimeout < MIN_ECHO_RETRY))
	{	
		Agent->Agenttimer.idleTimeout= MAX_ECHO_RETRY;
	}
	CWParseMessageElementEnd();
}

WTPBool WTPParseEchoTimer(CWProtocolMessage *msgPtr, int len,struct VcapwapAgent* Agent) 
{	
	CWParseMessageElementStartWithoutvalPtr();

	Agent->Agenttimer.discoveryTimer = CWProtocolRetrieve8(msgPtr);

	if(Agent->Agenttimer.discoveryTimer < CW_DISCOVER_TIME_OFFSET)
	{
		Agent->Agenttimer.discoveryTimer = CW_DISCOVER_TIME_OFFSET;
	}
	Agent->Agenttimer.echoRequestTimer = CWProtocolRetrieve8(msgPtr);

	if((Agent->Agenttimer.echoRequestTimer > MAX_ECHO_TIMER) || (Agent->Agenttimer.echoRequestTimer < MIN_ECHO_TIMER))
	{
		Agent->Agenttimer.echoRequestTimer = MIN_ECHO_TIMER;
	}

	CWParseMessageElementEnd();
}

WTPBool WTPParseWtpName(CWProtocolMessage *msgPtr, int len, struct VcapwapAgent* Agent) 
{	
	
	CWParseMessageElementStartWithoutvalPtr();

	CWProtocolRetrieveStr(msgPtr,len, (unsigned char *)Agent->wtp_name);

	printf("Agent->wtp_name is %s\n",Agent->wtp_name);

	huayi_uci_set_str(PKG_THINAP,"global","wtp_name",Agent->wtp_name);

	CWParseMessageElementEnd();
}



WTPBool WTPParseACName(CWProtocolMessage *msgPtr, int len, struct VcapwapAgent* Agent) 
{	
	
	CWParseMessageElementStartWithoutvalPtr();

	CWProtocolRetrieveStr(msgPtr,len, (unsigned char *)Agent->ac_name);

	CWParseMessageElementEnd();
}

WTPBool WTPParseImageIdentifier(CWProtocolMessage *msgPtr, int len, IMAGE_IDENTIFIER_CODE *valPtr) 
{	
	unsigned int vendorid = 0;

	CWParseMessageElementStart();

	vendorid = CWProtocolRetrieve32(msgPtr);
	
	printf("vendorid is %d\n",vendorid);

	*valPtr = CWProtocolRetrieve8(msgPtr);
	
	if(valPtr == NULL) return WTP_FALSE;
		
	CWParseMessageElementEnd(); 
}

WTPBool WTPParseControlIPv4Addresses(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{
	unsigned short connections;
	unsigned int acip;
	
	CWParseMessageElementStartWithoutvalPtr();
	
	acip = CWProtocolRetrieve32(msgPtr);

	acip = ntohl(acip);
	
	inet_ntop(AF_INET, &acip, Agent->ac_ip, CW_IP_STR_LEN);

	Agent->need_broadcast = WTP_FALSE;
	
	WTPUnicastcastDiscoverySocket(Agent);

	connections =  CWProtocolRetrieve16(msgPtr); 

	printf("connections is %d\n",connections);
	
	wtp_set_socket_acaddr(Agent);
	
	/**/
	CWParseMessageElementEnd(); 
}


WTPBool WTPParsePayloadManagePort(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{
	
	CWParseMessageElementStartWithoutvalPtr();
	
	Agent->manage_port = CWProtocolRetrieve16(msgPtr);
	
	wtp_set_socket_acaddr(Agent);
	
	CWParseMessageElementEnd(); 
}

WTPBool WTPParsePayloadSoftwareUrl(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{
	CWParseMessageElementStartWithoutvalPtr();
	
	CWProtocolRetrieveStr(msgPtr,len-CW_IEEE80211_PAYLOAD_HEAD_SIZE,Agent->firmware_url);
	printf("Agent->firmware_url is %s\n",Agent->firmware_url);
	CWParseMessageElementEnd(); 
}

WTPBool WTPParsePayloadMsgElemRadioConfig(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{
	unsigned char radioid,if_admin,wirelessmode,htmode,channel,txpower,country_len;
	unsigned short beacon;
	unsigned char country[CW_OPTION_STR_LEN] = {0};
	int offset = 0;	
	
	CWParseMessageElementStartWithoutvalPtr();
	radioid = CWProtocolRetrieve8(msgPtr);
	if_admin = CWProtocolRetrieve8(msgPtr);
	wirelessmode = CWProtocolRetrieve8(msgPtr);
	htmode = CWProtocolRetrieve8(msgPtr);
	channel = CWProtocolRetrieve8(msgPtr);
	txpower = CWProtocolRetrieve8(msgPtr);
	beacon = CWProtocolRetrieve16(msgPtr);
	country_len =  CWProtocolRetrieve8(msgPtr);
	CWProtocolRetrieveStr(msgPtr,country_len,country);

	pthread_mutex_lock(&config_mutex);
	
	huayi_capwap_config.radio_config[radioid-1].if_admin=if_admin;
	huayi_capwap_config.radio_config[radioid-1].wirelessmode=wirelessmode;
	huayi_capwap_config.radio_config[radioid-1].htmode=htmode;
	huayi_capwap_config.radio_config[radioid-1].channel=channel;
	huayi_capwap_config.radio_config[radioid-1].txpower=txpower;
	huayi_capwap_config.radio_config[radioid-1].beacon=beacon;
	huayi_capwap_config.radio_config[radioid-1].country_len=country_len;
	CW_ZERO_MEMORY(huayi_capwap_config.radio_config[radioid-1].country,country_len);
	CW_COPY_MEMORY(huayi_capwap_config.radio_config[radioid-1].country,country,country_len);
	
	pthread_mutex_unlock(&config_mutex);

	printf("radio id is %d\n",radioid);

	printf("radio config:admin %d\n",huayi_capwap_config.radio_config[radioid-1].if_admin);

	printf("radio config:wirelessmode %d\n",huayi_capwap_config.radio_config[radioid-1].wirelessmode);

	printf("radio config:htmode %d\n",huayi_capwap_config.radio_config[radioid-1].htmode);

	printf("radio config:channel %d\n",huayi_capwap_config.radio_config[radioid-1].channel);

	printf("radio config:txpower %d\n",huayi_capwap_config.radio_config[radioid-1].txpower);
	
	printf("radio config:beacon %d\n",huayi_capwap_config.radio_config[radioid-1].beacon);

	printf("radio config:country %s\n",huayi_capwap_config.radio_config[radioid-1].country);

	printf("\n");
	
	offset = (len - CW_IEEE80211_PAYLOAD_HEAD_SIZE - 9 - country_len);
	
	msgPtr->offset += offset;

	CWParseMessageElementEnd(); 
}

WTPBool WTPParsePayloadMsgElemWlanConfig(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{

	unsigned char	ssid[CW_SSID_STR_LEN] = {0};
	unsigned char 	passphrase[CW_PASSPHRASE_LEN] = {0};
	unsigned char 	radioid,hide,isolate,encry,maxsta;
	unsigned short	vlanid;
	unsigned char	ssid_len,passphrase_len;
	int offset = 0;
	
	CWParseMessageElementStartWithoutvalPtr();
	radioid = CWProtocolRetrieve8(msgPtr);
	ssid_len = CWProtocolRetrieve8(msgPtr);
	memset(ssid,0,sizeof(ssid));
	if(ssid_len)
	{
		CWProtocolRetrieveStr(msgPtr,ssid_len,ssid);
	}
	hide = CWProtocolRetrieve8(msgPtr);
	isolate = CWProtocolRetrieve8(msgPtr);
	encry = CWProtocolRetrieve8(msgPtr);
	passphrase_len = CWProtocolRetrieve8(msgPtr);
	memset(passphrase,0,sizeof(passphrase));
	if(passphrase_len)
	{
		CWProtocolRetrieveStr(msgPtr,passphrase_len,passphrase);
	}
	maxsta = CWProtocolRetrieve8(msgPtr);
	vlanid = CWProtocolRetrieve16(msgPtr);

	pthread_mutex_lock(&config_mutex);
	printf("wlan config radioid %d\n",radioid);
	printf("wlan config ssid_len %d\n",ssid_len);
	printf("wlan config ssid %s\n",ssid);
	printf("wlan config hide %d\n",hide);
	printf("wlan config encry %d\n",encry);
	printf("wlan config passphrase %s\n",passphrase);
	printf("wlan config maxsta %d\n",maxsta);
	
	CW_ZERO_MEMORY(huayi_capwap_config.ssid_config[radioid-1].ssid,CW_SSID_STR_LEN);
	CW_COPY_MEMORY(huayi_capwap_config.ssid_config[radioid-1].ssid,ssid,ssid_len);
	huayi_capwap_config.ssid_config[radioid-1].ssid_len=ssid_len;
	huayi_capwap_config.ssid_config[radioid-1].hide=hide;
	huayi_capwap_config.ssid_config[radioid-1].isolate=isolate;
	huayi_capwap_config.ssid_config[radioid-1].maxsta=maxsta;
	huayi_capwap_config.ssid_config[radioid-1].vlanid=vlanid;
	huayi_capwap_config.ssid_config[radioid-1].encry=encry;
	huayi_capwap_config.ssid_config[radioid-1].passphrase_len=passphrase_len;
	CW_ZERO_MEMORY(huayi_capwap_config.ssid_config[radioid-1].passphrase,CW_PASSPHRASE_LEN);
	CW_COPY_MEMORY(huayi_capwap_config.ssid_config[radioid-1].passphrase,passphrase,passphrase_len);
	
	pthread_mutex_unlock(&config_mutex);
	printf("huayi_capwap_config.ssid_config[radioid-1].ssid %s\n",huayi_capwap_config.ssid_config[radioid-1].ssid);
	printf("huayi_capwap_config.ssid_config[radioid-1].passphrase %s\n",huayi_capwap_config.ssid_config[radioid-1].passphrase);
	printf("\n");
	


	offset = (len - CW_IEEE80211_PAYLOAD_HEAD_SIZE - 9 - ssid_len - passphrase_len);
	
	msgPtr->offset += offset;
	CWParseMessageElementEnd(); 
}

WTPBool WTPParsePayloadMsgElemGloabalAutoChannel(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{
	CWParseMessageElementStartWithoutvalPtr();
	
	pthread_mutex_lock(&config_mutex);
	
	huayi_capwap_config.sys_config.gloabal_auto_channel = CWProtocolRetrieve8(msgPtr);
	
	pthread_mutex_unlock(&config_mutex);
	
	CWParseMessageElementEnd(); 
}

WTPBool WTPParsePayloadMsgElemGloabalRoaming(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{
	CWParseMessageElementStartWithoutvalPtr();

	pthread_mutex_lock(&config_mutex);
	
	huayi_capwap_config.sys_config.gloabal_roaming = CWProtocolRetrieve8(msgPtr);

	pthread_mutex_unlock(&config_mutex);
	
	CWParseMessageElementEnd(); 
}

WTPBool WTPParsePayloadMsgElemRoamingThreshold2G(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{
	CWParseMessageElementStartWithoutvalPtr();

	pthread_mutex_lock(&config_mutex);
	
	huayi_capwap_config.sys_config.roaming_threshold_2g = CWProtocolRetrieve8(msgPtr);

	pthread_mutex_unlock(&config_mutex);
	
	CWParseMessageElementEnd(); 
}

WTPBool WTPParsePayloadMsgElemRoamingThreshold5G(CWProtocolMessage *msgPtr,int len,struct VcapwapAgent* Agent)
{
	CWParseMessageElementStartWithoutvalPtr();

	pthread_mutex_lock(&config_mutex);
	
	huayi_capwap_config.sys_config.roaming_threshold_5g = CWProtocolRetrieve8(msgPtr);

	pthread_mutex_unlock(&config_mutex);
	
	CWParseMessageElementEnd(); 
}


WTPBool WTPParsePayloadMsgElem(CWProtocolMessage *msgPtr, unsigned short int len, struct VcapwapAgent* Agent)
{
	unsigned int VendorId = 0;
	unsigned short PayLoadType = 0;
	
	if(msgPtr == NULL)
	{
		printf("WTPParsePayloadMsgElem ERROR RETURN...\n");
		return WTP_FALSE;
	}

	VendorId = CWProtocolRetrieve32(msgPtr);
	if(VendorId != VENDOR_ID)
	{
		return WTP_FALSE;
	}
	PayLoadType = CWProtocolRetrieve16(msgPtr);

	switch(PayLoadType)
	{
		case CW_ME_TYPE_VENDOR_MANAGE_PORT:
			WTPParsePayloadManagePort(msgPtr,len,Agent);
			break;
		case CW_ME_TYPE_VENDOR_SOFTWARE_URL:
			WTPParsePayloadSoftwareUrl(msgPtr,len,Agent);
			break;
		case CW_ME_TYPE_VENDOR_RADIO_CONFIG:
			WTPParsePayloadMsgElemRadioConfig(msgPtr,len,Agent);
			break;
		case CW_ME_TYPE_VENDOR_WLAN_CONFIG:
			WTPParsePayloadMsgElemWlanConfig(msgPtr,len,Agent);
			break;
		case CW_ME_TYPE_VENDOR_GLOABAL_AUTO_CHANNEL:
			WTPParsePayloadMsgElemGloabalAutoChannel(msgPtr,len,Agent);
			break;
		case CW_ME_TYPE_VENDOR_GLOABAL_ROAMING:
			WTPParsePayloadMsgElemGloabalRoaming(msgPtr,len,Agent);
			break;
		case CW_ME_TYPE_VENDOR_ROAMING_THRESHOLD_2G:
			WTPParsePayloadMsgElemRoamingThreshold2G(msgPtr,len,Agent);
			break;
		case CW_ME_TYPE_VENDOR_ROAMING_THRESHOLD_5G:
			WTPParsePayloadMsgElemRoamingThreshold5G(msgPtr,len,Agent);
			break;
		default:
			printf("Unrecognized Message PayLoad:%d\n",PayLoadType);
			CWParseNoAction(msgPtr,len);
			break;
	}
	return WTP_TRUE;
}


WTPBool WTPParseConfigActionType(CWProtocolMessage *msgPtr, int len, CW_WTP_VENDOR_ACTION *actionType) 
{	
	CWParseMessageElementStartWithoutvalPtr();

	*actionType=CWProtocolRetrieve8(msgPtr);

	CWParseMessageElementEnd(); 
}

