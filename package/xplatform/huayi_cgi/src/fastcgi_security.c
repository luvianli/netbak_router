#include "fastcgi_common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>


#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
int base64_encode(char *in_str, int in_len, char *out_str)
{
    BIO *b64, *bio;
    BUF_MEM *bptr = NULL;
 
    if (in_str == NULL || out_str == NULL)
        return -1;
 
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
 
    BIO_write(bio, in_str, in_len);
    BIO_flush(bio);
 
    BIO_get_mem_ptr(bio, &bptr);
    memcpy(out_str, bptr->data, bptr->length);
    //out_str[bptr->length] = '\0';
 
    BIO_free_all(bio);
    return bptr->length;
}
 
int base64_decode(char *in_str, int in_len, char *out_str)
{
    BIO *b64, *bio;
    int size = 0;
 
    if (in_str == NULL || out_str == NULL)
        return -1;
 
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
 
    bio = BIO_new_mem_buf(in_str, in_len);
    bio = BIO_push(b64, bio);
 
    size = BIO_read(bio, out_str, in_len);
    out_str[size] = '\0';
 
    BIO_free_all(bio);
    return size;
}
#endif

#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)

const char *publicKey = "-----BEGIN PUBLIC KEY-----\n\
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDppUIZsbdDZlC61T4HiHhGvISt\n\
2UW8Sx2DE7v65X2RqEZa9Pybhj0XbjFcWz0DwTsaXtijKL8zTmeGw2VkWcNAEZra\n\
PWmm3LeaWO/F0gGJMMRT0MW1m0Wf04/vb0AcQEDCkNW0Ou7wBV57O0RK1LAEmZpf\n\
LUaG8bYgtcMzdcur+wIDAQAB\n\
-----END PUBLIC KEY-----";
 
const char *privateKey = "-----BEGIN RSA PRIVATE KEY-----\n\
MIICXgIBAAKBgQDppUIZsbdDZlC61T4HiHhGvISt2UW8Sx2DE7v65X2RqEZa9Pyb\n\
hj0XbjFcWz0DwTsaXtijKL8zTmeGw2VkWcNAEZraPWmm3LeaWO/F0gGJMMRT0MW1\n\
m0Wf04/vb0AcQEDCkNW0Ou7wBV57O0RK1LAEmZpfLUaG8bYgtcMzdcur+wIDAQAB\n\
AoGBAIvnrAQMuYs/Wfn2NJV4L2VT/zLGlo5ggER77aUYBC/TDoXqitRp9mzwS9hc\n\
vdVgwI5S2vAgNxV2zmEQ4cbMRTAUNZcY16Y+L+oz2a+QT5gtRe57IKgQT5Ae99N/\n\
bgc5eHyjfRF3G2eI58dYAHRD05rK4+GOSxjTanMgL5Hpm2uxAkEA9O6hzH/45hYQ\n\
6OUaIXXQmx/JuzRIMet9i9S6l0MyghvPVGb4kkILXqqgg7JqtcQx2vVk1x5p0fIk\n\
qWi3zb0T4wJBAPQ0EBw4bh6RCwO9b8+Tq4GBDasjEZ7EwcnNhXB4Zdd7uyDDD+N8\n\
NFVLVydHYDhWYT1nYxN2kGc8GPFp3widcwkCQQDX6157Vv1maGFzzjPg92l0tusx\n\
uv9B8qV0lY4ANt7Z0AZQE45h8RLw214xeyGTJ2gD7Hx/H3KN//bN2WDfYCWdAkEA\n\
v2/Q8U6LhQrPR69J58ArwuoYfQCQ35X/FOQ4p2K686HboGNKIVtM61CUkgYyN/f/\n\
YtOZp7ham5NROm90mJxVgQJAFaxAreALZqwm0M+O25UBHIiUw9IP3Mwy4xcdgQ0U\n\
UBQ/TesrJ/u3pYd815OhJuOFBuk+AvOCWhG9igDGxHjyJQ==\n\
-----END RSA PRIVATE KEY-----";

