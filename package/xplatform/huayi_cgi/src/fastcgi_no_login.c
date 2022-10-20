
#include "fastcgi_no_login.h"

#if defined(CONFIG_HUAYI_SUPPORT_CONTROL_TELNET_OPTION)
int cgi_get_telnet_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	int telnet_disenable = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	huayi_uci_get_int(PKG_TELNET, "telnet", "disenable", &telnet_disenable);
	
	cJSON_AddNumberToObject(result_root, "enable", telnet_disenable?0:1);
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}
int cgi_set_telnet_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	int enable;
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root, "enable", &enable))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_int(PKG_TELNET, "telnet", "disenable", enable?0:1);
	huayi_uci_commit(PKG_TELNET);

	system("huayi-reset huayi_telnet");

	return FCGI_RESP_CODE_SUCCESS;
}
#endif
FCGI_BOOL FastCgiHandleNoLoginFunctionInit(void)
{	
	memset(&FastCgiHandleNoLoginFunction,0,sizeof(MAX_NO_LOGIN_HYID*sizeof(int)));
#if defined(CONFIG_HUAYI_SUPPORT_CONTROL_TELNET_OPTION)
	FastCgiHandleNoLoginFunction[HYID_GET_TELNET_CONFIG] = cgi_get_telnet_config;
	FastCgiHandleNoLoginFunction[HYID_SET_TELNET_CONFIG] = cgi_set_telnet_config;
#endif
	
	return FCGI_TRUE;
}

int main(int argc,char *argv[])
{
    char  *rest_data		= NULL;
	char  *content_type		= NULL;
	FCGI_RESP_CODE   rest_code;
	int   content_len 		= 0;
	char  *query_data 		= NULL;
	cJSON *request_root 	= NULL;
	cJSON *response_root 	= NULL;
	int lock_time			= 0;
	int  HYID = 0;

	if(!FastCgiHandleNoLoginFunctionInit())
	{
		return FCGI_FALSE;
	}

    //FCGX_Init();
	
	//while (FCGI_Accept() >= 0) 
	{
		rest_data		= NULL;
		content_type	= NULL;
		rest_code		= FCGI_RESP_CODE_SUCCESS;
		content_len		= 0;
		query_data 		= NULL;
		request_root 	= NULL;
		response_root	= NULL;
		lock_time		= 0;
		HYID			= 0;

		response_root = cJSON_CreateObject();
		
		rest_data = getenv("QUERY_STRING");
		
		if(rest_data && (strlen(rest_data) > 0))
		{
			rest_code = FCGI_RESP_CODE_DATA_FORMAT_ERROR;
			
			goto v_out;
		}
		
		content_type = getenv("CONTENT_TYPE");

		if(strstr(content_type,"application/json") == NULL)
		{
			rest_code = FCGI_RESP_CODE_DATA_FORMAT_ERROR;
			
			goto v_out;
		}
		
		content_len = strtol(getenv("CONTENT_LENGTH"), NULL, 10) + 1;
		
		if(content_len <= 1)
		{
			rest_code = FCGI_RESP_CODE_DATA_FORMAT_ERROR;

			goto v_out;
		}
		
	 	query_data = malloc(content_len);

		if(!query_data)
		{
			rest_code = FCGI_RESP_CODE_SERVER_FAILURE;

			goto v_out;
		}

		memset(query_data,0,content_len);
		
		fread(query_data, 1, content_len, stdin);
		
		request_root = cJSON_Parse(query_data);
		
		if (!request_root) 
		{
			rest_code = FCGI_RESP_CODE_DATA_FORMAT_ERROR;
			
			goto v_out;
		}
		
		if(!huayi_json_get_int(request_root,"HYID",&HYID))
		{
			rest_code = FCGI_RESP_CODE_HYID_MISS;
			
			goto v_out;
		}
		
		if(HYID < HYID_GET_TELNET_CONFIG || HYID >= MAX_NO_LOGIN_HYID)
		{
			rest_code = FCGI_RESP_CODE_ILLEGAL_HYID;
			
			goto v_out;
		}
		
		if(FastCgiHandleNoLoginFunction[HYID] != NULL)
		{		
			rest_code = FastCgiHandleNoLoginFunction[HYID](request_root,response_root,&lock_time);
		}else
		{
			rest_code = FCGI_RESP_CODE_ILLEGAL_HYID;
		}
		
v_out:
		fast_cgi_response_web(response_root,rest_code,lock_time);

		cJSON_Delete(request_root);
		
		cJSON_Delete(response_root);
		
		if(query_data)
		{
			free(query_data);
		}
		//continue;
	}
	return FCGI_TRUE;
}
