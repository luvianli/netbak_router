
#include "fastcgi_updownload.h"

/*
sample response
{
  "restcode": 1,
  "errmsg": "success",
  "locktime": "0",
  "result": [
  	{
        "connect_type":	"pppoe",
        "user":	"huayi_pppoe",
        "passwd":	"pppoe_password",
        "dnsmode":	"0",
        "dns1":	"10.2.2.1",
        "mtu":	"1492",
  	}
  	{
  		...
  	}
  ],
}

*/
int main (int argc, char *argv[])
{
	char  *rest_data		= NULL;
	cJSON *response_root	= NULL;
	FCGI_RESP_CODE   rest_code;
	int   lock_time			= 0;
	int		opmode = 0;
	char	reconnection_addr[OPTION_STR_LEN] = {0};
	
	//FCGX_Init();
	
	//while (FCGI_Accept() >= 0) 
	{
		
		rest_data		= NULL;
		response_root	= NULL;
		rest_code		= FCGI_RESP_CODE_FILE_FORMAT_ERROR;
		lock_time		= 0;
		opmode = 0;
		memset(reconnection_addr,0,OPTION_STR_LEN);

#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
		rest_code = check_landing_status();		
		if(FCGI_RESP_CODE_UNAUTHORIZED == rest_code)
		{
			goto upload_file;
		}
#endif
		rest_data = getenv("QUERY_STRING");
		
		if(rest_data && strlen(rest_data))
		{
			if(strstr(rest_data,"streamid=supermode&file="))
			{
				do_download_supermode(rest_data+strlen("streamid=supermode&file="));
				goto download_file;
			}
			
			if(!strcmp(rest_data,"streamid=1")) /*download syslog*/
			{
				do_download_syslog();
				goto download_file;
			}

			if(!strcmp(rest_data,"streamid=2")) /*download backup files*/
			{
				do_backup_system_config();
				goto download_file;
			}

			if(!strcmp(rest_data,"streamid=3")) /*restore backup files*/
			{
				rest_code = do_restore_system_config(&lock_time);

				huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode);
				if(opmode == HUAYI_ROUTE_MODE)
				{
					huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",reconnection_addr);
				}else
				{
					huayi_uci_get_str(PKG_ABOUT,"about_system","domain_name",reconnection_addr);
				}
				goto upload_file;
			}

			if(!strcmp(rest_data,"streamid=4&keep_setting=1"))/*upgrade keep setting*/
			{
				system("huayi_cmd set system upgrade_keep_setting 1");
				
				rest_code = do_system_upgrade(&lock_time);

				huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode);
				if(opmode == HUAYI_ROUTE_MODE)
				{
					huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",reconnection_addr);
					if (strcmp(reconnection_addr,"0.0.0.0") == 0)
					{
						huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",reconnection_addr);
					}
				}else
				{
					huayi_uci_get_str(PKG_ABOUT,"about_system","domain_name",reconnection_addr);
				}
				goto upload_file;
			}

			if(!strcmp(rest_data,"streamid=4&keep_setting=0"))/*upgrade not keep setting*/
			{
				system("huayi_cmd set system upgrade_keep_setting 0");
				
				rest_code = do_system_upgrade(&lock_time);

				huayi_uci_get_str(PKG_NETWORK,"lan","def_ip",reconnection_addr);
				
				goto upload_file;
			}

            if(!strcmp(rest_data,"streamid=7"))/*upgrade not keep setting*/
			{
                rest_code = do_download_script(lock_time);
				goto upload_file;
			}
            if(!strcmp(rest_data,"streamid=8"))/*upgrade not keep setting*/
			{
                rest_code = do_exe_script(lock_time);
				goto upload_file;
			}
			if(!strcmp(rest_data,"streamid=9"))/*upgrade not keep setting*/
			{
                rest_code = do_openVPN_upgrade(&lock_time);
				goto upload_file;
			}
			if(strstr(rest_data,"streamid=10&name="))/*upgrade not keep setting*/
			{
				
                rest_code = do_openVPN_configure_upgrade(rest_data+strlen("streamid=8&name="),&lock_time);
				goto upload_file;
			}
		}
		
download_file:
		return FCGI_TRUE;

upload_file:
			response_root = cJSON_CreateObject();
			
			if(lock_time > 0)
			{
				if((strlen(reconnection_addr) != 0)&&(strcmp(reconnection_addr,"0.0.0.0")))				
				{
					cJSON_AddTrueToObject(response_root,"reconnection");

					cJSON_AddStringToObject(response_root,"reconnection_addr",reconnection_addr);
				}
				else
				{
					cJSON_AddFalseToObject(response_root,"reconnection");

					cJSON_AddStringToObject(response_root,"reconnection_addr","");
				}
			}
			
			fast_cgi_response_web(response_root,rest_code,lock_time);
			cJSON_Delete(response_root);
			//continue;
	}
	
	return FCGI_TRUE;
}


