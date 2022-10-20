/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "WTPepoll.h"

int wtp_eloop_fd;

struct eloop_socket_list wtp_eloop_sock[CW_TOTAL_SOCKET_NUM];

WTPBool wtp_eloop_init(void)
{
	memset(wtp_eloop_sock,0,sizeof(wtp_eloop_sock));

	if((wtp_eloop_fd = epoll_create(CW_TOTAL_SOCKET_NUM)) == -1)
	{
		return WTP_FALSE;
	}

	return WTP_TRUE;
}

WTPBool wtp_eloop_destory(void)
{
	close(wtp_eloop_fd);
	
	return WTP_TRUE;
}

WTPBool wtp_eloop_register_socket(int socket,void (*callback)(void*), void* arg)
{
	struct epoll_event event;
	int i = 0;
	event.events = EPOLLIN;
	event.data.fd = socket;
	
	if (epoll_ctl(wtp_eloop_fd, EPOLL_CTL_ADD, socket, &event) == -1) 
	{
		perror("epoll_ctl");
		printf("wtp_eloop_register_socket failed\n");
		return WTP_FALSE;
	}

	for(i=0; i< CW_TOTAL_SOCKET_NUM; i++) {
		if(wtp_eloop_sock[i].used == WTP_FALSE) {
			wtp_eloop_sock[i].used = WTP_TRUE;
			wtp_eloop_sock[i].socket = socket;
			wtp_eloop_sock[i].callback= callback;
			wtp_eloop_sock[i].arg = arg;
			return WTP_TRUE;
		}
	}
	return WTP_FALSE;
}

WTPBool wtp_eloop_remove_socket(int socket)
{
	struct epoll_event event;
	event.data.fd = socket;
	
	if (epoll_ctl(wtp_eloop_fd, EPOLL_CTL_DEL, socket, &event) == -1) 
	{
		perror("epoll_ctl");
		printf("wtp_eloop_register_socket failed\n");
		return WTP_FALSE;
	}
	return WTP_TRUE;
}

void wtp_eloop_execution(void)
{
	struct epoll_event events[20];
	int nfds = 0;
	int i = 0,k = 0;
	int epoll_socket;
	
	nfds=epoll_wait(wtp_eloop_fd,events,CW_TOTAL_SOCKET_NUM,-1);

	for(i=0;i<nfds;i++)
	{
		 if(events[i].events&EPOLLIN)                
		 {
		 	
		 	if ((epoll_socket = events[i].data.fd) < 0)
		 	{
				continue;
		 	}

			for(k=0;k<CW_TOTAL_SOCKET_NUM;k++)
			{	
				if(epoll_socket == wtp_eloop_sock[k].socket)
				{
					if(wtp_eloop_sock[k].callback != NULL)
					{
						wtp_eloop_sock[k].callback(wtp_eloop_sock[i].arg);
					}
				}
			}
		 }
	}
	
	return;
}
