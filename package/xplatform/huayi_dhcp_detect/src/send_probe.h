#ifndef __SEND_PROBE__
#define __SEND_PROBE__

typedef struct dhcp_server
{
	char mac[6];
	char ip[4];
}DHCP_SERVER;

typedef struct lisener_agent{
	int socket;
	int detect_count;
	int server_num;
	struct dhcp_server server[128];
}LISENER_Agent;

typedef enum {
	HB_FALSE = 0,
	HB_TRUE = 1
} HB_BOOL;

#define OVER_TIME_INTERVAL  2
#define DHCP_DETECT_RESULT 	"/tmp/dhcp_detect_result"  

#endif
