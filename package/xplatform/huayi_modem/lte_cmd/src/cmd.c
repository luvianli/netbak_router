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

#include "main.h"
#include "cm_dbg.h"
#include "cmd.h"


pthread_t test_detach_thread;

void* test_detach_loop(void* arg);


static u32  cmd_ate(u32 argc, char *argv[]);
static u32  cmd_help(u32 argc, char *argv[]);
static u32  cmd_at_manual(u32 argc, char *argv[]);
static u32  cmd_at_swv(u32 argc, char *argv[]);
static u32  cmd_at_pls(u32 argc, char *argv[]);
static u32  cmd_at_lteinit(u32 argc, char *argv[]);
static u32  cmd_at_get_sim_status(u32 argc, char *argv[]);
static u32  cmd_at_get_imsi(u32 argc, char *argv[]);
static u32  cmd_at_system(u32 argc, char *argv[]);
static u32  cmd_at_detach(u32 argc, char *argv[]);
static u32  cmd_at_attach(u32 argc, char *argv[]);
static u32 	cmd_at_ipinfo(u32 argc, char *argv[]);
static u32 	cmd_set_host_ip(u32 argc, char *argv[]);
static u32 	cmd_at_sys_reset(u32 argc, char *argv[]);

static u32  cmd_at_script(u32 argc, char *argv[]);
static void cmd_help_detail(const char* cmd, u32 index);


/**********************************************************************************/
/*              Please DO NOT move from this line to cmd_help_detail(),           */
/*                to add new function easily                                      */
/**********************************************************************************/
typedef enum _FUNCTION_INDEX {
	CMD_ATE_REQUEST= 0,
	CMD_AT_SWV_REQUEST,
	CMD_AT_LTE_INIT_REQUEST,
	CMD_AT_PLS_REQUEST,
	CMD_AT_ATTACH_REQUEST,
	CMD_AT_DETACH_REQUEST,
	CMD_AT_GET_IPINFO,
	CMD_AT_GET_SIM_STATUS_REQUEST,
	CMD_AT_GET_IMSI,
	CMD_AT_SYSTEM,
	CMD_SET_HOST_IP,
	CMD_SYS_RESET,
	/*	Add new list above	*/
	CMD_AT_COMMAND_MANUAL,
	CMD_AT_COMMAND_SCRIPT,
	CMD_HELP,
	CMD_EXIT,
	CMD_MAX,
}FUNCTION_INDEX, *PFUNCTION_INDEX;


/* All command list need to have same order with above function index. */
static struct cmd_t cmd_list[] = {
	{"ate", 	cmd_ate, 		CMD_ATE_REQUEST, 	"Echo on/off"},
	{"swv", 	cmd_at_swv, 	CMD_AT_SWV_REQUEST, "Return the SW version"},
	{"lteinit", cmd_at_lteinit, CMD_AT_LTE_INIT_REQUEST, "LTE device init"},
	{"pls", 	cmd_at_pls, 	CMD_AT_PLS_REQUEST, "Request network setup automatically"},
	{"at", 		cmd_at_attach, 	CMD_AT_ATTACH_REQUEST, "Attach request"},
	{"dt", 		cmd_at_detach, 	CMD_AT_DETACH_REQUEST, "Detach request"},
	{"ipinfo", 	cmd_at_ipinfo, 	CMD_AT_GET_IPINFO, "PDP context read dynamic parameters and setup VLAN for Host"},
	{"gsim", 	cmd_at_get_sim_status, CMD_AT_GET_SIM_STATUS_REQUEST, "Get SIM Status"},
	{"gimsi", 	cmd_at_get_imsi, CMD_AT_GET_IMSI, 	"Get IMSI"},
	{"system", 	cmd_at_system, 	CMD_AT_SYSTEM, 		"Send system call to device"},
	{"ipset", 	cmd_set_host_ip, 	CMD_SET_HOST_IP, 		"Set Host IP"},
	{"reset", 	cmd_at_sys_reset, 	CMD_SYS_RESET, 		"Reset GDM7243"},
	/*	Add new list above	*/
	{"atc", 	cmd_at_manual, CMD_AT_COMMAND_MANUAL, "Send AT command manually"},
	{"script", 	cmd_at_script, CMD_AT_COMMAND_SCRIPT, "Run script file including AT command list"},
	{"help", 	cmd_help, CMD_HELP, "Get help(\"help [command]\" for each command help)"},
	{"exit", 	NULL, CMD_EXIT, "CM terminate"},
	{NULL, 		NULL, CMD_MAX, ""},

};



