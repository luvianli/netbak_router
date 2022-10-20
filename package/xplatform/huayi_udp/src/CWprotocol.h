#ifndef _CWPROTOCOL_HEADER_
#define _CWPROTOCOL_HEADER_

#include "CWmessage.h"

#define MAX_DTLS_HEADER_SIZE			25   // see http://crypto.stanford.edu/~nagendra/papers/dtls.pdf
#define MAX_CAPWAP_HEADER_SIZE 			8    // note: this include optional Wireless field

#define MAXCAPWAPHEADERSIZEBINDING 24

#define CW_BINDING_HLEN				4
#define CW_BINDING_WIRELESSID			1
#define CW_BINDING_DATALENGTH			4

#define NUM_QOS_PROFILES			4
#define UNUSED_QOS_VALUE			255

#define VOICE_QUEUE_INDEX			0
#define VIDEO_QUEUE_INDEX			1
#define BESTEFFORT_QUEUE_INDEX			2
#define BACKGROUND_QUEUE_INDEX			3

#define BINDING_MIN_ELEM_TYPE			1024
#define BINDING_MAX_ELEM_TYPE			2047

#define UNUSED_OFDM_VALUE			255

typedef enum
{
	INIT_IDENTIIER          = 0,
	SOFTWARE_UPGRADE 		= 1,
	BOOT_CODE_UPGRADE		= 2,
}IMAGE_IDENTIFIER_CODE;


typedef struct {
	unsigned int messageTypeValue;
	unsigned char seqNum;
	unsigned short msgElemsLen;
} CWControlHeaderValues;


typedef struct {
	char RSSI;
	char SNR;
	int dataRate;
	char radiomac[CW_MAC_LEN];
} CWBindingTransportHeaderValues;


typedef struct {
	int payloadType;
	int type;
	int isFragment;
	int last;
	int fragmentID;
	int fragmentOffset;
	int keepAlive;
	CWBindingTransportHeaderValues *bindingValuesPtr;
} CWProtocolTransportHeaderValues;


typedef struct {
	int vendorIdentifier;
	enum {
		CW_WTP_MODEL_NUMBER	= 0,
		CW_WTP_SERIAL_NUMBER	= 1,
		CW_BOARD_ID		= 2,
		CW_BOARD_REVISION	= 3,
		CW_BASE_MAC_ADDRESS = 4,
		
		CW_WTP_HARDWARE_VERSION	= 0,
		CW_WTP_SOFTWARE_VERSION	= 1,
		CW_BOOT_VERSION		= 2 ,
		CW_WTP_COMPANY_SHORTNAME = 4,
		CW_FW_UPDATA_CODE = 5,
		CW_FW_PRODUCT_NAME = 6
	} type;
	int length;
	unsigned char valuePtr[CW_OPTION_STR_LEN];
} CWWTPVendorInfoValues;

typedef struct  {
	int vendorInfosCount;
	CWWTPVendorInfoValues vendorInfos[MAXVENDORCOUTE];
} CWWTPVendorInfos;

typedef struct  {
	unsigned char vendorid;
	CWWTPVendorInfoValues vendorInfos[MAXVENDORCOUTE];
} CWWTPBoardData;

typedef enum {
	CW_LOCAL_BRIDGING = 0x02,
	CW_802_DOT_3_BRIDGING = 0x04,
	CW_NATIVE_FRAME = 0x08
} CWframeTunnelMode;

typedef enum {
	CW_LOCAL_MAC = 0,
	CW_SPLIT_MAC = 1,
	CW_BOTH = 2
} CWMACType;

typedef enum{
	CW_UNKNOWN_DISCOVERY = 0,
	CW_STATIC_CONFIGURE_DISCOVERY,
	CW_DHCP_DISCOVERY,
	CW_DNS_DISCOVERY,
}CWDISCOVERYType;


