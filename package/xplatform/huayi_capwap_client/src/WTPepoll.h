#ifndef _WTPEPOLL_HEADER_
#define _WTPEPOLL_HEADER_

#include "WTPcommon.h"

typedef struct eloop_socket_list{
	int socket;
	WTPBool used;
	void (*callback)(void*);
	void* arg;
}ELOOP_SOCKET_LIST;


WTPBool wtp_eloop_init(void);




WTPBool wtp_eloop_destory(void);


WTPBool wtp_eloop_register_socket(int socket,void (*callback)(void*), void* arg);


void wtp_eloop_execution(void);

#endif
