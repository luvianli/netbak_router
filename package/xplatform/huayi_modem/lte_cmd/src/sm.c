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
#include <pthread.h>

#include "sm.h"
#include "cm_dbg.h"
#include "atc_parser.h"
#include "main.h"

pthread_mutex_t sm_mutex;

static int current_state = STATE_INIT;
static int network_registration_state;

int sm_init()
{
	pthread_mutex_init(&sm_mutex, NULL);
	network_registration_state = REGISTRATION_READY; // initial state is "ready".
	return RTN_OK;
}

int sm_get_current_state()
{
	return current_state;
}

int is_reg_network(int creg_state) 
{
	//+CREG:
	//	1,5,6,7,8,9,10  -- ready
	//  0,2,3,4 - not ready
	switch (creg_state)
	{
	case 0:
	case 2:
	case 3:
	case 4:
		return REGISTRATION_NOT_READY;
	default:
		return REGISTRATION_READY;
	}
	return REGISTRATION_UNKNOWN;
	
}


char* sm_get_state_str(int state)
{
	switch (state)
	{
	case STATE_INIT:
		return "STATE_INIT";
	case STATE_DEVICE_READY:
		return "STATE_DEVICE_READY";
	case STATE_GETTING_STATUS:
		return "STATE_GETTING_STATUS";
	case STATE_WAITING_PLMN_SEARCH:
		return "STATE_WAITING_PLMN_SEARCH";
	case STATE_WAITING_ATTACH:
		return "STATE_WAITING_ATTACH";
	case STATE_WAITING_IP_INFO:
		return "STATE_WAITING_IP_INFO";
	case STATE_ATTACHED:
		return "STATE_ATTACHED";
	case STATE_WAITING_DETACH:
		return "STATE_WAITING_DETACH";
	case STATE_DETACHED:
		return "STATE_DETACHED";
	case STATE_NULL:
		return "STATE_NULL";
	default:
		return "Unknown Status";
	}

	return "Unknown Status";
}

int sm_set_current_state(int st)
{
	func_in();

	//cm_print(LOG_LEVEL_TRACE, "%s ==> %s\n", sm_get_state_str(current_state), sm_get_state_str(st));
	printf("\n%s ==> %s\n", sm_get_state_str(current_state), sm_get_state_str(st));
	pthread_mutex_lock(&sm_mutex);
	current_state = st;
	pthread_mutex_unlock(&sm_mutex);

	func_out();

	return RTN_OK;
}	

