#ifndef _WTPTIMER_HEADER_
#define _WTPTIMER_HEADER_

#include "WTPcommon.h"
#include <sys/sysinfo.h>

#define TASK_QUEEN_SIZE					256

struct task_queen {
    int    task_id;
    WTPBool is_runing;
	long nextEventTime;
	WTPBool (*processFunc)(void *arg);
	void (*printFunc)(void*);
	void* argumentPointer;
	struct task_queen* pnext_task;
};

WTPBool wtp_timer_init(struct VcapwapAgent *Agent);
void wtp_timer_process(struct VcapwapAgent *Agent);
WTPBool wtp_timer_destory(struct VcapwapAgent *Agent);
void wtp_add_task(unsigned int secondOffset, int task_id, WTPBool (*processFunc)(void*), void (*printFunc)(void*), void* argumentPointer);
void wtp_task_queen_destory();
void wtp_delete_first_task_by_id(int task_id);


#endif
