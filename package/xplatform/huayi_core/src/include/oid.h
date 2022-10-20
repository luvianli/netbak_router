
#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE				0x8BE0
#endif
#define SIOCIWFIRSTPRIV				SIOCDEVPRIVATE
#endif

#ifndef AP_MODE
#define RT_PRIV_IOCTL				(SIOCIWFIRSTPRIV + 0x0E)
#else
#define RT_PRIV_IOCTL				(SIOCIWFIRSTPRIV + 0x01)
#endif	/* AP_MODE */


#define RTPRIV_IOCTL_GET_MAC_TABLE			(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_WSC_PROFILE			(SIOCIWFIRSTPRIV + 0x12)
#define RTPRIV_IOCTL_GET_BSSIDX_STANUM		(SIOCIWFIRSTPRIV + 0x1D)


#define RT_OID_SYNC_RT61                     0x0D010750
#define RT_OID_WSC_QUERY_STATUS              ((RT_OID_SYNC_RT61 + 0x01) & 0xffff)
#define RT_OID_WSC_PIN_CODE                  ((RT_OID_SYNC_RT61 + 0x02) & 0xffff)

#define RT_OID_802_11_WSC_QUERY_PROFILE      0x0750
#define OID_802_11_ACL_LIST                  0x052A
#define OID_802_11_STATISTICS                0x060E
#define RT_OID_APCLI_QUERY_STATUS            0x0990

#define MAX_NUMBER_OF_ACL	64

#define MAX_NUMBER_OF_MAC   116 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211


typedef struct _RT_802_11_ACL_ENTRY {
	unsigned char Addr[6];
	unsigned short Rsv;
} RT_802_11_ACL_ENTRY, *PRT_802_11_ACL_ENTRY;

typedef struct _RT_802_11_ACL {
	unsigned long Policy;           /* 0-disable, 1-positive list, 2-negative list */
	unsigned long Num;
	RT_802_11_ACL_ENTRY Entry[MAX_NUMBER_OF_ACL];
} RT_802_11_ACL, *PRT_802_11_ACL;

typedef union _MACHTTRANSMIT_7603_SETTING {
	struct  {
		unsigned short  MCS:7;  // MCS
		unsigned short  BW:1;   //channel bandwidth 20MHz or 40 MHz
		unsigned short  ShortGI:1;
		unsigned short  STBC:2; //SPACE
		unsigned short	rsv:3;
		unsigned short  MODE:2; // Use definition MODE_xxx.
	} field;
	unsigned short      word;
} MACHTTRANSMIT_7603_SETTING;

typedef union _MACHTTRANSMIT_7663_SETTING {
	struct  {
		unsigned short  MODE:3; // Use definition MODE_xxx.
		unsigned short  iTxBF:1;
		unsigned short  eTxBF:1;
		unsigned short  STBC:1; //SPACE
		unsigned short  ShortGI:1;
		unsigned short  BW:2;   //channel bandwidth 20MHz or 40 MHz
		unsigned short  ldpc:1;
		unsigned short  MCS:6;  // MCS
	} field;
	unsigned short      word;
} MACHTTRANSMIT_7663_SETTING;

typedef struct _RT_802_11_7603_MAC_ENTRY {
	unsigned char			ApIdx;
	unsigned char           Addr[6];
	unsigned char           Aid;
	unsigned char           Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char           MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char                    AvgRssi0;
	char                    AvgRssi1;
	char                    AvgRssi2;
	unsigned int            ConnectedTime;
	MACHTTRANSMIT_7603_SETTING	TxRate;
	unsigned int			LastRxRate;
	short					StreamSnr[3];
	short					SoundingRespSnr[3];
} RT_802_11_7603_MAC_ENTRY;

typedef struct _RT_802_11_7663_MAC_ENTRY {
	unsigned char			ApIdx;
	unsigned char           Addr[6];
	unsigned char           Aid;
	unsigned char           Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char           MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char                    AvgRssi0;
	char                    AvgRssi1;
	char                    AvgRssi2;
	unsigned int            ConnectedTime;
	MACHTTRANSMIT_7663_SETTING	TxRate;
	unsigned int			LastRxRate;
	short					StreamSnr[3];
	short					SoundingRespSnr[3];
} RT_802_11_7663_MAC_ENTRY;


typedef struct _RT_802_11_7603_MAC_TABLE {
	unsigned long            Num;
	RT_802_11_7603_MAC_ENTRY      Entry[MAX_NUMBER_OF_MAC]; //MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_7603_MAC_TABLE;

typedef struct _RT_802_11_7663_MAC_TABLE {
	unsigned long            Num;
	RT_802_11_7663_MAC_ENTRY      Entry[MAX_NUMBER_OF_MAC]; //MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_7663_MAC_TABLE;


