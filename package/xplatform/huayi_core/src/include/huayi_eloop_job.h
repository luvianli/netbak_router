#ifndef _HUAYI_ELOOP_JOB_HEADER_
#define _HUAYI_ELOOP_JOB_HEADER_
#include <linux/sysinfo.h>
#include "huayi_core_base.h"

#define HUAYI_CORE_TOTAL_SOCKET_NUM 		4
#define TASK_QUEEN_SIZE				  		255


typedef struct eloop_socket_list{
	int socket;
	VTBool used;
	void (*callback)(void*);
	void* arg;
}ELOOP_SOCKET_LIST;

struct task_queen {
    int    task_id;
    VTBool is_runing;
	long nextEventTime;
	void (*processFunc)(void *arg);
	void* argumentPointer;
	struct task_queen* pnext_task;
};

VTBool huayi_core_timer_init(struct VcoreAgent *Agent);
void huayi_core_timer_process(struct VcoreAgent *Agent);
VTBool huayi_core_timer_destory(struct VcoreAgent *Agent);
void huayi_core_add_task(unsigned int secondOffset, int task_id, void (*processFunc)(void*), void* argumentPointer);
void huayi_core_delete_first_task_by_id(int task_id);


VTBool huayi_core_eloop_init(void);
VTBool huayi_core_eloop_destory(void);
VTBool huayi_core_eloop_register_socket(int socket,void (*callback)(void*), void* arg);
VTBool huayi_core_eloop_remove_socket(int socket);
void huayi_core_eloop_execution(void); 
#endif
