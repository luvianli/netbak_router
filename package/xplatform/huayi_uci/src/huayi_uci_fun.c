#include <huayi_uci_fun.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <error.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <pthread.h>
#include <limits.h>
#include "huayi_atcmd.h"





//Combined get str

HUAYI_BOOL huayi_uci_get_str(int title,char *section,char *option,char *value)
{
	char cli_str[CMD_STR_LEN];
	struct huayi_uci_get_context* uci_lib = NULL;

	if((title <= PKG_UNDEFINE)||(title >= PKG_NUM_OF_PKG)||(section == NULL)||(option == NULL)||(value == NULL))
	{
		return HUAYI_FALSE;
	}

	memset(cli_str, 0, CMD_STR_LEN);

	snprintf(cli_str,CMD_STR_LEN,"%s.%s.%s",PKG_ID_TOFILE(title),section,option);

	uci_lib = huayi_uci_get_uci_context(title, NULL);
	
	if(uci_lib)
		return huayi_uci_get_option(uci_lib,cli_str,value);
	else
		return HUAYI_FALSE;
}

HUAYI_BOOL huayi_uci_get_int(int title,char *section,char *option,int *value)
{
	char tmp_result[RESULT_STR_LEN];

	memset(tmp_result, 0, RESULT_STR_LEN);
	
	if(!huayi_uci_get_str(title, section, option, tmp_result))
	{
		return HUAYI_FALSE;
	}

	*value = atoi(tmp_result);
	
	return HUAYI_TRUE;
}

HUAYI_BOOL huayi_uci_get_hex(int title,char *section,char *option,int *value)
{
	char tmp_result[RESULT_STR_LEN];

	memset(tmp_result, 0, RESULT_STR_LEN);
	
	if(!huayi_uci_get_str(title, section, option, tmp_result))
	{
		return HUAYI_FALSE;
	}

	*value = strtol(tmp_result, NULL, 16);
	
	return HUAYI_TRUE;
}

HUAYI_BOOL huayi_uci_get_long(int title,char *section,char *option,unsigned long *value)
{
	char tmp_result[OPTION_STR_LEN];

	memset(tmp_result, 0, OPTION_STR_LEN);
	
	if(!huayi_uci_get_str(title, section, option, tmp_result))
	{
		return HUAYI_FALSE;
	}

	*value = strtoul(tmp_result,0,10);
	
	return HUAYI_TRUE;
}

HUAYI_BOOL huayi_uci_get_long_long(int title,char *section,char *option,unsigned long long *value)
{
	char tmp_result[TEMP_STR_LEN];

	memset(tmp_result, 0, TEMP_STR_LEN);
	
	if(!huayi_uci_get_str(title, section, option, tmp_result))
	{
		return HUAYI_FALSE;
	}

	*value = strtoull(tmp_result,0,10);
	
	return HUAYI_TRUE;
}

HUAYI_BOOL huayi_uci_set_str(int title,char *section,char *option,char *value)
{
	int ret = 0;
	struct uci_context *uci_ctx;
	struct uci_ptr ptr;
	char cli_str[CMD_STR_LEN] = {0};
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM)||(section == NULL)||(option == NULL)||(value == NULL))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}
	
	snprintf(cli_str,sizeof(cli_str),"%s.%s.%s",PKG_ID_TOFILE(title),section,option);
	
	uci_lookup_ptr(uci_ctx,&ptr,cli_str,true);
	
	ptr.value = value;
	
	ret = uci_set(uci_ctx, &ptr);
	
	if (ret == UCI_OK)
		uci_save(uci_ctx, ptr.p);
	
	uci_free_context(uci_ctx);
	
	return HUAYI_TRUE;
}



HUAYI_BOOL huayi_uci_set_int(int title,char *section,char *type,int value)
{
	char tmp_result[RESULT_STR_LEN];

	memset(tmp_result, 0, RESULT_STR_LEN);

	snprintf(tmp_result,sizeof(tmp_result),"%d",value);

	return huayi_uci_set_str(title, section, type, tmp_result);
}

HUAYI_BOOL huayi_uci_set_hex(int title,char *section,char *type,int value)
{
	char tmp_result[RESULT_STR_LEN];

	memset(tmp_result, 0, RESULT_STR_LEN);

	snprintf(tmp_result,sizeof(tmp_result),"%0x",value);

	return huayi_uci_set_str(title, section, type, tmp_result);
}

HUAYI_BOOL huayi_uci_set_long(int title,char *section,char *option,long value)
{
	char tmp_result[OPTION_STR_LEN];

	memset(tmp_result, 0, OPTION_STR_LEN);

	snprintf(tmp_result,sizeof(tmp_result),"%ld",value);

	return huayi_uci_set_str(title, section, option, tmp_result);
}


HUAYI_BOOL huayi_uci_set_long_long(int title,char *section,char *option,unsigned long long value)
{
	char tmp_result[TEMP_STR_LEN];

	memset(tmp_result, 0, TEMP_STR_LEN);

	snprintf(tmp_result,sizeof(tmp_result),"%llu",value);

	return huayi_uci_set_str(title, section, option, tmp_result);
}

