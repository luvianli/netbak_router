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


