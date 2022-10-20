#include <cJSON.h>
#include <stdio.h>

#define PACKED  __attribute__ ((packed))
#define USHORT  unsigned short
#define UCHAR   unsigned char
typedef struct PACKED _WSC_CONFIGURED_VALUE {
    USHORT  WscConfigured; // 1 un-configured; 2 configured
    UCHAR   WscSsid[32 + 1];
    USHORT  WscAuthMode;   // mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x
    USHORT  WscEncrypType; // 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
    UCHAR   DefaultKeyIdx;
    UCHAR   WscWPAKey[64 + 1];
} WSC_CONFIGURED_VALUE;

typedef struct PACKED _NDIS80211SSID
{
	unsigned int    SsidLength;   // length of SSID field below, in bytes;
                                  // this can be zero.
	unsigned char   Ssid[32];     // SSID information field
} NDIS80211SSID;

// WSC configured credential
typedef struct  _WSC_CREDENTIAL
{
    NDIS80211SSID       SSID;               // mandatory
    USHORT              AuthType;           // mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa-psk2
    USHORT              EncrType;           // mandatory, 1: none, 2: wep, 4: tkip, 8: aes
    UCHAR               Key[64];            // mandatory, Maximum 64 byte
    USHORT              KeyLength;
    UCHAR               MacAddr[6];         // mandatory, AP MAC address
    UCHAR               KeyIndex;           // optional, default is 1
    UCHAR               Rsvd[3];            // Make alignment
}   WSC_CREDENTIAL, *PWSC_CREDENTIAL;


// WSC configured profiles
typedef struct  _WSC_PROFILE
{
#ifndef UINT
#define UINT	unsigned long
#endif
    UINT           	ProfileCnt;
    UINT		    ApplyProfileIdx;  // add by johnli, fix WPS test plan 5.1.1
    WSC_CREDENTIAL  Profile[8];       // Support up to 8 profiles
}   WSC_PROFILE, *PWSC_PROFILE;