int sm_change_state_and_action(int notimsg, void* arg)
{
	char* cmd;

	func_in();
#if defined(MODEM_IML_C2400W)
#ifndef NOT_USE_CONNECTION_MANAGER 
	if (notimsg == MSG_DEVICE_CONNECTED)
	{
		/*  Extended errors */
		send_at_command("AT+CMEE=1");

		/* Network registration : 1=enable network registration unsolicited result code */
		send_at_command("AT+CREG=1");

		/*  Enable +CGEV GPRS event notifications, but don't buffer */
		send_at_command("AT+CGEREP=1,0");
		send_at_command("AT+CEREG=2");
		
		sm_set_current_state(STATE_GETTING_STATUS);
		sleep(DETACH_OK_DELAY);
		send_at_command("AT+CGATT?");	// Get attach status
	}

	if (notimsg == MSG_DEVICE_DISCONNECTED)
	{
		/* DO NOTHING */
	}

	if (notimsg == MSG_CGEV_RESP) 
	{
		cgev_resp_t *rt;
		rt = (cgev_resp_t*)arg;
		if (rt->type == CGEV_NET_TYPE_MOBILE && rt->action == CGEV_ACTION_DETACH) // mobile detached
		{
			if (is_reg_network(network_registration_state) == REGISTRATION_READY)
			{
				/* network registered state */
				sleep(DETACH_OK_DELAY);
				sm_set_current_state(STATE_GETTING_STATUS);
				send_at_command("AT+CGATT?");	// Get current attach status
				return RTN_OK;
			} 
		}
	}

	switch (current_state)
	{
	case STATE_INIT:
		break;
	case STATE_GETTING_STATUS: // getting attach state 
		if (notimsg == MSG_CGATT_RESP)
		{
#if 1

			cgatt_resp_t* cg;
			cg = arg;
			cm_print(LOG_LEVEL_TRACE, "state(%d)\n", cg->state);
			if (cg->state == PDN_DEACTIVATE)
			{
				sm_set_current_state(STATE_WAITING_PLMN_SEARCH);
				send_at_command("AT+COPS=0,,,7"); 
			} else {
				cm_print(LOG_LEVEL_TRACE, "internet PDN activated\n");
				sm_set_current_state(STATE_ATTACHED);
			}
#endif
			
		}

		if (notimsg == MSG_CGACT_RESP)
		{
			cgact_resp_t* cg;
			cg = arg;
			cm_print(LOG_LEVEL_TRACE, "cid(%d), state(%d)\n", cg->cid, cg->state);
			if (cg->cid == INTERNET_PDN_TYPE && cg->state == PDN_DEACTIVATE)  // internet PDN deactivated.
			{
				cm_print(LOG_LEVEL_TRACE, "internet PDN deactivated\n");
				sm_set_current_state(STATE_WAITING_PLMN_SEARCH);
				send_at_command("AT+COPS=0,,,7");  // auto, E-UTRAN

			}

			if (cg->cid == INTERNET_PDN_TYPE && cg->state == PDN_ACTIVATE)  // internet PDN act.
			{
				cm_print(LOG_LEVEL_TRACE, "internet PDN activated\n");
				sm_set_current_state(STATE_ATTACHED);
			}
		}
		break;
	case STATE_WAITING_PLMN_SEARCH:
		if (notimsg == MSG_CEREG_RESP) 
		{
			cereg_resp_t *rt;
			rt = (cereg_resp_t*) arg;
			if (rt->state == 0) // ready
			{
				cmd = (char*)malloc(AT_COMMAND_STRING_MAX);
				if (!cmd) { cm_print(LOG_LEVEL_ERROR, "malloc failed\n"); return RTN_ERR;}
				memset(cmd, 0x00, AT_COMMAND_STRING_MAX);
				sprintf(cmd, "AT+CGDCONT=%d,\"%s\",\"%s\"",INTERNET_PDN_CID, PDP_TYPE_IPV4V6, DEFAULT_INTERNET_APN_NAME);
				send_at_command(cmd);

				memset(cmd, 0x00, AT_COMMAND_STRING_MAX);
				sprintf(cmd, "AT%%GPDNTYPE=%d,%d", INTERNET_PDN_CID, INTERNET_PDN_TYPE);				
				send_at_command(cmd);

				memset(cmd, 0x00, AT_COMMAND_STRING_MAX);

				sm_set_current_state(STATE_WAITING_ATTACH);
				/* attach request */
				sprintf(cmd, "AT+CGACT=1,%d",INTERNET_PDN_CID);
				send_at_command(cmd);
				free(cmd);
			}
		}

		break;
	case STATE_WAITING_ATTACH:

		if (notimsg == MSG_CGEV_RESP) 
		{
			cgev_resp_t *rt;
			char* buffer;

			rt = (cgev_resp_t*)arg;
			if (rt->type == 1 && rt->action == 1) // mobile attached
			{
				cmd = (char*)malloc(AT_COMMAND_STRING_MAX);
				if (!cmd) { cm_print(LOG_LEVEL_ERROR, "malloc failed\n"); return RTN_ERR;}
				memset(cmd, 0x00, AT_COMMAND_STRING_MAX);
				// requeet IP address
				sprintf(cmd, "AT+CGCONTRDP=%d", rt->cid);
				cm_print(LOG_LEVEL_TRACE, "cmd: %s\n", cmd);
				sm_set_current_state(STATE_WAITING_IP_INFO);
				send_at_command(cmd);
				free(cmd);
			}
		}

		break;
	case STATE_WAITING_IP_INFO:
		if (notimsg == MSG_CGCONTRSP_RESP) 
		{
			cgcontrsp_resp_t *rt;
			char* buffer;
			int rtn;

			rt = (cgcontrsp_resp_t*)arg;

			cmd = (char*)malloc(AT_COMMAND_STRING_MAX);
			if (!cmd) { cm_print(LOG_LEVEL_ERROR, "malloc failed\n"); return RTN_ERR;}
			memset(cmd, 0x00, AT_COMMAND_STRING_MAX);

			sprintf(cmd, "./vlan_setup.sh %s %s %s %s %s", rt->ipaddr, rt->gateway, rt->subnetmask, rt->dns1, rt->dns2);
			printf("cmd: %s\n", cmd);
#ifndef NOT_USE_SETUP_IP
			rtn = system(cmd);
#endif
			free(cmd);
			sm_set_current_state(STATE_ATTACHED);
		}
		break;
	case STATE_ATTACHED:
		if (notimsg == MSG_CREG_RESP)
		{
			creg_resp_t *rt;
			rt = (creg_resp_t*)arg;

			network_registration_state = rt->stat;

			cm_print(LOG_LEVEL_TRACE, "Received CREG:%d\n", network_registration_state);
			/* detach will be perform when CGEV error received */
		}

		if (notimsg == MSG_CGEV_RESP) 
		{
			cgev_resp_t *rt;

			rt = (cgev_resp_t*)arg;
			if (rt->type == CGEV_NET_TYPE_MOBILE && rt->action == CGEV_ACTION_DETACH) // mobile detached
			{
				if (is_reg_network(network_registration_state) == REGISTRATION_READY)
				{
					//send_at_command("AT+CGATT=0");	 // send detach again.
					/* network registered state */
					sleep(DETACH_OK_DELAY);
					sm_set_current_state(STATE_GETTING_STATUS);
					send_at_command("AT+CGATT?");	// Get current attach status
				} else { 
					/* network not registered state */
					//send_at_command("AT+CGATT=0");	 // send detach again.
					sm_set_current_state(STATE_DETACHED);
				}
			}
		}
		break;
	case STATE_WAITING_DETACH:
		break;
	case STATE_DETACHED:
		if (notimsg == MSG_CME_ERR_RESP)
		{
			
		}
		if (notimsg == MSG_CREG_RESP)
		{
			creg_resp_t *rt;
			rt = (creg_resp_t*)arg;
			network_registration_state = rt->stat;

			if (is_reg_network(network_registration_state) == REGISTRATION_READY)
			{
				sleep(DETACH_OK_DELAY);
				sm_set_current_state(STATE_GETTING_STATUS);
				send_at_command("AT+CGATT?");	// Get current attach status
			}
		}
		break;
	default:
		break;

	}

__end:
	func_out();
#endif //#ifndef NOT_USE_CONNECTION_MANAGER 

#endif
	return RTN_OK;
}