HUAYI_BOOL huayi_uci_delete_option(int title,char *section, char *option)
{
	int ret = 0;
	struct uci_context *uci_ctx;
	struct uci_ptr ptr;
	char cli_str[CMD_STR_LEN] = {0};
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM)||(section == NULL)||(option == NULL))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}
	
	snprintf(cli_str,sizeof(cli_str),"%s.%s.%s",PKG_ID_TOFILE(title),section,option);
	
	uci_lookup_ptr(uci_ctx,&ptr,cli_str,true);
	
	ret = uci_delete(uci_ctx, &ptr);
	
	if (ret == UCI_OK)
		ret = uci_save(uci_ctx, ptr.p);
	
	uci_free_context (uci_ctx);
	
	return HUAYI_TRUE;
}




HUAYI_BOOL huayi_uci_add_list(int title,char *section, char *option,char *value)
{
	int ret = 0;
	struct uci_context *uci_ctx;
	struct uci_ptr ptr;
	char cli_str[CMD_STR_LEN] = {0};
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM)||(section == NULL)||(option == NULL)||(value == NULL))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}
	
	snprintf(cli_str,sizeof(cli_str),"%s.%s.%s",PKG_ID_TOFILE(title),section,option);
	
	uci_lookup_ptr(uci_ctx,&ptr,cli_str,true);

	ptr.value = value;

	ret = uci_add_list(uci_ctx, &ptr);

	if (ret == UCI_OK)
		ret = uci_save(uci_ctx, ptr.p);

	uci_free_context (uci_ctx);
	
	return HUAYI_TRUE;
}


HUAYI_BOOL huayi_uci_del_list(int title,char *section, char *option,char *value)
{
	int ret = 0;
	struct uci_context *uci_ctx;
	struct uci_ptr ptr;
	char cli_str[CMD_STR_LEN] = {0};
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM)||(section == NULL)||(option == NULL)||(value == NULL))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}
	
	snprintf(cli_str,sizeof(cli_str),"%s.%s.%s",PKG_ID_TOFILE(title),section,option);
	
	uci_lookup_ptr(uci_ctx,&ptr,cli_str,true);
	
	ptr.value = value;
	
	ret = uci_del_list(uci_ctx, &ptr);
	
	if (ret == UCI_OK)
		ret = uci_save(uci_ctx, ptr.p);
	
	uci_free_context (uci_ctx);
	
	return HUAYI_TRUE;
}


HUAYI_BOOL huayi_uci_add_section(int title,char *section_type)
{
	int ret = 0;
	struct uci_context *uci_ctx;
	struct uci_package *p;
	struct uci_section *s = NULL;
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM)||(section_type == NULL))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}

	if(uci_load(uci_ctx, PKG_ID_TOFILE(title),&p) != UCI_OK)
	{
		return HUAYI_FALSE;
	}

	ret = uci_add_section(uci_ctx,p,section_type,&s);

	if (ret == UCI_OK)
		ret = uci_save(uci_ctx, p);

	uci_free_context (uci_ctx);
	
	return HUAYI_TRUE;
}

HUAYI_BOOL huayi_uci_add_name_section(int title,char *section, char *value)
{
	int ret = 0;
	struct uci_context *uci_ctx;
	struct uci_ptr ptr;
	char cli_str[CMD_STR_LEN] = {0};
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM)||(section == NULL)||(value == NULL))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}
	
	snprintf(cli_str,sizeof(cli_str),"%s.%s=%s",PKG_ID_TOFILE(title),value,section);
	
	uci_lookup_ptr(uci_ctx,&ptr,cli_str,true);
	
	ret = uci_set(uci_ctx, &ptr);
	
	if (ret == UCI_OK)
		ret = uci_save(uci_ctx, ptr.p);
	
	uci_free_context (uci_ctx);
	
	return HUAYI_TRUE;
}



HUAYI_BOOL huayi_uci_del_section(int title,char *section_name)
{
	int ret = 0;
	struct uci_context *uci_ctx;
	struct uci_ptr ptr;
	char cli_str[CMD_STR_LEN] = {0};
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}
	
	snprintf(cli_str,sizeof(cli_str),"%s.%s",PKG_ID_TOFILE(title),section_name);
	
	uci_lookup_ptr(uci_ctx,&ptr,cli_str,true);

	ret = uci_delete(uci_ctx,&ptr);

	if (ret == UCI_OK)
		ret = uci_save(uci_ctx, ptr.p);

	uci_free_context (uci_ctx);
	
	return HUAYI_TRUE;
}


HUAYI_BOOL huayi_uci_commit(int title)
{
	struct uci_context *uci_ctx;
	struct uci_ptr ptr;
	char cli_str[CMD_STR_LEN] = {0};
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}
	
	snprintf(cli_str,sizeof(cli_str),"%s",PKG_ID_TOFILE(title));
	
	uci_lookup_ptr(uci_ctx,&ptr,cli_str,true);
	
	uci_commit(uci_ctx,&ptr.p,false);
	
	if (ptr.p)
	{
		uci_unload(uci_ctx,NULL);
	}
	
	uci_free_context(uci_ctx);
	
	huayi_uci_clean_uci_context_cache(title,false);
	
	return HUAYI_TRUE;
}


