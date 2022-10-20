/**
 * @file huayi_cmd.c
 *
 * Created commands to change huayi config
 *
 * This tool is responsible to change the configuration 
 * of the huayi system.
 * 
 * =========================================================
 * Details about the command mode.
 * huayi_cmd <op>  <cmd> <interface><value>
 * @op: set or get
 * @cmd: suppoted cmd listed in cmd_help.txt
 * @interface: interface listed in cmd_help.txt, as sector/wifi. If empty, means system
 * @value: If op is get, option value will not be used. If op is set, the value's type
 * defined in cmd_help.txt(type), supported number of values defined in cmd_help.txt(num)
 *
 * Example:
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <ctype.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <huayi-universal/huayi_control.h>
#include "huayi_uci_fun.h"

typedef struct{
	int code;           /* Number of the ioctl to issue */
	int deep;          /* Number of interface layer */
	char type;           /* Type of value */
	char format;
	char intrface[2][10];       /* Keep interface */
	char ifname[40];  /* Name of the command */
	char section[20];   /* Name of the section */
	PKG_INDEX pkg_index;
}huayi_cmd_t;


#define TABLE_MAX_ENTRY  1000

static huayi_cmd_t huayi_control_cmds[TABLE_MAX_ENTRY];  /* a table of commands from huayi_control.conf */
static int cmds_tbl_set = 0;  /* indicate if the table had already set and the length of table*/

static void
usage(char *section)
{
	int i, k;
	
	printf("Usage: huayi_cmd <op> <section> <cmd_name> <interfaces> [paramter]\n");
	for(i = 0; i < cmds_tbl_set; i++){
		if (!strcmp (section, "\0")){
			printf("huayi_cmd get/set %s %s [", huayi_control_cmds[i].section, huayi_control_cmds[i].ifname);
			for (k=0; k<huayi_control_cmds[i].deep; k++)
				printf("%s ", huayi_control_cmds[i].intrface[k]);
			printf("] [parameter]\n");
		}
		else{
			if (!strcmp( huayi_control_cmds[i].section, section)){
				printf("huayi_cmd get/set %s %s [", huayi_control_cmds[i].section, huayi_control_cmds[i].ifname);
				for (k=0; k<huayi_control_cmds[i].deep; k++)
					printf("%s ", huayi_control_cmds[i].intrface[k]);
				printf("] [parameter]\n");
			}
			else
				continue;
		}
	}
}