void *rsa_encryption(char *plaintext)
{
	BIO *bio = NULL;
	RSA *publicRsa = NULL;
	unsigned char *encryptMsg = NULL;
	int rsa_len = 0, len = 0, ret = 0;
	
	if ((bio = BIO_new_mem_buf((void *)publicKey, -1)) == NULL)
	{
		return NULL;
	}

	if ((publicRsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL)) == NULL) 
	{
		BIO_free_all(bio);
		return NULL;
	}
	
	BIO_free_all(bio);

	rsa_len = RSA_size(publicRsa);
 
	encryptMsg = (unsigned char *)malloc(rsa_len);
	
	if(NULL != encryptMsg)
	{
		memset(encryptMsg, 0, rsa_len);
		
		len = rsa_len - 11;

		ret = RSA_public_encrypt(len,(unsigned char *)plaintext, encryptMsg, publicRsa, RSA_PKCS1_PADDING);

		RSA_free(publicRsa);
		if (ret < 0)
		{
			return NULL;
		}
		else
		{
			return encryptMsg;
		}
	}
	else
	{
		RSA_free(publicRsa);
		return NULL;
	}
}

char *rsa_decrypt(void *ciphertext)
{
	BIO *bio = NULL;
	RSA *privateRsa = NULL;
	unsigned char *decryptMsg = NULL;
	int rsa_len = 0, mun = 0;

	if ((bio = BIO_new_mem_buf((void *)privateKey, -1)) == NULL)
	{
		return NULL;
	}
	
	if ((privateRsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL)) == NULL) 
	{
		BIO_free_all(bio);
		return NULL;
	}
	
	BIO_free_all(bio);
	
	rsa_len = RSA_size(privateRsa);
	
	decryptMsg = (unsigned char *)malloc(rsa_len);
	
	memset(decryptMsg, 0, rsa_len);
    
	mun =  RSA_private_decrypt(rsa_len, ciphertext, decryptMsg, privateRsa, RSA_PKCS1_PADDING);

	RSA_free(privateRsa);
	if ( mun < 0)
	{
		return NULL;
	}
	else
	{
		return (char *)decryptMsg;
	}
}
#endif

#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
FCGI_BOOL web_cookie_init(void)
{
	char cmd[CMD_STR_LEN] = {0};

	snprintf(cmd,sizeof(cmd),"rm -rf %s 2>/dev/null", WEB_SESS_COOKIE_PATH);
	system(cmd);

	snprintf(cmd,sizeof(cmd),"mkdir -p %s", WEB_SESS_COOKIE_PATH);
	system(cmd);
	
	return FCGI_TRUE;
}

FCGI_BOOL get_cookie_from_session(char cookie_sess_id[])
{
	char *tmp = NULL;
	char *sess_id = NULL;
	char *p = NULL;
	
	tmp = getenv("HTTP_COOKIE");

	if(NULL != tmp)
	{
		if((sess_id = strstr(tmp, "SESSION_ID=")) != NULL)
		{
			sess_id += strlen("SESSION_ID=");

			strncpy(cookie_sess_id, sess_id, OPTION_STR_LEN);
			
			if((p = strstr(cookie_sess_id, ";")) != NULL)
			{
				*p = '\0';
			}

			return FCGI_TRUE;
		}
		else
		{
			return FCGI_FALSE;
		}		
	}

	return FCGI_FALSE;
}

