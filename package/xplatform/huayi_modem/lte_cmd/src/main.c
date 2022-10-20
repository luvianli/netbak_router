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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>  

#include <sys/stat.h>   
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <dlfcn.h>

#include "main.h"
#include "cmd.h"
#include "tty.h"
#include "sm.h"
#include "atc_parser.h"
#include "q_thread_mq.h"
#include "thread_wrapper.h"

//#define DEBUG 0
#if DEBUG
#define apf(fmt, args...) do { \
		FILE *fp = fopen("/tmp/usb_debug.log", "a+"); \
		if (fp) { \
			fprintf(fp, fmt , ## args); \
			fclose(fp); \
		} \
} while (0)

#define zdbg(fmt, args...) printf("(%s:%d)=> " fmt, __FUNCTION__, __LINE__, ## args)
#else
#define zdbg(fmt, args...)
#endif

int init_unix_socket(void)
{
	unlink(ATCM_UNIX_DOMAIN);
	/* create a socket */  
	int server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);	

	struct sockaddr_un server_addr;  
	server_addr.sun_family = AF_UNIX;  
	strcpy(server_addr.sun_path, ATCM_UNIX_DOMAIN);  

	/* bind with the local file */  
	bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)); 

	/* listen */	
	listen(server_sockfd, 5);
	return server_sockfd;
}

pthread_mutex_t atc_mutex;
pthread_mutex_t write_mutex;
pthread_cond_t  atc_cond;
pthread_t at_thread;
pthread_t msg_q_thread;

static struct timespec sync_ts;
static struct timeval now;
static struct TTYInfo gtty;
static int canonical = DEFAULT_CANONICAL_MODE_SUPPORT;
static char tty_name[] = DEFAULT_DEV_ATC;
void* gp_dev = NULL;
int msg_q_id;

int is_usb_connected = 0;

int at_resp_msg_send(char* msg, int type);




void sig_handler(int sig)
{
	switch (sig)
	{
	case SIGINT:
		cm_print(LOG_LEVEL_ERROR, "SIGINT\n");
		unlink(ATCM_UNIX_DOMAIN);
		lte_device_term();
		exit(1);
	case SIGUSR1:
		break;
	default:
		break;
	}

}

int at_write_nonblock(void* dev, char* buf)
{
	size_t len = strlen(buf);
	int ret;
	int rsp;
    unsigned int i;

	if (dev == NULL) {
		cm_print(LOG_LEVEL_TRACE, "dev == NULL\n");
		return -1;
	}

	/* Check empty line */
	if (len <= 1 || buf[0] == '\n') {
		cm_print(LOG_LEVEL_TRACE, "Empty line\n");
		return -1;
	}

	ret = device_write(dev, buf);
	return ret;
}


int at_write(void* dev, char* buf)
{
	size_t len = strlen(buf);
	int ret;
	int rsp;
    unsigned int i;

	if (dev == NULL) {
		cm_print(LOG_LEVEL_TRACE, "dev == NULL\n");
		return -1;
	}

	/* Check empty line */
	if (len <= 1 || buf[0] == '\n') {
		cm_print(LOG_LEVEL_TRACE, "Empty line\n");
		return -1;
	}

	pthread_mutex_lock(&write_mutex);
	ret = device_write(dev, buf);
	
	gettimeofday(&now, NULL);
 	sync_ts.tv_sec = now.tv_sec + AT_TIMEOUT_SEC;
	sync_ts.tv_nsec = now.tv_usec * AT_TIMEOUT_USEC;		
	cm_print(LOG_LEVEL_TRACE, "signal wait\n");
	pthread_mutex_lock(&atc_mutex);
	rsp = thr_wrp_pthread_cond_timedwait(&atc_cond, &atc_mutex, &sync_ts); 
	cm_print(LOG_LEVEL_TRACE, "out of timedwait\n");
	pthread_mutex_unlock(&atc_mutex);
#if 0
	if (rsp == ETIMEDOUT)
	{
		printf("\t[WARNING] \"OK\" or \"ERROR\" not received in %ds.(%s)\n", AT_TIMEOUT_SEC, buf);
	}
#endif
	pthread_mutex_unlock(&write_mutex);

	return ret;
}

