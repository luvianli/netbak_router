/*
 * Copyright (c) 2015 GCT Semiconductor, Inc. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * ------------------------
 * History: {yyyy-mm-dd},{name}: {description}
 *   2015-02-27, Roy Park(roypark@gctsemi.com), header added. 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

#include "cm_dbg.h"
#include "thread_wrapper.h"
	
typedef struct _thr_wrp_id_t
{
	int is_signal_received;
	pthread_cond_t *cond;
	pthread_mutex_t *mutex;
	struct _thr_wrp_id_t* next;
}thr_wrp_id_t;

int list_count = 0;
thr_wrp_id_t* list_head;
thr_wrp_id_t* list_tail;


int thr_wrp_pthread_cond_wait(pthread_cond_t *cond,
         pthread_mutex_t *mutex)
{
#ifdef __UCLINUX__
	thr_wrp_id_t* new_wrp_id;
	thr_wrp_id_t* current_thr = NULL;
	thr_wrp_id_t* prev_thr = NULL;
	int i;

	if (cond == NULL || mutex == NULL)	
		return EINVAL; // cond, mutex, abstime is invalid

	if (list_count == 0)
	{
		new_wrp_id = (thr_wrp_id_t*)malloc(sizeof(thr_wrp_id_t));
		if (!new_wrp_id) {printf("malloc failed\n"); return -1;}

		new_wrp_id->is_signal_received = 0;
		new_wrp_id->cond = cond;
		new_wrp_id->mutex = mutex;
		new_wrp_id->next = NULL;
		list_head = new_wrp_id;
		list_tail = list_head;
		current_thr = new_wrp_id;
		list_count++;

			
	} else {
		/* try to find current linked list if same cond object is saved in */
		current_thr = list_head;
		for (i=0;i<list_count;i++)
		{
			if (current_thr->cond == cond) // found
			{
				current_thr->is_signal_received = 0;
				break;
			}

			if (current_thr->next != NULL)
			{
				prev_thr = current_thr;
				current_thr = current_thr->next;
				continue;
			}
			else  // not found and reach to end -> create new
			{
				new_wrp_id = (thr_wrp_id_t*)malloc(sizeof(thr_wrp_id_t));
				if (!new_wrp_id) {printf("malloc failed\n"); return -1;}

				new_wrp_id->is_signal_received = 0;
				new_wrp_id->cond = cond;
				new_wrp_id->mutex = mutex;
				new_wrp_id->next = NULL;
				list_tail = new_wrp_id;
				current_thr->next = new_wrp_id;
				current_thr = new_wrp_id;
				list_count++;
				break;
			}
		}
	}


	pthread_mutex_unlock(mutex);
	
	while (1)
	{
		usleep(1000);

 		if (current_thr->is_signal_received)
		{
			cm_print(LOG_LEVEL_TRACE, " signal received\n");
			return 0; // success
		}

	}
	return 0; // success
#else
	return pthread_cond_wait(cond, mutex);
#endif
}





int thr_wrp_pthread_cond_timedwait(pthread_cond_t *cond,
         pthread_mutex_t *mutex,
         const struct timespec *abstime)
{
#ifdef __UCLINUX__
	int remain_utime;
	struct timeval now;
	thr_wrp_id_t* new_wrp_id;
	thr_wrp_id_t* current_thr = NULL;
	thr_wrp_id_t* prev_thr = NULL;
	int i;
	
	if (cond == NULL || mutex == NULL || abstime == NULL)	
		return EINVAL; // cond, mutex, abstime is invalid

	if (list_count == 0)
	{
		new_wrp_id = (thr_wrp_id_t*)malloc(sizeof(thr_wrp_id_t));
		if (!new_wrp_id) {printf("malloc failed\n"); return -1;}

		new_wrp_id->is_signal_received = 0;
		new_wrp_id->cond = cond;
		new_wrp_id->mutex = mutex;
		new_wrp_id->next = NULL;
		list_head = new_wrp_id;
		list_tail = list_head;
		current_thr = new_wrp_id;
		list_count++;

			
	} else {
		/* try to find current linked list if same cond object is saved in */
		current_thr = list_head;
		for (i=0;i<list_count;i++)
		{
			if (current_thr->cond == cond) // found
			{
				current_thr->is_signal_received = 0;
				break;
			}

			if (current_thr->next != NULL)
			{
				prev_thr = current_thr;
				current_thr = current_thr->next;
				continue;
			}
			else  // not found and reach to end -> create new
			{
				new_wrp_id = (thr_wrp_id_t*)malloc(sizeof(thr_wrp_id_t));
				if (!new_wrp_id) {printf("malloc failed\n"); return -1;}

				new_wrp_id->is_signal_received = 0;
				new_wrp_id->cond = cond;
				new_wrp_id->mutex = mutex;
				new_wrp_id->next = NULL;
				list_tail = new_wrp_id;
				current_thr->next = new_wrp_id;
				current_thr = new_wrp_id;
				list_count++;
				break;
			}
		}
	}


	gettimeofday(&now, NULL);
	remain_utime = (abstime->tv_sec-now.tv_sec)*1000000+(abstime->tv_nsec/1000-now.tv_usec);
	//cm_print(LOG_LEVEL_TRACE, " time remaining:[(%d)-(%d)]*1000000 = %d us\n", abstime->tv_sec, now.tv_sec, remain_utime);

	pthread_mutex_unlock(mutex);
	
	while (1)
	{
		usleep(1000);
		remain_utime -=1000;
		if (remain_utime < 0)
		{
			cm_print(LOG_LEVEL_TRACE, " timeout\n");
			return ETIMEDOUT;
		}
 		if (current_thr->is_signal_received)
		{
			cm_print(LOG_LEVEL_TRACE, " signal received\n");
			return 0; // success
		}

	}
	return 0; // success
#else
	return pthread_cond_timedwait(cond, mutex, abstime);
#endif
}



int thr_wrp_pthread_cond_signal(pthread_cond_t *cond)
{
#ifdef __UCLINUX__
	thr_wrp_id_t* current_thr;
	int i;
	if (list_count > 0)
	{
		current_thr = list_head;
		for (i=0;i<list_count;i++)
		{
			if (current_thr->cond == cond) // found
				current_thr->is_signal_received = 1;
				cm_print(LOG_LEVEL_TRACE, " signal flag 1\n");
				return 0; // success

			if (current_thr->next != NULL)
			{
				current_thr = current_thr->next;
				continue;
			}
			else  // not found and reach to end -> create new
			{
				cm_print(LOG_LEVEL_TRACE, " matched cond is not found in the list\n");
				break;
			}
		}
	}

	return 0; // success
#else
	return pthread_cond_signal(cond);
#endif
}



