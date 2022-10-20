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
#include <stdarg.h>

#include "cm_dbg.h"

void dbg_print(u16 level, const char *fmt, ...)
{
	va_list args;
	char *buf = NULL;

	if(!(CURRENT_LOG_LEVEL & level)) 
		return;

	buf = (char *)calloc(1,1024); 
	if(NULL == buf)
	{
		printf("buf alloc fail\n");
		return;
	}

	va_start(args, fmt);
	vsnprintf(buf, 1024, fmt, args);

	printf("%s", buf);

	free(buf);

	va_end(args);
}



