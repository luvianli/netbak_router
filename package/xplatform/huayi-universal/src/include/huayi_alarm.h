#ifndef __HUAYI_ALARM_H__
#define __HUAYI_ALARM_H__

#define CNOTE_MAC_LEN           6
#define CNOTE_MAX_DEVICE_ID     10
#define CNOTE_MAX_REASON_SHORT  20
#define CNOTE_MAX_REASON        100
#define CNOTE_MAX_VERSION       20
#define CNOTE_MAX_WIRELESS_MODE 20
#define CNOTE_MAX_FILENAME      20
#define CNOTE_MAX_CONFIG        100
#define CNOTE_MAX_SSID          32
#define CNOTE_BUF_LEN			175

typedef enum {
    // sta
    CNOTE_STA_ASSOCIATION = 0,
    CNOTE_USB_STORAGE_ADD,
    CNOTE_USB_STORAGE_REMOVE,
    CNOTE_HUAYI_BUTTON_PRESSED,
    CNOTE_HUAYI_BUTTON_RELEASED,
    CNOTE_HUAYI_SCHE_REBOOT_UPDATE,
    CNOTE_HUAYI_UPDATE_AC_CONFIG,
    CNOTE_CHECK_IP_ADDR_CHG,
    CNOTE_MAX_ID,    // the last one
} HUAYI_NOTE_ID;

#define HUAYI_NOTE_ID_TOSTR(id) \
 ((id == CNOTE_STA_ASSOCIATION) ? "STA association" : \
  (id == CNOTE_USB_STORAGE_ADD) ? "Usb storage Add" : \
  (id == CNOTE_USB_STORAGE_REMOVE) ? "Usb storage Remove" : \
  (id == CNOTE_HUAYI_BUTTON_PRESSED) ? "button pressed" : \
  (id == CNOTE_HUAYI_BUTTON_RELEASED) ? "button released" : \
  (id == CNOTE_HUAYI_SCHE_REBOOT_UPDATE) ? "sche reboot update" : \
  (id == CNOTE_HUAYI_UPDATE_AC_CONFIG) ? "update AC config" : \
  (id == CNOTE_CHECK_IP_ADDR_CHG) ? "IP address change" : "unknown id")

#define HUAYI_NOTE_ID_TOCMD(id) \
 ((id == CNOTE_STA_ASSOCIATION) ? "association" : \
 (id == CNOTE_USB_STORAGE_ADD) ? "usb_storage_add" : \
 (id == CNOTE_USB_STORAGE_REMOVE) ? "usb_storage_remove" : \
 (id == CNOTE_HUAYI_BUTTON_PRESSED) ? "button_pressed" : \
 (id == CNOTE_HUAYI_BUTTON_RELEASED) ?"button_released" : \
 (id == CNOTE_HUAYI_SCHE_REBOOT_UPDATE) ?"sche_reboot_update" : \
 (id == CNOTE_HUAYI_UPDATE_AC_CONFIG) ?"update_ac_config" : \
 (id == CNOTE_CHECK_IP_ADDR_CHG) ? "check_ip_change" : "invalid")

#define HUAYI_NOTE_ID_TOUSAGE(id) \
 ((id == CNOTE_STA_ASSOCIATION) ? "not_supported" : \
 (id == CNOTE_USB_STORAGE_ADD) ? "usb_storage_add" : \
 (id == CNOTE_USB_STORAGE_REMOVE) ? "usb_storage_remove" : \
 (id == CNOTE_HUAYI_BUTTON_PRESSED) ? "button_pressed" : \
 (id == CNOTE_HUAYI_BUTTON_RELEASED) ? "button_released" : \
 (id == CNOTE_HUAYI_SCHE_REBOOT_UPDATE) ? "sche_reboot_update" : \
 (id == CNOTE_HUAYI_UPDATE_AC_CONFIG) ? "update_ac_config" : \
 (id == CNOTE_CHECK_IP_ADDR_CHG) ? "check_ip_change" : "invalid")

typedef enum {
    CNOTE_TYPE_STA      = 0,
    CNOTE_TYPE_SYSTEM   = 1,
    CNOTE_TYPE_WIRELESS = 2,
    CNOTE_TYPE_WAPI     = 3,
} HUAYI_NOTE_TYPE;

#define HUAYI_NOTE_TYPE_TOSTR(e) \
 ((e == CNOTE_TYPE_STA)      ? "STA"      : \
  (e == CNOTE_TYPE_SYSTEM)   ? "SYSTEM"   : \
  (e == CNOTE_TYPE_WIRELESS) ? "WIRELESS" : \
  (e == CNOTE_TYPE_WAPI)     ? "WAPI"     : "unknown")