HUAYI_BOOL huayi_uci_revert(int title)
{
	struct uci_context *uci_ctx;
	struct uci_ptr ptr;
	char cli_str[CMD_STR_LEN] = {0};
	
	if((title>=PKG_NUM_OF_PKG)||(title<PKG_SYSTEM))
	{
		return HUAYI_FALSE;
	}
	
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		
		return HUAYI_FALSE;
	}
	
	snprintf(cli_str,sizeof(cli_str),"%s",PKG_ID_TOFILE(title));
	
	uci_lookup_ptr(uci_ctx,&ptr,cli_str,true);
	
	uci_revert(uci_ctx, &ptr);
	
	uci_free_context (uci_ctx);
	
	return HUAYI_TRUE;
}


void get_ac_ip_by_hostname(char *achostname,char *acip)
{
	struct hostent *h;
	if((achostname == NULL)||(acip ==NULL))
	{
	    HUAYI_UCI_LOG("get_ac_ip_by_hostname:host name is null\n");
	    return;
	}
	if((h=gethostbyname(achostname))==NULL)
 	{
	    HUAYI_UCI_LOG("get_ac_ip_by_hostname:can not get IP by ac name!!!!\n");
	    strcpy(acip,"0.0.0.0");
	} else
	{
	    strcpy(acip,inet_ntoa(*((struct in_addr *)h->h_addr)));
	    HUAYI_UCI_LOG("get_ac_ip_by_hostname:AC HostName :%s,AC IP Address :%s\n",h->h_name,acip);
	}

	return;

}

void MAC17_TO_MAC6(char * MAC_17,char * MAC_6)
{

	int i = 0;
	memset(MAC_6,0,6);	
	for(i = 0; i < 6; i++)
	{		
		MAC_6[i] = strtoul((unsigned char *)(&(MAC_17[i*3])), 0, 16);
	}	
	return;

}

void MAC6_TO_MAC17(char * MAC_6,char * MAC_17)
{
	snprintf(MAC_17, 18, "%02x:%02x:%02x:%02x:%02x:%02x", MAC_6[0], MAC_6[1], \
									MAC_6[2], MAC_6[3], MAC_6[4], MAC_6[5]);

	return;
}


HUAYI_BOOL get_cmd_result(char *cmd, char *resultbuf, size_t buf_size)
{
	char *pchar = NULL;
	
	FILE *fp = popen(cmd, "r");

	if(!fp) 
	{
		return HUAYI_FALSE;
	}
	
	fgets(resultbuf, buf_size, fp);

	pclose(fp);
	
	if((pchar = strstr(resultbuf, "\n")))
		*pchar = '\0';

	resultbuf[buf_size-1] = '\0';

	return HUAYI_TRUE ;
}


HUAYI_BOOL huayi_file_exists(const char *path)	// note: anything but a directory
{
	struct stat st;
	
	if((stat(path, &st) == 0) && (!S_ISDIR(st.st_mode)))
		return HUAYI_TRUE;
	else
		return HUAYI_FALSE;
}

HUAYI_BOOL huayi_dir_exists(const char *path)	//  directory only
{
	struct stat st;
	if((stat(path, &st) == 0) && (S_ISDIR(st.st_mode)))
		return HUAYI_TRUE;
	else
		return HUAYI_FALSE;
}

unsigned long huayi_file_size(const char *path)	// 4GB-1	-1 = error
{
	struct stat st;

	if (stat(path, &st) == 0) 
		return st.st_size;
	
	return (unsigned long)-1;
}

HUAYI_BOOL huayi_json_get_int(cJSON *root, char *key, int *value)
{
	cJSON *sp;
	
	if ((sp = cJSON_GetObjectItem(root, key)) != NULL)
	{
		if (sp->valuestring) 
		{
			*value=atoi(sp->valuestring);
			return HUAYI_TRUE;
		}
		else if (sp->type==cJSON_False)
		{
			*value= cJSON_False;
			return HUAYI_TRUE;
		}
		else if (sp->type==cJSON_True)
		{
			*value= cJSON_True;
			return HUAYI_TRUE;
		}
		else if (sp->type==cJSON_Number)
		{
			*value=(sp->valueint);
			return HUAYI_TRUE;
		}
		else
		{
			return HUAYI_FALSE;
		}	
	}

	return HUAYI_FALSE;
}


HUAYI_BOOL huayi_json_get_str(cJSON *root, char *key,char *value,unsigned int len)
{
	cJSON	*sp;

	if((!value)||(!len))
	{
		return HUAYI_FALSE;
	}
	
	if ((sp = cJSON_GetObjectItem(root, key)) != NULL) 
	{
		if (sp->valuestring)
		{
			if(len < strlen(sp->valuestring))
			{
				return HUAYI_FALSE;
			}
				
			snprintf(value,len,"%s",sp->valuestring);
			return HUAYI_TRUE;
		}
		else if (sp->type==cJSON_False)
		{
			snprintf(value,len,"0");
			return HUAYI_TRUE;
		}
		else if (sp->type==cJSON_True)
		{
			snprintf(value,len,"1");
			return HUAYI_TRUE;
		}
		else if (sp->type==cJSON_Number)
		{
			snprintf(value,len,"%d",sp->valueint);
			return HUAYI_TRUE;
		}
		else
		{
			return HUAYI_FALSE;
		}	
	}
	
	return HUAYI_FALSE;
}