typedef struct {
	int ID;
	enum {
		CW_802_DOT_11b = 1,
		CW_802_DOT_11a = 2,
		CW_802_DOT_11g = 4,
		CW_802_DOT_11n = 8,
		CW_ALL_RADIO_TYPES = 0x0F
	} type;
} CWRadioInformationValues;


typedef struct {
	int radiosCount;
	CWRadioInformationValues *radios;
} CWRadiosInformation;

typedef struct {
	unsigned char maxRadios;
	unsigned char radiosInUse;
	unsigned int encCapabilities;
	unsigned int software_version;
	unsigned int boot_version;
	CWWTPVendorInfos vendorInfos;
} CWWTPDescriptor;

typedef struct {
	unsigned int wtpUptime;
	unsigned char baseMac[CW_MAC_LEN];
	unsigned char model[CW_MODEL_STR_LEN];
	unsigned int software_version;
	unsigned int boot_version;
	unsigned char radio_type;
}CWWTPVendor;

typedef struct {
	unsigned int localip;
	CWframeTunnelMode frameTunnelMode;
	CWMACType MACType;
	CWDISCOVERYType discoveryType;
	CWWTPVendor vendorSpecific;
} CWProtocolDiscoveryRequestValues;

typedef struct {
	unsigned char 	locationData[CW_OPTION_STR_LEN];
	unsigned char 	wtpName[CW_OPTION_STR_LEN];
	unsigned int 	localip;
	CWWTPVendor vendorSpecific;
} CWProtocolJoinRequestValues;

typedef struct {
	unsigned int 			result_code;
	unsigned char 			ac_name[CW_OPTION_STR_LEN];
	IMAGE_IDENTIFIER_CODE 	imageidentifier;
} CWProtocolJoinResponseValues;

typedef struct {
	unsigned int 			result_code;
} CWProtocolConfigResponseValues;

typedef struct {
	CWWTPVendor vendorSpecific;
} CWProtocolConfigRequestValues;

typedef struct {
	CWWTPVendor vendorSpecific;
} CWProtocolChangeStateRequestValues;

typedef struct {
	CWWTPVendor vendorSpecific;
	unsigned int   result_code;
} CWProtocolChangeStateResponseValues;

typedef struct {
	CWWTPVendor vendorSpecific;
} CWProtocolEchoRequestValues;

typedef struct {
	CWWTPVendor vendorSpecific;
} CWProtocolImageDataRequestValues;

typedef enum
{
	CW_ACTION_ADD_WLAN 						=1,
	CW_ACTION_UPDATE_WLAN					=2,
	CW_ACTION_UPDATE_RADIO					=3,
	CW_ACTION_UPDATE_BASIC					=4,
	CW_ACTION_UPDATE_GLOABAL				=5,
	CW_ACTION_UPDATE_GLOABAL_ADVANCE		=6,
	CW_ACTION_REBOOT						=7,
	CW_ACTION_RESET							=8,
	CW_ACTION_UPGRADE						=9,
	CW_ACTION_LED_UP						=10,
	CW_ACTION_LED_OFF						=11,
	CW_ACTION_DELETE_WTP					=12,
}CW_WTP_VENDOR_ACTION;

typedef struct {
	CWWTPVendor vendorSpecific;
	CW_WTP_VENDOR_ACTION actionType;
	int			rspMsgType;
} CWProtocolCommonResponseValues;

typedef struct {
	CW_WTP_VENDOR_ACTION 	actionType;
	unsigned char 			baseMac[CW_MAC_LEN];
	int 					apid;
	int						gid;
}CWProtocolInternalValues;


#define CWSetField32(src,start,len,val)					src |= ((~(0xFFFFFFFF << len)) & val) << (32 - start - len)
#define CWGetField32(src,start,len)						((~(0xFFFFFFFF<<len)) & (src >> (32 - start - len)))
#define CWGetField8(src,start,len)						((~(0xFF<<len)) & (src >> (8 - start - len)))
#define CW_REWIND_BYTES(buf, bytes, type)				(buf) = (type*)(((char*) (buf)) - bytes)
#define CW_SKIP_BYTES(buf, bytes, type)					(buf) = (type*)(((char*) (buf)) + bytes)
#define CW_SKIP_BITS(buf, bits, type)					(buf) = (type*)(((char*) (buf)) + ((bits) / 8))
#define CW_BYTES_TO_STORE_BITS(bits)					((((bits) % 8) == 0) ? ((bits) / 8) : (((bits) / 8)+1))





