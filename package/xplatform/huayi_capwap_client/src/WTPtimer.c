/*******************************************************************************************
 * Project:  huayi_capwap_client                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/

#include "WTPcommon.h"
#include "WTPtimer.h"

struct task_queen task_list[TASK_QUEEN_SIZE];
struct task_queen* ptask_pool;
struct task_queen* ptask_queue;

void wtp_task_queen_init()
{
	int count;
	
	memset(task_list, 0, sizeof(task_list));
	
	ptask_pool = task_list;
	
	for(count=0; count < TASK_QUEEN_SIZE; count++) 
	{
		if(count >= TASK_QUEEN_SIZE) 
		{
			task_list[count].pnext_task = NULL;
			
			break;
		}
		else
		{
			task_list[count].pnext_task = &(task_list[(count + 1)]);
		}
	}
	
	ptask_queue = NULL;
	
	return;
}


void wtp_task_remove_task_from_queue(struct task_queen* expired_task)
{
	 memset(expired_task, 0, sizeof(struct task_queen));
	 
	 expired_task->pnext_task = ptask_pool;
	 
	 ptask_pool = expired_task;
	 
	 return;
}

void wtp_task_queen_destory(void) 
{
	struct task_queen *tmp;

	while(ptask_queue != NULL) 
	{
		tmp = ptask_queue;
		
		ptask_queue = ptask_queue->pnext_task;

		wtp_task_remove_task_from_queue(tmp);
	}

	ptask_queue = NULL;
	
	return;
}

WTPBool wtp_timer_init(struct VcapwapAgent *Agent)
{
    int ret;
    struct itimerspec new_value;

    new_value.it_value.tv_sec = 1;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_nsec = 0;

	if((Agent->wtp_timer_fd = timerfd_create(CLOCK_MONOTONIC, 0)) <0)
	{
		printf("wtp_timerfd_init timerfd create fail!\n");
		return WTP_FALSE;
	}

    ret = timerfd_settime(Agent->wtp_timer_fd, 0, &new_value, NULL);
    if (ret < 0) 
	{
        printf("wtp_add_timer error\n");
		close(Agent->wtp_timer_fd);
        return WTP_FALSE;
    }

	wtp_task_queen_init();

	return WTP_TRUE;
}


WTPBool wtp_timer_destory(struct VcapwapAgent *Agent)
{
	close(Agent->wtp_timer_fd);

	wtp_task_queen_destory();

	return WTP_TRUE;
}

void wtp_excute_task(void)
{
	/*Execute all jobs before current time*/
	struct task_queen *currentJob;
	struct sysinfo info;
	sysinfo(&info);

	while(ptask_queue != NULL)
	{
		if(ptask_queue->nextEventTime <= info.uptime) 
		{
			/*Detach job from job queue*/
			currentJob = ptask_queue;
			ptask_queue = ptask_queue->pnext_task;

			/*Run Job Func*/
			if(currentJob->processFunc != NULL) 
			{
				currentJob->is_runing = WTP_TRUE;
				currentJob->processFunc(currentJob->argumentPointer);
			}
			
			/*Put job back to pool*/
			wtp_task_remove_task_from_queue(currentJob);
		}else
		{
			
			break;
		}
}
  return;
}


void wtp_timer_process(struct VcapwapAgent *Agent)
{
	ssize_t s;
	uint64_t expired = 0;
    
    s = read(Agent->wtp_timer_fd, &expired, sizeof(uint64_t));
	
	if(s != sizeof(uint64_t))
	{
		printf("wtp_timer_process read invaild timer\n");
		return;
	}
	
	wtp_excute_task();
}


void wtp_delete_first_task_by_id(int task_id) {
  struct task_queen *currentjob; 
  struct task_queen *prevjob;

  prevjob = NULL;
  
  for(currentjob = ptask_queue; currentjob != NULL; currentjob = currentjob->pnext_task) 
  {
      if((currentjob->task_id == task_id) && (currentjob->is_runing== WTP_FALSE)) 
	  {
          if (prevjob != NULL) {
              prevjob->pnext_task = currentjob->pnext_task;
          } else {
              ptask_queue = currentjob->pnext_task;
          }
          wtp_task_remove_task_from_queue(currentjob);
		  return;
	  } else {
          prevjob = currentjob;
	  }
  }
  return;
}

void wtp_insert_task_to_queue(struct task_queen* newJob)
{
	struct task_queen* pCurrentJob;
	struct task_queen* temp;
	
	if(ptask_queue ==  NULL) {
		ptask_queue = newJob;
		newJob->pnext_task = NULL;
		return;
	}

	if(ptask_queue->nextEventTime >= newJob->nextEventTime) {
		temp = ptask_queue;
		ptask_queue = newJob;
		newJob->pnext_task = temp;
		return;
	}

		for(pCurrentJob = ptask_queue; pCurrentJob->pnext_task != NULL; pCurrentJob = pCurrentJob->pnext_task) {
			if((pCurrentJob->nextEventTime < newJob->nextEventTime) && (pCurrentJob->pnext_task->nextEventTime >= newJob->nextEventTime)) {
				temp = pCurrentJob->pnext_task;
				pCurrentJob->pnext_task = newJob;
				newJob->pnext_task = temp;
			return;
			}
		}
		/*Reached End*/
		pCurrentJob->pnext_task = newJob;
		newJob->pnext_task = NULL;
	return;
}

void wtp_add_task(unsigned int secondOffset, int task_id, WTPBool (*processFunc)(void*), void (*printFunc)(void*), void* argumentPointer)
{
	struct task_queen* temp;
	struct task_queen* newJobPoolHead;
	struct sysinfo info;
	
	sysinfo(&info);
	
	if(ptask_pool == NULL) 
	{
		printf("JobQueueAddJob Error JobPool full\n");
		return;
	}

    temp = ptask_pool;
	newJobPoolHead = ptask_pool->pnext_task;
	
	/*Fill in values for job*/
	temp->pnext_task = NULL;
	temp->task_id= task_id;

	temp->nextEventTime = info.uptime + secondOffset;
	temp->processFunc = processFunc;
	temp->printFunc = printFunc;
	temp->argumentPointer = argumentPointer;
	
	/*Update JobPool*/
	ptask_pool = newJobPoolHead;

	wtp_insert_task_to_queue(temp);
	
	return;
}