typedef enum {
    CNOTE_AUTH_SUCC = 0,
    CNOTE_AUTH_FAIL,
    CNOTE_ASSO_SUCC,
    CNOTE_ASSO_FAIL,
    CNOTE_DISASSO,
    CNOTE_REASSO,
    CNOTE_WDS_ADD,
    CNOTE_WDS_DEL,
} HUAYI_NOTE_STA_ASSOCIATION__STATUS;

#define HUAYI_NOTE_ASSOSTAT_TOSTR(e) \
 ((e == CNOTE_AUTH_SUCC) ? "Auth Succ" : \
  (e == CNOTE_AUTH_FAIL) ? "Auth Fail" : \
  (e == CNOTE_ASSO_SUCC) ? "Asso Succ" : \
  (e == CNOTE_ASSO_FAIL) ? "Asso Fail" : \
  (e == CNOTE_DISASSO)   ? "Disassociated" : \
  (e == CNOTE_REASSO)    ? "Reassociation" : \
  (e == CNOTE_WDS_ADD)   ? "WDS Add" : \
  (e == CNOTE_WDS_DEL)   ? "WDS Del" : "unknown")


#define CNOTE_REASON_OFFSET   50
#define CNOTE_STATUS_TOREASON(_s) ((_s==0||_s==1)?_s:(CNOTE_REASON_OFFSET + _s))

typedef enum {
	 /*
     * ref: atheros-9.2/wlan/include/ieee80211.h
     * ref: atheros-9.2/apps/wapid/include/wai_sta.h
     */
    CNOTE_REASON_SUCCESS            = 0,
    CNOTE_REASON_UNSPECIFIED        = 1,
    CNOTE_REASON_AUTH_EXPIRE        = 2,
    CNOTE_REASON_AUTH_LEAVE         = 3,
    CNOTE_REASON_ASSOC_EXPIRE       = 4,
    CNOTE_REASON_ASSOC_TOOMANY      = 5,
    CNOTE_REASON_NOT_AUTHED         = 6,
    CNOTE_REASON_NOT_ASSOCED        = 7,
    CNOTE_REASON_ASSOC_LEAVE        = 8,
    CNOTE_REASON_ASSOC_NOT_AUTHED   = 9,
    CNOTE_REASON_RSN_REQUIRED       = 11,
    CNOTE_REASON_RSN_INCONSISTENT   = 12,
    CNOTE_REASON_IE_INVALID         = 13,
    CNOTE_REASON_MIC_FAILURE        = 14,
    CNOTE_REASON_QOS                = 32,
    CNOTE_REASON_QOS_BANDWITDH      = 33,
    CNOTE_REASON_QOS_CH_CONDITIONS  = 34,
    CNOTE_REASON_QOS_TXOP           = 35,
    CNOTE_REASON_QOS_LEAVE          = 36,
    CNOTE_REASON_QOS_DECLINED       = 37,
    CNOTE_REASON_QOS_SETUP_REQUIRED = 38,
    CNOTE_REASON_QOS_TIMEOUT        = 39,
    CNOTE_REASON_QOS_CIPHER         = 45,
    CNOTE_REASON_CAPINFO            = CNOTE_REASON_OFFSET+10,
    CNOTE_REASON_ASSOC_NOT_EXIST    = CNOTE_REASON_OFFSET+11,
    CNOTE_REASON_OTHER              = CNOTE_REASON_OFFSET+12,
    CNOTE_REASON_ALG                = CNOTE_REASON_OFFSET+13,
    CNOTE_REASON_SEQUENCE           = CNOTE_REASON_OFFSET+14,
    CNOTE_REASON_CHALLENGE          = CNOTE_REASON_OFFSET+15,
    CNOTE_REASON_TIMEOUT            = CNOTE_REASON_OFFSET+16,
    CNOTE_REASON_TOOMANY            = CNOTE_REASON_OFFSET+17,
    CNOTE_REASON_BASIC_RATE         = CNOTE_REASON_OFFSET+18,
    CNOTE_REASON_SP_REQUIRED        = CNOTE_REASON_OFFSET+19,
    CNOTE_REASON_PBCC_REQUIRED      = CNOTE_REASON_OFFSET+20,
    CNOTE_REASON_CA_REQUIRED        = CNOTE_REASON_OFFSET+21,
    CNOTE_REASON_TOO_MANY_STATIONS  = CNOTE_REASON_OFFSET+22,
    CNOTE_REASON_RATES              = CNOTE_REASON_OFFSET+23,
    CNOTE_REASON_SHORTSLOT_REQUIRED = CNOTE_REASON_OFFSET+25,
    CNOTE_REASON_DSSSOFDM_REQUIRED  = CNOTE_REASON_OFFSET+26,
    CNOTE_REASON_NO_HT              = CNOTE_REASON_OFFSET+27,
    CNOTE_REASON_REFUSED            = CNOTE_REASON_OFFSET+37,
    CNOTE_REASON_INVALID_PARAM      = CNOTE_REASON_OFFSET+38,

    CNOTE_REASON_WAPI_ADDR_REDIRECT  = 52,
    CNOTE_REASON_WAPI_LOW_SAFE_LEVEL = 53,
    CNOTE_REASON_WAPI_STA_REPLAY     = 54,
    CNOTE_REASON_WAPI_TAMPER_ATTACK  = 55,
    CNOTE_REASON_WAPI_ILLEGAL_CERT   = 56,

} HUAYI_NOTE_STA_ASSOCIATION__REASON_CODE;