static int IsCommand(void)
{
	FILE *fStream;
	char linebuf[1024], *pline;
	int i=0, j;
	char section[20];
	char intrface[2][10] = {"\0","\0"};
	char type[20];
	char format;

	if ((fStream = fopen("/etc/huayi/huayi_control.conf", "r")) == NULL ){
		printf("Failed to open huayi_control.conf\n");
		return 0;
	}
	while (fgets(linebuf, 1000, fStream) != NULL ){
		pline = linebuf;
		while(isspace(*pline)) pline++;
		if(*pline == '#' || strlen(pline) == 0){
			continue;
		}
		else if (strncmp("section", pline, strlen("section")) == 0){
			pline = strchr(pline, '=');
			pline++;
			if(!sscanf(pline, "%s", section)){
				printf("unable to read the section.\n");
				printf("fail to get command or the cmd_help.txt is empty.\n");
				fclose(fStream);
				return 0;
			}
			
		}
		else if (strncmp("cmd", pline, strlen("cmd")) == 0){
			pline = strchr(pline, '=');
			pline++;
			if(!sscanf(pline, "%s", huayi_control_cmds[i].ifname)){
				printf("unable to read the cmd.\n");
				printf("fail to get command or the cmd_help.txt is empty.\n");
				fclose(fStream);
				return 0;
			}
			if(!strcmp(section,"about"))
			{
				huayi_control_cmds[i].pkg_index = PKG_ABOUT;
			}else if(!strcmp(section,"system"))
			{
				huayi_control_cmds[i].pkg_index = PKG_STATUS_SYSTEM;
			}else if(!strcmp(section,"network"))
			{
				huayi_control_cmds[i].pkg_index = PKG_STATUS_NETWORK;
			}
			else if(!strcmp(section,"modem"))
			{
				huayi_control_cmds[i].pkg_index = PKG_STATUS_MODEM;
			}else if(!strcmp(section,"blend"))
			{
				huayi_control_cmds[i].pkg_index = PKG_STATUS_BLEND;
			}
			else
			{
				huayi_control_cmds[i].pkg_index = PKG_UNDEFINE;
				printf("unknown section:%s\n",section);
				return 0;
			}
		}
		else if (strncmp("interface", pline, strlen("interface")) == 0){
			pline = strchr(pline, '=');
			pline++;
			if (!strcmp(intrface[0],"\0")){
				if(!sscanf(pline, "%s", intrface[0])){
					printf("unable to read the interface.\n");
					printf("fail to get command or the cmd_help.txt is empty.\n");
					fclose(fStream);
					return 0;
				}
				huayi_control_cmds[i].deep = 1;
			}
			else if(!strcmp(intrface[1],"\0")){
				if(!sscanf(pline, "%s", intrface[1])){
					printf("unable to read the interface.\n");
					printf("fail to get command or the cmd_help.txt is empty.\n");
					fclose(fStream);
					return 0;
				}
				huayi_control_cmds[i].deep = 2;
			}
			else
				printf("too much interface, ignore.\n");
		}
		else if (strncmp("type", pline, strlen("type")) == 0){
			pline = strchr(pline, '=');
			pline++;
			if(!sscanf(pline, "%s", type)){
				printf("unable to read the type.\n");
				printf("fail to get command parameter type.\n");
				fclose(fStream);
				return 0;
			}
			else{
				if (!strcmp(type, "char"))
					huayi_control_cmds[i].type = TYPE_CHAR;
				else if (!strcmp(type, "stream"))
					huayi_control_cmds[i].type = TYPE_STREAM;
				else if (!strcmp(type, "int"))
					huayi_control_cmds[i].type = TYPE_INT;
				else if (!strcmp(type, "short"))
					huayi_control_cmds[i].type = TYPE_SHORT;
				else{
					huayi_control_cmds[i].type = TYPE_CHAR;
					printf("type %s is not supported, suppose is char.\n", type);
				}
			}
		}
		else if (strncmp("format", pline, strlen("format")) == 0){
			pline = strchr(pline, '=');
			pline++;
			if(!sscanf(pline, "%c", &format)){
				printf("unable to read the format.\n");
				printf("fail to get command parameter format.\n");
				fclose(fStream);
				return 0;
			}
			else{
				if (format == 'd')
					huayi_control_cmds[i].format = FORMAT_D;
				else if (format == 'x')
					huayi_control_cmds[i].format = FORMAT_X;
				else if (format == 's')
					huayi_control_cmds[i].format = FORMAT_S;
				else{
					printf("format %c is not supported.\n", format);
					return 0;
				}
			}
		}
		else if (strncmp("endcmd", pline, strlen("endcmd")) == 0){
			huayi_control_cmds[i].code = i;
			strcpy(huayi_control_cmds[i].section, section);
			for (j =0; j < huayi_control_cmds[i].deep; j++){
				strcpy(huayi_control_cmds[i].intrface[j], intrface[j]);
				intrface[j][0] = '\0';
			}
			i++;
			huayi_control_cmds[i].deep = 0;
			huayi_control_cmds[i].intrface[0][0] = '\0';
			huayi_control_cmds[i].intrface[1][0] = '\0';
		}
		else if (strncmp("endsection", pline, strlen("endsection")) == 0){
			section[0] = '\0';
		}
		else{
			printf("%s unknown symbol!\n", linebuf);
		}
		if (i == TABLE_MAX_ENTRY){
			printf("the command table is full.\n");
			break;
		}
	}
	fclose(fStream);
	cmds_tbl_set = i;
	return 1;
}

