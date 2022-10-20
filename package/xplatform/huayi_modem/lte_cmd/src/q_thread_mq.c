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
#include <pthread.h>
#include <unistd.h>

#include "q_thread_mq.h"
#include "cm_dbg.h"

#define	Q_MAX_QUEUE_COUNT	10

#define	Q_MESSAGE_Q_INVALID		0
#define	Q_MESSAGE_Q_VALID	1

pthread_mutex_t q_msg_q_lock;
static struct q_Msg_q_hdr q_msg_q_hdr[Q_MAX_QUEUE_COUNT];


void q_message_queue_init(void)
{
	int i;
	func_in();
	pthread_mutex_lock(&q_msg_q_lock);
	for(i = 0; i < Q_MAX_QUEUE_COUNT; i++)
	{
		q_msg_q_hdr[i].valid = Q_MESSAGE_Q_INVALID;

	}
	pthread_mutex_unlock(&q_msg_q_lock);
}

int q_create_message_queue(void)
{
	int i, q_id = -1;

	func_in();
	pthread_mutex_lock(&q_msg_q_lock);

	for(i = 0; i < Q_MAX_QUEUE_COUNT; i++)
	{
		if( q_msg_q_hdr[i].valid == Q_MESSAGE_Q_INVALID) {
			q_id = i;
			q_msg_q_hdr[i].valid = Q_MESSAGE_Q_VALID;
			break;
		}
	}

	if(q_id < 0) {
		pthread_mutex_unlock(&q_msg_q_lock);
		return -1;
	}

	q_msg_q_hdr[q_id].count = 0;
	pthread_mutex_init(&q_msg_q_hdr[q_id].mutex, NULL);
	pthread_cond_init(&q_msg_q_hdr[q_id].cond, NULL);
	q_msg_q_hdr[q_id].head = NULL;
	q_msg_q_hdr[q_id].tail = NULL;
	pthread_mutex_unlock(&q_msg_q_lock);

	return q_id;
}

int q_destory_message_queue(int q_id)
{
	int i;
	struct q_Msg_q_hdr *q_hdr;
	struct q_Msg_list *cur, *next;

	func_in();
	pthread_mutex_lock(&q_msg_q_lock);
	if( q_msg_q_hdr[q_id].valid == Q_MESSAGE_Q_INVALID ) {
		pthread_mutex_unlock(&q_msg_q_lock);
		return -1;
	}

	q_hdr = &q_msg_q_hdr[q_id];
	cur = q_hdr->head;
	for( i = 0 ; i < q_hdr->count; i++) {
		next = cur->next;
		free(cur->mesg);
		free(cur);
		cur = next;
	}

	q_hdr->head = NULL;
	q_hdr->tail = NULL;
	q_hdr->count = 0;

	pthread_mutex_destroy(&q_hdr->mutex);
	pthread_cond_destroy(&q_hdr->cond);

	q_hdr->valid = Q_MESSAGE_Q_INVALID;
	pthread_mutex_unlock(&q_msg_q_lock);

	return 0;
}

static struct q_Msg_list *q_get_new_msg_list(char *mesg)
{
	struct q_Msg_list *msg_list;

	func_in();
	msg_list = malloc(sizeof(struct q_Msg_q_hdr));
	if (msg_list == NULL)
	{
		fprintf(stderr, "[%s:%d] malloc failed\n", __FUNCTION__, __LINE__);
	}
	msg_list->next = NULL;
	msg_list->mesg = mesg;

	return msg_list;
}

int q_send_mesg(int q_id, void *mesg)
{
	struct q_Msg_q_hdr *q_hdr;
	struct q_Msg_list *cur;
	struct q_Msg_list *prev;

	func_in();

	//printf("[%s:%d] q_id(%d), queue count=%d\n", __FUNCTION__, __LINE__, q_id, q_msg_q_hdr[q_id].count);

	if(q_id < 0 || q_id > Q_MAX_QUEUE_COUNT)
		return -1;

	q_hdr = &q_msg_q_hdr[q_id];
	pthread_mutex_lock(&q_msg_q_lock);
	if( q_hdr->valid == Q_MESSAGE_Q_INVALID) {
		pthread_mutex_unlock(&q_msg_q_lock);
		return -1;
	}

	cur = q_get_new_msg_list((char *)mesg);
	if (q_hdr->count == 0) {
		q_hdr->tail = q_hdr->head = cur;
	} else {
		prev = q_hdr->tail;
		q_hdr->tail = cur;
		prev->next = cur;
	} 

	q_hdr->count++;
	pthread_mutex_unlock(&q_msg_q_lock);
	pthread_mutex_lock(&q_hdr->mutex);
	pthread_cond_signal(&q_hdr->cond);
	pthread_mutex_unlock(&q_hdr->mutex);

	return 0;
}

void *q_recv_mesg(int q_id)
{
	struct q_Msg_q_hdr *q_hdr;
	struct q_Msg_list *cur;
	char *mesg;
	func_in();

	if(q_id < 0 || q_id > Q_MAX_QUEUE_COUNT)
		return NULL;

	q_hdr = &q_msg_q_hdr[q_id];
	pthread_mutex_lock(&q_msg_q_lock);
	if( q_hdr->valid == Q_MESSAGE_Q_INVALID) {
		pthread_mutex_unlock(&q_msg_q_lock);
		return NULL;
	}

#ifndef __UCLINUX__ 
	while (q_hdr->count == 0) {
		pthread_mutex_unlock(&q_msg_q_lock);

		pthread_mutex_lock(&q_hdr->mutex);
		pthread_cond_wait(&q_hdr->cond, &q_hdr->mutex);
		pthread_mutex_unlock(&q_hdr->mutex);
		pthread_mutex_lock(&q_msg_q_lock);
	}
#else
	while (q_hdr->count == 0) {
		pthread_mutex_unlock(&q_msg_q_lock);
		pthread_mutex_lock(&q_hdr->mutex);
		usleep(1000);
		pthread_mutex_unlock(&q_hdr->mutex);
		pthread_mutex_lock(&q_msg_q_lock);
	}
#endif

	if (q_hdr->count == 1) {
		cur = q_hdr->head;
		q_hdr->head = NULL;
		q_hdr->tail = NULL;
	} else {
		cur = q_hdr->head;
		q_hdr->head = cur->next;
	}
	q_hdr->count--;
	pthread_mutex_unlock(&q_msg_q_lock);
	mesg = cur->mesg;
	free(cur);

	return (void *)mesg;
}

int q_flush_mesg_q(int q_id)
{
	struct q_Msg_q_hdr *q_hdr;
	struct q_Msg_list *cur, *next;
	int i;

	func_in();
	if(q_id < 0 || q_id > Q_MAX_QUEUE_COUNT)
		return -1;

	q_hdr = &q_msg_q_hdr[q_id];
	pthread_mutex_lock(&q_msg_q_lock);
	if( q_hdr->valid == Q_MESSAGE_Q_INVALID) {
		pthread_mutex_unlock(&q_msg_q_lock);
		return -1;
	}

	cur = q_hdr->head;
	for(i = 0; i < q_hdr->count; i++) {
		next = cur->next;
		free(cur->mesg);
		free(cur);
		cur = next;
	}
	q_hdr->head = NULL;
	q_hdr->tail = NULL;
	q_hdr->count = 0;
	pthread_mutex_unlock(&q_msg_q_lock);

	return 0;
}
