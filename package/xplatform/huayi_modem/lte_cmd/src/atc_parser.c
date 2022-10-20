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

#include "atc_parser.h"
#include "main.h"
#include "cm_dbg.h"
#include "ltetype.h"
#include "at_tok.h"
#include "sm.h"

static char latest_command[1024];


int at_set_latest_cmd(char* command)
{
	strcpy(latest_command, command);
	return RTN_OK;
}

int at_get_latest_cmd(char** out)
{
	strcpy(*out, latest_command);
	return RTN_OK;
}

int at_process_cme_error(at_cme_error_t err)
{
	switch (err)
	{
	case CME_ERROR_NON_CME:
		printf("CME_ERROR_NON_CME\n");
		break;
	case CME_SUCCESS:
		printf("CME_SUCCESS\n");
		break;
	case CME_SIM_NOT_INSERTED:
		printf("CME_SIM_NOT_INSERTED\n");
		break;
	case CME_SIM_FAILURE:
		printf("CME_SIM_FAILURE\n");
		break;
	case CME_NO_NETWORK_SERVICE:
		printf("CME_NO_NETWORK_SERVICE\n");
		break;
	case CME_NETWORK_TIMEOUT:
		printf("CME_NETWORK_TIMEOUT\n");
		break;
	case CME_ILLEGAL_MS:
		printf("CME_ILLEGAL_MS\n");
		break;
	case CME_ILLEGAL_ME:
		printf("CME_ILLEGAL_ME\n");
		break;
	case CME_S_OPTION_NOT_SUPPORTED:
		printf("CME_S_OPTION_NOT_SUPPORTED\n");
		break;
	case CME_S_OPTION_NOT_SUBSCRIBED:
		printf("CME_S_OPTION_NOT_SUBSCRIBED\n");
		break;
	case CME_S_OPTION_OUT_OF_ORDER:
		printf("CME_S_OPTION_OUT_OF_ORDER\n");
		break;
	case CME_UNSPECIFIED_GPRS:
		printf("CME_UNSPECIFIED_GPRS\n");
		break;
	case CME_PDN_DISCON_NOT_ALLOWED:
		printf("CME_PDN_DISCON_NOT_ALLOWED\n");
		break;
	case CME_PLS_ALREADY_PROGRESSING:
		printf("CME_PLS_ALREADY_PROGRESSING\n");
		break;
	case CME_EPS_SERVICE_NOT_ALLOWED:
		printf("CME_EPS_SERVICE_NOT_ALLOWED\n");
		break;
	case CME_EPS_NON_EPS_NOT_ALLOWED:
		printf("CME_EPS_NON_EPS_NOT_ALLOWED\n");
		break;
	case CME_APN_VERIFICATION_FAILED:
		printf("CME_APN_VERIFICATION_FAILED\n");
		break;
	case CME_NETWORK_REJECT:
		printf("CME_NETWORK_REJECT\n");
		break;
	case CME_ALREADY_IN_PROGRESS:
		printf("CME_ALREADY_IN_PROGRESS\n");
		break;
	default:
		break;
	}	
	return RTN_OK;
}

int at_get_cme_error(char* resp)
{
	char *p_cur;
	int ret;
	int err;

	if (resp == NULL)
		return CME_ERROR_NON_CME;

	p_cur = resp;
	err = at_tok_start(&p_cur);

	if (err < 0) {
		return CME_ERROR_NON_CME;
	}

	err = at_tok_nextint(&p_cur, &ret);

	if (err < 0) {
		return CME_ERROR_NON_CME;
	}
	return ret;

}

int at_process_creg(char* resp)
{
	char *p_cur;
	int ret;
	int err;
	creg_resp_t rt;

	if (resp == NULL)
		return RTN_ERR;

	p_cur = resp;
	err = at_tok_start(&p_cur);

	if (err < 0) {
		return RTN_ERR;
	}

	err = at_tok_nextint(&p_cur, &ret);

	if (err < 0) {
		return RTN_ERR;
	}

	rt.stat = ret;	
	sm_change_state_and_action(MSG_CREG_RESP, (void*)&rt);

	return ret;

}

int at_process_cgcontrdp(char* resp)
{
	char *p_cur;
	int ret;
	int err = -1;
	int cid;
	int bearer_id;
	char *skip;
	char *gateway = NULL;
	char *subnetmask= NULL;
	char *ip_addr;
	char *dns1;
	char *dns2;
	char cmd[1024];
	cgcontrsp_resp_t rt;

	if (resp == NULL)
		goto error;

	p_cur = resp;
	err = at_tok_start(&p_cur);

	if (err < 0) {
		goto error;
	}

	err = at_tok_nextint(&p_cur, &cid);
	if (err < 0)
		goto error;

	err = at_tok_nextint(&p_cur, &bearer_id);
	if (err < 0)
		goto error;

	err = at_tok_nextstr(&p_cur, &skip);
	if (err < 0)
		goto error;

	// IP address
	err = at_tok_nextstr(&p_cur, &ip_addr);
	if (err < 0)
		goto error;

	if (strlen(ip_addr) == 0) 
		return RTN_ERR;
	
	// Subnet Mask
	err = at_tok_nextstr(&p_cur, &subnetmask);
	if (err < 0)
		goto error;

	if (strlen(subnetmask) == 0) 
		return RTN_ERR;
	

	// Gateway
	err = at_tok_nextstr(&p_cur, &gateway);
	if (err < 0)
		goto error;

	if (strlen(gateway) == 0) 
		return RTN_ERR;

	// Primary DNS 
	err = at_tok_nextstr(&p_cur, &dns1);
	if (err < 0)
		goto error;

	if (strlen(dns1) == 0) 
		return RTN_ERR;

	// Secondary DNS 
	err = at_tok_nextstr(&p_cur, &dns2);
	if (err < 0)
		goto error;

	printf("IP: 		%s\n", ip_addr);	
	printf("Subnet: 	%s\n", subnetmask);	
	printf("Gateway: 		%s\n", gateway);	
	printf("DNS1: 		%s\n", dns1);	
	printf("DNS2: 		%s\n", dns2);	

	rt.ipaddr = ip_addr;
	rt.subnetmask = subnetmask;
	rt.gateway = gateway;
	rt.dns1 = dns1;
	rt.dns2 = dns2;

	sm_change_state_and_action(MSG_CGCONTRSP_RESP, (void*)&rt);
	
	err = RTN_OK;

error:

	if (err < 0) {
		return err;
	}
	return RTN_OK;

}