int main(int argc, char *argv[])
{
	huayi_control_t cmd;
	int fd;
	int found = 0;
	int i, k;
	char intrface[2][10];
	char format;
	char val_c;
	int val_i;
	short val_h;
	char* cp;
	int huayi_cmd_debug = 0;
	PKG_INDEX uci_title = PKG_UNDEFINE;
	int lockfd = 0;
	int lockret = 0;

	lockfd = open("/var/run/huayi_cmd.lock",O_RDWR);
	if(lockfd<0)
	{
		printf("open huayi_cmd.lock fail!\n");
		return 1;
	}

	lockret = flock(lockfd,LOCK_EX);
	if(lockret<0)
	{
		close(lockfd);
		printf("flock huayi_cmd.lock fail!\n");
		return 1;
	}
	
	fd = open("/dev/huayi_control",O_RDWR);
	if(fd<0){
		flock(lockfd,LOCK_UN);
		close(lockfd);
		printf("open huayi_control fail!\n");
		return 1;
	}

	huayi_uci_get_int(PKG_SYSTEM,"global","huayi_cmd_debug",&huayi_cmd_debug);
	
	if (cmds_tbl_set == 0 && !IsCommand()){
		printf("Fail to set up the command table!\n");
		close(fd);
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return 2;
	}

	if (argc < 3){
		usage("\0");
		close(fd);
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return 0;
	}
	if (argc < 4){
		usage(argv[2]);
		close(fd);
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return 0;
	}
	
	strcpy(cmd.op, argv[1]);
	strcpy (cmd.section, argv[2]);
	strcpy(cmd.ifname, argv[3]);
	argc = argc - 4;
	argv = argv + 4;
	
	for(i=0; i<cmds_tbl_set; i++){
		if (!strncmp(cmd.section, huayi_control_cmds[i].section, strlen(cmd.section))){
			if (!strncmp(cmd.ifname, huayi_control_cmds[i].ifname, strlen(cmd.ifname))){
				cmd.type = huayi_control_cmds[i].type;
				format = huayi_control_cmds[i].format;
				for (k = 0; k < huayi_control_cmds[i].deep; k++){
					strcpy(intrface[k], huayi_control_cmds[i].intrface[k]);
				}
				uci_title = huayi_control_cmds[i].pkg_index;
				found = 1;
				break;
			}
		}
		else
			continue;
	}
	if (!found){
		printf("unknown command or mismatched section and command, please check huayi_control.conf!\n");
		close(fd);
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return 3;
	}
		
	if (huayi_control_cmds[i].deep > 0){
		if (argc < huayi_control_cmds[i].deep){
			printf("missing the inteface, it should be %s %s!\n",intrface[0], intrface[1]);
			close(fd);
			flock(lockfd,LOCK_UN);
			close(lockfd);
			return 4;
		}
		else{
            if (strncmp(intrface[0], argv[0], strlen(intrface[0])) != 0) {
                printf("unknown interface or missing the first inteface, it should be %s!\n", intrface[0]);
                close(fd);
				flock(lockfd,LOCK_UN);
				close(lockfd);
                return 4;
            }
            cp = argv[0] + strlen(intrface[0]);
            if (*cp < '0' || *cp > '9') {
                printf("unknown interface or missing the first inteface, it should be %s!\n", intrface[0]);
                close(fd);
				flock(lockfd,LOCK_UN);
				close(lockfd);
                return 4;
            }
            strcpy(cmd.intrface[0], argv[0]);
            if (huayi_control_cmds[i].deep == 2){
                if (strncmp(intrface[1], argv[1], strlen(intrface[1])) != 0){
                    printf("unknown interface or missing the second inteface, it should be %s!\n", intrface[1]);
                    close(fd);
					flock(lockfd,LOCK_UN);
					close(lockfd);
                    return 4;
                }
                cp = argv[1] + strlen(intrface[1]);
                if (*cp < '0' || *cp > '9') {
                    printf("unknown interface or missing the second inteface, it should be %s!\n", intrface[1]);
                    close(fd);
					flock(lockfd,LOCK_UN);
					close(lockfd);
                    return 4;
                }
                strcpy(cmd.intrface[1], argv[1]);
            }
            argc -= huayi_control_cmds[i].deep;
            argv += huayi_control_cmds[i].deep;
		}
	}
	else{
		for (k = 0; k < 2; k++){
			cmd.intrface[k][0] = '\0';
		}
	}
	
	if (!strcmp(cmd.op, "get") || argc == 0){
		if (ioctl(fd, HUAYI_CTL_IOC_R, &cmd) < 0) {
			err(1, "huayi_cmd");
			close(fd);
			flock(lockfd,LOCK_UN);
			close(lockfd);
			return 5;
		}
		/*printf("HUAYI CTL get: %s %s ", cmd.section, cmd.ifname);
		for (k=0; k<2; k++){
			printf("%s ", cmd.intrface[k]);
		}*/
		switch (cmd.type){
			case TYPE_CHAR:
				if (format & FORMAT_D)
					printf("%d",cmd.buf.buf_c[1]);
				else if (format & FORMAT_X)
					printf("%x",cmd.buf.buf_c[0]);
				else
					printf("error format.");
				break;
			case TYPE_STREAM:
				printf("%s",cmd.buf.buf_s);
				break;
			case TYPE_INT:
				if (format & FORMAT_D)
					printf("%d",cmd.buf.buf_i[1]);
				else if (format & FORMAT_X)
					printf("%x",cmd.buf.buf_i[0]);
				else
					printf("error format.");
				break;
			case TYPE_SHORT:
				if (format & FORMAT_D)
					printf("%d",cmd.buf.buf_h[1]);
				else if (format & FORMAT_X)
					printf("%x",cmd.buf.buf_h[0]);
				else
					printf("error format.");
				break;
			default:
				break;
		}
		printf("\n");
	}
	else{
		if (argc != 1){
			printf("only support one parameter!\n");
			close(fd);
			flock(lockfd,LOCK_UN);
			close(lockfd);
			return 6;
		}	
		for (; argc > 0; argc--, argv++) {
			switch (cmd.type){
			case TYPE_CHAR:
				val_c = (char)strtoul(argv[0], NULL, 16);
				cmd.buf.buf_c[0] = val_c;
				val_c = (char)strtoul(argv[0], NULL, 10);
				cmd.buf.buf_c[1] = val_c;
				break;
			case TYPE_STREAM:
				strcpy (cmd.buf.buf_s, argv[0]);
				break;
			case TYPE_INT:
				val_i = (int)strtoul(argv[0], NULL, 16);
				cmd.buf.buf_i[0] = val_i;
				val_i = (int)strtoul(argv[0], NULL, 10);
				cmd.buf.buf_i[1] = val_i;
				break;
			case TYPE_SHORT:
				val_h = (short)strtoul(argv[0], NULL, 16);
				cmd.buf.buf_h[0] = val_h;
				val_h = (short)strtoul(argv[0], NULL, 10);
				cmd.buf.buf_h[1] = val_h;
				break;
			default:
				break;
			}
		}
		if (ioctl(fd, HUAYI_CTL_IOC_W, &cmd) < 0) {
			err(1, "huayi_cmd");
			close(fd);
			flock(lockfd,LOCK_UN);
			close(lockfd);
			return 7;
		}

		huayi_uci_clean_uci_context_cache(uci_title,false);
		
		if(huayi_cmd_debug)
		{
			printf("HUAYI CONTROL set: %s %s ", cmd.section, cmd.ifname);
			
			for (k=0; k<2; k++){
				printf("%s ", cmd.intrface[k]);
			}
		}
		switch (cmd.type){
			case TYPE_CHAR:
				if (format & FORMAT_D)
				{
					if(huayi_cmd_debug)
					printf("-> %d",cmd.buf.buf_c[1]);
				}
				else if (format & FORMAT_X)
				{
					if(huayi_cmd_debug)
					printf("-> %x",cmd.buf.buf_c[0]);
				}
				else
					printf("error format.");
				break;
			case TYPE_STREAM:
				if(huayi_cmd_debug)
				{
					printf("-> %s",cmd.buf.buf_s);
				}
				break;
			case TYPE_INT:
				if (format & FORMAT_D)
				{
					if(huayi_cmd_debug)
					printf("-> %d",cmd.buf.buf_i[1]);
				}
				else if (format & FORMAT_X)
				{	
					if(huayi_cmd_debug)
					printf("-> %x",cmd.buf.buf_i[0]);
				}
				else
					printf("error format.");
				break;
			case TYPE_SHORT:
				if (format & FORMAT_D)
				{	
					if(huayi_cmd_debug)
					printf("-> %d",cmd.buf.buf_h[1]);
				}
				else if (format & FORMAT_X)
				{
					if(huayi_cmd_debug)
					printf("-> %x",cmd.buf.buf_h[0]);
				}
				else
					printf("error format.");
				break;
			default:
				break;
			}
		if(huayi_cmd_debug)
		{
			printf("\n");
		}
	}
	close(fd);
	flock(lockfd,LOCK_UN);
	close(lockfd);
	return 0;
}
