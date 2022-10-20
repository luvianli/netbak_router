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
#ifndef _CLI_ATC_MAIN_H_
#define _CLI_ATC_MAIN_H_

#include "cm_dbg.h"
#include "atc_parser.h"
#include "huayi_atcmd.h"
#include "huayi_uci_fun.h"


#define AT_PROMPT_SND 		"CM> "
#define AT_PROMPT_RCV		">> "

#define DEFAULT_CANONICAL_MODE_SUPPORT	0
#define USB_DEVICE_STATUS_FILE "/var/log/usb_status"
#define USB_DEVICE_STATUS_FILE_TEMP "/var/log/usb_status.tmp"

typedef struct {
	at_resp_type_t type;
	char msg[AT_CMD_MAX_LEN];
} atc_resp_t;


int send_at_command(char* buf);
int send_at_command_nonblock(char* buf);

int device_write(void* dev, char* buf);
int device_read(void* dev, char* buf);
int device_scan();
void* device_open(char* name, void* tty_info, int canonical);
int device_close(void* dev);


#endif //_CLI_ATC_MAIN_H_