#define     MAX_UDP_PACKET_SIZE					65536
#define		CW_CONTROL_PORT						1234
#define		CW_PROTOCOL_VERSION					0
#define		CW_IANA_ENTERPRISE_NUMBER				13277	
#define 	CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS			3		//Offset "Seq Num" - "Message Elements"
#define     CW_CAPWAP_SPECICAL_HEADER_LENGTH      6
#define     CW_CAPWAP_RADIOVAP_LENGTH                     2
#define		CW_MAX_SEQ_NUM						255
#define 	CW_MAX_FRAGMENT_ID					65535
#define 	CLEAR_DATA						1
#define		DTLS_ENABLED_DATA					2
#define		CW_PACKET_PLAIN						0
#define		CW_PACKET_CRYPT						1
#define 	CW_DATA_MSG_FRAME_TYPE					1
#define		CW_DATA_MSG_STATS_TYPE					2
#define     CW_DATA_MSG_FREQ_STATS_TYPE             3 /* 2009 Update */
#define  	CW_EVENT_RESPONSE_FAILCOUNT                   5

// <TRANSPORT_HEADER_FIELDS>
// CAPWAP version (currently 0)
#define 	CW_TRANSPORT_HEADER_VERSION_START			0
#define 	CW_TRANSPORT_HEADER_VERSION_LEN				4

// Mauro
#define		CW_TRANSPORT_HEADER_TYPE_START				4
#define		CW_TRANSPORT_HEADER_TYPE_LEN				4

// Radio ID number (for WTPs with multiple radios)
#define 	CW_TRANSPORT_HEADER_RID_START				13
#define 	CW_TRANSPORT_HEADER_RID_LEN				5

// Length of CAPWAP tunnel header in 4 byte words 
#define 	CW_TRANSPORT_HEADER_HLEN_START				8
#define 	CW_TRANSPORT_HEADER_HLEN_LEN				5

// Wireless Binding ID
#define 	CW_TRANSPORT_HEADER_WBID_START				18
#define 	CW_TRANSPORT_HEADER_WBID_LEN				5

// Format of the frame
#define 	CW_TRANSPORT_HEADER_T_START				23
#define 	CW_TRANSPORT_HEADER_T_LEN				1

// Is a fragment?
#define 	CW_TRANSPORT_HEADER_F_START				24
#define 	CW_TRANSPORT_HEADER_F_LEN				1

// Is NOT the last fragment?
#define 	CW_TRANSPORT_HEADER_L_START				25
#define 	CW_TRANSPORT_HEADER_L_LEN				1

// Is the Wireless optional header present?
#define 	CW_TRANSPORT_HEADER_W_START				26
#define 	CW_TRANSPORT_HEADER_W_LEN				1

// Is the Radio MAC Address optional field present?
#define 	CW_TRANSPORT_HEADER_M_START				27
#define 	CW_TRANSPORT_HEADER_M_LEN				1

// Is the message a keep alive?
#define 	CW_TRANSPORT_HEADER_K_START				28
#define 	CW_TRANSPORT_HEADER_K_LEN				1

// Set to 0 in this version of the protocol
#define 	CW_TRANSPORT_HEADER_FLAGS_START				29
#define 	CW_TRANSPORT_HEADER_FLAGS_LEN				3

// ID of the group of fragments
#define 	CW_TRANSPORT_HEADER_FRAGMENT_ID_START			0
#define 	CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN			16

// Position of this fragment in the group 
#define 	CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START		16
#define 	CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN			13

