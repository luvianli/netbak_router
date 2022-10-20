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
#ifndef __THREAD_MESSAGE_QUEUE__
#define	__THREAD_MESSAGE_QUEUE__

#include <pthread.h>

struct q_Msg_list {
	struct q_Msg_list *next;
	char *mesg;
};

struct q_Msg_q_hdr {
	int valid;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int count;
	struct q_Msg_list *head;
	struct q_Msg_list *tail;
};

void q_message_queue_init(void);
int q_create_message_queue(void);
int q_destory_message_queue(int q_id);
int q_send_mesg(int q_id, void *msg);
void *q_recv_mesg(int qid);
int q_flush_mesg_q(int q_id);

#endif
