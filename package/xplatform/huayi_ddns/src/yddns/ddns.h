/*==============================================================================
//
// Project:
//		userapp-ddns module
//
//------------------------------------------------------------------------------
// Description:
//
//         The inclde file of ddns module
==============================================================================*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
//==============================================================================
//                                    MACROS
//==============================================================================
#define DYNDNS_NAME "dyndns"
#define DYNDNS_DEFAULT_SERVER "members.dyndns.org"
#define DYNDNS_DEFAULT_PORT 80
#define DYNDNS_REQUEST "/nic/update"
#define DYNDNS_STAT_REQUEST "/nic/update"
#define DYNDNS_MAX_INTERVAL (25*24*3600)

#define QDNS_NAME "qdns"
#define QDNS_DEFAULT_SERVER "members.3322.net"
#define QDNS_DEFAULT_PORT 80
#define QDNS_REQUEST "/dyndns/update"
#define QDNS_STAT_REQUEST "/dyndns/update"
#define QDNS_MAX_INTERVAL (25*24*3600)

#define TZO_NAME "tzo"
#define TZO_DEFAULT_SERVER "cgi.tzo.com"
#define TZO_DEFAULT_PORT 80
#define TZO_REQUEST "/webclient/signedon.html"
#define TZO_SUCCESS 1


#define NOIP_NAME "noip"
#define NOIP_DEFAULT_SERVER "dynupdate.no-ip.com"
#define NOIP_DEFAULT_PORT 80
#define NOIP_REQUEST ""
#define NOIP_STAT_REQUEST ""
#define NOIP_MAX_INTERVAL (25*24*3600)
#define NOIP_ALREADYSET 0
#define NOIP_SUCCESS 1

#define INTELBRAS_NAME "intelbras"
#define INTELBRAS_DEFAULT_SERVER "www.ddns-intelbras.com.br"
#define INTELBRAS_DEFAULT_PORT 80
#define INTELBRAS_REQUEST "/add.aspx"
#define INTELBRAS_STAT_REQUEST ""
#define INTELBRAS_MAX_INTERVAL (25*24*3600)
#define INTELBRAS_PORT 80
#define INTELBRAS_DEVMODEL "NCLOUD"
#define INTELBRAS_HDDMODEL 1
#define INTELBRAS_DEVSN 14
#define INTELBRAS_WIFI "true"
#define INTELBRAS_VOIP "false"
#define INTELBRAS_PRODUCT "Roteador"
#define NSTR(x)			inet_ntoa(*(struct in_addr *)&x)

#define MY_IP			SYS_wan_ip
#define MY_IP_STR		NSTR(SYS_wan_ip)

#define DEVICE_NAME		"DDNS Server"
#define FW_VERSION		"V1.1"
#define FW_OS			"LINUX"
#define AUTHOR			"by chenfei"

#define SUCCESS_INTERVAL (24*3600)
#define FAIL_INTERVAL    (10*60)

#include <huayi-universal/huayi_uci_fun.h>



//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================
struct service_cxy
{
  char *name;
  int (*update_entry)();
  char *default_server;
  unsigned short default_port;
  char *default_request;
};

enum {
  UPDATERES_OK = 0,
  UPDATERES_OK_2,
  UPDATERES_ERROR,
  UPDATERES_SHUTDOWN,
};

struct userInfo_cxy{
	struct service_cxy *service;
	char usrname[65];
	char usrpwd[65];
	char devmac[18];
	char host[65];
	char mx[65];
	char backmx[65];
	char url[130];
	char wildcard;
	int trytime;
	int ip;
	int updated_time;
	char status;
	struct userInfo_cxy *next;
};