// Set to 0 in this version of the protocol
#define 	CW_TRANSPORT_HEADER_RESERVED_START			29
#define 	CW_TRANSPORT_HEADER_RESERVED_LEN			3
// </TRANSPORT_HEADER_FIELDS>

#define 	MINIMUM_CAPWAP_HEADER_LEN	8
#define 	MINIMUM_CAPWAP_MGS_HEADER_LEN	4
//802.11 type element
#define 	CW_MSG_ELEMENT_RSNE			48

// Message Type Values
#define		CW_MSG_TYPE_VALUE_DISCOVERY_REQUEST						1
#define		CW_MSG_TYPE_VALUE_DISCOVERY_RESPONSE					2
#define		CW_MSG_TYPE_VALUE_JOIN_REQUEST							3
#define		CW_MSG_TYPE_VALUE_JOIN_RESPONSE							4
#define		CW_MSG_TYPE_VALUE_CONFIGURE_REQUEST						5
#define		CW_MSG_TYPE_VALUE_CONFIGURE_RESPONSE					6
#define		CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_REQUEST				7
#define		CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_RESPONSE				8
#define 	CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST						9
#define 	CW_MSG_TYPE_VALUE_WTP_EVENT_RESPONSE					10
#define		CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_REQUEST			11
#define		CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_RESPONSE			12
#define		CW_MSG_TYPE_VALUE_ECHO_REQUEST							13
#define		CW_MSG_TYPE_VALUE_ECHO_RESPONSE							14
#define		CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST					15
#define		CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE					16
#define		CW_MSG_TYPE_VALUE_RESET_REQUEST							17
#define		CW_MSG_TYPE_VALUE_RESET_RESPONSE						18
#define		CW_MSG_TYPE_VALUE_PRIMARY_DISCOVERY_REQUEST				19
#define		CW_MSG_TYPE_VALUE_PRIMARY_DISCOVERY_RESPONSE			20
#define		CW_MSG_TYPE_VALUE_DATA_TRANSFER_REQUEST					21
#define		CW_MSG_TYPE_VALUE_DATA_TRANSFER_RESPONSE				22
#define		CW_MSG_TYPE_VALUE_CLEAR_CONFIGURATION_REQUEST			23
#define		CW_MSG_TYPE_VALUE_CLEAR_CONFIGURATION_RESPONSE			24
#define		CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_REQUEST			25
#define		CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_RESPONSE		26
#define		CW_MSG_TYPE_VALUE_WLAN_CONFIG_REQUEST					3398913
#define		CW_MSG_TYPE_VALUE_WLAN_CONFIG_RESPONSE					3398914
#define		CW_MSG_TYPE_VALUE_STA_INFO_REQUEST		   	 			3398915
#define    	CW_MSG_TYPE_VALUE_STA_INFO_RSP 							3398916
#define 	CW_MSG_TYPE_VALUE_STA_NOTIFICATION_RSP      			3398917
#define 	CW_MSG_TYPE_VALUE_DTLS_HANDSHAKE_RSP      				3398918