FCGI_BOOL check_session_active(char *checkip, char *cookie_sess_id)
{
	int fp = 0 ;
	char cookiefile[CMD_STR_LEN] = {0};
	char buff[CMD_STR_LEN] = {0};
	char cmd [CMD_STR_LEN] = {0};
	long int last_time = 0;
	char remoteip[OPTION_STR_LEN] = {0};
	char cookie[OPTION_STR_LEN] = {0};
	struct sysinfo info;
	
	sysinfo(&info);
	//FASTCGI_LOG("remoteip = %s\n",remoteip);
	snprintf(cookiefile, sizeof(cookiefile), "%s/%s", WEB_SESS_COOKIE_PATH, checkip);
	if(huayi_file_exists(cookiefile))
	{
		//FASTCGI_LOG("huayi_file_exists is true\n");
		fp = open(cookiefile,O_RDONLY);
		if(-1 == fp)
		{
			//FASTCGI_LOG("open cookiefile fail\n");
			return FCGI_FALSE;
		}	
		memset(buff,0,sizeof(buff));
		read(fp,buff,sizeof(buff));	
		close(fp);
		if(strlen(buff) == 0)
		{	
			//FASTCGI_LOG("buff is 0\n");
			return FCGI_TRUE;
		}

		//FASTCGI_LOG("remoteip=%s,checkip=%s\n",remoteip,checkip);
		//FASTCGI_LOG("cookie=%s,cookie_sess_id=%s\n",cookie,cookie_sess_id);
		//FASTCGI_LOG("last_time=%d,uptime=%d\n",last_time,info.uptime);
		
		sscanf(buff,"%s %ld %s",remoteip,&last_time,cookie);
		if(0 == strcmp(remoteip,checkip) && \
		   0 == strcmp(cookie,cookie_sess_id) && \
		   (last_time + WEB_RELEASE_TIME) > info.uptime)
		{	
			fp = open(cookiefile,O_WRONLY|O_TRUNC|O_CREAT);
			snprintf(cmd,sizeof(cmd),"%s %ld %s",remoteip,info.uptime,cookie);
			write(fp,cmd,sizeof(cmd));
			close(fp);
			return FCGI_TRUE;					
		}
	}
	else
	{
		//FASTCGI_LOG("huayi_file_exists is not\n");
	}

	//FASTCGI_LOG("check_session_active is fail\n");
	return FCGI_FALSE;	
}	
		


FCGI_RESP_CODE check_landing_status(void)
{
	char sys_password[OPTION_STR_LEN] = {0};
	char cookie_sess_id[OPTION_STR_LEN] = {0};
	char *remoteip = NULL;

	if(huayi_uci_get_str(PKG_SYSTEM,"login","http_pass",sys_password) && strlen(sys_password) > 0)
	{
		remoteip = getenv("REMOTE_ADDR");
		//FASTCGI_LOG("remoteip = %s\n",remoteip);
		if(get_cookie_from_session(cookie_sess_id))
		{
			//FASTCGI_LOG("cookie_sess_id = %s\n",cookie_sess_id);
			if(check_session_active(remoteip,cookie_sess_id))
			{
				return FCGI_RESP_CODE_SUCCESS;
			}
		}
		else
		{
			//FASTCGI_LOG("get_cookie_from_session fail\n");
		}
		return FCGI_RESP_CODE_UNAUTHORIZED;
	}
	else
	{
		return FCGI_RESP_CODE_SUCCESS;
	}
	
}

int get_login_users(void)
{
	DIR *pDir = NULL;
	struct dirent * pEnt = NULL;
	int fp = 0;
	char *remoteip = NULL;
	int num = 0;
	long int last_time = 0;
	char cookie_file[CMD_STR_LEN] = {0}; //tmp_data[LONG_BUFF_LEN] = {0};
	char webusers_list[MAX_WEB_SESSION_NUM][OPTION_STR_LEN] = {0};//user_ip[OPTION_STR_LEN] = {0};
	int i = 0;
	char buff[CMD_STR_LEN] = {0};
	char cookie[OPTION_STR_LEN] = {0};
	char userip[OPTION_STR_LEN] = {0};
	struct sysinfo info;
	
	memset(webusers_list, 0, sizeof(webusers_list));
	pDir = opendir(WEB_SESS_COOKIE_PATH);
	if (NULL == pDir)
	{
		return -1;
	}

	while(1)
	{
		pEnt = readdir(pDir);
		if(NULL != pEnt)
		{
			if(0 != strcmp(".",pEnt->d_name) && 0 != strcmp("..",pEnt->d_name))
			{
				strncpy(webusers_list[num], pEnt->d_name, OPTION_STR_LEN);
				num++;
			}
		}
		else
		{
			break;
		}
	}
	closedir(pDir);

	if(num >= MAX_WEB_SESSION_NUM)
	{
		remoteip = getenv("REMOTE_ADDR");
		sysinfo(&info);
		i = 0;
		while(i < MAX_WEB_SESSION_NUM && strlen(webusers_list[i]) > 0)
		{
			memset(cookie_file, 0, sizeof(cookie_file));
			snprintf(cookie_file, sizeof(cookie_file), "%s/%s", WEB_SESS_COOKIE_PATH, webusers_list[i]);
			
			if(huayi_file_exists(cookie_file))
			{
				fp = open(cookie_file,O_RDONLY);
				if(-1 == fp)
				{
					return -1;
				}
				read(fp,buff,sizeof(buff));
				sscanf(buff,"%s %ld %s",userip,&last_time,cookie);
				if(0 == strcmp(remoteip,userip) || \
					0 != strcmp(userip, webusers_list[i]) || \
					(last_time + WEB_RELEASE_TIME) < info.uptime)
				{
					remove(cookie_file);
					num--;
				}						
				close(fp);
			}
			i++;
		}
	}
	return num;
}