HUAYI_BOOL huayi_get_nth_value(int index, char *value, char delimit, char *result, int len)
{
    int i=0, result_len=0;
    char *begin, *end;

    if(!value || !result || !len)
    {
        return HUAYI_FALSE;
    }

    begin = value;
	
    end = strchr(begin, delimit);

    while(i<index && end)
	{
        begin = end+1;
		
        end = strchr(begin, delimit);

		i++;
    }

    if(!end)
	{
		if(i == index)
		{
			end = begin + strlen(begin);
			result_len = (len-1) < (end-begin) ? (len-1) : (end-begin);
		}else
		{
			return HUAYI_FALSE;
		}
			
	}else
	{
		result_len = (len-1) < (end-begin)? (len-1) : (end-begin);
	}
		

	memcpy(result, begin, result_len );
	
	*(result+ result_len ) = '\0';
	
	return HUAYI_TRUE;
}


int is_phyconnected(int portNum)
{
	FILE *fp=NULL;
	int portLink=0;
	char *p=NULL;
	char valBuf[TEMP_STR_LEN]={0}, tmpBuf[TEMP_STR_LEN]={0};

	sprintf(tmpBuf, "mii_mgr -g -p %d -r 1", portNum);	
	
	fp = popen (tmpBuf, "r" );
	if (!fp) 
		return 0;
	
	if(fgets(valBuf, 64, fp)) 
	{
		p = strstr(valBuf, "49");
		
		if(p != NULL) 
			portLink=0;
		else 
			portLink=1;
	}
	
	pclose(fp);
	
	return portLink;
}


HUAYI_BOOL huayiHanldeEscapeCharacter(char *value, char delimit, char *result)
{
	int result_len = 0;
    char *begin = NULL, *end = NULL, *tmpret = NULL;

	if(!value || !result)
    {
        return HUAYI_FALSE;
    }

	begin = value;
	end = strchr(begin, delimit);
	tmpret = result;

	while(end)
	{
		result_len = end - begin;
		memcpy(tmpret, begin, result_len);
		tmpret += result_len;
		memcpy(tmpret,"''", 2);
		begin = end + 1;
		tmpret += 2;
		end = strchr(begin, delimit);
	}

	memcpy(tmpret, begin, strlen(begin));
	*(tmpret + strlen(begin) + 1) = '\0';
	
	return HUAYI_TRUE;
}


char* strrstr(const char *haystack, const char *needle)
{  	
	char *r = NULL;  
	if (!needle[0])	
		return (char*)haystack + strlen(haystack);  
	while (1)  
	{    
		char *p = strstr(haystack, needle);    
		if (!p)     
			return r;    
		r = p;   
		haystack = p + 1;  
	}
}

int internet_do_ping_detect(const char *host, char *ifname)
{
	char cmd[CMD_STR_LEN] = {0};
	int i = 0;

	if ((NULL == host) || (strlen(host)<7))
		goto fail;

	for(i = 0;i < 3; i++)
	{
		if (ifname == NULL)
		{
			snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s > /dev/null && touch %s", 1, host, PING_TMPFILE);
		}
		else
		{
			snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s -I %s > /dev/null && touch %s", 1, host, ifname, PING_TMPFILE);
		}
		system(cmd);
		usleep(500);
		if ( access(PING_TMPFILE, F_OK) == 0)
		{
			unlink(PING_TMPFILE);
			return 1;
		}
	}
fail:
	return 0;
}

int first_internet_do_ping(const char *host, char *ifname) {
    char cmd[CMD_STR_LEN];
    char wan_gateway[IPV4_STR_LEN];
    char resultbuf[SHORT_STR_LEN];
    int i,count = 3,cpe_count = 0;
    char strifname[OPTION_STR_LEN];
    float delay =  0.0;

    if (host == NULL || strlen(host) < 7) {
        goto fail;
    }

	if (strcmp(ifname, "eth0.2") == 0) 
	{
		huayi_uci_get_str(PKG_STATUS_BLEND, "wired", "gateway", wan_gateway);
	}else
		huayi_uci_get_str(PKG_STATUS_BLEND, "cpe", "gateway", wan_gateway);		

    memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "route -n | grep %s | grep UG | grep %s | wc -l", ifname, wan_gateway);
	get_cmd_result(cmd, resultbuf, sizeof(resultbuf));

	if (atoi(resultbuf) == 0 && strncmp(wan_gateway, "0.0.0.0", strlen("0.0.0.0"))) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "route add default gw %s dev %s", wan_gateway, ifname);
		system(cmd);
	}
	
	for(i = 0;i < 3; i++)
	{
		if (ifname == NULL)
		{
			snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s > /dev/null && touch %s", 1, host, PING_TMPFILE);
		}
		else
		{
			snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s -I %s > /dev/null && touch %s", 1, host, ifname, PING_TMPFILE);
		}
		system(cmd);
		usleep(500);
		if ( access(PING_TMPFILE, F_OK) == 0)
		{
			unlink(PING_TMPFILE);
			return 1;
		}
	}

fail: 
    return 0;
}

