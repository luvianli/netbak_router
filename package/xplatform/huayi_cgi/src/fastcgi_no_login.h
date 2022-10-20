#ifndef __FASTCGI_NO_LOGIN_HEADER__
#define __FASTCGI_NO_LOGIN_HEADER__
#include "fastcgi_common.h"
#include "huayi_log.h"

typedef enum
{
	HYID_GET_TELNET_CONFIG = 0,
	HYID_SET_TELNET_CONFIG = 1,
    MAX_NO_LOGIN_HYID,
}FGI_HYID_NO_LOGIN;



int (*FastCgiHandleNoLoginFunction[MAX_NO_LOGIN_HYID])(cJSON *request_root, cJSON *response_root,int *lock_time);

#if defined(CONFIG_HUAYI_SUPPORT_CONTROL_TELNET_OPTION)
int cgi_get_telnet_config(cJSON *request_root, cJSON *response_root,int *lock_time);
int cgi_set_telnet_config(cJSON *request_root, cJSON *response_root,int *lock_time);
#endif

#endif
