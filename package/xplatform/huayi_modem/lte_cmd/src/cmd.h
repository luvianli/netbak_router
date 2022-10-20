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
#ifndef _CMD_H_
#define _CMD_H_
#include "ltetype.h"

typedef u32 cmd_func(u32 argc, char *argv[]);

struct cmd_t {
	const char 	*cmd;
	cmd_func	*func;
	u32 		func_index;
	const char  *description;
};



void do_cmd(char *cmd);

#endif
