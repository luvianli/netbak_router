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
#ifndef _CM_DBG_H_
#define _CM_DBG_H_

#include "ltetype.h"

void dbg_print(u16 level, const char *fmt, ...);

#define func_in(void)\
	dbg_print(LOG_LEVEL_TRACE, "[%s+]\n", __FUNCTION__)
#define func_out(void)\
	dbg_print(LOG_LEVEL_TRACE, "[%s-]\n", __FUNCTION__)
#define cm_print(level, fmt, args...)\
	dbg_print(level, "[%-25s] " fmt, __FUNCTION__, ## args)

#endif //_CM_GDB_H_