static void cmd_help_detail(const char* cmd, u32 index)
{
	printf("\n%s: %s\n", cmd, cmd_list[index].description);	
	printf("-------Usage-------\n");	
	printf("   CM> %s ", cmd);	

	/* Description of options for the command. */
	switch (index)
	{
	case CMD_ATE_REQUEST:
		printf("[echo mode]\n"); // option 
		printf("\t0 : echo mode off\n"); // option detailed description.
		printf("\t1 : echo mode on\n");
		break;
	case CMD_AT_COMMAND_MANUAL:
		printf("[AT command]\n"); 
		printf("\tex) at AT%%SWV\n");
		break;
	case CMD_AT_COMMAND_SCRIPT:		
		printf("\n No option required.");
		printf("\n You can input file name when you meet the query \"Enter the script file name: \".\n");
		break;
	case CMD_AT_SYSTEM:
		printf("[command]\n"); 
		printf("\tex) system \"ifconfig\"\n");
		printf("\t    system \"ls -al\"\n");
		break;
	case CMD_SET_HOST_IP:
		printf("[ip] [gateway] [netmask] [primary dns] [secondary dns]\n"); 
		printf("\tex) 10.136.106.188 10.0.0.67 168.126.63.1 211.40.215.10\n");
		break;
	case CMD_AT_DETACH_REQUEST:
		printf("or dt [test seconds]\n");
		printf("\tex) dt 10 - this will send detach command every 10 seconds\n");
		printf("\t    dt 0 - this will stop sending detach command\n");
		break;
	case CMD_HELP:
		break;
	case CMD_EXIT:
		break;
	default:
		printf("\nNo more instruction for this command\n");
		break;
	}
	printf("\n");
}



static u32 	cmd_at_sys_reset(u32 argc, char *argv[])
{
	return (u32)send_at_command("AT%SYSRESET");	// PDP context read dynamic parameters 
}


static u32 	cmd_set_host_ip(u32 argc, char *argv[])
{
	char command[256];
	int rtn;
	if (argc < 5)
	{
		cmd_help_detail(cmd_list[CMD_SET_HOST_IP].cmd, CMD_SET_HOST_IP);
		return RTN_ERR;
	}
	
	memset(command, 0x00, 256);
	sprintf(command, "./vlan_setup.sh %s %s %s %s %s ", argv[1], argv[2],argv[3],argv[4],argv[5]);
	rtn = system(command);
	if (rtn < 0)
	{
		cm_print(LOG_LEVEL_ERROR, "system call error\n");
	}
	return RTN_OK;
	

}

static u32 	cmd_at_ipinfo(u32 argc, char *argv[])
{
	int rtn;
	char* cmd;
	cmd = (char*)malloc(AT_COMMAND_STRING_MAX);
	if (!cmd) { cm_print(LOG_LEVEL_ERROR, "malloc failed\n"); return RTN_ERR;}
	memset(cmd, 0x00, AT_COMMAND_STRING_MAX);
	sprintf(cmd, "AT+CGCONTRDP=%d",INTERNET_PDN_CID);
	rtn = send_at_command(cmd);
	free(cmd);
	return (u32)rtn;
}



void* test_detach_loop(void* arg)
{
	int test_sec = *(int*)arg;
	
	if (test_sec < 0)
	{
		printf("test_sec < 0\n");
		return NULL;
	}	

	printf("\ntest detach thread start, arg(%d)\n", test_sec);

	while (1)
	{
		/* send detach */	
		sleep((unsigned int)test_sec);
		send_at_command("AT+CGATT=0");	// Detach 0;
	}

	return NULL;
}


static u32  cmd_at_detach(u32 argc, char *argv[])
{

	static int test_sec;

	if (argc > 2) 
	{
		cmd_help_detail(cmd_list[CMD_AT_DETACH_REQUEST].cmd, CMD_AT_DETACH_REQUEST);
		return RTN_ERR;
	}

	if (argc == 1)
	{
		return (u32)send_at_command("AT+CGATT=0");	// Detach 0;
	} 

	if (argc == 2)
	{
		test_sec = atoi(argv[1]);
		if (test_sec == 0)
		{
			/* stop test thread */
			pthread_cancel(test_detach_thread);
			pthread_detach(test_detach_thread);
        	printf("\nTest thread terminated.\n");	
		} 
		else if (test_sec < 10)
		{
			/* too short */
        	printf("\nTest seconds should be more than 10 seconds.\n");	
			return RTN_ERR;
		}
		else if (test_sec >= 10)
		{
			/* run test thread */
			if(pthread_create(&test_detach_thread, NULL, test_detach_loop, &test_sec) < 0)
			{
				cm_print(LOG_LEVEL_ERROR, "Error: Failed to created test thread\n");
				return RTN_ERR;
			}
	
		}
	}
	return RTN_OK;
}