// Message Elements Type Values
#define 	CW_MSG_ELEMENT_AC_DESCRIPTOR_CW_TYPE					1
#define 	CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE						2
#define 	CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE						3
#define 	CW_MSG_ELEMENT_AC_NAME_CW_TYPE							4
#define 	CW_MSG_ELEMENT_AC_NAME_INDEX_CW_TYPE					5
#define		CW_MSG_ELEMENT_TIMESTAMP_CW_TYPE						6
#define		CW_MSG_ELEMENT_ADD_MAC_ACL_CW_TYPE						7
#define		CW_MSG_ELEMENT_ADD_STATION_CW_TYPE						8
#define		CW_MSG_ELEMENT_ADD_STATIC_MAC_ACL_CW_TYPE				9
#define 	CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE			10
#define 	CW_MSG_ELEMENT_CW_CONTROL_IPV6_ADDRESS_CW_TYPE			11
#define		CW_MSG_ELEMENT_CW_TIMERS_CW_TYPE						12
#define		CW_MSG_ELEMENT_DATA_TRANSFER_DATA_CW_TYPE				13
#define		CW_MSG_ELEMENT_DATA_TRANSFER_MODE_CW_TYPE				14
#define 	CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_CW_TYPE				15
#define 	CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_PERIOD_CW_TYPE		16
#define 	CW_MSG_ELEMENT_DELETE_MAC_ACL_CW_TYPE					17
#define 	CW_MSG_ELEMENT_DELETE_STATION_CW_TYPE					18
#define 	CW_MSG_ELEMENT_DELETE_STATIC_MAC_ACL_CW_TYPE			19
#define 	CW_MSG_ELEMENT_DISCOVERY_TYPE_CW_TYPE					20
#define 	CW_MSG_ELEMENT_DUPLICATE_IPV4_ADDRESS_CW_TYPE			21
#define 	CW_MSG_ELEMENT_DUPLICATE_IPV6_ADDRESS_CW_TYPE			22
#define 	CW_MSG_ELEMENT_IDLE_TIMEOUT_CW_TYPE						23
#define 	CW_MSG_ELEMENT_IMAGE_DATA_CW_TYPE						24
#define 	CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE					25
#define 	CW_MSG_ELEMENT_IMAGE_INFO_CW_TYPE						26
#define 	CW_MSG_ELEMENT_INITIATED_DOWNLOAD_CW_TYPE				27
#define 	CW_MSG_ELEMENT_LOCATION_DATA_CW_TYPE					28
#define 	CW_MSG_ELEMENT_MAX_MSG_LEN_CW_TYPE						29
#define 	CW_MSG_ELEMENT_WTP_IPV4_ADDRESS_CW_TYPE					30
#define 	CW_MSG_ELEMENT_RADIO_ADMIN_STATE_CW_TYPE				31
#define 	CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE				32
#define 	CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE						33
#define 	CW_MSG_ELEMENT_RETURNED_MSG_ELEM_CW_TYPE				34
#define 	CW_MSG_ELEMENT_SESSION_ID_CW_TYPE						35
#define 	CW_MSG_ELEMENT_STATISTICS_TIMER_CW_TYPE					36
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE				37
#define 	CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE					38
#define 	CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE					39
#define 	CW_MSG_ELEMENT_WTP_FALLBACK_CW_TYPE						40
#define 	CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE			41
#define     CW_MSG_ELEMENT_CONFIG_UPDATE_ACTION_TYPE				42
//#define   RESERVERD												43
#define 	CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE						44
#define 	CW_MSG_ELEMENT_WTP_NAME_CW_TYPE							45
#define 	CW_MSG_ELEMENT_WTP_OPERAT_STATISTICS_CW_TYPE			46
#define 	CW_MSG_ELEMENT_WTP_RADIO_STATISTICS_CW_TYPE				47
#define 	CW_MSG_ELEMENT_WTP_REBOOT_STATISTICS_CW_TYPE			48
#define 	CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE_WITH_PAYLOAD			49
#define 	CW_MSG_ELEMENT_WTP_IPV6_ADDRESS_CW_TYPE 				50
#define 	CW_MSG_ELEMENT_CW_TRANSPORT_PROTOCOL_CW_TYPE			51
#define 	CW_MSG_ELEMENT_ECN_SUPPORT_CW_TYPE						53
#define		CW_MSG_ELEMENT_80211N_STATION_CW_TYPE					130
#define		CW_MSG_ELEMENT_ENCRYPTION_EXTEND            			141

