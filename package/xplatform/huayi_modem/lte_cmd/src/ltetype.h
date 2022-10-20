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

#ifndef _LTE_TYPE_H_
#define _LTE_TYPE_H_

#ifndef LTE_TYPE_DEFINE
#define LTE_TYPE_DEFINE
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;
typedef char				s8;
typedef short				s16;
typedef int 				s32;
typedef long long 			s64;
typedef void* PMUTEX;
typedef void* APIHAND;
#endif // LTE_TYPE_DEFINE



#define LOG_LEVEL_OFF		((u16) 0x00)	// Silent
#define LOG_LEVEL_ON		((u16) 0x01)	// LOG_LEVEL_FORCE available
#define LOG_LEVEL_ERROR		((u16) 0x02)	// error case
#define LOG_LEVEL_NOTICE	((u16) 0x04)	// When wanted to be notice something
#define LOG_LEVEL_TRACE		((u16) 0x10)	// func in, func out and specific point
#define LOG_LEVEL_DATA		((u16) 0x40)	// print buffer data

#define LOG_LEVEL_INFO		(LOG_LEVEL_ERROR | LOG_LEVEL_NOTICE)
#define LOG_LEVEL_LOUD		(LOG_LEVEL_INFO | LOG_LEVEL_TRACE)
#define LOG_LEVEL_ALL		(LOG_LEVEL_LOUD | LOG_LEVEL_DATA)

#define LOG_LEVEL_FORCE		((u16) 0x7F)	// Temporary use only
#define LOG_LEVEL_INVALID	((u16) 0xFF)

//#define CURRENT_LOG_LEVEL LOG_LEVEL_ALL	
#define CURRENT_LOG_LEVEL	LOG_LEVEL_INFO


#define RTN_OK		0
#define RTN_ERR		1


#endif