static u32  cmd_at_attach(u32 argc, char *argv[])
{
	int rtn;
	char* cmd;	
	cmd = (char*)malloc(AT_COMMAND_STRING_MAX);
	if (!cmd) { cm_print(LOG_LEVEL_ERROR, "malloc failed\n"); return RTN_ERR;}
	memset(cmd, 0x00, AT_COMMAND_STRING_MAX);
	sprintf(cmd, "AT+CGDCONT=%d,\"%s\",\"%s\"",INTERNET_PDN_CID, PDP_TYPE_IPV4V6, DEFAULT_INTERNET_APN_NAME);
	rtn = send_at_command(cmd);

	memset(cmd, 0x00, AT_COMMAND_STRING_MAX);
	sprintf(cmd, "AT%%GPDNTYPE=%d,%d", INTERNET_PDN_CID, INTERNET_PDN_TYPE);				
	rtn = send_at_command(cmd);

	memset(cmd, 0x00, AT_COMMAND_STRING_MAX);
	sprintf(cmd, "AT+CGACT=1,%d",INTERNET_PDN_CID);
	rtn = send_at_command(cmd);

	free(cmd);
	return (u32)rtn;
}

static u32  cmd_at_system(u32 argc, char *argv[])
{
	char* buffer;
	int cnt, arg_cnt;

	if (argc < 2)
	{
		cmd_help_detail(cmd_list[CMD_AT_SYSTEM].cmd, CMD_AT_SYSTEM);
		return RTN_ERR;
	}

	arg_cnt = (int)argc;

	buffer  = (char*)malloc(256);
	if (buffer == NULL)
	{ 
		cm_print(LOG_LEVEL_ERROR, "memory allocation failed\n");
		return RTN_ERR;
	}

	sprintf(buffer, "AT%%SYSCMD=%s", argv[1]);
	cnt = 2;
	while (arg_cnt > cnt)
	{
		sprintf(buffer, "%s %s", buffer, argv[cnt]);
		cnt++;
	}	
	
	//cm_print(LOG_LEVEL_ERROR, "cmd(%s)\n", buffer);
	send_at_command(buffer); 

	free(buffer);

	return RTN_ERR;
}

static u32  cmd_at_get_imsi(u32 argc, char *argv[])
{
	return (u32)send_at_command("AT+CIMI"); 
}

static u32  cmd_at_get_sim_status(u32 argc, char *argv[])
{
	return (u32)send_at_command("AT+CPIN?"); 
}

static u32  cmd_at_lteinit(u32 argc, char *argv[])
{

	/* GCT Device procotol statch initailize */
	send_at_command("AT%LTEINIT"); 

	/*  No auto-answer */
	send_at_command("ATS0=0");

	/*  Extended errors */
	send_at_command("AT+CMEE=1");

	/* Network registration : 1=enable network registration unsolicited result code */
	send_at_command("AT+CREG=1");

	/*  Enable +CGEV GPRS event notifications, but don't buffer */
	send_at_command("AT+CGEREP=1,0");

	send_at_command("AT+CFUN=1");	

	send_at_command("AT+CEREG=2");
	send_at_command("AT+CGEREP=1,1");
	
	return RTN_OK;
	
}

static u32  cmd_at_pls(u32 argc, char *argv[])
{	
	/*
	* +COPS=[<mode>[,<format>[,<oper>[,<AcT>]]]]
	* <mode>
	* 0 : automatic (<oper> field is ignored)
	* 1 : manual (<oper> field shall be present, and <AcT> optionally)
	* - <format>
	* 0 : long format alphanumeric <oper>
	* 1 : short format alphanumeric <oper>
	* 2 : numeric <oper>
	* - Set command forces an attempt to select
	* <AcT> access technology selected
	* 0 : GSM
	* 1: GSM compact
	* 2: UTRAN
	* 3: GSM w/EGPRS
	* 4: UTRAN w/HSDPA
	* 5: UTRAN w/HSUPA
	* 6: UTRAN w/HSDPA and HSUPA
	* 7: E-UTRAN
	*/
	
	return (u32)send_at_command("AT+COPS=0,,,7"); 
	 
}