#define		CW_MSG_ELEMENT_ADD_WLAN_CW_TYPE							1024
#define		CW_MSG_ELEMENT_ANTENNA_CW_TYPE							1025
#define		CW_MSG_ELEMENT_ASSIGNED_WTP_BSSID_CW_TYPE				1026
#define		CW_MSG_ELEMENT_DELETE_WLAN_CW_TYPE						1027
#define		CW_MSG_ELEMENT_SEQUENCE_CONTROL							1028
#define     CW_MSG_ELEMENT_INFO_ELEMENT_CW_TYPE						1029
#define		CW_MSG_ELEMENT_MAC_OPERATION_CW_TYPE					1030
#define		CW_MSG_ELEMENT_MULTI_DOMAIN_CAPABILITY_CW_TYPE			1032
#define		CW_MSG_ELEMENT_DFDM_COMTROL								1033
#define		CW_MSG_ELEMENT_RATE_SET_CW_TYPE							1034
#define		CW_MSG_ELEMENT_802_DOT_11_STA_CW_TYPE					1036
#define		CW_MSG_ELEMENT_STA_QOS_PROFILE_CW_TYPE					1037
#define		CW_MSG_ELEMENT_STA_SESSION_KEY_CW_TYPE					1038
#define		CW_MSG_ELEMENT_80211_STATISTICS 		    			1039
#define		CW_MSG_ELEMENT_TX_POWER_CW_TYPE							1041
#define		CW_MSG_ELEMENT_TX_POWER_LEVEL							1042
#define		CW_MSG_ELEMENT_UPDATE_WLAN_CW_TYPE						1044
#define		CW_MSG_ELEMENT_WTP_RADIO_CONFIG_CW_TYPE					1046
#define		CW_MSG_ELEMENT_WTP_RADIO_INFO_CW_TYPE					1048
#define		CW_MSG_ELEMENT_WTP_WALLED_GARDEN_CW_TYPE				2065

#define 	CW_IEEE80211_PAYLOAD_HEAD_SIZE							6

#define 	CW_ME_TYPE_VENDOR_MANAGE_PORT										1
#define 	CW_ME_TYPE_VENDOR_SOFTWARE_URL										2
#define 	CW_ME_TYPE_VENDOR_UPTIME											3
#define 	CW_ME_TYPE_VENDOR_BASE_MAC											4
#define 	CW_ME_TYPE_VENDOR_WTP_MODEL											5
#define 	CW_ME_TYPE_VENDOR_WTP_SOFTWARE_VERSION								6
#define 	CW_ME_TYPE_VENDOR_WTP_BOOT_VERSION									7
#define 	CW_ME_TYPE_VENDOR_WTP_REBOOT										8
#define 	CW_ME_TYPE_VENDOR_WTP_RESET											9
#define 	CW_ME_TYPE_VENDOR_RADIO_TYPE										10
#define 	CW_ME_TYPE_VENDOR_RADIO_CONFIG										11
#define 	CW_ME_TYPE_VENDOR_GLOABAL_AUTO_CHANNEL								12
#define 	CW_ME_TYPE_VENDOR_GLOABAL_ROAMING									13		
#define 	CW_ME_TYPE_VENDOR_WLAN_CONFIG										14		
#define 	CW_ME_TYPE_VENDOR_ROAMING_THRESHOLD_2G								15
#define 	CW_ME_TYPE_VENDOR_ROAMING_THRESHOLD_5G								16
#define 	CW_ME_TYPE_VENDOR_TRUST_ID											17


typedef enum
{
	CW_REASON_INIT   				=0,
	CW_REASON_NETWORK_DOWN			=1,
	CW_REASON_HEART_FAIL			=2,
	CW_REASON_DATA_ERROR			=3,
	CW_REASON_RESET					=4,
	CW_REASON_REBOOT				=5,
	CW_REASON_UPGRADE				=6,
}CW_WTP_CONNECT_REASON;