int simcard_internet_check(const char *host, char *ifname)
{
	char cmd[CMD_STR_LEN] = {0};
	static int internet_flag = 0;

	if ((NULL == host) || (strlen(host)<7))
		goto fail;

retry:
	if (ifname == NULL)
	{
		snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s > /dev/null && touch %s", 1, host, PING_TMPFILE);
	}
	else
	{
		snprintf(cmd, sizeof(cmd), "ping -c 1 -W %d %s -I %s > /dev/null && touch %s", 1, host, ifname, PING_TMPFILE);
	}
	system(cmd);

	if ( access(PING_TMPFILE, F_OK) == 0)
	{
		internet_flag = 0;
		unlink(PING_TMPFILE);
		return 1;
	}
	else
	{
		if(3 > internet_flag) 
		{
			internet_flag += 1;
			sleep(2);    //total 6 sec
			goto retry;
		}
		else
		{
			internet_flag = 0;
		}
	}
	
fail:
	return 0;
}

long get_current_uptime_sec(void)
{   
	struct sysinfo info;    
	sysinfo(&info);    
	return info.uptime;
}

int HuayiSystem(char *command, int printFlag)
{
	int pid = 0, status = 0;
	long start = 0, current = 0;	
	pid_t  pid_ret=0;

	if (!command)
	{
		printf("HuayiSystem: Null Command, Error!");
		return -1;
	}

	pid = fork();
	if (pid == -1)
	{
		return -1;
	}

	if (pid == 0)
	{
		char *argv[4];
		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = 0;
		
		if (printFlag)
		{
			printf("[system]: %s\r\n", command);
		}
		
		execv("/bin/sh", argv);
		exit(127);
	}

	start = get_current_uptime_sec();
	/* wait for child process return */
	do
	{
		pid_ret = waitpid(pid, &status, WNOHANG);
		if (pid_ret < 0)
		{
			if (errno != EINTR)
			{
				return -1;
			}
		}
		else if (pid_ret == 0)
		{
			;// do nothing!
		}
		else
		{
			return status;
		}
		
		current = get_current_uptime_sec();
		
		if (SYSTEM_TIME_OUT <= (current - start))
		{
			break;
		}
	} while (1);

	return status;
}

void flush_arp_list(void)
{
	system("ip neigh flush dev br-lan");
	
	return;
}

char* str2upper(char *str)
{
	char *p = str;
	while (*str)
	{
		*str = toupper(*str);
		str++;
	}
	return p;
}

char* str2lower(char *str)
{
	char *p = str;
	while (*str)
	{
		*str = tolower(*str);
		str++;
	}
	return p;
}

char _dig_vec_upper[] =  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

unsigned long __attribute__((stdcall)) huayi_hex_string(char *to, const char *from, unsigned long length)
{
  char *to0= to;
  const char *end;
            
  for (end= from + length; from < end; from++)
  {
    *to++= _dig_vec_upper[((unsigned char) *from) >> 4];
    *to++= _dig_vec_upper[((unsigned char) *from) & 0x0F];
  }
  *to= '\0';
  return (unsigned long) (to-to0);
}

void Str2Hex( const char *sSrc,  char *sDest, int nSrcLen )
{
    int  i;
    char szTmp[3];
    for( i = 0; i < nSrcLen; i++ )
    {
        sprintf( szTmp, "%02x", (unsigned char) sSrc[i] );
        memcpy( &sDest[i * 2], szTmp, 2 );
    }
    return ;
}


void HexStrToStr(const char* source, unsigned char* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;
    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;
 
        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return ;
}

int get_dev_sn(char *sn)
{
	int i = 0;
	unsigned char all_addr[OPTION_STR_LEN]={0};
	char mtd_ubootcfg[OPTION_STR_LEN]={0};
	
	snprintf(mtd_ubootcfg, OPTION_STR_LEN, "/dev/mtdblock%d", 2);
	int fd = open(mtd_ubootcfg, O_RDONLY);
	if (fd < 0) 
	{
		printf("Could not open mtd device: %s\n", mtd_ubootcfg);
		return -1;
	}	

	lseek(fd, 0, SEEK_SET);
	if (read( fd, all_addr, OPTION_STR_LEN) != OPTION_STR_LEN)
	{
		printf("read() failed\n");
		close(fd);
		return -1;
	}
	all_addr[OPTION_STR_LEN+1]='\0';
	for(i = 0; all_addr[i] != 0; i++) {
    	if(isxdigit(all_addr[i]) == 0)
			all_addr[i]='\0';
	}
	HexStrToStr(all_addr, sn, strlen(all_addr));
	close(fd);

	return 0;
}

#define MAX_TOKEN_LEN 64

int parseRequest(cJSON *request_root) {
    // 将 cJSON 对象转换为字符串
    char *json_str = cJSON_Print(request_root);
    
    if (json_str == NULL) {
        CLIENT_SOCKET_LOG("Conversion failed!\n");
        return 0;
    }
    
    // 输出结果
    CLIENT_SOCKET_LOG("Analysis results:\n%s\n", json_str);
    
    // 释放内存
    free(json_str);
    
    return 1;
}



HUAYI_BOOL ParsingStrings(const char* resultbuf, int count, char (*stringArray)[64]) {
    if (resultbuf == NULL || count <= 0 || stringArray == NULL) {
        return HUAYI_FALSE;
    }

    char* token = strtok((char*)resultbuf, " ");
    int i = 0;
    while (token != NULL && i < count) {
        size_t len = strlen(token);
        if (len >= MAX_TOKEN_LEN) {
            len = MAX_TOKEN_LEN - 1;
        }
        strncpy(stringArray[i], token, len);
        stringArray[i][len] = '\0';
        token = strtok(NULL, " ");
        i++;
    }

    return HUAYI_TRUE;
}


