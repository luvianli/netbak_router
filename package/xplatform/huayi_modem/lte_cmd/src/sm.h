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
#ifndef _SM_H_
#define _SM_H_

#define DETACH_OK_DELAY		2



typedef enum {
	STATE_INIT = 0,
	STATE_DEVICE_READY,
	STATE_GETTING_STATUS,
	STATE_WAITING_PLMN_SEARCH,
	STATE_WAITING_ATTACH,
	STATE_WAITING_IP_INFO,
	STATE_ATTACHED,
	STATE_WAITING_DETACH,
	STATE_DETACHED,
	STATE_NULL
}state_t;


typedef enum {
	REGISTRATION_NOT_READY = 0,
	REGISTRATION_READY,
	REGISTRATION_UNKNOWN
} net_state_t;



typedef enum {
	MSG_DEVICE_CONNECTED = 0,
	MSG_COPS_RESP, 
	MSG_CGACT_RESP, 
	MSG_CGATT_RESP, 
	MSG_OK_ERROR_RESP,
	MSG_CGEV_RESP,
	MSG_CGCONTRSP_RESP,
	MSG_DEVICE_DISCONNECTED,
	MSG_CREG_RESP,
	MSG_CME_ERR_RESP,
	MSG_CEREG_RESP,
	MSG_NULL
}noti_message_t;

// MSG_COPS_RESP, 
typedef struct {
	int cops_mode;
}cops_resp_t;

// MSG_CGACT_RESP
typedef struct {
	int cid;
	int state; // attach state: 0=deactivated, 1=activated
}cgact_resp_t;

// MSG_CGATT_RESP
typedef struct {
	int state; // attach state: 0=deactivated, 1=activated
}cgatt_resp_t;


//MSG_OK_ERROR_RESP,
typedef struct {
	int ok_error;
}ok_error_resp_t;

// MSG_CGEV_RESP,
typedef enum {
	CGEV_NET_TYPE_NETWORK = 0,
	CGEV_NET_TYPE_MOBILE = 1,
	CGEV_NET_TYPE_UNKNOWN
}cgev_net_type_t;

typedef enum {
	CGEV_ACTION_DETACH = 0,
	CGEV_ACTION_ATTACH = 1,
	CGEV_ACTION_CLASS = 2
}cgev_action_t;

typedef struct {
	int type; // 0=network, 1=mobile
	int action; // 0=detach, 1=attach, 2=class_change
	int cid; // cid number of PDN ACT
}cgev_resp_t;

// MSG_CGCONTRSP,
typedef struct {
	char* ipaddr;
	char* subnetmask;
	char* gateway;
	char* dns1;
	char* dns2;	
}cgcontrsp_resp_t;

// MSG_CREG_RESP
typedef struct {
	int stat;	
	char lac[10];
	int ci[10];
	int AcT;
	int cause_type;
	int reject_cause;
}creg_resp_t;

//MSG_CME_ERR_RESP,
typedef struct {
	int error;	
}cme_err_resp_t;

// MSG_CEREG_RESP
typedef struct {
	int state; // state: 0=ready, others=error
}cereg_resp_t;



int sm_init();
int sm_get_current_state();
int sm_set_current_state(int st);
int sm_change_state_and_action(int notimsg, void* arg);

#endif
