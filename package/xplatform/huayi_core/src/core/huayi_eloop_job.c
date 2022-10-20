/*******************************************************************************************
 * Project:  huayi_capwap_server                                                           *
 *                                                                                         *
 * Author :  huayi									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20180817 add by huayi  Version 0.0 													   *
 ******************************************************************************************/
#include "huayi_eloop_job.h"

int huayi_core_eloop_fd;
struct task_queen task_list[TASK_QUEEN_SIZE];
struct task_queen* ptask_pool;
struct task_queen* ptask_queue;


struct eloop_socket_list huayi_core_eloop_sock[HUAYI_CORE_TOTAL_SOCKET_NUM];

VTBool huayi_core_eloop_init(void)
{
	memset(huayi_core_eloop_sock,0,sizeof(huayi_core_eloop_sock));

	if((huayi_core_eloop_fd = epoll_create(HUAYI_CORE_TOTAL_SOCKET_NUM)) == -1)
	{
		return VT_FALSE;
	}

	return VT_TRUE;
}

VTBool huayi_core_eloop_destory(void)
{
	close(huayi_core_eloop_fd);
	
	return VT_TRUE;
}

VTBool huayi_core_eloop_register_socket(int socket,void (*callback)(void*), void* arg)
{
	struct epoll_event event;
	int i = 0;
	event.events = EPOLLIN;
	event.data.fd = socket;
	
	if (epoll_ctl(huayi_core_eloop_fd, EPOLL_CTL_ADD, socket, &event) == -1) 
	{
		perror("epoll_ctl");
		printf("huayi_core_eloop_register_socket failed\n");
		return VT_FALSE;
	}

	for(i=0; i< HUAYI_CORE_TOTAL_SOCKET_NUM; i++) {
		if(huayi_core_eloop_sock[i].used == VT_FALSE) {
			huayi_core_eloop_sock[i].used = VT_TRUE;
			huayi_core_eloop_sock[i].socket = socket;
			huayi_core_eloop_sock[i].callback= callback;
			huayi_core_eloop_sock[i].arg = arg;
			return VT_TRUE;
		}
	}
	return VT_FALSE;
}

VTBool huayi_core_eloop_remove_socket(int socket)
{
	struct epoll_event event;
	event.data.fd = socket;
	
	if (epoll_ctl(huayi_core_eloop_fd, EPOLL_CTL_DEL, socket, &event) == -1) 
	{
		perror("epoll_ctl");
		printf("huayi_core_eloop_register_socket failed\n");
		return VT_FALSE;
	}
	return VT_TRUE;
}

void huayi_core_eloop_execution(void)
{
	struct epoll_event events[20];
	int nfds = 0;
	int i = 0,k = 0;
	int epoll_socket;
	
	nfds=epoll_wait(huayi_core_eloop_fd,events,HUAYI_CORE_TOTAL_SOCKET_NUM,-1);

	for(i=0;i<nfds;i++)
	{
		 if(events[i].events&EPOLLIN)                
		 {
		 	
		 	if ((epoll_socket = events[i].data.fd) < 0)
		 	{
				continue;
		 	}

			for(k=0;k<HUAYI_CORE_TOTAL_SOCKET_NUM;k++)
			{	
				if(epoll_socket == huayi_core_eloop_sock[k].socket)
				{
					if(huayi_core_eloop_sock[k].callback != NULL)
					{
						huayi_core_eloop_sock[k].callback(huayi_core_eloop_sock[i].arg);
					}
				}
			}
		 }
	}
	
	return;
}


void huayi_core_task_queen_init()
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


void huayi_core_task_remove_task_from_queue(struct task_queen* expired_task)
{
	 memset(expired_task, 0, sizeof(struct task_queen));
	 
	 expired_task->pnext_task = ptask_pool;
	 
	 ptask_pool = expired_task;
	 
	 return;
}

void huayi_core_task_queen_destory(void) 
{
	struct task_queen *tmp;

	while(ptask_queue != NULL) 
	{
		tmp = ptask_queue;
		
		ptask_queue = ptask_queue->pnext_task;

		huayi_core_task_remove_task_from_queue(tmp);
	}

	ptask_queue = NULL;
	
	return;
}

VTBool huayi_core_timer_init(struct VcoreAgent *Agent)
{
    int ret;
    struct itimerspec new_value;

    new_value.it_value.tv_sec = 1;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_nsec = 0;

	if((Agent->huayi_timer_fd = timerfd_create(CLOCK_MONOTONIC, 0)) <0)
	{
		printf("huayi_core_timerfd_init timerfd create fail!\n");
		return VT_FALSE;
	}

    ret = timerfd_settime(Agent->huayi_timer_fd, 0, &new_value, NULL);
    if (ret < 0) 
	{
        printf("huayi_core_add_timer error\n");
		close(Agent->huayi_timer_fd);
        return VT_FALSE;
    }

	huayi_core_task_queen_init();

	return VT_TRUE;
}


VTBool huayi_core_timer_destory(struct VcoreAgent *Agent)
{
	close(Agent->huayi_timer_fd);

	huayi_core_task_queen_destory();

	return VT_TRUE;
}

void huayi_core_excute_task(void)
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
				currentJob->is_runing = VT_TRUE;
				currentJob->processFunc(currentJob->argumentPointer);
			}
			
			/*Put job back to pool*/
			huayi_core_task_remove_task_from_queue(currentJob);
		}else
		{
			
			break;
		}
}
  return;
}


void huayi_core_timer_process(struct VcoreAgent *Agent)
{
	ssize_t s;
	uint64_t expired = 0;
    
    s = read(Agent->huayi_timer_fd, &expired, sizeof(uint64_t));
	
	if(s != sizeof(uint64_t))
	{
		printf("huayi_core_timer_process read invaild timer\n");
		return;
	}
	
	huayi_core_excute_task();
}


void huayi_core_delete_first_task_by_id(int task_id) {
  struct task_queen *currentjob; 
  struct task_queen *prevjob;

  prevjob = NULL;
  
  for(currentjob = ptask_queue; currentjob != NULL; currentjob = currentjob->pnext_task) 
  {
      if((currentjob->task_id == task_id) && (currentjob->is_runing== VT_FALSE)) 
	  {
          if (prevjob != NULL) {
              prevjob->pnext_task = currentjob->pnext_task;
          } else {
              ptask_queue = currentjob->pnext_task;
          }
          huayi_core_task_remove_task_from_queue(currentjob);
		  return;
	  } else {
          prevjob = currentjob;
	  }
  }
  return;
}

void huayi_core_insert_task_to_queue(struct task_queen* newJob)
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

void huayi_core_add_task(unsigned int secondOffset, int task_id,void (*processFunc)(void*),void* argumentPointer)
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
	temp->argumentPointer = argumentPointer;
	
	/*Update JobPool*/
	ptask_pool = newJobPoolHead;

	huayi_core_insert_task_to_queue(temp);
	
	return;
}