int isCharValid(char c) {
    return ((c >= 48 && c <= 58) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122));
}
_BOOL isStringValid(const char* str) {
    size_t len = strlen(str);
    size_t i = 0;
    for (i = 0; i < len; i++) {
        if (!isCharValid(str[i])) {
            return _false;
        }
    }
    return _true;
}



HUAYI_BOOL toParsing(const char* resultbuf, int count, ...) {
    if (resultbuf == NULL || count <= 0) {
        return HUAYI_FALSE;
    }

    va_list args;
    va_start(args, count);

    char* token = strtok((char*)resultbuf, " ");
    int i = 0;
    while (token != NULL && i < count) {
        void* arg = va_arg(args, void*);

        if (arg == NULL || token == NULL) {
            continue;
        }

        if (strncmp(token, " ", 1) == 0) {
            token = strtok(NULL, " ");
            continue;
        }

       // CLIENT_SOCKET_LOG("%s", token);

        // 判断token是否为整数
        int intVal;
        if (strlen(token)<4 && sscanf(token, "%d", &intVal) == 1) {
            int* intPtr = (int*)arg;
            *intPtr = intVal;
        } else {
            char* strPtr = (char*)arg;
            strncpy(strPtr, token, OPTION_STR_LEN -1);
            strPtr[OPTION_STR_LEN -1] = '\0';

        }

        token = strtok(NULL, " ");
        i++;
    }

    va_end(args);
    return HUAYI_TRUE;
}




HUAYI_BOOL CustomCommunicationClient(const char *sendBuf,char * recvBuf){
    
    struct DataPacket datapacket;
    struct DataPacket receivePackets;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            CLIENT_SOCKET_LOG("socket");
            goto fail;
        }
    
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(9595);
        server_addr.sin_addr.s_addr = inet_addr("192.168.64.1");
    
        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            CLIENT_SOCKET_LOG("Failed to connect to server");
            goto fail;
        }
        CLIENT_SOCKET_LOG("Connected to server.\n");
    datapacket.sockfd=sock;
    datapacket.choose=TcpExeCmd;
    strncpy(datapacket.cmdBuf,sendBuf, sizeof(datapacket.cmdBuf));
    strncpy(datapacket.returnValue, "", sizeof(datapacket.returnValue));
    
    int written = send(sock, &datapacket, sizeof(datapacket), 0);
    if (written == -1) {
        CLIENT_SOCKET_LOG("Write error");
        goto fail;
    } else if (written < sizeof(datapacket)){
        CLIENT_SOCKET_LOG("Only partial data was written");
    } else {
        CLIENT_SOCKET_LOG("Write#fd:%d# choose:%d# cmdBuf:%s# returnValue:%s#",datapacket.sockfd,datapacket.choose,datapacket.cmdBuf,datapacket.returnValue);
    }
   int dataLen = recv(sock, &receivePackets, sizeof(receivePackets), 0);
    if (dataLen > 0) {
            // 成功读取了一定数量的数据
            CLIENT_SOCKET_LOG("###read######fd:%d,cmdBuf:%s# returnValue:%s",receivePackets.sockfd,receivePackets.cmdBuf,receivePackets.returnValue);
            strncpy(recvBuf,receivePackets.returnValue, strlen(receivePackets.returnValue));
        } else if (dataLen == 0) {
            // 对端关闭了连接
            CLIENT_SOCKET_LOG("Connection closed\n");
            goto fail;
        } else if (dataLen == -1) {
            // 出现了错误
            CLIENT_SOCKET_LOG("Failed to receive data");
            goto fail;
        }
    close(sock);
    return HUAYI_TRUE;
fail:
    close(sock);
    return HUAYI_FALSE;
}


void getCmdResult(const char* cmd, char** resultbuf, size_t* bufsize) {
    FILE* fp = popen(cmd, "r");
    if (fp) {
        *resultbuf = malloc(*bufsize);
        fgets(*resultbuf, *bufsize, fp);
        pclose(fp);
    }
}

HUAYI_BOOL checkFileExistsAndNotEmpty(const char* filename) {
    // 检查文件是否存在
    if (access(filename, F_OK) == -1) {
        return HUAYI_FALSE;
    }

    // 检查文件大小是否为0
    struct stat fileStat;
    if (stat(filename, &fileStat) == -1) {
        return HUAYI_FALSE;
    }

    if (fileStat.st_size == 0) {
        return HUAYI_FALSE;
    }

    return HUAYI_TRUE;
}
int CheckDns() {
    const char* filename = "tmp/resolv.conf.auto";

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        CON_LOG("open files error\n");
        return 1;
    }

    char line[256];
    int interfaceCount = 0;
    int hasZeroIP = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, filename) != NULL) {
            interfaceCount = 0;
            hasZeroIP = 0;
        }

        if (strstr(line, "Interface") != NULL) {
            interfaceCount++;
        }

        if (strstr(line, "0.0.0.0") != NULL) {
            hasZeroIP = 1;
        }
    }

    fclose(file);

    if (strstr(filename, "tmp/resolv.conf.auto") != NULL && interfaceCount >= 2) {
        return 0;
    } else if (hasZeroIP) {
        return 0;
    } else {
        return 1;
    }
}