FCGI_BOOL find_login_user(char *sess_id_set)
{
	int fp = 0;
	long int last_time = 0;
	char cmd[CMD_STR_LEN] = {0};
	char buff[CMD_STR_LEN] = {0};
	char cookiefile[CMD_STR_LEN] = {0};
	//char cookie[OPTION_STR_LEN] = {0};
	char userip[OPTION_STR_LEN] = {0};
	char *remoteip = NULL;
	struct sysinfo info;

	remoteip = getenv("REMOTE_ADDR");

	snprintf(cookiefile, sizeof(cookiefile), "%s/%s", WEB_SESS_COOKIE_PATH, remoteip);

	if(huayi_file_exists(cookiefile))
	{
		fp = open(cookiefile,O_RDWR);
		if(-1 == fp)
		{
			return FCGI_FALSE;
		}
		read(fp,buff,sizeof(buff));
		sscanf(buff,"%s %ld %s",userip,&last_time,sess_id_set);
		lseek(fp, 0, SEEK_SET);
		sysinfo(&info);
		snprintf(cmd,sizeof(cmd),"%s %ld %s",remoteip,info.uptime,sess_id_set);
		write(fp,cmd,sizeof(cmd));
							
		close(fp);
		return FCGI_TRUE;
	}
	else
	{
		return FCGI_FALSE;
	}
	
	return FCGI_FALSE;
}
					

FCGI_BOOL add_session_login(char *sess_id_set)
{
	int fp = 0;
	char cookiefile[CMD_STR_LEN] = {0}, sess_id_tmp[CMD_STR_LEN] = {0};
	char tmp[OPTION_STR_LEN] = {0};
	char macaddr_str[MAC_STR_LEN] = {0};
	int i = 0;
	char *remoteip = NULL;
	char buff[CMD_STR_LEN] = {0} ;
	struct sysinfo info;

	remoteip = getenv("REMOTE_ADDR");
	sysinfo(&info);
	
	snprintf(cookiefile, sizeof(cookiefile), "%s/%s", WEB_SESS_COOKIE_PATH, remoteip);
	
	huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","mac_address",macaddr_str);

	snprintf(sess_id_tmp,CMD_STR_LEN,"%s*%ld*%s",remoteip,info.uptime,macaddr_str);
	base64_encode(sess_id_tmp, strlen(sess_id_tmp), tmp);

	while(i<sizeof(tmp) && tmp[i] != 0)
	{
		if('\n' == tmp[i] || '=' == tmp[i])
		{
			tmp[i] = '\0';
		}
		i++;
	}
	snprintf(sess_id_set,OPTION_STR_LEN,"%s",tmp);
	
	fp = open(cookiefile,O_WRONLY|O_TRUNC|O_CREAT);

	if(-1 == fp)		
	{					
		return FCGI_FALSE;
	}
	snprintf(buff,sizeof(buff),"%s %ld %s",remoteip,info.uptime,sess_id_set);
	write(fp,buff,sizeof(buff));
	close(fp);
	return FCGI_TRUE;		
}


FCGI_BOOL session_logout(char *remoteip)
{
	char cmd[CMD_STR_LEN] = {0};
	char cookiefile[CMD_STR_LEN] = {0};

	if(0 == strcmp(remoteip, "all"))
	{
		snprintf(cmd,sizeof(cmd),"rm -rf %s/* 2>/dev/null", WEB_SESS_COOKIE_PATH);
		system(cmd);
	}
	else
	{
		snprintf(cookiefile, sizeof(cookiefile), "%s/%s", WEB_SESS_COOKIE_PATH, remoteip);

		if(huayi_file_exists(cookiefile))
		{
			remove(cookiefile);
		}
	}
	
	return FCGI_TRUE;
}

#endif

