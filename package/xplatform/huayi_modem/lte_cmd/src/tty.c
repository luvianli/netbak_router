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
#if defined(__linux__)


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>

#include "main.h"
#include "tty.h"



int device_write(void* dev, char* buf)
{
	struct TTYInfo* tty = dev;
	int len = (int)strlen(buf);
	int ret;
	char sbuf[len+2];
	int send_len = len+2;

	/* Check if the AT command is null terminated */
	tcgetattr(tty->fd, &tty->newtio);
	strncpy(sbuf, buf, (size_t)len);

	sbuf[len] = '\r';
	sbuf[len+1] = '\n'; 

	if (sbuf[len-1] == 0x1a)
		send_len = len;
	
	ret = (int)write(tty->fd, sbuf, (size_t)send_len);

	return ret;
}

int device_read(void* dev, char* buf)
{
	struct TTYInfo* tty = dev;
	int ret;
	
	ret = (int)read(tty->fd, buf, AT_CMD_MAX_LEN); 
	
	return ret;
}

int device_scan()
{
	return RTN_OK;
}

void* device_open(char* tty_name, void* tty_info, int canonical)
{
	struct TTYInfo* tty = (struct TTYInfo*) tty_info;

	/* Clear TTY structure */
	memset(tty, 0, sizeof(struct TTYInfo));
	
	tty->fd = open(tty_name, O_RDWR | O_NOCTTY); 
	if (tty->fd <0) 
	{
		printf("Opening device: failed - %s\n", strerror(errno));
		return NULL;
	}

	tcgetattr(tty->fd,&tty->oldtio);
	memset(&tty->newtio, 0, sizeof(tty->newtio));

	/* 
	 * Control Flag 
	 */
	tty->newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD; 
	
	/* 
	 * Input Flag 
	 * 	IGNPAR : Ignore parity
	 *	ICRNL : replace CR to NL - need this only to test manual typing from a pc terminal
	 */
	tty->newtio.c_iflag = IGNPAR;
	
	/* 
	 * Output Flag
	 */
	tty->newtio.c_oflag = 0;

	/* 
	 * Local Flag 
	 *	ECHO : echo all
	 * 	ECHONL : echo NL (useful for half-duplex)
	 *	ICANON: Canonical mode
	 */
	if (canonical)
		tty->newtio.c_lflag = ICANON;
	else
		tty->newtio.c_lflag = 0;

#ifdef TTY_REFERENCE_SETTING
	tty->newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
	tty->newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	tty->newtio.c_cc[VERASE]   = 0;     /* del */
	tty->newtio.c_cc[VKILL]    = 0;     /* @ */
	tty->newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
	tty->newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	tty->newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	tty->newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	tty->newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
	tty->newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	tty->newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	tty->newtio.c_cc[VEOL]     = 0;     /* '\0' */
	tty->newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	tty->newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	tty->newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	tty->newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	tty->newtio.c_cc[VEOL2]    = 0;     /* '\0' */
#endif
	tty->newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	tty->newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */

	tcflush(tty->fd, TCIFLUSH);
	tcsetattr(tty->fd,TCSANOW,&tty->newtio);

	
	return (void*) tty;
}

int device_close(void* dev)
{
	struct TTYInfo* tty = dev;
		
	/* tcsetattr(tty->fd,TCSANOW,&tty->oldtio); */
	close(tty->fd);

	return RTN_OK;
}

#endif