static u32  cmd_at_script(u32 argc, char *argv[])
{
	char buf[AT_CMD_MAX_LEN];
	char* filename;
	char* ret;
	int len;
	FILE* sfp;
	
	printf("Enter the script file name: ");
	filename = fgets(buf, 256, stdin);
	if (!filename)
	{
		printf("Bad file name\n");
		return RTN_ERR;
	}

	len = (int)strlen(filename);
	if (filename[len-1] == '\n')
		filename[len-1] = '\0';
	
	sfp = fopen(filename, "r");
	if (!sfp)
	{
		printf("Failed to open script file [%s]\n", filename);
		return RTN_ERR;
	}

	while (1)
	{
		ret = fgets(buf, 1024, sfp);
		if (ret == NULL)
		{
			break;
		}
	
		len = (int)strlen(buf);
		
		if (strncasecmp(buf, "ATSLEEP", 7)==0)
		{
			printf("SLEEP 1 second ...\n");
			usleep(1000000);
		}
		else
		{
			printf(AT_PROMPT_SND "%s", buf);
			send_at_command(buf); 
		}
	}
		
	fclose(sfp);
	


	return RTN_OK;
}

static u32  cmd_at_swv(u32 argc, char *argv[])
{	
	return (u32)send_at_command("AT%SWV"); 
}

static u32  cmd_at_manual(u32 argc, char *argv[])
{
	char* buffer;

	if (argc < 2)
	{
		cmd_help_detail(cmd_list[CMD_AT_COMMAND_MANUAL].cmd, CMD_AT_COMMAND_MANUAL);
		return RTN_ERR;
	}
	
	buffer  = (char*)malloc(256);
	if (buffer == NULL)
	{ 
		cm_print(LOG_LEVEL_ERROR, "memory allocation failed\n");
		return RTN_ERR;
	}
	sprintf(buffer, "%s", argv[1]);
	send_at_command(buffer); 
	free(buffer);

	return RTN_OK;
}



static u32  cmd_ate(u32 argc, char *argv[])
{
	u32 echo_mode = 0;
	char* buffer;
	if (argc != 2)
	{
		cmd_help_detail(cmd_list[CMD_ATE_REQUEST].cmd, CMD_ATE_REQUEST);
		return RTN_ERR;
	}
	
	echo_mode = (u32)atoi(argv[1]);
	buffer  = (char*)malloc(256);
	if (buffer == NULL)
	{ 
		cm_print(LOG_LEVEL_ERROR, "memory allocation failed\n");
		return RTN_ERR;
	}
	sprintf(buffer, "ATE%d", echo_mode);
	send_at_command(buffer); 
	free(buffer);
	return RTN_OK;
}

static u32  cmd_help(u32 argc, char *argv[])
{
	struct cmd_t *list = cmd_list;
	if (argc == 2)
	{
		while(list->cmd)
		{
			if(!strcmp(list->cmd, argv[1]))
			{
				cmd_help_detail(list->cmd, list->func_index);	
				return RTN_OK;
			}
			list++;
		}
		printf("Command \"%s\" is not found in the command list\n", argv[1]);
		return RTN_OK;
	}

	/* Show all command list */
	printf("\n***** Command List *****\n");
	while(list->cmd)
	{
		if (list->func_index == CMD_MAX)
			break;

		
		printf("%s		%s\n", list->cmd, list->description);
		list++;
	}

	printf("\n");
	return RTN_OK;
}




static void get_token_arg(char *cmd_line, u32 *argc, char *argv[])
{
	u32 found_arg = 1, bind_on = 0;
	u32 argn = 0, i = 2;

	func_in();

	while (*cmd_line) 
	{
		if (*cmd_line == '\n') {
			*cmd_line = '\0';
			break;
		}
		if (0 == bind_on && (*cmd_line == ' ')) {
			found_arg = 1;
			*cmd_line = '\0';
		}
		else if (found_arg) {
			argv[argn++] = cmd_line;
			found_arg = 0;
		}

		cmd_line++;
	}

	*argc = argn;
	if(argn > 2){
		for(i=2;i<argn;i++){
			*(argv[i]-1) = ' ';
		}
	}

	func_out();
}


void do_cmd(char *cmd)
{
	struct cmd_t *list = cmd_list;
	u32 argc;
	char *argv[30];

	func_in();

	get_token_arg(cmd, &argc, argv);

	if(argc) 
	{
		while(list->cmd)
		{
			if(!strcmp(list->cmd, argv[0]))
			{
				list->func(argc, argv);
				break;
			}
			list++;
		}
		if(list->func == NULL)
			cm_print(LOG_LEVEL_ERROR, "[%s] is not a recognized command\n", argv[0]);
	}

	func_out();

}