int at_read(void* dev)
{
	char buf[AT_CMD_MAX_LEN];
	char buf_print[AT_CMD_MAX_LEN];
	char* p1;
	char* p2;
	int len;
	int remain;
	int tok_len;
	int r_fd=0;

	if (dev == NULL) {
		cm_print(LOG_LEVEL_TRACE, "dev == NULL\n");
		return -1;
	}

	usleep(0);	
	len = remain = device_read(dev, buf); 
	p1 = buf;

	r_fd = open(AT_RESULT_FILE,O_CREAT|O_APPEND|O_RDWR);

	while ( remain > 0 )
	{
		/* Remove \r\n */
		if ((*p1 == '\r') || (*p1 == '\n'))
		{
			remain --;
			p1++;
			continue;
		}

		/* Get token length */
		tok_len = 0;
		p2 = p1;

		while ( (*p2 != '\r') && (*p2 != '\n') && tok_len <= remain ) 
		{
			p2++;
			tok_len++;
		}

		strncpy(buf_print, p1, (size_t)tok_len);
		buf_print[tok_len] = '\0';
		p1+= tok_len;
		remain -= tok_len;

		write(r_fd,buf_print,tok_len);
		write(r_fd,"\n",1);

	#if DEBUG
		printf(AT_PROMPT_RCV "%s\n", buf_print);
		fflush(stdout);
	#endif
	
	#if 1//fast respone.
		if (strncasecmp((const char*) buf_print, "OK", 2)==0)
		{
			pthread_mutex_lock(&atc_mutex);
			thr_wrp_pthread_cond_signal(&atc_cond);
			pthread_mutex_unlock(&atc_mutex);
			at_resp_msg_send(buf_print, RESP_OK);
		}
		else
		{
			pthread_mutex_lock(&atc_mutex);
			thr_wrp_pthread_cond_signal(&atc_cond);
			pthread_mutex_unlock(&atc_mutex);
			at_resp_msg_send(buf_print, RESP_ERROR);
		} 
	#else	
		if (strncasecmp((const char*) buf_print, "OK", 2)==0)
		{
			pthread_mutex_lock(&atc_mutex);
			thr_wrp_pthread_cond_signal(&atc_cond);
			pthread_mutex_unlock(&atc_mutex);
			at_resp_msg_send(buf_print, RESP_OK);
		}
		else if (strncasecmp((const char*) buf_print, "ERROR", 5)==0)
		{
			pthread_mutex_lock(&atc_mutex);
			thr_wrp_pthread_cond_signal(&atc_cond);
			pthread_mutex_unlock(&atc_mutex);
			at_resp_msg_send(buf_print, RESP_ERROR);
		} 
		else if (strncasecmp((const char*) buf_print, "+CME", 4)==0)
		{
			at_resp_msg_send(buf_print, RESP_CME);
		}
		else if (strncasecmp((const char*) buf_print, "+CGEV", 5)==0)
		{
			at_resp_msg_send(buf_print, RESP_CGEV);
		}
		else if (strncasecmp((const char*) buf_print, "+CGCONTRDP", 10)==0)
		{
			at_resp_msg_send(buf_print, RESP_CGCONTRDP);
		}
		else if (strncasecmp((const char*) buf_print, "+CREG", 5)==0)
		{
			at_resp_msg_send(buf_print, RESP_CREG);
		}
		else if (strncasecmp((const char*) buf_print, "+CEREG", 6)==0)
		{
			at_resp_msg_send(buf_print, RESP_CEREG);
		}
		else if (strncasecmp((const char*) buf_print, "+CGACT", 6)==0)
		{
			at_resp_msg_send(buf_print, RESP_CGACT);
		}
		else if (strncasecmp((const char*) buf_print, "+CGATT", 6)==0)
		{
			at_resp_msg_send(buf_print, RESP_CGATT);
		}
	#endif	
	}
	close(r_fd);
	return len;
}