#define HUAYI_NOTE_REASON_TOSTR(e) \
 ((e == CNOTE_REASON_SUCCESS)            ? "Successful" : \
  (e == CNOTE_REASON_UNSPECIFIED)        ? "Unspecified reason" : \
  (e == CNOTE_REASON_AUTH_EXPIRE)        ? "Previous authentication no longer valid" : \
  (e == CNOTE_REASON_AUTH_LEAVE)         ? "Deauthenticated because sending STA is leaving (or has left) IBSS or ESS" : \
  (e == CNOTE_REASON_ASSOC_EXPIRE)       ? "Disassociated due to inactivity" : \
  (e == CNOTE_REASON_ASSOC_TOOMANY)      ? "Disassociated because AP is unable to handle all currently associated STAs" : \
  (e == CNOTE_REASON_NOT_AUTHED)         ? "Class 2 frame received from non authenticated STA" : \
  (e == CNOTE_REASON_NOT_ASSOCED)        ? "Class 3 frame received from non associated STA" : \
  (e == CNOTE_REASON_ASSOC_LEAVE)        ? "Disassociated because sending STA is leaving (or has left) BSS" : \
  (e == CNOTE_REASON_ASSOC_NOT_AUTHED)   ? "STA requesting (re)association is not authenticated with responding STA" : \
  (e == CNOTE_REASON_RSN_REQUIRED)       ? "Disassociated because the RSN credentials is missing" : \
  (e == CNOTE_REASON_RSN_INCONSISTENT)   ? "Disassociated due to RSN credentials inconsistent" : \
  (e == CNOTE_REASON_IE_INVALID)         ? "Invalid information element" : \
  (e == CNOTE_REASON_MIC_FAILURE)        ? "Message integrity code (MIC) failure" : \
  (e == CNOTE_REASON_QOS)                ? "Disassociated for unspecified, QoS-related reason" : \
  (e == CNOTE_REASON_QOS_BANDWITDH)      ? "Disassociated because QoS AP lacks sufficient bandwidth for this QoS STA" : \
  (e == CNOTE_REASON_QOS_CH_CONDITIONS)  ? "Disassociated due to poor channel conditions" : \
  (e == CNOTE_REASON_QOS_TXOP)           ? "Disassociated because STA is transmitting outside the limits of its TXOPs" : \
  (e == CNOTE_REASON_QOS_LEAVE)          ? "Requested from peer STA as the STA is leaving the BSS (or resetting)" : \
  (e == CNOTE_REASON_QOS_DECLINED)       ? "Requested from peer STA as it does not want to use the mechanism" : \
  (e == CNOTE_REASON_QOS_SETUP_REQUIRED) ? "Requested from peer STA as the STA received frames using mechanism requires setup" : \
  (e == CNOTE_REASON_QOS_TIMEOUT)        ? "Requested from peer STA due to timeout" : \
  (e == CNOTE_REASON_QOS_CIPHER)         ? "Peer STA does not support the requested cipher suite" : \
  (e == CNOTE_REASON_CAPINFO)            ? "Cannot support all requested capabilities in the Capability Information field" : \
  (e == CNOTE_REASON_ASSOC_NOT_EXIST)    ? "Reassociation denied due to inability to confirm that association exists" : \
  (e == CNOTE_REASON_OTHER)              ? "Association denied due to reason outside the scope of this standard" : \
  (e == CNOTE_REASON_ALG)                ? "Responding STA does not support the specified authentication algorithm" : \
  (e == CNOTE_REASON_SEQUENCE)           ? "Received an Authentication frame with invalid transaction sequence number" : \
  (e == CNOTE_REASON_CHALLENGE)          ? "Authentication rejected because of challenge failure" : \
  (e == CNOTE_REASON_TIMEOUT)            ? "Authentication rejected due to timeout waiting for next frame in sequence" : \
  (e == CNOTE_REASON_TOOMANY)            ? "Association denied because AP is unable to handle additional associated STAs" : \
  (e == CNOTE_REASON_BASIC_RATE)         ? "Association denied due to requesting STA not supporting all data rates" : \
  (e == CNOTE_REASON_SP_REQUIRED)        ? "Association denied due torequesting STA not supporting the short preamble option" : \
  (e == CNOTE_REASON_PBCC_REQUIRED)      ? "Association denied due torequesting STA not supporting the PBCC modulation option" : \
  (e == CNOTE_REASON_CA_REQUIRED)        ? "Association denied due torequesting STA not supporting the Channel Agility option" : \
  (e == CNOTE_REASON_TOO_MANY_STATIONS)  ? "Association denied because AP is unable to handle additional associated STAs" : \
  (e == CNOTE_REASON_RATES)              ? "Association denied due to requesting STA not supporting all data rates" : \
  (e == CNOTE_REASON_SHORTSLOT_REQUIRED) ? "Association denied due to requesting STA not supporting the Short Slot Time option" : \
  (e == CNOTE_REASON_DSSSOFDM_REQUIRED)  ? "Association denied due to requesting STA not supporting the DSSS-OFDM option" : \
  (e == CNOTE_REASON_NO_HT)              ? "Association denied because the requesting STA does not support HT features" : \
  (e == CNOTE_REASON_REFUSED)            ? "The request has been declined" : \
  (e == CNOTE_REASON_INVALID_PARAM)      ? "The request has not been successful as one or more parameters have invalid values" : \
  (e == CNOTE_REASON_WAPI_ADDR_REDIRECT) ? "WAPI - Address Redirection Attack" : \
  (e == CNOTE_REASON_WAPI_LOW_SAFE_LEVEL) ? "WAPI - Low Safe Level Attack" : \
  (e == CNOTE_REASON_WAPI_STA_REPLAY)    ? "WAPI - STA Repititive Attack" : \
  (e == CNOTE_REASON_WAPI_TAMPER_ATTACK) ? "WAPI - Tamper Attack" : \
  (e == CNOTE_REASON_WAPI_ILLEGAL_CERT)  ? "WAPI - User with invalid cert inbreak network" : "unknown")


