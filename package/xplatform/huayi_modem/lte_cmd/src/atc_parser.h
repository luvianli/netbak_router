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
#ifndef _ATC_PARSER_H_
#define _ATC_PARSER_H_

#define IMS_PDN_TYPE		1
#define ADMIN_PDN_TYPE		2
#define INTERNET_PDN_TYPE 	3

#define INTERNET_PDN_CID	1
#define IMS_PDN_CID			2
#define ADMIN_PDN_CID		3

#define DEFAULT_IMS_APN_NAME		"CMNET"
#define DEFAULT_ADMIN_APN_NAME		"CMNET"
#define DEFAULT_INTERNET_APN_NAME	"internet.lguplus.co.kr"//"CMNET"

#define PDN_ACTIVATE	1
#define PDN_DEACTIVATE	0

#define PDP_TYPE_IPV4		"IPV4"
#define PDP_TYPE_IPV6		"IPV6"
#define PDP_TYPE_IPV4V6		"IPV4V6"

#define AT_COMMAND_STRING_MAX	1024

typedef enum {
	RESP_OK = 0,
	RESP_ERROR,
	RESP_CME,
	RESP_CGEV,
	RESP_CGCONTRDP,
	RESP_CREG,
	RESP_CEREG,
	RESP_COPS,
	RESP_CGACT,
	RESP_CGATT,
	RESP_MAX
}at_resp_type_t;

typedef enum {
    CME_ERROR_NON_CME = -1,
    CME_SUCCESS = 0,
    CME_SIM_NOT_INSERTED = 10,
    CME_SIM_FAILURE = 13,
	CME_NO_NETWORK_SERVICE = 30,
	CME_NETWORK_TIMEOUT	= 31,
	CME_ILLEGAL_MS = 103,
	CME_ILLEGAL_ME = 106,
	CME_S_OPTION_NOT_SUPPORTED = 132, 
	CME_S_OPTION_NOT_SUBSCRIBED = 133,
	CME_S_OPTION_OUT_OF_ORDER = 134,
	CME_UNSPECIFIED_GPRS = 148,
	CME_PDN_DISCON_NOT_ALLOWED = 151,
	CME_PLS_ALREADY_PROGRESSING = 240,
	CME_EPS_SERVICE_NOT_ALLOWED = 241,
	CME_EPS_NON_EPS_NOT_ALLOWED = 242,
	CME_APN_VERIFICATION_FAILED = 243,
	CME_NETWORK_REJECT = 244,
	CME_ALREADY_IN_PROGRESS = 245,
} at_cme_error_t;


int at_process_response(char* resp, at_resp_type_t res_type);
int at_process_cme_error(at_cme_error_t err);
int at_get_cme_error(char* resp);
int at_process_cgev(char* resp);
int at_set_latest_cmd(char* command);
int at_get_latest_cmd(char** out);



#endif
