
#include "huayi_core_internal.h"
#include "huayi_unix_socket.h"
#include "huayi_utimer_functions.h"
#include "huayi_core_queue.h"

VTBool huayi_core_ipc_init(struct VcoreAgent *Agent)
{
    if ((Agent->ipcSocket.huayi_ipc_fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
	{
		printf("huayi_core_ipc_init Error Creating Socket...\n");
		return VT_FALSE;
	}
	
	memset(&(Agent->ipcSocket.address),0,sizeof(Agent->ipcSocket.address));

	unlink (HUAYI_CORE_DOMAIN);
	
	strncpy(Agent->ipcSocket.address.sun_path,HUAYI_CORE_DOMAIN,sizeof(Agent->ipcSocket.address.sun_path)-1);
	
    Agent->ipcSocket.address.sun_family = AF_UNIX;

	/*we use recv buf default,because our msg is short*/
	#if 0
   	ret = setsockopt(Agent->ipcSocket.huayi_ipc_fd, SOL_SOCKET, SO_RCVBUF, &recvBufSize, len);
   	if(ret == -1)
   	{
	   perror("setsockopt error.");
 	}
	#endif
	
	if( bind(Agent->ipcSocket.huayi_ipc_fd,( struct sockaddr * )&Agent->ipcSocket.address, sizeof(Agent->ipcSocket.address)) < 0 )
	{
		printf("huayi_core_ipc_init Error Binding Socket...\n");
		return VT_FALSE;
	}
	return VT_TRUE;
}


void huayi_core_ipc_msg_process(struct VcoreAgent *Agent,unsigned char *RecvMsgArray,unsigned int byteReceived)
{
	MsgType RecvMsg = 0;
	struct huayi_handle_msg node;
	VTBool	push_need = VT_FALSE;
	
	memset(&node,0,sizeof(node));
	
	node.input_root = NULL;
	
	node.input_root = cJSON_Parse((char *)RecvMsgArray);

	if(node.input_root == NULL)
	{
		printf("not json\n");
		return;
	}

	node.rec_time =(long)time(NULL);
	
	huayi_json_get_int(node.input_root,HUAYI_OPTION_MSGTYPE_KEY,&RecvMsg);
	
	if( (RecvMsg>= HUAYI_MSG_PAYLOAD_OFFSET) && (RecvMsg < HUAYI_MSG_RELOAD_MAX))
	{	
		node.msgid = RecvMsg;
		
		huayi_msg_queue_print();

		switch(node.msgid)
		{
			/*add no need to remove duplicate msg here to case*/
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
			case HUAYI_MSG_RELOAD_FIREWALL:
#endif
				push_need = VT_TRUE;
				break;
			default:
				push_need = (huayi_msg_check_node_last(&node) ? VT_FALSE : VT_TRUE);
				break;
		}
		if(push_need == VT_TRUE)
		{
			huayi_msg_queue_push(&node);
			huayi_core_delete_first_task_by_id(HUAYI_RELOAD_TASK);
			huayi_core_add_task(HUAYI_RELOAD_DELAY_TIME,HUAYI_RELOAD_TASK,(void *)&huayi_execute_need_reload,NULL);
			return;
		}else
		{
			printf("rece msg:%d at last queen\n",RecvMsg);
			goto ignore;
		}
	}else
	{
		printf("RecvMsg unknown msg: %d\n",RecvMsg);
		goto ignore;
	}

ignore:
	cJSON_Delete(node.input_root);
	return;
}



void huayi_core_receive_ipc_msg(struct VcoreAgent *Agent)
{
	unsigned char RecvMsgArray[MAX_MSG_SIZE];
	unsigned int  RecvAddrLen = sizeof(Agent->ipcSocket.address);
	unsigned int byteReceived;
	struct sockaddr_un RecvAddr;
	
	memset(RecvMsgArray, 0, sizeof(RecvMsgArray));
	
    byteReceived = recvfrom(Agent->ipcSocket.huayi_ipc_fd, RecvMsgArray,MAX_MSG_SIZE, 0, (struct sockaddr*)&RecvAddr, &RecvAddrLen);
	
    if(byteReceived < 0)
    {
        printf("huayi_core_receive_ipc_msg Receive Msg Error\n");
		return;
    }
	
	huayi_core_ipc_msg_process(Agent,RecvMsgArray,byteReceived);
	
	return;
}

VTBool huayi_core_ipc_destory(struct VcoreAgent *Agent)
{
	close(Agent->ipcSocket.huayi_ipc_fd);

	return VT_TRUE;
}


