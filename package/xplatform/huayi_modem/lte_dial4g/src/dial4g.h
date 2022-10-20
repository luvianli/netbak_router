
#ifndef _DIAL4G_MAIN_H_
#define _DIAL4G_MAIN_H_

#include <uci.h>
#include "huayi_uci_fun.h"

#include "huayi_atcmd.h"
#include "huayi_sms.h"

#define IFNAME_3G_DEV	"3g-wan4g"


extern int debug;
#define yprintf(fmt, args...) do { \
	if (debug){ printf("(%s:%d)=> " fmt, __FUNCTION__, __LINE__, ## args); } \
	}while(0)



enum connectType{
	CT_23G,
	CT_4G,
	CT_NO_SERVICE,
};


enum {
	MODEM_IDLE=0,
	MODEM_NO_MODEM,
	MODEM_NO_CONN,
	MODEM_NO_SIM,
	MODEM_SIN_LOCK,
	MODEM_PUK_LOCK,
	MODEM_CONN_SIG_L,
	MODEM_CONN_SIG_M,
	MODEM_CONN_SIG_H,
	MODEM_STATUS_MAX,
};

extern int lte_led_status_old;


int net_usb_link(struct SMSAgent *Agent);
int net_3g_link(void);
int net_ttyusb_sysinfo(void);
int net_ttyusb_cgreg(void);
int get_common_4gmode(void);

#endif