typedef enum {
    CNOTE_NORMAL = 0,
    CNOTE_MONITOR,
    CNOTE_SEMI_MONITOR,
} HUAYI_NOTE_AP_MT_WORK_MODE_CHG__MONITOR_MODE;

typedef enum {
    CNOTE_MANUAL = 0,
    CNOTE_AUTO,
} HUAYI_NOTE_AP_CHANNEL_CHG__CHANGE_MODE;

typedef struct huayi_notification {
    HUAYI_NOTE_ID   notification_id;
    HUAYI_NOTE_TYPE type;
    unsigned long  time;
    u_int64_t      status;
    char           reason[CNOTE_MAX_REASON_SHORT];

    union {
        /* common header for device_id indexing */
        struct {
            u_int8_t ap_macaddr[CNOTE_MAC_LEN];
            char     device_id[CNOTE_MAX_DEVICE_ID];
        } device;

        /* STA */
        struct {
            char     device_id[CNOTE_MAX_DEVICE_ID];
            u_int8_t vap_id;
            char     bssid[CNOTE_MAC_LEN];
            char     ssid[CNOTE_MAX_SSID];
            u_int8_t client_macaddr[CNOTE_MAC_LEN];
            u_int8_t status;
            u_int8_t auth_mode;
            u_int8_t channel;
            int8_t   rssi;
            u_int8_t qos;
            u_int8_t reason_code;
            char     reason[CNOTE_MAX_REASON];
        } sta_association;

        /* SYSTEM */
		struct{
			u_int8_t pre_ip_addr;
            u_int8_t cur_ip_addr;
		}ip_addr_chg;
    } info;

} huayi_notification;

#endif /* __HUAYI_ALARM_H__ */