void* read_thread(void* arg)
{
	struct TTYInfo* tty = (struct TTYInfo*) arg;

	cm_print(LOG_LEVEL_TRACE, "read thread start\n");
#if defined(__linux__)	
	sigset_t newmask;
#endif

#if defined(__linux__)
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &newmask, NULL);
#endif

	pthread_mutex_lock(&atc_mutex);
	cm_print(LOG_LEVEL_TRACE, "atc_cond signaling\n");
	thr_wrp_pthread_cond_signal(&atc_cond);
	pthread_mutex_unlock(&atc_mutex);

	while (1)
	{
		if (at_read(tty) <= 0)
			usleep(10000);
	}

	return RTN_OK;
}


int lte_device_init()
{
	struct TTYInfo *tty;

	/* device open and start read thread */
	gp_dev = device_open(tty_name, &gtty, canonical);

	if (gp_dev == NULL) 
		return RTN_ERR;

	tty = (struct TTYInfo *)gp_dev;
	
	if (tty->fd < 0) 
		return RTN_ERR;

	/* Start reading thread */
	pthread_mutex_lock(&atc_mutex);
	if(pthread_create(&at_thread, NULL, read_thread, gp_dev ) < 0)
	{
		cm_print(LOG_LEVEL_ERROR, "Error: Failed to created throughput read thread\n");
		return RTN_ERR;
	}

	thr_wrp_pthread_cond_wait(&atc_cond, &atc_mutex); // wait until read_thread created.

	pthread_mutex_unlock(&atc_mutex);
	sm_change_state_and_action(MSG_DEVICE_CONNECTED, NULL);
	return RTN_OK;

}

int lte_device_term()
{

	pthread_cancel(at_thread);
	pthread_detach(at_thread);
	if (gp_dev)
		device_close(gp_dev);

	gp_dev = NULL;
	sm_change_state_and_action(MSG_DEVICE_DISCONNECTED, NULL);
	return RTN_OK;
}

int is_exist_dev_ttyact()
{
	return huayi_file_exists(tty_name);
}

void* device_check_thread(void* arg)
{
	int dev_ttyacm_exist = 0, dev_ttyacm_change = 0;
#if defined(__linux__)	
	sigset_t newmask;
#endif

#if defined(__linux__)
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &newmask, NULL);
#endif

	/* Before run the while loop to check USB device connect, let's try to open it first. */
	if(lte_device_init() == RTN_OK) 
	{ 
		is_usb_connected = 1;
		zdbg("\nUSB device is connected\n");	
	}

	while(1)
	{
		dev_ttyacm_exist = is_exist_dev_ttyact();
				
 		if( dev_ttyacm_exist 
			&& is_usb_connected == 0 ) /* removed --> added */
		{
			zdbg("\nUSB device is connected\n");	
			if(lte_device_init() == RTN_ERR) { sleep(5); continue; }
			is_usb_connected = 1;
			
		}
		else if( !dev_ttyacm_exist
			&& is_usb_connected == 1) /* added --> removed */
		{
			/* device close and stop read thread */
			lte_device_term();
			is_usb_connected = 0;
			zdbg("\nUSB device is removed\n");	
		}

		sleep(5);
	}
	return RTN_OK;
	
}

void* at_resp_process_thr(void* arg)
{
	atc_resp_t* p_resp;
	atc_resp_t resp;
		
	while(1)
	{
		p_resp = q_recv_mesg(msg_q_id);
		memcpy(&resp, p_resp, sizeof(atc_resp_t));
		free(p_resp);
		/* PARSE */
		at_process_response(resp.msg, resp.type);
		usleep(100);
	}

	return NULL;
}

int at_resp_msg_send(char* msg, int type)
{
	atc_resp_t *resp_msg;
	func_in();
	resp_msg = (atc_resp_t*)malloc(sizeof(atc_resp_t));
	memset(resp_msg, 0x00, sizeof(atc_resp_t));
	strncpy(resp_msg->msg, msg, AT_CMD_MAX_LEN);	
	resp_msg->type = type;
	
	q_send_mesg(msg_q_id, resp_msg); // allocation memory will be deallocated by msg read thread.
	func_out();

	return RTN_OK;
	
}

int at_start_resp_process_thr()
{
	q_message_queue_init();
	msg_q_id = q_create_message_queue();
	if(pthread_create(&msg_q_thread, NULL, at_resp_process_thr, NULL) < 0)
	{
		printf("Error: Failed to created at_resp_process_thr thread \n");
		return -1;
	}
	return RTN_OK;

}

