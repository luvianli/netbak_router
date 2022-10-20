#ifndef _HUAYI_CORE_QUEUE_HEADER_
#define _HUAYI_CORE_QUEUE_HEADER_

#include "huayi_core_base.h"

typedef struct huayi_handle_list
{
        struct huayi_handle_msg  msg_data;
        struct huayi_handle_list* next;
}HUAYI_HANDLE_LIST;
 
typedef struct huayi_reload_hanlde_queue
{
        struct huayi_handle_list* pre;
        struct huayi_handle_list* next;
        int size;
}HUAYI_RELOAD_HANDLE_QUEUE;


void huayi_msg_queue_init();
VTBool huayi_msg_queue_push(struct huayi_handle_msg *node);
VTBool huayi_msg_queue_pop(struct huayi_handle_msg *node);
void huayi_msg_queue_destory();
VTBool huayi_msg_queue_print();
VTBool huayi_msg_check_node_last( struct huayi_handle_msg *node);




#endif