int checkInternet(const char* address) {
    char command[200];
    sprintf(command, "ping -c 1 %s 2>&1", address); // 构造 ping 命令，将错误输出重定向到标准输出

    FILE* fp = popen(command, "r"); // 执行 ping 命令并获取输出结果
    if (fp == NULL) {
        printf("Failed to execute ping command.\n");
        return 0;
    }

    char buffer[100];
    int received = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "1 packets transmitted") != NULL && strstr(buffer, "1 received") != NULL) {
            received = 1;
            break;
        }
        if (strstr(buffer, "bad address") != NULL) {
            received = 0;
            break;
        }
    }

    pclose(fp); // 关闭文件指针

    return received;
}


void DetectInternetFaults() {
    char resultbuf[CMD_STR_LEN] = {0};
    char cmd_route[CMD_STR_LEN] = {0},cmd[CMD_STR_LEN] = {0};
    char charValue[OPTION_STR_LEN] = {0};
    if (LTE_FALSE == atcmd_check()) {
        //...at error
        strcpy(charValue, "at error");
    }

    char ifname[OPTION_STR_LEN] = {0};
    int state;
    huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ifname", ifname);
    if (!strcmp(ifname, "apcli0") || !strcmp(ifname, "eth0.2")) {
        atcmd_get_cpin(&state);
        if (state == MODEM_SIN_READY) {
            memset(resultbuf, 0, sizeof(resultbuf));
            if (atcmd_get_lac(resultbuf) == LTE_TRUE && strlen(resultbuf) > 2) {
                atcmd_get_signal_for_cloud(&state);
                if (state != -121) {
                    goto check;
                } else {
                    // no signal
                    strcpy(charValue, "no signal");
                }
            } else {
                // SIM card not activated
                strcpy(charValue, "SIM card not activated");
            }
        } else {
            // SIM card not found
            strcpy(charValue, "SIM card not found");
        }
    } else {
        // Non-4G
        goto check;
    }

check:
    memset(cmd_route, 0, sizeof(cmd_route));//one "default" ->1 ,otherwise 0
    strncpy(cmd_route, "route | awk '$1 == \"default\" && $2 != \"0.0.0.0\" { count++ } END { if (count == 1) print 1; else print 0 }'", sizeof(cmd_route));
    get_cmd_result(cmd_route, resultbuf, sizeof(resultbuf));
    if (atoi(resultbuf)) {
        if (CheckDns() == 1) {
            // DNS check passed
            memset(resultbuf, 0, sizeof(resultbuf));
            huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",resultbuf);
            if(checkInternet(resultbuf) != 0) {
                strcpy(charValue, "No fault detected");
            } else {
            //internet error
                strcpy(charValue, "internet error");
            }
        } else {
            strcpy(charValue, "dnserr");
        }
    } else {
        strcpy(charValue, "route error");
    }
    snprintf(cmd,sizeof(cmd),"echo '%s' > /etc/config/internet_state",charValue);
    system(cmd);
}


void save_useing_siminfo(int i)
{
	char cmd[OPTION_STR_LEN] = {0};
	int  netoperator = 0;
	char tmp[OPTION_STR_LEN] = {0};
	int ret = 0, count = 0, gpio = 0;
	int i_ready = 0, i_signal = 0;
	char lte_value[OPTION_STR_LEN] = {0};
    int AsciiNum[22], j;
	int signal = 0;

	sprintf(cmd, "sim%d", i);
	count = 0;
retry_getccid:
    memset(lte_value, 0, OPTION_STR_LEN);
	atcmd_get_iccid(lte_value);
	for(j=0;j<strlen(lte_value) ,AsciiNum[j]=lte_value[j];j++)		
	if(strlen(lte_value) < 5 || (strstr(lte_value,"ERR")!= NULL) ||!((AsciiNum[j]>=48 && AsciiNum[j]<=58) || (AsciiNum[j]>=65 && AsciiNum[j]<=90) || (AsciiNum[j]>=97 && AsciiNum[j]<=122)))
	{
		count += 1;
        strcpy(lte_value,"");
        usleep(800);
        if(count < 10)
            goto retry_getccid;
	}
    count = 0;
	huayi_uci_set_str(PKG_M_SYSTEM, "usesiminfo", "iccid", lte_value);
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid", lte_value);


retry_getlac:     //锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷LAC
    memset(lte_value, 0, OPTION_STR_LEN);
    atcmd_get_lac(lte_value);
    //printf("rex ====   %s, %d  ---lac = %s-\n", __FUNCTION__, __LINE__, lte_value);
    if(strlen(lte_value) < 2)
    {
        strcpy(lte_value, "");
        count+=1;
        if(count < 10)
            goto retry_getlac;
    }
    huayi_uci_set_str(PKG_M_SYSTEM, "usesiminfo", "lac", lte_value);
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "lac", lte_value);
    count = 0;

retry_getci:      //锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷ci
    memset(lte_value, 0, OPTION_STR_LEN);
    atcmd_get_ci(lte_value);
    //printf("rex ====   %s, %d  --ci =%s--\n", __FUNCTION__, __LINE__, lte_value);
    if(strlen(lte_value) < 2)
    {
        strcpy(lte_value, "");
        count+=1;
        if(count < 10)
            goto retry_getci;
    }
    huayi_uci_set_str(PKG_M_SYSTEM, "usesiminfo", "ci", lte_value);
	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "ci", lte_value);
    count = 0;