int at_process_cgev(char* resp)
{
	char *p_cur;
	int ret;
	int err;
	char* buffer;
	cgev_resp_t cgev;

	if (resp == NULL)
		return RTN_ERR;

	p_cur = resp;
	err = at_tok_start(&p_cur);

	if (err < 0) {
		return RTN_ERR;
	}
	
	if (strstr(p_cur, "ME DETACH"))
	{
		cgev.type=1; // mobile
		cgev.action=0; // detach
		cgev.cid = -1;
		sm_change_state_and_action(MSG_CGEV_RESP, (void*)&cgev);
	}
	else if (strstr(p_cur, "ME PDN ACT"))
	{
		cgev.type=1; // mobile
		cgev.action=1; // attach 

		p_cur += strlen("ME PDN ACT ");
		err = at_tok_nextint(&p_cur, &ret);

		cgev.cid = ret;
		sm_change_state_and_action(MSG_CGEV_RESP, (void*)&cgev);
	}

	return RTN_OK;
}



int at_process_cops(char* resp)
{
	char *p_cur;
	int ret;
	int err;

	p_cur = resp;
	err = at_tok_start(&p_cur);

	if (err < 0) {
		return RTN_ERR;
	}

	err = at_tok_nextint(&p_cur, &ret);

	if (err < 0) {
		return RTN_ERR;
	} else {
		sm_change_state_and_action(MSG_COPS_RESP, &ret);
	}

	return RTN_OK;

}

int at_process_cgact(char* resp)
{
	char *p_cur;
	int ret;
	int err;
	cgact_resp_t cg;

	p_cur = resp;
	err = at_tok_start(&p_cur);

	if (err < 0) {
		return RTN_ERR;
	}

	err = at_tok_nextint(&p_cur, &ret);

	if (err < 0) {
		return RTN_ERR;
	} else {
		cg.cid=ret;
	}

	err = at_tok_nextint(&p_cur, &ret);

	if (err < 0) {
		return RTN_ERR;
	} else {
		cg.state=ret;
	}
	sm_change_state_and_action(MSG_CGACT_RESP, &cg);

	return RTN_OK;
}

int at_process_cereg(char* resp)
{
	char *p_cur;
	int ret;
	int err;
	cereg_resp_t cg;

	p_cur = resp;
	err = at_tok_start(&p_cur);

	if (err < 0) {
		return RTN_ERR;
	}

	err = at_tok_nextint(&p_cur, &ret);

	if (err < 0) {
		return RTN_ERR;
	} else {
		cg.state =ret;
	}


	sm_change_state_and_action(MSG_CEREG_RESP, &cg);

	return RTN_OK;
}

int at_process_cgatt(char* resp)
{
	char *p_cur;
	int ret;
	int err;
	cgatt_resp_t cg;

	p_cur = resp;
	err = at_tok_start(&p_cur);

	if (err < 0) {
		return RTN_ERR;
	}

	err = at_tok_nextint(&p_cur, &ret);

	if (err < 0) {
		return RTN_ERR;
	} else {
		cg.state=ret;
	}

	sm_change_state_and_action(MSG_CGATT_RESP, &cg);

	return RTN_OK;
}


int at_process_response(char* resp, at_resp_type_t res_type)
{
	char *p_cur;
	int err;

	if (resp == NULL)
		return RTN_ERR;

	switch (res_type)
	{
	case RESP_OK:
		//printf("result: OK\n");
		{
			ok_error_resp_t ret;
			ret.ok_error = res_type;
			sm_change_state_and_action(MSG_OK_ERROR_RESP, &ret);
		}
		
		break;
	case RESP_ERROR:
		//printf("result: ERROR\n");
		{
			ok_error_resp_t ret;
			ret.ok_error = res_type;
			sm_change_state_and_action(MSG_OK_ERROR_RESP, &ret);
		}
		break;
	case RESP_CME:
		{
			err = at_get_cme_error(resp);

			if (err != CME_ERROR_NON_CME)
			{
				at_process_cme_error((at_cme_error_t)err);
			}
			
			return RTN_OK;
		}
		break;
	case RESP_CGEV:
		at_process_cgev(resp);
		break;
	case RESP_CGCONTRDP:
		at_process_cgcontrdp(resp);
		break;
	case RESP_CREG:
		at_process_creg(resp);
		break;
	case RESP_CEREG:
		at_process_cereg(resp);
		break;
	case RESP_COPS:
		at_process_cops(resp);
		break;
	case RESP_CGACT:
		at_process_cgact(resp);
		break;
	case RESP_CGATT:
		at_process_cgatt(resp);
		break;
	default:
		break;
	}	
	
	return RTN_OK;
}





