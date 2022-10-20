#ifndef _HUAYI_CLOUD_CLIENT_HEADER_
#define _HUAYI_CLOUD_CLIENT_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include "huayi_uci_fun.h"
#include "curl/curl.h"



#define HEART_DOMAIN_DEFAULT "http://8.129.188.169:8080/admin/ylapi/uploadData"
#define HEART_PORT_DEFAULT 8888
#define HEART_DOMAIN_SUFFIX "api/sys.heart" 
#define HEART_REQ_DATALEN  20480
#define HEART_RES_DATALEN  20480

#define WAN_TYPE_4G		"mobile"

/*curl call back func define */
typedef int (callback_func)(char *data, size_t size, size_t nmemb, char *userData); 

#endif