typedef enum {
	CW_PROTOCOL_SUCCESS				= 0, //	Success
	CW_PROTOCOL_FAILURE_AC_LIST			= 1, // AC List message MUST be present
	CW_PROTOCOL_SUCCESS_NAT				= 2, // NAT detected
	CW_PROTOCOL_FAILURE				= 3, // unspecified
	CW_PROTOCOL_FAILURE_RES_DEPLETION		= 4, // Resource Depletion
	CW_PROTOCOL_FAILURE_UNKNOWN_SRC			= 5, // Unknown Source
	CW_PROTOCOL_FAILURE_INCORRECT_DATA		= 6, // Incorrect Data
	CW_PROTOCOL_FAILURE_ID_IN_USE			= 7, // Session ID Alreadyin Use
	CW_PROTOCOL_FAILURE_WTP_HW_UNSUPP		= 8, // WTP Hardware not supported
	CW_PROTOCOL_FAILURE_BINDING_UNSUPP		= 9, // Binding not supported
	CW_PROTOCOL_FAILURE_UNABLE_TO_RESET		= 10, // Unable to reset
	CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR		= 11, // Firmware write error
	CW_PROTOCOL_FAILURE_SERVICE_PROVIDED_ANYHOW	= 12, // Unable to apply requested configuration 
	CW_PROTOCOL_FAILURE_SERVICE_NOT_PROVIDED	= 13, // Unable to apply requested configuration
	CW_PROTOCOL_FAILURE_INVALID_CHECKSUM		= 14, // Image Data Error: invalid checksum
	CW_PROTOCOL_FAILURE_INVALID_DATA_LEN		= 15, // Image Data Error: invalid data length
	CW_PROTOCOL_FAILURE_OTHER_ERROR			= 16, // Image Data Error: other error
	CW_PROTOCOL_FAILURE_IMAGE_ALREADY_PRESENT	= 17, // Image Data Error: image already present
	CW_PROTOCOL_FAILURE_INVALID_STATE		= 18, // Message unexpected: invalid in current state
	CW_PROTOCOL_FAILURE_UNRECOGNIZED_REQ		= 19, // Message unexpected: unrecognized request
	CW_PROTOCOL_FAILURE_MISSING_MSG_ELEM		= 20, // Failure: missing mandatory message element
	CW_PROTOCOL_FAILURE_UNRECOGNIZED_MSG_ELEM	= 21  // Failure: unrecognized message element


} CWProtocolResultCode;


#define	CW_FREE_OBJECTS_ARRAY(ar_name, ar_size)	{int _i = 0; for(_i = ((ar_size)-1); _i >= 0; _i--) {if(((ar_name)[_i]) != NULL){ free((ar_name)[_i]);}} free(ar_name); (ar_name) = NULL; }
#define	CW_PRINT_STRING_ARRAY(ar_name, ar_size)	{int i = 0; for(i = 0; i < (ar_size); i++) printf("[%d]: **%s**\n", i, ar_name[i]);}



#define		CW_COPY_PROTOCOL_MESSAGE(mess, size, messptr, err)		CW_CREATE_OBJECT_SIZE_ERR(((mess).msg), (size), err);		\
									CW_ZERO_MEMORY(((mess).msg), (size));						\
									(mess).offset = messptr.offset;                         \
									memcpy ((mess).msg, (messptr).msg, size);              


															
#define		CWParseMessageElementStart()				int oldOffset;												\
									if(msgPtr == NULL || valPtr == NULL) return LIB_FALSE;	\
									oldOffset = msgPtr->offset
									
#define		CWParseMessageElementStartWithoutvalPtr()	int oldOffset;												\
									if(msgPtr == NULL ) return LIB_FALSE;	\
									oldOffset = msgPtr->offset
						
#define		CWParseMessageElementEnd()	return LIB_TRUE;


LIBBool CWParseFormatMsgElem(CWProtocolMessage *completeMsg,unsigned short int *type,unsigned short int *len);
LIBBool CWParseCWHeader(CWProtocolMessage *msgPtr, CWControlHeaderValues *valPtr);
LIBBool CWParseNoAction(CWProtocolMessage *msgPtr,int len);



LIBBool CWAssembleMessage(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr, int PMTU, unsigned char seqNum, int msgTypeValue, CWProtocolMessage *msgElems, const int msgElemNum, CWProtocolMessage *msgElemsBinding, const int msgElemBindingNum,int is_crypted);

#endif
