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
#ifndef __THR_WRP_H_
#define __THR_WRP_H_

#include <pthread.h>

int thr_wrp_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int thr_wrp_pthread_cond_timedwait(pthread_cond_t *cond,
         pthread_mutex_t *mutex,
         const struct timespec *abstime);
int thr_wrp_pthread_cond_signal(pthread_cond_t *cond);

#endif //#ifndef __THR_WRP_H_