int at_stop_resp_process_thr()
{

	pthread_cancel(msg_q_thread);
	pthread_detach(msg_q_thread);
	return RTN_OK;
}

int send_at_command(char* buf)
{
	at_set_latest_cmd(buf);	
	cm_print(LOG_LEVEL_TRACE, "cmd:%s\n", buf);
	return at_write(gp_dev, buf);
}


int send_at_command_nonblock(char* buf)
{
	at_set_latest_cmd(buf);	
	cm_print(LOG_LEVEL_TRACE, "cmd:%s\n", buf);
	return at_write_nonblock(gp_dev, buf);
}



int main(int argc, char** argv)
{
#if defined(__linux__)
	struct sigaction sa;
#endif
	pthread_t device_chk_thr;
	char buf[AT_CMD_MAX_LEN];
	char* cmd;
	
	
	if (argc != 3 && argc != 2)
	{
		printf("Usage:\n");
		printf("\t%s <tty device name> <canonical>\n", argv[0]);
		printf("\nUsing default...\n  dev:%s, canonical:%d\n", 
				DEFAULT_DEV_ATC, DEFAULT_CANONICAL_MODE_SUPPORT);
	}
	
	if (argc >= 2)
	{
		printf("device: %s\n", argv[1]);
		strcpy(tty_name, argv[1]);
	}

	if (argc == 3)
	{
		canonical = atoi(argv[2]);
		printf("Canonical Mode = %d\n", canonical);
	}

	printf("****************************************\n");
	printf("*           CLI CM based on ATC        *\n");
	printf("****************************************\n");

	/* Configure signal handler */
#if defined(__linux__)
	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
#endif

	/* State Machine Initialization */
	sm_init();

	/* Initialize mutex */
	pthread_mutex_init(&atc_mutex, NULL);
	pthread_cond_init(&atc_cond, NULL);
	pthread_mutex_init(&write_mutex, NULL);

	if (at_start_resp_process_thr() < 0)
	{
		return -1;
	}

	if(pthread_create(&device_chk_thr, NULL, device_check_thread, NULL) < 0)
	{
		printf("Error: Failed to created device checking thread \n");
		return -1;
	}

	int server_sockfd, client_sockfd;
	int ret = -1;
	struct sockaddr_un client_addr;  
	socklen_t socklen = sizeof(client_addr);
	int recv_len=0;

	server_sockfd = init_unix_socket();

	/* Process commands */
 	while(1)
	{
	#if 0
		printf(AT_PROMPT_SND);

		/* Get write command */
		cmd = fgets(buf, AT_CMD_MAX_LEN, stdin);
		if (!cmd)
		{
			break;
		}

		if (strncmp("exit", buf, 4) == 0)
		{
			break;
		}
		else 
		{
		    do_cmd(buf); 
		}
	#else
		zdbg("server waiting:\n");
		/* accept a connection */  
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &socklen);  

		memset(buf,0,AT_CMD_MAX_LEN);  
		/* exchange data */  
		recv_len = read(client_sockfd, buf,sizeof(buf));
		zdbg("Message from client (%d)) :%s\n",recv_len,buf);
		if ( is_usb_connected )
		{
			//do_cmd(buf);
			ret = send_at_command(buf);
			//printf("ret=%d\n", ret);
			if ( ret < 0 )
				write(client_sockfd,"error",5);
			else
				write(client_sockfd,"ok",2);
		}
		SAFE_CLOSE(client_sockfd);
	#endif
	}

	/* close the socket */	
	SAFE_CLOSE(server_sockfd);  
    unlink(ATCM_UNIX_DOMAIN); 

#if defined(__linux__)
	if (at_thread) {
		pthread_cancel(at_thread);
		pthread_detach(at_thread);
	}
	if (device_chk_thr)
	{
		pthread_cancel(device_chk_thr);
		pthread_detach(device_chk_thr);
	}
	at_stop_resp_process_thr();
#endif

	/* close the device */
	if (gp_dev)
		device_close(gp_dev);

	printf("\n");
	printf("****************************************\n");
	printf("*              Exiting                 *\n");
	printf("****************************************\n");

	return RTN_OK;
}
