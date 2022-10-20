/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
 
#include "WTPsocket.h"
#include "WTPmessagehandler.h"

WTPBool wtp_send_msg_to_ac(struct VcapwapAgent *Agent,CWProtocolMessage **messagesPtr,int fragmentsNum)
{
	int sendbyte = 0;
	int size;
	unsigned char *sendMsg = NULL;

   	sendMsg = messagesPtr[0]->msg;

	size =  messagesPtr[0]->offset;
	
	sendbyte = sendto(Agent->master_socket.socket, sendMsg, size, 0, (struct sockaddr *)&Agent->master_socket.address, sizeof(Agent->master_socket.address));
	
	if (sendbyte != size)
	{
		CW_FREE_PROTOCOL_MESSAGE(*messagesPtr[0]);
		CW_FREE_OBJECT(*messagesPtr);
			
		return WTP_FALSE;
	}
	
	CW_FREE_PROTOCOL_MESSAGE(*messagesPtr[0]);
	CW_FREE_OBJECT(*messagesPtr);
	return WTP_TRUE;
}

void wtp_master_receive_msg(struct VcapwapAgent *Agent)
{
	unsigned char recv_array[MAX_UDP_MSG_SIZE];
	unsigned int  addr_len = sizeof(Agent->master_socket.address);
	unsigned int  recv_byte;
	struct WTPManageSocket response_socket;
	
	memset(recv_array, 0, sizeof(recv_array));
	
	recv_byte = recvfrom(Agent->master_socket.socket, recv_array,MAX_UDP_MSG_SIZE, 0, (struct sockaddr*)&response_socket.address, &addr_len);
	
	if(recv_byte < 0)
	{
		printf("wtp_master_receive_msg Receive Msg Error\n");
		return;
	}
	
	printf("wtp_master_receive_msg:\n");
	response_socket.socket = Agent->master_socket.socket;

	wtp_process_master_msg(Agent,recv_array,recv_byte);
	
	return;
}


void wtp_set_socket_acaddr(struct VcapwapAgent *Agent)
{
    inet_aton(Agent->ac_ip, &(Agent->master_socket.address.sin_addr));
	Agent->master_socket.address.sin_port = htons(Agent->manage_port);
}


WTPBool wtp_master_socket_init(struct VcapwapAgent *Agent)
{
	int nReuseFlag = 1;
	
	if ((Agent->master_socket.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		printf("wtp_master_socket_init Error Creating Socket\n");
		return WTP_FALSE;
	}

		
	memset(&(Agent->master_socket.address),0,sizeof(Agent->master_socket.address));
	
	Agent->master_socket.address.sin_family = AF_INET;
	
	Agent->master_socket.address.sin_addr.s_addr = htonl(INADDR_ANY);

	Agent->master_socket.address.sin_port = htons(CW_MASTER_UDP_PORT);

	if(setsockopt(Agent->master_socket.socket, SOL_SOCKET, SO_REUSEADDR, &nReuseFlag, sizeof(int)) < 0)
	{ 
		printf("wtp_master_socket_init:Set socket option	failed\n "); 
		close(Agent->master_socket.socket); 
		return WTP_FALSE; 
	} 

	if((bind(Agent->master_socket.socket, (struct sockaddr *)&Agent->master_socket.address, sizeof(Agent->master_socket.address))) < 0)
	{
		printf("wtp_master_socket_init:Error Binding Socket \n");
		close(Agent->master_socket.socket); 
		return WTP_FALSE;
	}

	wtp_set_socket_acaddr(Agent);
	
	return WTP_TRUE;
}

WTPBool wtp_socket_init(struct VcapwapAgent *Agent)
{
	if(!wtp_master_socket_init(Agent))							
	{
		return WTP_FALSE;
	}
	
	return WTP_TRUE;
}

WTPBool wtp_socket_destory(struct VcapwapAgent *Agent)
{
	close(Agent->master_socket.socket);
	
	return WTP_TRUE;
}

WTPBool wtp_events_init(struct VcapwapAgent *Agent)
{
	int sk, ret;
	HUAYI_EVENTS_TYPE eid[1] = {HUAYI_EVENTS_NOTE};

	sk	= huayi_events_create();
	
	ret = huayi_events_add(eid, 1, sk);
	
	if (ret != EVENTS_SUCC) 
	{
		return WTP_FALSE;
	}
	
	Agent->wtp_event_fd = sk;
	
	return WTP_TRUE;
}

void wtp_events_destory(struct VcapwapAgent *Agent)
{	
	HUAYI_EVENTS_TYPE eid[1] = {HUAYI_EVENTS_NOTE};
	
	huayi_events_del(eid,1,Agent->wtp_event_fd);
	
	huayi_events_destroy(Agent->wtp_event_fd);
	
	return;
}

void wtp_events_receive(struct VcapwapAgent *Agent)
{
	int ret;
	char buf[ALT_MAX_PAYLOAD];
	struct huayi_notification *event = NULL;
	
	 ret = huayi_events_recv(Agent->wtp_event_fd, -1, buf, ALT_MAX_PAYLOAD);
	 if (ret == 0) {
		printf("huayi_events_receive: timeout\n");
	 } else if (ret == -1) {
		printf("huayi_events_receive: error\n");
	 } else 
	 {
		event = (struct huayi_notification *)buf;

		wtp_events_process(Agent,event);

		return;
	}
}


