
#include "huayi_handle_reload.h"
#include "huayi_core_queue.h"

HUAYI_RELOAD_HANDLE_QUEUE g_msg_queue;
pthread_mutex_t msg_queue_mutex;


void huayi_msg_queue_init()
{
	pthread_mutex_init(&msg_queue_mutex,NULL);

	pthread_mutex_lock(&msg_queue_mutex);

	g_msg_queue.pre = NULL;

	g_msg_queue.next = NULL;

	g_msg_queue.size = 0;
	
	pthread_mutex_unlock(&msg_queue_mutex);
	return;
}

VTBool huayi_msg_check_node_last( struct huayi_handle_msg *node)
{
	struct huayi_handle_list *ptmp = NULL;
	
	pthread_mutex_lock(&msg_queue_mutex);

	if(g_msg_queue.size == 0)
	{
		pthread_mutex_unlock(&msg_queue_mutex);
		return VT_FALSE;
	}

	ptmp= g_msg_queue.pre;
	
    while(ptmp)
	{
		printf("huayi_msg_show_node ptmp->msg_data.msgid is %d\n", ptmp->msg_data.msgid);
		if(ptmp->next == NULL)
		{
			break;
		}
        ptmp = ptmp->next;
    }

	printf("last node  is %d\n", ptmp->msg_data.msgid);

	if(node->msgid == ptmp->msg_data.msgid)
	{
		pthread_mutex_unlock(&msg_queue_mutex);
		
		return VT_TRUE;
	}
	
	pthread_mutex_unlock(&msg_queue_mutex);

	return VT_FALSE;
}



VTBool huayi_msg_queue_print()
{
	struct huayi_handle_list *ptmp = NULL;
	
	pthread_mutex_lock(&msg_queue_mutex);

	if(g_msg_queue.size == 0)
	{
		pthread_mutex_unlock(&msg_queue_mutex);
		return VT_FALSE;
	}

	ptmp= g_msg_queue.pre->next;
	
    while(ptmp)
	{
		printf("huayi_msg_queue_print.msgid is %d\n", ptmp->msg_data.msgid);
        ptmp = ptmp ->next;
    }
	
	pthread_mutex_unlock(&msg_queue_mutex);
	
	return VT_FALSE;
}

VTBool huayi_msg_queue_push(struct huayi_handle_msg *node)
{
	struct huayi_handle_list *ptmp;

	ptmp = (struct huayi_handle_list*)malloc(sizeof(struct huayi_handle_list));

	if(!ptmp)
   	{
   		return VT_FALSE;
   	}

	memcpy(&ptmp->msg_data,node,sizeof(struct huayi_handle_msg));
	
    ptmp->next = NULL;
	
	pthread_mutex_lock(&msg_queue_mutex);

	if(g_msg_queue.size == 0)
    {
        g_msg_queue.pre = ptmp;
        g_msg_queue.next = ptmp;
    }
    else
    {            
        g_msg_queue.next->next = ptmp;
        g_msg_queue.next = ptmp;
    }
    ++g_msg_queue.size;
	
	pthread_mutex_unlock(&msg_queue_mutex);

	return VT_TRUE;
}


VTBool huayi_msg_queue_pop(struct huayi_handle_msg *node)
{
	struct huayi_handle_list *ptmp;

	pthread_mutex_lock(&msg_queue_mutex);
	if(g_msg_queue.size == 0)
    {
    	pthread_mutex_unlock(&msg_queue_mutex);
       	return VT_FALSE;
    }
	
	ptmp = g_msg_queue.pre;
	memcpy(node,&ptmp->msg_data,sizeof(struct huayi_handle_msg));
	g_msg_queue.pre = g_msg_queue.pre->next;

	free(ptmp);
	--g_msg_queue.size;

	pthread_mutex_unlock(&msg_queue_mutex);
	
	return VT_TRUE;
}


void huayi_msg_queue_destory()
{
     struct huayi_handle_list	*ptmp;
	 
	 pthread_mutex_lock(&msg_queue_mutex);
     while(g_msg_queue.pre)
     {
         ptmp = g_msg_queue.pre;
         g_msg_queue.pre = g_msg_queue.pre->next;
         free(ptmp);
     }
	 
	 g_msg_queue.size = 0;
	 pthread_mutex_unlock(&msg_queue_mutex);

	 pthread_mutex_destroy(&msg_queue_mutex);
	 return;
}
 