retry_getoperator:    //锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷operator info
    memset(lte_value,0,OPTION_STR_LEN);
    atcmd_get_imsi(lte_value);
    //printf("rex ====   %s, %d  ---operator = %s-\n", __FUNCTION__, __LINE__, lte_value);
    if (strncmp(lte_value,"46000",5) == 0 || strncmp(lte_value,"46002",5) == 0 || strncmp(lte_value,"46004",5) == 0 ||
        strncmp(lte_value,"46007",5) == 0 || strncmp(lte_value,"46008",5) == 0 || strncmp(lte_value,"46013",5) == 0)
    {
        netoperator = 3;
    }
    else if (strncmp(lte_value,"46001",5) == 0 || strncmp(lte_value,"46006",5) == 0 || strncmp(lte_value,"46009",5) == 0 ||
        strncmp(lte_value,"46010",5) == 0 )
    {
        netoperator = 2;
    }
    else if (strncmp(lte_value,"46003",5) == 0 || strncmp(lte_value,"46005",5) == 0 || strncmp(lte_value,"46011",5) == 0 ||
        strncmp(lte_value,"460012",5) == 0 )
    {
        netoperator = 1;
    }
    else
    {
        netoperator = 0;
        count+=1;
        if(count < 10)
            goto retry_getoperator;
    }
    huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "operator", netoperator);
	huayi_uci_set_int(PKG_M_SYSTEM, cmd, "operator", netoperator);
    count = 0;

	
	atcmd_get_signal(&signal);
	huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "signal", signal);
	huayi_uci_set_int(PKG_M_SYSTEM, cmd, "signal", signal);



	memset(tmp, 0, OPTION_STR_LEN);
	memset(lte_value, 0, OPTION_STR_LEN);
	huayi_uci_get_str(PKG_M_SYSTEM, cmd, "speedcmd", tmp);
	if(strlen(tmp) > 5)
	{
		ret = get_cmd_result(tmp, lte_value, sizeof(lte_value));
		if(ret == HUAYI_TRUE)
		{
			//todo iperf
			huayi_uci_set_str(PKG_M_SYSTEM, "usesiminfo", "speedcmd", tmp);
		}
		else
		{
			huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "speeddownlink", 0);
			huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "speeduplink", 0);
		}
	}
	else
	{
		huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "speeddownlink", 0);
		huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "speeduplink", 0);
		huayi_uci_set_str(PKG_M_SYSTEM, "usesiminfo", "speedcmd", "");
	}

	huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "card_slot", i);
	huayi_uci_commit(PKG_M_SYSTEM);

	return;
}

int isAlphanumeric(char *str) {
    int i;

    for (i = 0; str[i] != '\0'; i++) {
        if (!isalnum(str[i])) {
            return 0;
        }
    }

    return 1;
}

float pingCheck(const char* host) {
    char command[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    FILE* fp;

    sprintf(command, "ping -c 3 -W 1 %s -I eth1 2>&1", host);

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        return -1.0;
    }

    size_t bytesRead = fread(buffer, sizeof(char), sizeof(buffer) - 1, fp);
    buffer[bytesRead] = '\0';

    pclose(fp);

    if (strstr(buffer, "bad address") != NULL) {
        return -1.0;
    }

    const char* delayStart = strstr(buffer, "time=");
    if (delayStart != NULL) {
        delayStart += 5;
        float delay = strtof(delayStart, NULL);
        return delay;
    }

    return -1.0;
}


int networkCheck(const char* hosts) {
    const char* addresses[MAX_HOSTS];
    int num_addresses = 0;
    char hosts_copy[256];
    char* token;
    char value[RESULT_STR_LEN] = {0};

    strcpy(hosts_copy, hosts);
    token = strtok(hosts_copy, ";");

    while (token != NULL && num_addresses < MAX_HOSTS) {
        addresses[num_addresses++] = token;
        token = strtok(NULL, ";");
    }

    static int prevNetworkStatus = -1;

    int networkStatus = 0;
    float pingDelay =0.0;
    int i = 0;

    for (i = 0; i < num_addresses; i++) {
        //INTERNET_LOG("Checking %s...\n", addresses[i]);


        pingDelay = pingCheck(addresses[i]);
        if (pingDelay >= 0.0) {
            networkStatus = 1;
            sprintf(value,"%.2f",pingDelay);
            huayi_uci_set_str(PKG_M_SYSTEM, "usesiminfo", "maxdelay",value);
            huayi_uci_commit(PKG_M_SYSTEM);
            break;
        }
    }
    
    if (networkStatus != prevNetworkStatus) {
        if (networkStatus == 1) {
            INTERNET_LOG("Checking %s Internet available\n",addresses[i]);
            INTERNET_LOG("Network is available (ping check). Delay: %.2f ms\n", pingDelay);
        } else {
            INTERNET_LOG("Internet error\n");
        }
        prevNetworkStatus = networkStatus;
    }


    return networkStatus;
}

int isProcessRunning(const char *processName) {
    FILE *fp;
    char buffer[1024];

    // Run ps command and read the output
    fp = popen("ps", "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    // Check if the processName is present in the output
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, processName) != NULL) {
            pclose(fp);
            return 1; // Process is running
        }
    }

    pclose(fp);
    return 0; // Process is not running
}

