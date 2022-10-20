#ifndef __ATCMDLIB_H__
#define __ATCMDLIB_H__

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define GSM         "2G"
#define UTRAN       "3G"
#define EGPRS       "E"
#define HSDPA       "H+"
#define E_UTRAN     "4G"
#define NSA         "5G-NSA"


#define AT_RESULT_FILE		"/var/atresult_1"
#define AT_CMD_LOCK_FILE	"/var/atlockfile"
#define ATCM_UNIX_DOMAIN	"/var/run/atcm_socket.pid"

#define DEFAULT_DEV_PATH "/dev"

#if defined(CONFIG_PACKAGE_KMOD_USB_ACM)
#define DEFAULT_DEV_ATC "/dev/ttyACM0"
#else
#define DEFAULT_DEV_ATC "/dev/ttyUSB0"
#endif

#define DEFAULT_MF210_DEV_ATC "/dev/ttyUSB2"


#define AT_TIMEOUT_SEC		3
#define AT_TIMEOUT_USEC		0

#define AT_VALUE_BUFF_LEN	64
#define AT_CMD_BUFF_LEN		128
#define AT_LONG_BUFF_LEN	256


#define MAX_SMS_VALUE_LEN			16
#define MAX_SMS_MSG_LEN				2048

#define GET_SMS_INTERVAL							6

#define AT_CMD_MAX_LEN		1024


#define GSM_7BIT        0
#define GSM_8BIT        4
#define GSM_UCS2        8


typedef struct{
   unsigned char TP_len; 
   unsigned char TP_udhi;  
   unsigned char TP_llen;
   unsigned char TP_id1;
   unsigned char TP_id2;
   unsigned char Pk_total;
   unsigned char Pk_number;
} LONG_MSG;

typedef struct {
    char SCA[MAX_SMS_VALUE_LEN];
    char TPA[MAX_SMS_VALUE_LEN];
    char TP_PID;
    char TP_DCS;
    char TP_SCTS[MAX_SMS_VALUE_LEN];
    char TP_UD[MAX_SMS_MSG_LEN];
    char index;
    LONG_MSG long_msg;
} SM_PARAM;


typedef enum{
	LTE_FALSE,
	LTE_TRUE,
}LTE_BOOL;

enum {
	ATCMD_GET=0,
	ATCMD_SET,
};


enum {
	GobiNet=0,
	ECM,
};


enum {
	MODEM_SIN_NOT_INSERTED	=0,
	MODEM_SIN_READY			=1,
	MODEM_SIN_PIN			=2,
	MODEM_SIN_PUK			=3,
	MODEM_SIN_ERROR			=4,
	MODEM_SIN_BUSY			=5,
	MODEM_SIN_UNKNOW		=6,
	MODEM_SIN_MAX,
};

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
extern char ATret[AT_CMD_MAX_LEN];


extern void at_value_get(char *val, char *src, int size, char end );
extern LTE_BOOL Atcmd(char *cmd, int len, int action);
extern LTE_BOOL AtcmdSendDelay(char *cmd, int len, int action);
extern LTE_BOOL AtcmdSendSms(char *cmd ,int cmd_len, char *pdu, int pdu_len);

extern LTE_BOOL atcmd_get_usbnet(int *value);
extern LTE_BOOL atcmd_get_cpin(int *value);
extern LTE_BOOL atcmd_set_cpin(char *pin);
extern LTE_BOOL atcmd_get_cpinr(int *pin_num, int *puk_num);
extern LTE_BOOL atcmd_get_clck(int *sim_clck);
extern LTE_BOOL atcmd_set_clck(int sim_clck, char *sim_pin);
extern LTE_BOOL atcmd_set_cpwd(char *old_pin, char *new_pin);
extern LTE_BOOL atcmd_get_signal(int *value);
extern LTE_BOOL atcmd_get_imsi(char *imsi);
extern LTE_BOOL atcmd_get_mccmnc(char *ccmnc);
extern LTE_BOOL atcmd_get_4gmode(char *netmode);
extern LTE_BOOL atcmd_check(void);
extern LTE_BOOL atcmd_get_quectel_bandLte(char *vlaue);
extern LTE_BOOL atcmd_get_EC20_imei(char *value);
extern LTE_BOOL restart_4gradio(void);
extern LTE_BOOL read_sms_center_number(char* center_number, int *center_len);


extern int string2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);
extern int bytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength);
extern int scanMultiBytes(const char *pSrc);
extern int encode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
extern int encode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
extern int decode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
extern int encode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
extern int decode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
extern int encodeUcs2(const char* pSrc, unsigned char* pDst, int nSrcLength);
extern int decodeUcs2(const unsigned char* pSrc, char* pDst, int nSrcLength);
extern int invertNumbers(const char* pSrc, char* pDst, int nSrcLength);
extern int serializeNumbers(const char* pSrc, char* pDst, int nSrcLength);
extern int rssi2dbm(char *rssi);
extern int rsrq2dbm(char *rsrq);
extern int rsrp2dbm(char *rsrp);
extern int type2string(char *type, char *pDst);
extern int formatNumber(const char* pSrc, char* pDst, int nSrcLength);

extern int send_sms(char* phone_number, char *center_number, char* sms_msg);
#endif

#endif /* __ATCMDLIB_H__ */
