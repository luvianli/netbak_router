
#include "fastcgi_modem.h"

#if defined(CONFIG_PACKAGE_HUAYI_MYSQL)
FCGI_BOOL huayiMySql2JsonArray(MYSQL *my_con,cJSON *array,char *query)
{
	char *result = NULL;
	MYSQL_FIELD *my_field =NULL;
	MYSQL_ROW row;
	unsigned int ncol = 0, i = 0, j = 0;
	unsigned long nrow = 0;
	cJSON *root;
	FCGI_BOOL ret = FCGI_FALSE;
	
	if((my_con == NULL) || (array == NULL) || (query == NULL))
	{
		return FCGI_FALSE;
	}

	if(!huayiMysqlGetTable(my_con,query,&nrow,&ncol,(void *)&result))
	{
		return FCGI_FALSE;
	}
	
	if((!nrow)||(!result))
	{
		ret = FCGI_TRUE;
		goto end;
	}

	my_field = mysql_fetch_fields((MYSQL_RES *)result);
	
	if(!my_field)
	{
		ret = FCGI_FALSE;
		goto end;
	}
	
	for(i = 0;i<nrow;i++)
	{
		root = NULL;
		
		root=cJSON_CreateObject();

		cJSON_AddItemToArray(array,root);

		row = mysql_fetch_row((MYSQL_RES *)result);

		if(!row)
		{
			break;
		}
		
		for(j= 0;j<ncol;j++)
		{
			cJSON_AddStringToObject(root, my_field[j].name,row[j]?row[j]:"");
		}
	}

	ret = FCGI_TRUE;
	
end:
	if(result)
	{
		mysql_free_result((MYSQL_RES *)result);
	}
	return ret;

}

FCGI_BOOL get_x55_result_str(char *cmd, char *key,char *value)
{
	FCGI_BOOL get_value = FCGI_FALSE;
	FILE *fp = NULL;
	char data[TEMP_STR_LEN] = {0};
	char *pvalue = NULL;
	
	if((!cmd)||(!key)||(!value))
	{
		return FCGI_FALSE;
	}
	
	fp = popen(cmd, "r");
	if(!fp) 
	{
		return FCGI_FALSE;
	}
	while(!feof(fp))
	{
		memset(data,0,sizeof(data));
		fgets(data,TEMP_STR_LEN,fp);
		if(strstr(data,key))
		{
			pvalue = strchr(data,':');
			strcpy(value,pvalue+2);
			value[strlen(value)-1] = '\0';
			get_value = FCGI_TRUE;
			break;
		}
	}

	pclose(fp);

	return get_value;
}

int cgi_get_new_sms_state(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	int unread_sms = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	snprintf(query,MAX_MYSQL_QUERY_LEN,"select count(*) from `%s` where %s>0;",SMS_HUAYI_TABLE,MYSQL_SMS_UNREAD_KEY_NAME);
	
	huayiMySqlGetInt(my_con,query,&unread_sms);

	huayiMySqlClose(my_con);

	result_root = cJSON_CreateObject();

	cJSON_AddNumberToObject(result_root,"messages_unread",unread_sms);


	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;

}

int cgi_get_breaf_sms_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *huayi_sms_json = NULL;
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};


	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_sms_json = cJSON_CreateArray();

	snprintf(query,MAX_MYSQL_QUERY_LEN,"select * from `%s` order by %s desc;",SMS_HUAYI_TABLE,MYSQL_SMS_LASTTIME_KEY_NAME);
	
	huayiMySql2JsonArray(my_con,huayi_sms_json,query);

	huayiMySqlClose(my_con);

	result_root = cJSON_CreateObject();

	cJSON_AddItemToObject(result_root,"huayi_sms", huayi_sms_json);
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;

}

int cgi_get_sms_session_by_number(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *sms_session_array = NULL;
	MYSQL *my_con = NULL;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	char phone_number[MAX_PHONE_NUMBER_LEN]= {0};


	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"number",phone_number,MAX_PHONE_NUMBER_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}
	
	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	sms_session_array = cJSON_CreateArray();

	snprintf(query,MAX_MYSQL_QUERY_LEN,"select * from `%s` order by %s asc;",phone_number,MYSQL_SMS_RECEIVE_TIME_KEY_NAME);

	huayiMySql2JsonArray(my_con,sms_session_array,query);

	huayiMySqlExec(my_con,"update `%s` set %s=0 where %s='%s'",SMS_HUAYI_TABLE,MYSQL_SMS_UNREAD_KEY_NAME,
																MYSQL_SMSNUMBER_KEY_NAME,phone_number);

	huayiMySqlClose(my_con);

	system("huayi-reset save_database");
	

	result_root = cJSON_CreateObject();

	cJSON_AddItemToObject(result_root,"sms_session", sms_session_array);
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;

}

int cgi_delete_sms_session_by_sid(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	char sid_array[LONG_BUFF_LEN] ={0};
	int i = 0;
	char sid[SMALL_STR_LEN] ={0};
	MYSQL *my_con = NULL;
	char phone_number[MAX_PHONE_NUMBER_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"messages_sid",sid_array,LONG_BUFF_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(strlen(sid_array))
	{
		while(huayi_get_nth_value(i++, sid_array, WEB_DEFAULT_DELIMIT, sid, sizeof(sid)))
		{

			memset(phone_number,0,MAX_PHONE_NUMBER_LEN);
			
			snprintf(query,MAX_MYSQL_QUERY_LEN,"select %s from `%s` where %s='%s';",MYSQL_SMSNUMBER_KEY_NAME,SMS_HUAYI_TABLE,MYSQL_SMSID_KEY_NAME,sid);
		
			if(!huayiMySqlGetStr(my_con,query,phone_number,MAX_PHONE_NUMBER_LEN))
			{
				huayiMySqlClose(my_con);

				return FCGI_RESP_CODE_SERVER_FAILURE;
			}

			snprintf(query,MAX_MYSQL_QUERY_LEN,"drop table `%s`;",phone_number);
			
			if(!huayiMySqlExec(my_con,query))
			{

				huayiMySqlClose(my_con);
				return FCGI_RESP_CODE_SERVER_FAILURE;
			}
		
			snprintf(query,MAX_MYSQL_QUERY_LEN,"delete from `%s` where %s='%s';",SMS_HUAYI_TABLE,MYSQL_SMSID_KEY_NAME,sid);
			
			if(!huayiMySqlExec(my_con,query))
			{
				huayiMySqlClose(my_con);
				return FCGI_RESP_CODE_SERVER_FAILURE;
			} 
		}
	}

	huayiMySqlClose(my_con);

	system("huayi-reset save_database");

	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_delete_sms_by_cid(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	char c_sid_array[LONG_BUFF_LEN] ={0};
	int i = 0;
	char c_sid[SMALL_STR_LEN] ={0};
	char phone_number[MAX_PHONE_NUMBER_LEN] = {0};
	MYSQL *my_con = NULL;
	int del_count = 0;
	int sms_count = 0;


	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"c_messages_sid",c_sid_array,LONG_BUFF_LEN)||
		!huayi_json_get_str(request_root,"phone_num",phone_number,MAX_PHONE_NUMBER_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}


	if(strlen(c_sid_array))
	{
		while(huayi_get_nth_value(i++, c_sid_array, WEB_DEFAULT_DELIMIT, c_sid, sizeof(c_sid)))
		{	
			snprintf(query,MAX_MYSQL_QUERY_LEN,"delete from `%s` where %s='%s';",phone_number,MYSQL_SMS_CHILD_ID_KEY_NAME,c_sid);
			
			if(!huayiMySqlExec(my_con,query))
			{
				huayiMySqlClose(my_con);
				return FCGI_RESP_CODE_SERVER_FAILURE;
			}
			del_count++;
		}
	}

	snprintf(query,MAX_MYSQL_QUERY_LEN,"select %s from `%s` where %s='%s';",MYSQL_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,MYSQL_SMSNUMBER_KEY_NAME,phone_number);

	huayiMySqlGetInt(my_con,query,&sms_count);

	if(del_count>0)
	{
		if(sms_count-del_count==0)
		{
			huayiMySqlExec(my_con,"delete from `%s` where %s='%s';",SMS_HUAYI_TABLE,MYSQL_SMSNUMBER_KEY_NAME,phone_number);
			huayiMySqlExec(my_con,"drop table `%s`;",phone_number);
		}else
		{
			huayiMySqlExec(my_con,"update `%s` inner join(select %s,%s,%s from `%s` order by %s desc limit 1) c set %s.%s=c.%s,%s.%s=c.%s,%s.%s=%s.%s-%d,%s.%s=c.%s where %s.%s='%s';",
																			SMS_HUAYI_TABLE,MYSQL_SMS_CHILD_ID_KEY_NAME,MYSQL_SMS_MSG_KEY_NAME,
																			MYSQL_SMS_RECEIVE_TIME_KEY_NAME,phone_number,MYSQL_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMS_CHILD_ID_KEY_NAME,MYSQL_SMS_CHILD_ID_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMS_MSG_KEY_NAME,MYSQL_SMS_MSG_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,MYSQL_SMS_COUNT_KEY_NAME,del_count,
																			SMS_HUAYI_TABLE,MYSQL_SMS_LASTTIME_KEY_NAME,MYSQL_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMSNUMBER_KEY_NAME,phone_number);
		}
		system("huayi-reset save_database");
	}

	huayiMySqlClose(my_con);
	
	return FCGI_RESP_CODE_SUCCESS;

}

int cgi_send_sms_messages(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	MYSQL *my_con = NULL;
	char sms_msg[MAX_SMS_MSG_LEN] = {0};
	char sms_msg_escape_characters[MAX_SMS_ESCAPE_CHARACTER_LEN] = {0};
	char phone_number_array[LONG_BUFF_LEN]= {0};
	char phone_number[MAX_PHONE_NUMBER_LEN] = {0};
	char center_number[AT_VALUE_BUFF_LEN] ={0};
	int i = 0;
	char query[MAX_MYSQL_QUERY_LEN] = {0};
	int send_count = 0;
	int success_count = 0;
	int fail_count = 0;
	int sim_state = 0;
	int sid =0;
	int ret = 0;
	cJSON *result_root = NULL;
	char *sql_hex = NULL;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"c_messages_sid",sms_msg,MAX_SMS_MSG_LEN)||
		!huayi_json_get_str(request_root,"phone_num",phone_number_array,LONG_BUFF_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if((my_con = mysql_init(NULL)) == NULL)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(!huayiMySqlOpen(my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	sql_hex = huayiMysqlHexString(sms_msg,strlen(sms_msg));
	if(NULL == sql_hex)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","center_number",center_number);

	if(strlen(phone_number_array))
	{
		while(huayi_get_nth_value(i++, phone_number_array, WEB_DEFAULT_DELIMIT, phone_number, sizeof(phone_number)))
		{
			if(0 == strlen(phone_number))
			{
				continue;
			}
			send_count++;
			
			ret = send_sms(phone_number, center_number, sms_msg);
			if(0 == ret)
			{
				success_count++;
				if(!huayiMysqlCheckTableExist(my_con,phone_number))
				{
					snprintf(query,sizeof(query),"create table if not exists `%s` ("
						"%s int unsigned not null primary key auto_increment,"
						"%s varchar(%d),"
						"%s timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,"
						"%s int unsigned not null default 0,"
						"UNIQUE (%s)"
						");",phone_number,MYSQL_SMS_CHILD_ID_KEY_NAME,MYSQL_SMS_MSG_KEY_NAME,MAX_SMS_MSG_LEN,
						MYSQL_SMS_RECEIVE_TIME_KEY_NAME,MYSQL_SMS_SRC_KEY_NAME,MYSQL_SMS_CHILD_ID_KEY_NAME);

						huayiMySqlExec(my_con,query);	
				}

				huayiMySqlExec(my_con,"insert into `%s` (%s,%s,%s) values(0x%s,now(),1);",phone_number,MYSQL_SMS_MSG_KEY_NAME,MYSQL_SMS_RECEIVE_TIME_KEY_NAME,MYSQL_SMS_SRC_KEY_NAME,sql_hex);
				
				
				snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",MYSQL_SMSID_KEY_NAME,SMS_HUAYI_TABLE,MYSQL_SMSNUMBER_KEY_NAME,phone_number);
				huayiMySqlGetInt(my_con,query,&sid);

				if(sid == 0)
				{
					huayiMySqlExec(my_con,"insert into `%s` (%s,%s,%s,%s,%s) select %s,%s,1,%s,%s from `%s` order by %s desc limit 1;",SMS_HUAYI_TABLE,MYSQL_SMSNUMBER_KEY_NAME,
																				MYSQL_SMS_MSG_KEY_NAME,MYSQL_SMS_COUNT_KEY_NAME,MYSQL_SMS_LASTTIME_KEY_NAME,
																				MYSQL_SMS_CHILD_ID_KEY_NAME,phone_number,MYSQL_SMS_MSG_KEY_NAME,MYSQL_SMS_RECEIVE_TIME_KEY_NAME,
																				MYSQL_SMS_CHILD_ID_KEY_NAME,phone_number,MYSQL_SMS_RECEIVE_TIME_KEY_NAME);
				}else
				{
					huayiMySqlExec(my_con,"update `%s` inner join(select %s,%s,%s from `%s` order by %s desc limit 1) c set %s.%s=c.%s,%s.%s=c.%s,%s.%s=%s.%s+1,%s.%s=c.%s where %s.%s=%d;",
																			SMS_HUAYI_TABLE,MYSQL_SMS_CHILD_ID_KEY_NAME,MYSQL_SMS_MSG_KEY_NAME,
																			MYSQL_SMS_RECEIVE_TIME_KEY_NAME,phone_number,MYSQL_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMS_CHILD_ID_KEY_NAME,MYSQL_SMS_CHILD_ID_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMS_MSG_KEY_NAME,MYSQL_SMS_MSG_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,MYSQL_SMS_COUNT_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMS_LASTTIME_KEY_NAME,MYSQL_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,MYSQL_SMSID_KEY_NAME,sid);
		
				}
			}else
			{
				fail_count++;
			}
		}
	}

	huayi_uci_get_int(PKG_STATUS_MODEM,"modem","sim_ready",&sim_state);
	if (sim_state > MODEM_SIN_PIN || strlen(center_number) == 0)
		sim_state = MODEM_SIN_NOT_INSERTED;

	result_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(result_root,"send_count",send_count);

	cJSON_AddNumberToObject(result_root,"success_count",success_count);

	cJSON_AddNumberToObject(result_root,"fail_count",fail_count);

	cJSON_AddNumberToObject(result_root,"sim_state",sim_state);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	if(success_count > 0)
	{
		system("huayi-reset save_database");
	}

	if(NULL != sql_hex)
	{
		free(sql_hex);
	}
	
	return FCGI_RESP_CODE_SUCCESS;
}
#endif

#if defined(CONFIG_PACKAGE_HUAYI_SQLITE3)
FCGI_BOOL huayiSqlite3JsonArray(sqlite3 *my_con,cJSON *array,char *query)
{
	char **result = NULL;
	int nrow = 0,ncol = 0, i = 0, j = 0;
	char *zErrMsg = NULL;
	int index = 0;
	int res = 0;
	cJSON *root;
	FCGI_BOOL ret = FCGI_FALSE;
	
	if((my_con == NULL) || (array == NULL) || (query == NULL))
	{
		return FCGI_FALSE;
	}

	res = sqlite3_get_table(my_con,query,&result,&nrow,&ncol,&zErrMsg);
	if(NULL != zErrMsg)
	{
		ret = FCGI_TRUE;
		goto end;
	}
	
	if((!nrow)||(!result))
	{
		ret = FCGI_TRUE;
		goto end;
	}
	
	for(i = 1;i<nrow + 1;i++)
	{
		root = NULL;
		
		root=cJSON_CreateObject();

		cJSON_AddItemToArray(array,root);
		
		for(j= 0;j<ncol;j++)
		{
			index = i * ncol + j;
			cJSON_AddStringToObject(root, result[j],result[index]?result[index]:"");
		}
	}

	ret = FCGI_TRUE;
	
end:
	sqlite3_free_table(result);
	return ret;
}

int cgi_get_new_sms_state(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	sqlite3 *my_con = NULL;
	char query[MAX_SQLITE3_QUERY_LEN] = {0};
	int unread_sms = 0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayiSqlite3Open(&my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	snprintf(query,MAX_SQLITE3_QUERY_LEN,"select count(*) from `%s` where %s>0;",SMS_HUAYI_TABLE,SQLITE3_SMS_UNREAD_KEY_NAME);
	
	huayiSqlite3GetInt(my_con,query,&unread_sms);

	huayiSqlite3Close(my_con);

	result_root = cJSON_CreateObject();

	cJSON_AddNumberToObject(result_root,"messages_unread",unread_sms);


	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_get_breaf_sms_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *huayi_sms_json = NULL;
	sqlite3 *my_con = NULL;
	char query[MAX_SQLITE3_QUERY_LEN] = {0};


	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayiSqlite3Open(&my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	huayi_sms_json = cJSON_CreateArray();

	snprintf(query,MAX_SQLITE3_QUERY_LEN,"select * from `%s` order by %s desc;",SMS_HUAYI_TABLE,SQLITE3_SMS_LASTTIME_KEY_NAME);

	//FASTCGI_LOG("query = %s\n",query);
	huayiSqlite3JsonArray(my_con,huayi_sms_json,query);

	huayiSqlite3Close(my_con);

	result_root = cJSON_CreateObject();

	cJSON_AddItemToObject(result_root,"huayi_sms", huayi_sms_json);
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_get_sms_session_by_number(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	cJSON *sms_session_array = NULL;
	sqlite3 *my_con = NULL;
	char query[MAX_SQLITE3_QUERY_LEN] = {0};
	char phone_number[MAX_PHONE_NUMBER_LEN]= {0};


	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"phone_num",phone_number,MAX_PHONE_NUMBER_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}
	
	if(!huayiSqlite3Open(&my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	sms_session_array = cJSON_CreateArray();

	snprintf(query,MAX_SQLITE3_QUERY_LEN,"select * from `%s` order by %s asc;",phone_number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME);

	huayiSqlite3JsonArray(my_con,sms_session_array,query);

	huayiSqlite3Exec(my_con,"update `%s` set %s=0 where %s='%s'",SMS_HUAYI_TABLE,SQLITE3_SMS_UNREAD_KEY_NAME,
																SQLITE3_SMSNUMBER_KEY_NAME,phone_number);

	huayiSqlite3Close(my_con);

	system("huayi-reset save_database");
	

	result_root = cJSON_CreateObject();

	cJSON_AddItemToObject(result_root,"sms_session", sms_session_array);
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_delete_sms_session_by_sid(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char query[MAX_SQLITE3_QUERY_LEN] = {0};
	char sid_array[LONG_BUFF_LEN] ={0};
	int i = 0;
	char sid[SMALL_STR_LEN] ={0};
	sqlite3 *my_con = NULL;
	char phone_number[MAX_PHONE_NUMBER_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"message_id",sid_array,LONG_BUFF_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!huayiSqlite3Open(&my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	if(strlen(sid_array))
	{
		while(huayi_get_nth_value(i++, sid_array, WEB_DEFAULT_DELIMIT, sid, sizeof(sid)))
		{

			memset(phone_number,0,MAX_PHONE_NUMBER_LEN);
			
			snprintf(query,MAX_SQLITE3_QUERY_LEN,"select %s from `%s` where %s='%s';",SQLITE3_SMSNUMBER_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMSID_KEY_NAME,sid);
		
			if(!huayiSqlite3GetStr(my_con,query,phone_number,MAX_PHONE_NUMBER_LEN))
			{
				huayiSqlite3Close(my_con);

				return FCGI_RESP_CODE_SERVER_FAILURE;
			}

			snprintf(query,MAX_SQLITE3_QUERY_LEN,"drop table `%s`;",phone_number);
			
			if(!huayiSqlite3Exec(my_con,query))
			{

				huayiSqlite3Close(my_con);
				return FCGI_RESP_CODE_SERVER_FAILURE;
			}
		
			snprintf(query,MAX_SQLITE3_QUERY_LEN,"delete from `%s` where %s='%s';",SMS_HUAYI_TABLE,SQLITE3_SMSID_KEY_NAME,sid);
			
			if(!huayiSqlite3Exec(my_con,query))
			{
				huayiSqlite3Close(my_con);
				return FCGI_RESP_CODE_SERVER_FAILURE;
			} 
		}
	}

	huayiSqlite3Close(my_con);

	system("huayi-reset save_database");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_delete_sms_by_cid(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char query[MAX_SQLITE3_QUERY_LEN] = {0};
	char c_sid_array[LONG_BUFF_LEN] ={0};
	int i = 0;
	char c_sid[SMALL_STR_LEN] ={0};
	char phone_number[MAX_PHONE_NUMBER_LEN] = {0};
	sqlite3 *my_con = NULL;
	int del_count = 0;
	int sms_count = 0;

	char **result = NULL;
	char *zErrMsg = NULL;
	int nrow = 0,ncol = 0; 
	int sql_c_sid = 0;
	int sql_sms_count = 0;
	char date_str[MAX_SQLITE3_VALUE_LEN] = {0};
	char data_msg[MAX_SQLITE3_QUERY_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"c_message_sid",c_sid_array,LONG_BUFF_LEN)||
		!huayi_json_get_str(request_root,"phone_num",phone_number,MAX_PHONE_NUMBER_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!huayiSqlite3Open(&my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}


	if(strlen(c_sid_array))
	{
		while(huayi_get_nth_value(i++, c_sid_array, WEB_DEFAULT_DELIMIT, c_sid, sizeof(c_sid)))
		{	
			snprintf(query,MAX_SQLITE3_QUERY_LEN,"delete from `%s` where %s='%s';",phone_number,SQLITE3_SMS_CHILD_ID_KEY_NAME,c_sid);
			
			if(!huayiSqlite3Exec(my_con,query))
			{
				huayiSqlite3Close(my_con);
				return FCGI_RESP_CODE_SERVER_FAILURE;
			}
			del_count++;
		}
	}

	snprintf(query,MAX_SQLITE3_QUERY_LEN,"select %s from `%s` where %s='%s';",SQLITE3_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,phone_number);

	huayiSqlite3GetInt(my_con,query,&sms_count);

	if(del_count>0)
	{
		if(sms_count-del_count==0)
		{
			huayiSqlite3Exec(my_con,"delete from `%s` where %s='%s';",SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,phone_number);
			huayiSqlite3Exec(my_con,"drop table `%s`;",phone_number);
		}else
		{
			/*huayiSqlite3Exec(my_con,"update `%s` inner join(select %s,%s,%s from `%s` order by %s desc limit 1) c set %s.%s=c.%s,%s.%s=c.%s,%s.%s=%s.%s-%d,%s.%s=c.%s where %s.%s='%s';",
																			SMS_HUAYI_TABLE,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
																			SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,phone_number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_CHILD_ID_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMS_COUNT_KEY_NAME,del_count,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_LASTTIME_KEY_NAME,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,phone_number);*/
			memset(query,0,sizeof(query));
			snprintf(query,sizeof(query),"select %s,%s,%s from `%s` order by %s desc limit 1;",
														SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
														SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,phone_number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME);
			sqlite3_get_table(my_con,query,&result,&nrow,&ncol,&zErrMsg);
			if(NULL != zErrMsg || nrow < 1 || !ncol)
			{
				goto end;
			}

			sql_c_sid = atoi(result[ncol]);
			sprintf(data_msg,result[ncol+1]);
			sprintf(date_str,result[ncol+2]);

			memset(query,0,sizeof(query));
			snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",SQLITE3_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,phone_number);
			huayiSqlite3GetInt(my_con,query,&sql_c_sid);

			memset(query,0,sizeof(query));
			snprintf(query,sizeof(query),"update `%s` set %s=%d,%s='%s',%s=%d,%s='%s' where %s='%s';",
													SMS_HUAYI_TABLE,
													SQLITE3_SMS_CHILD_ID_KEY_NAME,sql_c_sid,
													SQLITE3_SMS_MSG_KEY_NAME,data_msg,
													SQLITE3_SMS_COUNT_KEY_NAME,sql_c_sid - del_count,
													SQLITE3_SMS_LASTTIME_KEY_NAME,date_str,
													SQLITE3_SMSNUMBER_KEY_NAME,phone_number);
			if(!huayiSqlite3Exec(my_con,query))
			{
				goto end;
			}															
		}
		system("huayi-reset save_database");
	}

end:
	sqlite3_free_table(result);
	huayiSqlite3Close(my_con);
	return FCGI_RESP_CODE_SUCCESS;

}

int cgi_send_sms_messages(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	sqlite3 *my_con = NULL;
	char sms_msg[MAX_SMS_MSG_LEN] = {0};
	char sms_msg_escape_characters[MAX_SMS_ESCAPE_CHARACTER_LEN] = {0};
	char phone_number_array[LONG_BUFF_LEN]= {0};
	char phone_number[MAX_PHONE_NUMBER_LEN] = {0};
	char center_number[AT_VALUE_BUFF_LEN] ={0};
	int i = 0;
	char query[MAX_SQLITE3_QUERY_LEN] = {0};
	int send_count = 0;
	int success_count = 0;
	int fail_count = 0;
	int sim_state = 0;
	int sid =0;
	int ret = 0;
	cJSON *result_root = NULL;

	char **result = NULL;
	char *zErrMsg = NULL;
	int nrow = 0,ncol = 0; 
	int c_sid = 0;
	int sms_count = 0;
	char date_str[MAX_SQLITE3_VALUE_LEN] = {0};
	char data_msg[MAX_SQLITE3_QUERY_LEN] = {0};

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"message_content",sms_msg,MAX_SMS_MSG_LEN)||
		!huayi_json_get_str(request_root,"phone_num",phone_number_array,LONG_BUFF_LEN))
	{
		return FCGI_RESP_CODE_DATA_MISS;	
	}

	if(!huayiSqlite3Open(&my_con))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}

	//sql_hex = huayiSqlite3HexString(sms_msg,strlen(sms_msg));
	ret = huayiHanldeEscapeCharacter(sms_msg,'\'',data_msg);
	if(HUAYI_FALSE == ret)
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	ret = 0;

	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","center_number",center_number);

	if(strlen(phone_number_array))
	{
		while(huayi_get_nth_value(i++, phone_number_array, WEB_DEFAULT_DELIMIT, phone_number, sizeof(phone_number)))
		{
			if(0 == strlen(phone_number))
			{
				continue;
			}
			send_count++;
			ret = send_sms(phone_number, center_number, sms_msg);
			if(0 == ret)
			{
				success_count++;
				if(!huayiSqlite3CheckTableExist(my_con,phone_number))
				{
					snprintf(query,sizeof(query),"create table if not exists `%s` ("
						"%s integer primary key autoincrement,"
						"%s varchar(%d),"
						"%s timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,"
						"%s int unsigned not null default 0,"
						"UNIQUE (%s)"
						");",phone_number,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,MAX_SMS_MSG_LEN,
						SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,SQLITE3_SMS_SRC_KEY_NAME,SQLITE3_SMS_CHILD_ID_KEY_NAME);

						huayiSqlite3Exec(my_con,query);	
				}

				//huayiSqlite3Exec(my_con,"insert into `%s` (%s,%s,%s) values('%s',datetime('now','localtime'),1);",phone_number,SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,SQLITE3_SMS_SRC_KEY_NAME,data_msg);
				memset(query,0,sizeof(query));
				snprintf(query,sizeof(query),"insert into `%s` (%s,%s,%s) values('%s',datetime('now','localtime'),1);",phone_number,SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,SQLITE3_SMS_SRC_KEY_NAME,data_msg);
	
				ret = sqlite3_exec(my_con,query, NULL, NULL, &zErrMsg);
				if(ret != SQLITE_OK)
				{
					printf("%s sql fail,error code:%d, erro:%s\n",__FUNCTION__, ret, zErrMsg);	
					goto end;
				}
				snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",SQLITE3_SMSID_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,phone_number);
				huayiSqlite3GetInt(my_con,query,&sid);

				if(sid == 0)
				{
					huayiSqlite3Exec(my_con,"insert into `%s` (%s,%s,%s,%s,%s) select %s,%s,1,%s,%s from `%s` order by %s desc limit 1;",SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,
																				SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_COUNT_KEY_NAME,SQLITE3_SMS_LASTTIME_KEY_NAME,
																				SQLITE3_SMS_CHILD_ID_KEY_NAME,phone_number,SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,
																				SQLITE3_SMS_CHILD_ID_KEY_NAME,phone_number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME);
				}else
				{
					/*huayiSqlite3Exec(my_con,"update `%s` inner join(select %s,%s,%s from `%s` order by %s desc limit 1) c set %s.%s=c.%s,%s.%s=c.%s,%s.%s=%s.%s+1,%s.%s=c.%s where %s.%s=%d;",
																			SMS_HUAYI_TABLE,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
																			SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,phone_number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_CHILD_ID_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMS_COUNT_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_LASTTIME_KEY_NAME,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMSID_KEY_NAME,sid);*/

					memset(query,0,sizeof(query));
					snprintf(query,sizeof(query),"select %s,%s,%s from `%s` order by %s desc limit 1;",
																SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
																SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,phone_number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME);
					sqlite3_get_table(my_con,query,&result,&nrow,&ncol,&zErrMsg);
					if(NULL != zErrMsg || nrow < 1 || !ncol)
					{
						goto end;
					}

					c_sid = atoi(result[ncol]);
					sprintf(data_msg,result[ncol+1]);
					sprintf(date_str,result[ncol+2]);

					memset(query,0,sizeof(query));
					snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",SQLITE3_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,phone_number);
					huayiSqlite3GetInt(my_con,query,&sms_count);

					memset(query,0,sizeof(query));
					snprintf(query,sizeof(query),"update `%s` set %s=%d,%s='%s',%s=%d,%s='%s' where %s=%d;",
															SMS_HUAYI_TABLE,
															SQLITE3_SMS_CHILD_ID_KEY_NAME,c_sid,
															SQLITE3_SMS_MSG_KEY_NAME,data_msg,
															SQLITE3_SMS_COUNT_KEY_NAME,sms_count +1,
															SQLITE3_SMS_LASTTIME_KEY_NAME,date_str,
															SQLITE3_SMSID_KEY_NAME,sid);
					ret = sqlite3_exec(my_con,query, NULL, NULL, &zErrMsg);
					if(ret != SQLITE_OK)
					{
						printf("%s sql fail,error code:%d, erro:%s\n",__FUNCTION__, ret, zErrMsg);	
						goto end;
					}
		
				}
			}else
			{
				fail_count++;
			}
		}
	}

	huayi_uci_get_int(PKG_STATUS_MODEM,"modem","sim_ready",&sim_state);
	if (sim_state > MODEM_SIN_PIN || strlen(center_number) == 0)
		sim_state = MODEM_SIN_NOT_INSERTED;

	result_root = cJSON_CreateObject();
	cJSON_AddNumberToObject(result_root,"send_count",send_count);

	cJSON_AddNumberToObject(result_root,"success_count",success_count);

	cJSON_AddNumberToObject(result_root,"fail_count",fail_count);

	cJSON_AddNumberToObject(result_root,"sim_state",sim_state);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	if(success_count > 0)
	{
		system("huayi-reset save_database");
	}

	
	return FCGI_RESP_CODE_SUCCESS;
end:
	sqlite3_free_table(result);
	return FCGI_RESP_CODE_SERVER_FAILURE;
}
#endif

int cgi_get_module_pin_state(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	int sim_state = 0;
	int pin_num = 0;
	int puk_num = 0;
	char module_status[OPTION_STR_LEN] = {0};
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","modemModel",module_status);
	if (strlen(module_status) == 0)
	{
		huayi_add_switch_to_json(result_root,"module_ready",0);
	}
	else
	{
		huayi_add_switch_to_json(result_root,"module_ready",1);
	}

	atcmd_get_cpin(&sim_state);
	
	if (sim_state > MODEM_SIN_PUK)
		sim_state = MODEM_SIN_NOT_INSERTED;
	
	atcmd_get_cpinr(&pin_num,&puk_num);

	cJSON_AddNumberToObject(result_root,"sim_state",sim_state);

	cJSON_AddNumberToObject(result_root,"last_times",pin_num);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}


int cgi_set_sim_lock_set_pin(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	char pin_code[SMALL_STR_LEN+1] ={0};
	int result = 0;
	int sim_clck = 0;
	int old_sim_clck = 0;
	int sim_state = LTE_TRUE;
	int pin_num = 0;
	int puk_num = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"pin_code",pin_code,(SMALL_STR_LEN+1))||
		!huayi_json_get_int(request_root,"sim_clck",&sim_clck))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	atcmd_get_clck(&old_sim_clck);
	
	atcmd_get_cpin(&sim_state);
	if (sim_state == MODEM_SIN_PIN)
	{
		result = atcmd_set_cpin(pin_code);
	}
	
	if (result == LTE_TRUE && old_sim_clck != sim_clck)
	{
		result = atcmd_set_clck(sim_clck,pin_code);
	}
	
	if (result == LTE_FALSE)
	{	
		atcmd_get_cpinr(&pin_num,&puk_num);
		
		cJSON_AddNumberToObject(response_root,"last_times",pin_num);
		
		return FCGI_RESP_CODE_PIN_CODE_ERROR;
	}
	*lock_time = HUAYI_RELOAD_4G_MODE;
	system("huayi-reset lte4g parameter 1");
	system("huayi_cmd set modem lteMode 1");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_get_sim_clck(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root =NULL;
	int sim_clck = 0;
	int pin_num = 0;
	int puk_num = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();

	atcmd_get_clck(&sim_clck);

	huayi_add_switch_to_json(result_root,"sim_clck",sim_clck);

	atcmd_get_cpinr(&pin_num,&puk_num);
	
	cJSON_AddNumberToObject(result_root,"last_times",pin_num);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}
	
int cgi_set_pin_clck(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	char pin_code[SMALL_STR_LEN+1] ={0};
	int sim_clck = 0;
	int result = 0;
	int pin_num = 0;
	int puk_num = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"pin_code",pin_code,(SMALL_STR_LEN+1))||
		!huayi_json_get_int(request_root,"sim_clck",&sim_clck))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result = atcmd_set_clck(sim_clck,pin_code);
	
	if (result == LTE_TRUE)
	{
		*lock_time = HUAYI_RELOAD_4G_MODE;
		system("huayi_cmd set modem lteMode 1");
		system("huayi-reset lte4g parameter 1");
		return FCGI_RESP_CODE_SUCCESS;
	}
	else
	{	
		atcmd_get_cpinr(&pin_num,&puk_num);
		
		cJSON_AddNumberToObject(response_root,"last_times",pin_num);
		return FCGI_RESP_CODE_PIN_CODE_ERROR;
	}
}

int cgi_change_module_pin_code(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	char old_pin_code[SMALL_STR_LEN+1] = {0};
	char new_pin_code[SMALL_STR_LEN+1] = {0};
	int result = 0;
	int pin_num = 0;
	int puk_num = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_str(request_root,"old_pin_code",old_pin_code,(SMALL_STR_LEN+1))||
		!huayi_json_get_str(request_root,"new_pin_code",new_pin_code,(SMALL_STR_LEN+1)))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	/*To modify SIM Card pin code, you need lock sim card*/
	result = atcmd_set_clck(1, old_pin_code);
	if (result == LTE_TRUE)
	{
		/*modify pin code*/
		result = atcmd_set_cpwd(old_pin_code,new_pin_code);
		if (result == LTE_TRUE)
		{
			//system("huayi_cmd set modem lteMode 1");
			goto update_succ;
		}
		else
		{
			goto update_fail;
		}
		
	} 
	else
	{
		goto update_fail;
	}
update_succ:
	/*unlock SIM Card*/
	result = atcmd_set_clck(0, new_pin_code);
	if (result == LTE_TRUE)
	{
		system("huayi_cmd set modem lteMode 1");
		return FCGI_RESP_CODE_SUCCESS;
	}

update_fail:		
	atcmd_get_cpinr(&pin_num,&puk_num);
	
	cJSON_AddNumberToObject(response_root,"last_times",pin_num);
	return FCGI_RESP_CODE_PIN_CODE_ERROR;
}

int cgi_get_module_apn_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON *result_root =NULL;
	int sim_state = 0;
	int module_ready = 0;
	char module_status[OPTION_STR_LEN] = {0};
	char wan_type[OPTION_STR_LEN] = {0};
	char auth_type[OPTION_STR_LEN] = {0};
	char apn_name[OPTION_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	int  use_apn = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	result_root = cJSON_CreateObject();
	
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","modemModel",module_status);
	if (strlen(module_status) == 0)
	{
		module_ready = 0;
	}
	else
	{
		module_ready = 1;
	}
	huayi_add_switch_to_json(result_root,"module_ready",module_ready);

	//atcmd_get_cpin(&sim_state);
	huayi_uci_get_int(PKG_STATUS_MODEM,"modem","sim_ready",&sim_state);
	if (sim_state > MODEM_SIN_PIN)
		sim_state = MODEM_SIN_NOT_INSERTED;
	cJSON_AddNumberToObject(result_root,"sim_state",sim_state);

	if((sim_state != MODEM_SIN_READY)||(module_ready != 1))
	{
		cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
		return FCGI_RESP_CODE_SUCCESS;
	}

	huayi_uci_get_int(PKG_MODEM,"status","mode",&use_apn);
	huayi_uci_get_str(PKG_MODEM,"status","validation",auth_type);
	huayi_uci_get_str(PKG_MODEM,"status","apn",apn_name);
	huayi_uci_get_str(PKG_MODEM,"status","user4g",username);
	huayi_uci_get_str(PKG_MODEM,"status","pass4g",password);
	
	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	cJSON_AddStringToObject(result_root,"wan_mode",wan_type);

	cJSON_AddNumberToObject(result_root,"use_apn",use_apn);
	cJSON_AddStringToObject(result_root,"auth_type",auth_type);
	cJSON_AddStringToObject(result_root,"apn_name",apn_name);
	cJSON_AddStringToObject(result_root,"apn_usr",username);
	cJSON_AddStringToObject(result_root,"apn_pwd",password);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_set_module_apn_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	int  auth_type = 0;
	char apn_name[OPTION_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	int  use_apn = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	if(!huayi_json_get_int(request_root,"use_apn",&use_apn))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	huayi_json_get_int(request_root,"auth_type",&auth_type);
	
	huayi_json_get_str(request_root,"apn_name",apn_name,OPTION_STR_LEN);
	
	huayi_json_get_str(request_root,"apn_usr",username,OPTION_STR_LEN);
	
	huayi_json_get_str(request_root,"apn_pwd",password,OPTION_STR_LEN);

	huayi_uci_set_int(PKG_MODEM,"status","mode",use_apn);
	huayi_uci_set_int(PKG_MODEM,"status","validation",auth_type);
	huayi_uci_set_str(PKG_MODEM,"status","apn",apn_name);
	huayi_uci_set_str(PKG_MODEM,"status","user4g",username);
	huayi_uci_set_str(PKG_MODEM,"status","pass4g",password);
	
	huayi_uci_commit(PKG_MODEM);

	system("huayi-reset lte4g parameter 1");
	
	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");
	
	*lock_time = HUAYI_RELOAD_NETWORK_TIME;
	
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_get_overview_info(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON *result_root = NULL;
	int  wan_up = 0;
	int  wan_link = 0;
	int  sim_state = 0;
	char wan_type[OPTION_STR_LEN] = {0};
	char mobile_type[OPTION_STR_LEN] = {0};
	char mobile_network[OPTION_STR_LEN] = {0};
	int  signal = 0, signal4g = 0;
	int  wan_uptime = 0;
	int  uptime = 0;
	int  client_num = 0;
	int  tx_bps = 0;
	int  rx_bps = 0;
	unsigned long long rx_bbbps = 0;
	unsigned long long tx_bbbps = 0;
	unsigned long long rx_bytes = 0;
	unsigned long long tx_bytes = 0;
	int  wifi_2g = 0;
	int  wifi_5g = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&wan_link);

	huayi_uci_get_int(PKG_STATUS_MODEM,"modem","sim_ready",&sim_state);
	
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","network",mobile_network);
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","mobile_type",mobile_type);
	
	huayi_uci_get_int(PKG_STATUS_MODEM,"modem","signal",&signal);
	signal = signal/5 - 120;

	huayi_uci_get_int(PKG_STATUS_MODEM,"modem","signal",&signal4g);
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_uptime",&wan_uptime);
	huayi_uci_get_int(PKG_STATUS_SYSTEM,"sys","uptime",&uptime);

	huayi_uci_get_int(PKG_WIRELESS,"radio0","disabled",&wifi_2g);
	wifi_2g = wifi_2g?0:1;

	huayi_uci_get_int(PKG_WIRELESS,"radio1","disabled",&wifi_5g);
	wifi_5g = wifi_5g?0:1;
	
	result_root = cJSON_CreateObject();

	cJSON_AddStringToObject(result_root,"wan_mode",wan_type);

	if (!strcmp(wan_type,WAN_TYPE_4G_MOBILE))
	{
		if (sim_state != MODEM_SIN_READY)
			wan_up = 0;
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_WIRED_PRIOROTY))
	{
		huayi_uci_get_int(PKG_STATUS_BLEND, "wired", "link", &wan_link);
		if (wan_link == 0)
		{
			if (sim_state != MODEM_SIN_READY)
			{
				wan_up = 0;
			}
		}
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_REP4G_PRIOROTY))
	{
		huayi_uci_get_int(PKG_STATUS_BLEND, "wisp", "link", &wan_link);
		if (wan_link == 0)
		{
			huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &sim_state);
			//if (sim_state != MODEM_SIN_READY)
			if(sim_state == 0)
			{
				wan_up = 0;
			}
		}
	}
#endif
	else
	{
		if (wan_link == 0)
			wan_up = 0;
	}

	cJSON_AddNumberToObject(result_root,"wan_up",wan_up);

	if(wan_up == 0)
	{
		cJSON_AddNumberToObject(result_root,"wan_uptime",0);	
	}else
	{		
		cJSON_AddNumberToObject(result_root,"wan_uptime",(uptime-wan_uptime));
	}
	
	cJSON_AddNumberToObject(result_root,"sim_state",sim_state);

	cJSON_AddStringToObject(result_root,"mobile_network",mobile_network);

	cJSON_AddStringToObject(result_root,"mobile_type",mobile_type);

	cJSON_AddNumberToObject(result_root,"signal",signal);
	cJSON_AddNumberToObject(result_root,"signal_4g",signal4g);

	get_active_arp(1,NULL,&client_num);
	cJSON_AddNumberToObject(result_root,"client_sum",client_num);
	
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","tx_bps",&tx_bps);
	tx_bbbps = ((unsigned long long)tx_bps * 8);
	cJSON_AddNumberToObject(result_root,"tx_bps",tx_bbbps);
	
	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","rx_bps",&rx_bps);
	rx_bbbps = ((unsigned long long)rx_bps * 8);
	cJSON_AddNumberToObject(result_root,"rx_bps",rx_bbbps);

	huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","rx_bytes",&rx_bytes);

	cJSON_AddNumberToObject(result_root,"rx_bytes",rx_bytes);
	
	huayi_uci_get_long_long(PKG_STATUS_NETWORK,"wan","tx_bytes",&tx_bytes);

	cJSON_AddNumberToObject(result_root,"tx_bytes",tx_bytes);

	cJSON_AddNumberToObject(result_root,"wifi_2g",wifi_2g);

	cJSON_AddNumberToObject(result_root,"wifi_5g",wifi_5g);

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}

FCGI_BOOL set_wisp_mode(cJSON *request_root)
{
	int 	channel  = 0, ht_extcha = 1,old_sys_mode = 0;
	char	authmode_uci[OPTION_STR_LEN] = {0},encryption_uci[OPTION_STR_LEN] = {0};
	char	wan_ifname[OPTION_STR_LEN] = {0};
	char	wan_lan_ifname[OPTION_STR_LEN] = {0};
	char	bssid[OPTION_STR_LEN] = {0},ssid[OPTION_STR_LEN] = {0};
	char	authmode[OPTION_STR_LEN] = {0},encryption[OPTION_STR_LEN] = {0},key[TEMP_STR_LEN] = {0};
	char	def_lan_ip[IPV4_STR_LEN] = {0};
	char	apcli_ifname[OPTION_STR_LEN] = {0};
	char	resultbuf[OPTION_STR_LEN] = {0};
	char	get_cmd[OPTION_STR_LEN] = {0};
	char cmd[CMD_STR_LEN] = {0};
	char apcli_ssid[64],disabled[64],apcli_enable[64];
	
		
	if(!huayi_json_get_int(request_root,"channel",&channel) || \
		!huayi_json_get_str(request_root,"bssid",bssid,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"ssid",ssid,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"auth_mode",authmode,OPTION_STR_LEN) || \
		!huayi_json_get_str(request_root,"encryption",encryption,OPTION_STR_LEN) || \
		!huayi_json_get_int(request_root,"ht_extcha",&ht_extcha))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_json_get_str(request_root,"key",key,TEMP_STR_LEN);

	if ((strncmp(authmode,"NONE",strlen("NONE ")) == 0) || (strncmp(authmode,"OPEN",strlen("OPEN ")) == 0))
	{
		strcpy(authmode_uci,"OPEN");
		strcpy(encryption_uci,"NONE");
	}
	else
	{
		strcpy(authmode_uci,get_wireless_config_authmode(authmode));
		strcpy(encryption_uci,get_wireless_config_encryptype(encryption));
	}
	
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	if (channel > 14)
	{

		//huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"wan","ifname", "apclii0");
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s", "apclii0");
		system(cmd);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled","0");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"network","wan");
		strcpy(wan_ifname, W58G_REP_IFNAME);
		
		huayi_uci_set_int(PKG_WIRELESS,W58G_DEV,"channel",channel);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_bssid",bssid);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",ssid);
		
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_authmode",authmode_uci);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_encryptype",encryption_uci);


		FASTCGI_LOG("===wisp2.4g close===");
		FASTCGI_LOG("###wisp5G###channel:%d,bssid:%s,ssid:%s,authmode_uci:%s,encryption_uci:%s", channel,bssid,ssid,authmode_uci,encryption_uci);

		if(strcmp("NONE",encryption_uci) == 0)
		{
		//huayi_uci_delete_option(PKG_WIRELESS,W58G_REP_IFACE,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_wpapsk",key);
		}
		huayi_uci_get_str(PKG_WIRELESS,W58G_REP_IFACE,"ifname",apcli_ifname);
	}
	else
#endif
	{
		//huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"wan","ifname", "apcli0");
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s", "apcli0");
		system(cmd);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",HUAYI_RANDOM_APCLI_NAME);
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled","1");
		huayi_uci_set_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","0");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_enable","1");
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"network","wan");
		strcpy(wan_ifname, W24G_REP_IFNAME);
		
		huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"channel",channel);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_bssid",bssid);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_ssid",ssid);
		
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_authmode",authmode_uci);
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_encryptype",encryption_uci);

		FASTCGI_LOG("===wisp5g close===");
		FASTCGI_LOG("###wisp2.4G###channel:%d,bssid:%s,ssid:%s,authmode_uci:%s,encryption_uci:%s", channel,bssid,ssid,authmode_uci,encryption_uci);
		if(strcmp("NONE",encryption_uci) == 0)
		{
			//huayi_uci_delete_option(PKG_WIRELESS,W24G_REP_IFACE,"key");
		}
		else
		{
			huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_wpapsk",key);
		}
		huayi_uci_get_str(PKG_WIRELESS,W24G_REP_IFACE,"ifname",apcli_ifname);
	}

	huayi_uci_commit(PKG_WIRELESS);
	huayi_uci_get_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_ssid",apcli_ssid);
	huayi_uci_get_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled",disabled);
	huayi_uci_get_str(PKG_WIRELESS,W24G_REP_IFACE,"apcli_enable",apcli_enable);
	FASTCGI_LOG("==W24G_REP_IFNAME==apcli_ssid:%s,disabled:%s,apcli_enable:%s===",apcli_ssid,disabled,apcli_enable);
	huayi_uci_get_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_ssid",apcli_ssid);
	huayi_uci_get_str(PKG_WIRELESS,W58G_REP_IFACE,"disabled",disabled);
	huayi_uci_get_str(PKG_WIRELESS,W58G_REP_IFACE,"apcli_enable",apcli_enable);
	FASTCGI_LOG("==W58G_REP_IFACE==apcli_ssid:%s,disabled:%s,apcli_enable:%s==",apcli_ssid,disabled,apcli_enable);


	huayi_uci_set_int(PKG_WIRELESS,W24G_DEV,"ht_extcha",ht_extcha);
	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&old_sys_mode);
	if(old_sys_mode != HUAYI_ROUTE_MODE && old_sys_mode != HUAYI_WISP_MODE)   //AP\REPEATER   ---> wisp MODE
	{	
		huayi_uci_get_str(PKG_NETWORK,"lan","def_ip",def_lan_ip);
		huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_lan_ifname",wan_lan_ifname);
			
		huayi_uci_set_str(PKG_NETWORK,"lan","ipaddr",def_lan_ip);
		huayi_uci_set_str(PKG_NETWORK,"lan","netmask",HUAYI_DEFAULT_NET_MASK);
		huayi_uci_set_str(PKG_NETWORK,"lan","proto","static");
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",wan_lan_ifname);
	}
	
	huayi_uci_set_str(PKG_DHCP,"lan","ignore","0");
	
	huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);

	huayi_uci_set_str(PKG_NETWORK,"wan","proto","dhcp");
	huayi_uci_delete_option(PKG_NETWORK,"wan","gateway");
	huayi_uci_delete_option(PKG_NETWORK,"wan","dns");

	/*keep original ssid and passphrase*/
	huayi_uci_set_str(PKG_WIRELESS,W24G_IFACE,"network","lan");

	sprintf(get_cmd,"cat /sys/class/net/%s/address | tr '[a-z]' '[A-Z]'",apcli_ifname);
	get_cmd_result(get_cmd,resultbuf,sizeof(resultbuf));
	if(strlen(resultbuf))
	{
		huayi_uci_set_str(PKG_NETWORK,"wan","macaddr",resultbuf);
	}

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_set_str(PKG_WIRELESS,W58G_IFACE,"network","lan");
#endif
#if defined(CONFIG_PACKAGE_HWNAT)
	huayi_uci_set_int(PKG_HWNAT,"global","enabled",1);
	huayi_uci_commit(PKG_HWNAT);
#endif
	
	return FCGI_TRUE;
}


int cgi_get_wan_or_wan4g_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	cJSON 	*result_root =NULL;
	char 	proto[OPTION_STR_LEN] = {0};
	char 	ipaddr[IPV4_STR_LEN] = {0};
	char 	netmask[IPV4_STR_LEN] = {0};
	char	gateway[IPV4_STR_LEN] = {0};
	char 	temp_dns[OPTION_STR_LEN] = {0};
	char 	dns[IPV4_STR_LEN] = {0};
	cJSON 	*dns_array = NULL;
	int 	i = 0;
	char 	username[PASSWD_LONG_STR_LEN] = {0};
	char 	password[PASSWD_LONG_STR_LEN] = {0};
	char 	ac[OPTION_STR_LEN] = {0};
	char 	service[OPTION_STR_LEN] = {0};
	char 	hostname[OPTION_STR_LEN] = {0};	
	char 	macaddr[MAC_STR_LEN] = {0};
	char 	def_macaddr[MAC_STR_LEN] = {0};
	char 	wan_type[OPTION_STR_LEN] = {0};
	int  	ppp_mtu = 0;
	int  	cc_mtu = 0;
	int 	link=0;
	char 	auth_type[OPTION_STR_LEN] = {0};
	char 	apn_name[OPTION_STR_LEN] = {0};
	char 	apnusername[OPTION_STR_LEN] = {0};
	char 	apnpassword[OPTION_STR_LEN] = {0};
	int  	use_apn = 0;
	char	detectip[OPTION_STR_LEN] = {0};
	int     detect_time = 0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	cJSON_AddStringToObject(result_root,"wan_mode",wan_type);

	if (!strcmp(wan_type,WAN_TYPE_4G_MOBILE))
	{
		huayi_uci_get_str(PKG_NETWORK,"wan4g","proto",proto);
		cJSON_AddStringToObject(result_root,"proto",proto);
	
		huayi_uci_get_str(PKG_NETWORK,"wan4g","hostname",hostname);
		if(strlen(hostname) <= 0)
		{
			huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
		}

		cJSON_AddStringToObject(result_root,"hostname",hostname);

		huayi_uci_get_str(PKG_NETWORK,"wan4g","macaddr",macaddr);
		cJSON_AddStringToObject(result_root,"mac_addr",macaddr);
		
		huayi_uci_get_str(PKG_NETWORK,"wan4g","def_macaddr",def_macaddr);
		cJSON_AddStringToObject(result_root,"def_mac_addr",def_macaddr);
		
		huayi_uci_get_int(PKG_NETWORK,"wan4g","ppp_mtu",&ppp_mtu);
		cJSON_AddNumberToObject(result_root,"ppp_mtu",ppp_mtu);
		
		huayi_uci_get_int(PKG_NETWORK,"wan4g","cc_mtu",&cc_mtu);
		cJSON_AddNumberToObject(result_root,"cc_mtu",cc_mtu);
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_WIRED_PRIOROTY)){
		huayi_uci_get_str(PKG_NETWORK,"wan4g","proto",proto);
		cJSON_AddStringToObject(result_root,"proto_4g",proto);

		huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
		cJSON_AddStringToObject(result_root,"proto",proto);
	
		huayi_uci_get_str(PKG_NETWORK,"wan","hostname",hostname);
		if(strlen(hostname) <= 0)
		{
			huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
		}

		cJSON_AddStringToObject(result_root,"hostname",hostname);

		huayi_uci_get_str(PKG_NETWORK,"wan","macaddr",macaddr);
		cJSON_AddStringToObject(result_root,"mac_addr",macaddr);
		
		huayi_uci_get_str(PKG_NETWORK,"wan","def_macaddr",def_macaddr);
		cJSON_AddStringToObject(result_root,"def_mac_addr",def_macaddr);
		
		huayi_uci_get_int(PKG_NETWORK,"wan","ppp_mtu",&ppp_mtu);
		cJSON_AddNumberToObject(result_root,"ppp_mtu",ppp_mtu);
		
		huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);
		cJSON_AddNumberToObject(result_root,"cc_mtu",cc_mtu);

		//huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
		//cJSON_AddStringToObject(result_root,"detect_ip",detectip);
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_REP4G_PRIOROTY)){
		huayi_uci_get_str(PKG_NETWORK,"wan4g","proto",proto);
		cJSON_AddStringToObject(result_root,"proto_4g",proto);

		huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
		cJSON_AddStringToObject(result_root,"proto",proto);
	
		huayi_uci_get_str(PKG_NETWORK,"wan","hostname",hostname);
		if(strlen(hostname) <= 0)
		{
			huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
		}
		
		cJSON_AddStringToObject(result_root,"hostname",hostname);
		//huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
		//cJSON_AddStringToObject(result_root,"detect_ip",detectip);

		huayi_uci_get_str(PKG_NETWORK,"wan","macaddr",macaddr);
		cJSON_AddStringToObject(result_root,"mac_addr",macaddr);
		
		huayi_uci_get_str(PKG_NETWORK,"wan","def_macaddr",def_macaddr);
		cJSON_AddStringToObject(result_root,"def_mac_addr",def_macaddr);
		
		huayi_uci_get_int(PKG_NETWORK,"wan","ppp_mtu",&ppp_mtu);
		cJSON_AddNumberToObject(result_root,"ppp_mtu",ppp_mtu);
		
		huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);
		cJSON_AddNumberToObject(result_root,"cc_mtu",cc_mtu);

		//huayi_uci_get_int(PKG_SYSTEM,"mode","detect_time", &detect_time);
		//cJSON_AddNumberToObject(result_root,"detect_time",detect_time);
	}
#endif
	else
	{
		//huayi_uci_get_int(PKG_SYSTEM,"mode","detect_time", &detect_time);
		//cJSON_AddNumberToObject(result_root,"detect_time",detect_time);
		
		//huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
		//cJSON_AddStringToObject(result_root,"detect_ip",detectip);
		huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
		cJSON_AddStringToObject(result_root,"proto",proto);
	
		huayi_uci_get_str(PKG_NETWORK,"wan","hostname",hostname);
		if(strlen(hostname) <= 0)
		{
			huayi_uci_get_str(PKG_SYSTEM,"system","hostname",hostname);
		}

		cJSON_AddStringToObject(result_root,"hostname",hostname);

		huayi_uci_get_str(PKG_NETWORK,"wan","macaddr",macaddr);
		cJSON_AddStringToObject(result_root,"mac_addr",macaddr);
		
		huayi_uci_get_str(PKG_NETWORK,"wan","def_macaddr",def_macaddr);
		cJSON_AddStringToObject(result_root,"def_mac_addr",def_macaddr);
		
		huayi_uci_get_int(PKG_NETWORK,"wan","ppp_mtu",&ppp_mtu);
		cJSON_AddNumberToObject(result_root,"ppp_mtu",ppp_mtu);
		
		huayi_uci_get_int(PKG_NETWORK,"wan","cc_mtu",&cc_mtu);
		cJSON_AddNumberToObject(result_root,"cc_mtu",cc_mtu);
	}
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	huayi_uci_get_int(PKG_SYSTEM,"mode","detect_time", &detect_time);
	cJSON_AddNumberToObject(result_root,"detect_time",detect_time);
#endif		
	huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip",detectip);
	cJSON_AddStringToObject(result_root,"detect_ip",detectip);
	
	huayi_uci_get_str(PKG_NETWORK,"wan","ipaddr",ipaddr);
	cJSON_AddStringToObject(result_root,"ip_addr",ipaddr);
	
	huayi_uci_get_str(PKG_NETWORK,"wan","netmask",netmask);
	cJSON_AddStringToObject(result_root,"subnet_mask",netmask);
	
	huayi_uci_get_str(PKG_NETWORK,"wan","gateway",gateway);
	cJSON_AddStringToObject(result_root,"gateway",gateway);

	huayi_uci_get_str(PKG_NETWORK,"wan","dns",temp_dns);
		
	dns_array = cJSON_CreateArray();
	while(huayi_get_nth_value(i++, temp_dns, WEB_SPACE_DELIMIT, dns, sizeof(dns)))
	{
		cJSON_AddItemToArray(dns_array,cJSON_CreateString(dns));
	}
	cJSON_AddItemToObject(result_root,"dns_arr",dns_array);

	huayi_uci_get_str(PKG_NETWORK,"wan","username",username);
	cJSON_AddStringToObject(result_root,"usr",username);

	huayi_uci_get_str(PKG_NETWORK,"wan","password",password);
	cJSON_AddStringToObject(result_root,"pwd",password);

	huayi_uci_get_str(PKG_NETWORK,"wan","ac",ac);
	cJSON_AddStringToObject(result_root,"ac",ac);

	huayi_uci_get_str(PKG_NETWORK,"wan","service",service);
	cJSON_AddStringToObject(result_root,"service",service);

	huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","link",&link);
	
	cJSON_AddNumberToObject(result_root,"link",link);
	
	huayi_uci_get_int(PKG_MODEM,"status","mode",&use_apn);
	huayi_uci_get_str(PKG_MODEM,"status","validation",auth_type);
	huayi_uci_get_str(PKG_MODEM,"status","apn",apn_name);
	huayi_uci_get_str(PKG_MODEM,"status","user4g",apnusername);
	huayi_uci_get_str(PKG_MODEM,"status","pass4g",apnpassword);
	
	huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	cJSON_AddStringToObject(result_root,"wan_mode",wan_type);

	cJSON_AddNumberToObject(result_root,"use_apn",use_apn);
	cJSON_AddStringToObject(result_root,"auth_type",auth_type);
	cJSON_AddStringToObject(result_root,"apn_name",apn_name);
	cJSON_AddStringToObject(result_root,"apn_usr",apnusername);
	cJSON_AddStringToObject(result_root,"apn_pwd",apnpassword);
	
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_set_wan_or_wan4g_config(cJSON *request_root, cJSON *response_root,int *lock_time)
{	
	char wan_type[OPTION_STR_LEN] = {0};
	char wan_ifname[OPTION_STR_LEN] = {0};
	char lan_ifname[OPTION_STR_LEN] = {0};
	char proto[OPTION_STR_LEN] = {0};
	char macaddr[MAC_STR_LEN] = {0};
	char hostname[OPTION_STR_LEN] = {0};
	char detectip[OPTION_STR_LEN] = {0};
	int  detect_time = 0;
	char cmd[CMD_STR_LEN] = {0};
	int cc_mtu = 0;
	int ppp_mtu =0;
	int  auth_type = 0;
	char apn_name[OPTION_STR_LEN] = {0};
	char username[OPTION_STR_LEN] = {0};
	char password[OPTION_STR_LEN] = {0};
	int  use_apn = 0, ret = 0;
	char pmodel[16] = { 0 };
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	if(!huayi_json_get_str(request_root,"wan_mode",wan_type,OPTION_STR_LEN))
	
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}

	huayi_uci_set_str(PKG_SYSTEM,"mode","wan_type",wan_type);

	if (!strcmp(wan_type,WAN_TYPE_4G_MOBILE))
	{
		if(!huayi_json_get_str(request_root,"proto",proto,OPTION_STR_LEN))
		{
			;//return FCGI_RESP_CODE_DATA_MISS;
		}
		
		huayi_uci_delete_option(PKG_NETWORK,"wan","ifname");
		
		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
		if(strlen(lan_ifname)>0)
		{
			huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);
		}
			
		memset(cmd,0,sizeof(cmd));
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
		if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
				&& strncmp("EC200",pmodel, 5) != 0)
		{
			huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device wwan0");
		}
		else
		{
			huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device eth1");
		}
		system(cmd);

		if(!huayi_json_get_int(request_root,"use_apn",&use_apn))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		if(1 == use_apn)
		{
			huayi_json_get_int(request_root,"auth_type",&auth_type);
			huayi_json_get_str(request_root,"apn_name",apn_name,OPTION_STR_LEN);	
			huayi_json_get_str(request_root,"apn_usr",username,OPTION_STR_LEN);	
			huayi_json_get_str(request_root,"apn_pwd",password,OPTION_STR_LEN);

			huayi_uci_set_int(PKG_MODEM,"status","validation",auth_type);
			huayi_uci_set_str(PKG_MODEM,"status","apn",apn_name);
			huayi_uci_set_str(PKG_MODEM,"status","user4g",username);
			huayi_uci_set_str(PKG_MODEM,"status","pass4g",password);
		}
		else
		{
			if(!huayi_json_get_str(request_root,"mac_addr",macaddr,MAC_STR_LEN))
			
			{
				;//return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_str(PKG_NETWORK,"wan4g","macaddr",macaddr);
			if(!huayi_json_get_int(request_root,"cc_mtu",&cc_mtu))
			{
				;//return FCGI_FALSE;
			}

			huayi_uci_set_int(PKG_NETWORK,"wan4g","cc_mtu",cc_mtu);
			huayi_uci_set_int(PKG_NETWORK,"wan4g","mtu",cc_mtu);

			huayi_json_get_str(request_root,"hostname",hostname,OPTION_STR_LEN);
			if(strlen(hostname)>0)
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","hostname",hostname);
			}
		}
		huayi_uci_set_str(PKG_NETWORK,"wan4g","proto",proto);
		huayi_uci_set_int(PKG_MODEM,"status","mode",use_apn);
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFACE,"disabled","1"); //off
		huayi_uci_set_str(PKG_WIRELESS,W24G_REP_IFNAME,"apcli_enable","1");
#endif
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_WIRED_PRIOROTY))
	{
		if(!huayi_json_get_str(request_root,"proto",proto,OPTION_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		{
			huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);

			huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
			huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);

			huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
			//snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend wired_device %s",wan_ifname);
			system(cmd);

			if(!strcmp(proto,HUAYI_NET_PROTO_STATIC))
			{
				if(!huayi_json_get_int(request_root,"cc_mtu",&cc_mtu))
				{
					return FCGI_FALSE;
				}
				if(!set_interface_static_config(request_root,"wan"))
				{
					huayi_uci_revert(PKG_NETWORK);
					return FCGI_RESP_CODE_DATA_MISS;
				}
				huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
				huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
			}else if(!strcmp(proto,HUAYI_NET_PROTO_PPPOE))
			{
				if(!huayi_json_get_int(request_root,"ppp_mtu",&ppp_mtu))
				{
					return FCGI_RESP_CODE_DATA_MISS;
				}
				if(!set_interface_pppoe_config(request_root,"wan"))
				{
					huayi_uci_revert(PKG_NETWORK);
					return FCGI_RESP_CODE_DATA_MISS;
				}
				huayi_uci_set_int(PKG_NETWORK,"wan","ppp_mtu",ppp_mtu);
				huayi_uci_set_int(PKG_NETWORK,"wan","mtu",ppp_mtu);
			}else if(!strcmp(proto,HUAYI_NET_PROTO_DHCP))
			{
				if(!huayi_json_get_int(request_root,"cc_mtu",&cc_mtu))
				{
					return FCGI_FALSE;
				}
				/*if(!set_interface_dhcp_config(request_root,"wan"))
				{
					huayi_uci_revert(PKG_NETWORK);
					return FCGI_RESP_CODE_DATA_MISS;
				}*/	
				huayi_json_get_str(request_root,"hostname",hostname,OPTION_STR_LEN);
				if(strlen(hostname)>0)
				{
					huayi_uci_set_str(PKG_NETWORK, "wan", "hostname",hostname);
				}
				huayi_json_get_str(request_root,"detect_ip", detectip, OPTION_STR_LEN);
				if(strlen(detectip)>0)
				{
					huayi_uci_set_str(PKG_SYSTEM, "mode", "detect_ip", detectip);
				}
				huayi_uci_delete_option(PKG_NETWORK, "wan", "dns");
				huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
				huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
			}
		
			huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
			if(!huayi_json_get_str(request_root,"mac_addr",macaddr,MAC_STR_LEN))
			{
				return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_str(PKG_NETWORK,"wan","macaddr",macaddr);
		}

		{
			memset(cmd,0,sizeof(cmd));
			huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
			if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
				&& strncmp("EC200",pmodel, 5) != 0)
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device wwan0");
			}
			else
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
				snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device eth1");
			}
			system(cmd);

			if(!huayi_json_get_int(request_root,"use_apn",&use_apn))
			{
				return FCGI_RESP_CODE_DATA_MISS;
			}
			if(1 == use_apn)
			{
				huayi_json_get_int(request_root,"auth_type",&auth_type);
				huayi_json_get_str(request_root,"apn_name",apn_name,OPTION_STR_LEN);	
				huayi_json_get_str(request_root,"apn_usr",username,OPTION_STR_LEN);	
				huayi_json_get_str(request_root,"apn_pwd",password,OPTION_STR_LEN);

				huayi_uci_set_int(PKG_MODEM,"status","validation",auth_type);
				huayi_uci_set_str(PKG_MODEM,"status","apn",apn_name);
				huayi_uci_set_str(PKG_MODEM,"status","user4g",username);
				huayi_uci_set_str(PKG_MODEM,"status","pass4g",password);
			}
			else
			{
				if(!huayi_json_get_str(request_root,"mac_addr",macaddr,MAC_STR_LEN))
				
				{
					return FCGI_RESP_CODE_DATA_MISS;
				}
				huayi_uci_set_str(PKG_NETWORK,"wan4g","macaddr",macaddr);
				if(!huayi_json_get_int(request_root,"cc_mtu",&cc_mtu))
				{
					return FCGI_FALSE;
				}

				huayi_uci_set_int(PKG_NETWORK,"wan4g","cc_mtu",cc_mtu);
				huayi_uci_set_int(PKG_NETWORK,"wan4g","mtu",cc_mtu);

				huayi_json_get_str(request_root,"hostname",hostname,OPTION_STR_LEN);
				if(strlen(hostname)>0)
				{
					huayi_uci_set_str(PKG_NETWORK,"wan4g","hostname",hostname);
				}
			}
			huayi_uci_set_int(PKG_MODEM,"status","mode",use_apn);
		}
	
	}
#endif

#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_type,WAN_TYPE_REP4G_PRIOROTY))
	{
		/*if(!huayi_json_get_str(request_root,"proto_4g",proto,OPTION_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}*/
		

		//huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);

		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);

		//huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
		//snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
	//	snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend wisp_device %s",wan_ifname);
		system(cmd);
		
		//huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
		//if(!huayi_json_get_str(request_root,"mac_addr",macaddr,MAC_STR_LEN))
		//{
		//	return FCGI_RESP_CODE_DATA_MISS;
		//}
		//huayi_uci_set_str(PKG_NETWORK,"wan","macaddr",macaddr);
		
		memset(cmd,0,sizeof(cmd));
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "modemModel", pmodel);
		if((strncmp("EC25",pmodel, 4) == 0 || strncmp("EC20",pmodel, 4) == 0)
				&& strncmp("EC200",pmodel, 5) != 0)
		{
			huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","wwan0");
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device wwan0");
		}
		else
		{
			huayi_uci_set_str(PKG_NETWORK,"wan4g","ifname","eth1");
			snprintf(cmd,CMD_STR_LEN,"huayi_cmd set blend cpe_device eth1");
		}
		system(cmd);
		
		huayi_json_get_str(request_root,"detect_ip", detectip, OPTION_STR_LEN);
		if(strlen(detectip)>0)
		{
			huayi_uci_set_str(PKG_SYSTEM, "mode", "detect_ip", detectip);
		}
		huayi_json_get_int(request_root,"detect_time", &detect_time);
		huayi_uci_set_int(PKG_SYSTEM, "mode", "detect_time", detect_time);
		ret = set_wisp_mode(request_root);
		if(FCGI_TRUE != ret)
			return ret;

		if(!huayi_json_get_int(request_root,"use_apn",&use_apn))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		if(1 == use_apn)
		{
			huayi_json_get_int(request_root,"auth_type",&auth_type);
			huayi_json_get_str(request_root,"apn_name",apn_name,OPTION_STR_LEN);	
			huayi_json_get_str(request_root,"apn_usr",username,OPTION_STR_LEN); 
			huayi_json_get_str(request_root,"apn_pwd",password,OPTION_STR_LEN);

			huayi_uci_set_int(PKG_MODEM,"status","validation",auth_type);
			huayi_uci_set_str(PKG_MODEM,"status","apn",apn_name);
			huayi_uci_set_str(PKG_MODEM,"status","user4g",username);
			huayi_uci_set_str(PKG_MODEM,"status","pass4g",password);
		}
		else
		{
			//if(!huayi_json_get_str(request_root,"mac_addr",macaddr,MAC_STR_LEN))
			//{
			//	return FCGI_RESP_CODE_DATA_MISS;
			//}
			//huayi_uci_set_str(PKG_NETWORK,"wan4g","macaddr",macaddr);
			//if(!huayi_json_get_int(request_root,"cc_mtu",&cc_mtu))
			//{
			//	return FCGI_FALSE;
			//}

			//huayi_uci_set_int(PKG_NETWORK,"wan4g","cc_mtu",cc_mtu);
			//huayi_uci_set_int(PKG_NETWORK,"wan4g","mtu",cc_mtu);

			huayi_json_get_str(request_root,"hostname",hostname,OPTION_STR_LEN);
			if(strlen(hostname)>0)
			{
				huayi_uci_set_str(PKG_NETWORK,"wan4g","hostname",hostname);
			}
		}
		huayi_uci_set_str(PKG_NETWORK,"wan","proto", "dhcp");
		huayi_uci_set_str(PKG_NETWORK,"wan4g","proto", "dhcp");
		huayi_uci_set_int(PKG_MODEM,"status","mode",use_apn);
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else
	{
		//huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);

		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);

		huayi_uci_delete_option(PKG_NETWORK,"wan4g","ifname");
		//huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
		//snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
		//system(cmd);
		ret = set_wisp_mode(request_root);
		if(FCGI_TRUE != ret)
			return ret;
		
		huayi_uci_set_str(PKG_NETWORK,"wan","proto", "dhcp");
	}
#else
	else
	{
		huayi_uci_get_str(PKG_CAPABILITY,"cap","wan_ifname",wan_ifname);

		huayi_uci_get_str(PKG_CAPABILITY,"cap","lan_ifname",lan_ifname);
		huayi_uci_set_str(PKG_NETWORK,"lan","ifname",lan_ifname);

		huayi_uci_delete_option(PKG_NETWORK,"wan4g","ifname");
		huayi_uci_set_str(PKG_NETWORK,"wan","ifname",wan_ifname);
		snprintf(cmd,CMD_STR_LEN,"huayi_cmd set network wan_device %s",wan_ifname);
		system(cmd);

		if(!strcmp(proto,HUAYI_NET_PROTO_STATIC))
		{
			if(!huayi_json_get_int(request_root,"cc_mtu",&cc_mtu))
			{
				return FCGI_FALSE;
			}
			if(!set_interface_static_config(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
		}else if(!strcmp(proto,HUAYI_NET_PROTO_PPPOE))
		{
			if(!huayi_json_get_int(request_root,"ppp_mtu",&ppp_mtu))
			{
				return FCGI_RESP_CODE_DATA_MISS;
			}
			if(!set_interface_pppoe_config(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return FCGI_RESP_CODE_DATA_MISS;
			}
			huayi_uci_set_int(PKG_NETWORK,"wan","ppp_mtu",ppp_mtu);
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",ppp_mtu);
		}else if(!strcmp(proto,HUAYI_NET_PROTO_DHCP))
		{
			if(!huayi_json_get_int(request_root,"cc_mtu",&cc_mtu))
			{
				return FCGI_FALSE;
			}
			/*if(!set_interface_dhcp_config(request_root,"wan"))
			{
				huayi_uci_revert(PKG_NETWORK);
				return FCGI_RESP_CODE_DATA_MISS;
			}*/	
			huayi_json_get_str(request_root,"hostname",hostname,OPTION_STR_LEN);
			if(strlen(hostname)>0)
			{
				huayi_uci_set_str(PKG_NETWORK, "wan", "hostname",hostname);
			}
			
			huayi_uci_delete_option(PKG_NETWORK, "wan", "dns");
			huayi_uci_set_int(PKG_NETWORK,"wan","cc_mtu",cc_mtu);
			huayi_uci_set_int(PKG_NETWORK,"wan","mtu",cc_mtu);
			huayi_uci_set_str(PKG_NETWORK,"wan","ipaddr", "");
			huayi_uci_set_str(PKG_NETWORK,"wan","netmask", "");
			huayi_uci_set_str(PKG_NETWORK,"wan","gateway", "");
		}
	
		huayi_uci_set_str(PKG_NETWORK,"wan","proto",proto);
		if(!huayi_json_get_str(request_root,"mac_addr",macaddr,MAC_STR_LEN))
		{
			return FCGI_RESP_CODE_DATA_MISS;
		}
		huayi_uci_set_str(PKG_NETWORK,"wan","macaddr",macaddr);
	}
#endif	
	huayi_uci_commit(PKG_SYSTEM);
	huayi_uci_commit(PKG_NETWORK);
	huayi_uci_commit(PKG_MODEM);
	huayi_uci_commit(PKG_WIRELESS);
	
	if (!strcmp(wan_type, WAN_TYPE_4G_MOBILE))
	{
		system("huayi-reset lte4g parameter 0");
	}
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
	else if(!strcmp(wan_type, WAN_TYPE_WIRED_PRIOROTY))
	{
		system("huayi-reset lte4g parameter 0");
	}
#endif
#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	else if(!strcmp(wan_type, WAN_TYPE_REP4G_PRIOROTY))
	{
		system("huayi-reset lte4g parameter 0");
	}
#endif
	
	system("huayi-reset network");
	*lock_time = HUAYI_RELOAD_NETWORK_TIME;

	cJSON_AddFalseToObject(response_root,"reconnection");
	cJSON_AddStringToObject(response_root,"reconnection_addr","");
	
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_get_4g_signal(cJSON *request_root, cJSON *response_root,int *lock_time)
{
	cJSON 	*result_root =NULL;
	int i_signal = 0,i_ready = 0;
	char ssid_2g[OPTION_STR_LEN] = {0},pwd_2g[TEMP_STR_LEN] = {0};
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	char ssid_5g[OPTION_STR_LEN] = {0},pwd_5g[TEMP_STR_LEN] = {0};
#endif

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	
	result_root = cJSON_CreateObject();

	atcmd_get_cpin(&i_ready);

	if(i_ready == 1)
	{
		atcmd_get_signal(&i_signal);
	}

	cJSON_AddNumberToObject(result_root,"signal",i_signal);

	if(!huayi_uci_get_str(PKG_WIRELESS,"radio0_0","ssid",ssid_2g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	huayi_uci_get_str(PKG_WIRELESS,"radio0_0","key",pwd_2g);

	cJSON_AddStringToObject(result_root,"ssid_2g",ssid_2g);
	cJSON_AddStringToObject(result_root,"pwd_2g",pwd_2g);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)

	if(!huayi_uci_get_str(PKG_WIRELESS,"radio1_0","ssid",ssid_5g))
	{
		return FCGI_RESP_CODE_SERVER_FAILURE;
	}
	
	huayi_uci_get_str(PKG_WIRELESS,"radio1_0","key",pwd_5g);

	cJSON_AddStringToObject(result_root,"ssid_5g",ssid_5g);
	cJSON_AddStringToObject(result_root,"pwd_5g",pwd_5g);
#endif

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	return FCGI_RESP_CODE_SUCCESS;
}

int cgi_get_dev_message(cJSON *request_root,cJSON *response_root,int *lock_time)
{
	cJSON 	*result_root =NULL;
	char *lte_value[OPTION_STR_LEN]={0},checksignalips[OPTION_STR_LEN]={0};
	int i_ready=0,i_signal=0,is_ok=0;
	
	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	result_root = cJSON_CreateObject();
	memset(lte_value,0,sizeof(lte_value));
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","iccid",lte_value);
	cJSON_AddStringToObject(result_root,"iccid",lte_value);

	//memset(lte_value,0,sizeof(lte_value));
	//strcpy(lte_value,"SN");
	
	
	memset(lte_value,0,sizeof(lte_value));
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","imei",lte_value);
	cJSON_AddStringToObject(result_root,"imei",lte_value);
	cJSON_AddStringToObject(result_root,"device_sn",lte_value);

//上网方式
	memset(lte_value,0,sizeof(lte_value));
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ifname", lte_value);
	if(!strcmp(lte_value, "apcli0"))
	{
		cJSON_AddStringToObject(result_root,"net_type", "WISP");
	}
	else if(!strcmp(lte_value, "eth1") || !strcmp(lte_value, "wwan0"))
	{
		cJSON_AddStringToObject(result_root,"net_type", "4G");
	}
	else if(!strcmp(lte_value, "eth0.2"))
	{
		cJSON_AddStringToObject(result_root,"net_type", "Wired");
	}
	else
	{
		cJSON_AddStringToObject(result_root,"net_type", "4G");
	}


	//上网是否成功
	//strcpy(checksignalips,"114.114.114.114");
	memset(lte_value,0,sizeof(lte_value));
	huayi_uci_get_str(PKG_SYSTEM,"mode","detect_ip", checksignalips);
	is_ok = internet_do_ping_detect(checksignalips, NULL);
	if(is_ok == 1)
	{
		get_maxdelay(checksignalips,lte_value);
		cJSON_AddStringToObject(result_root,"net_delay",lte_value);
	}
	else
	{
		cJSON_AddStringToObject(result_root,"net_delay", "0");
	}
	cJSON_AddNumberToObject(result_root,"intelnet_ok",is_ok);
	
	atcmd_get_cpin(&i_ready);
	if(i_ready == 1)
	{
		atcmd_get_signal(&i_signal);
	}
	else
	{
		i_signal = 0;
	}
	cJSON_AddNumberToObject(result_root,"4g_signal",i_signal);
	
	memset(lte_value,0,sizeof(lte_value));
	get_relay_wf(lte_value);
	cJSON_AddStringToObject(result_root,"wifi_rssi",lte_value);	

	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);

	return FCGI_RESP_CODE_SUCCESS;
}


int cgi_get_card(cJSON *request_root, cJSON *response_root,int *lock_time){
	//FASTCGI_LOG("%s___%d___\n",__FUNCTION__,__LINE__);
 	cJSON  *info_root=NULL,*card_info_root=NULL,*result_root=NULL;
 	int i_card_slot=0,i_operator=0,i_simnum=0,i=0,i_lac=0,info_card_slot=0,info_operator=0,count=0;
 	char cmd[OPTION_STR_LEN]={0},i_lte_value[OPTION_STR_LEN]={0},lte_value[OPTION_STR_LEN]={0},name[OPTION_STR_LEN]={0};
 	
	memset(lte_value, 0,OPTION_STR_LEN);
	memset(cmd,0,OPTION_STR_LEN);
 	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	count=0;
again:
	
	result_root=cJSON_CreateObject();
	card_info_root=cJSON_CreateArray();
	memset(lte_value, 0,OPTION_STR_LEN);
	huayi_uci_get_int(PKG_M_SYSTEM,"usesiminfo","card_slot",&info_card_slot);
	huayi_uci_get_int(PKG_M_SYSTEM,"usesiminfo","operator",&info_operator);
	huayi_uci_get_str(PKG_M_SYSTEM,"usesiminfo","iccid",lte_value);
	cJSON_AddNumberToObject(result_root,"card",info_card_slot);
	cJSON_AddNumberToObject(result_root,"operator",info_operator);
	
	huayi_uci_get_int(PKG_M_SYSTEM,"system","simnum",&i_simnum);
	for(i=1;i<=i_simnum;i++)
	{
		
		memset(cmd,0,OPTION_STR_LEN);
		sprintf(cmd,"sim%d",i);

		huayi_uci_get_int(PKG_M_SYSTEM,cmd,"card_slot",&i_card_slot);
		huayi_uci_get_int(PKG_M_SYSTEM,cmd,"operator",&i_operator);
		memset(i_lte_value, 0,OPTION_STR_LEN);
		huayi_uci_get_str(PKG_M_SYSTEM,cmd,"iccid",i_lte_value);
		//FASTCGI_LOG("%s___%d___infocard:%d__simcard:%d\n",__FUNCTION__,__LINE__,info_operator,i_operator);
			if(info_card_slot==i&&info_operator!=i_operator&&strcmp(lte_value,i_lte_value))
				{
					cJSON_Delete(result_root);
					cJSON_Delete(card_info_root);
					cJSON_Delete(info_root);
					count++;
					if(count<=3)
					{
						sleep(1);
						goto again;
					}
					else
					{	system("huayi-reset switch_sim");
						return FCGI_RESP_CODE_DATA_MISS;
					}
				}
		info_root=cJSON_CreateObject();
		cJSON_AddNumberToObject(info_root,"card",i_card_slot);
		cJSON_AddNumberToObject(info_root,"operator",i_operator);
		cJSON_AddStringToObject(info_root,"iccid",i_lte_value);
		cJSON_AddItemToArray(card_info_root,info_root);
	}
	cJSON_AddItemToObject(result_root,"card_info",card_info_root);
	cJSON_AddItemToObject(response_root,WEB_RETURN_DATA_NAME,result_root);
	//char *output =cJSON_Print(response_root);
	//FASTCGI_LOG("%s___%d___%s\n",__FUNCTION__,__LINE__,output);
	return FCGI_RESP_CODE_SUCCESS;

}

int cgi_set_card(cJSON *request_root, cJSON *response_root,int *lock_time){
	FASTCGI_LOG("%s___%d___\n",__FUNCTION__,__LINE__);

	char cmd[OPTION_STR_LEN]={0};
	int card=0,operator=0,i_card_slot=0,i_operator=0,i_simnum=0;

	if((NULL == request_root)||(NULL == response_root) || (NULL == lock_time)) 
	{
		return FCGI_RESP_CODE_DATA_MISS;			
	}
	if(!huayi_json_get_int(request_root,"card",&card))
	{
		return FCGI_RESP_CODE_DATA_MISS;
	}
	huayi_uci_set_int(PKG_M_SYSTEM,"usesiminfo","next_card_slot",card);
	//FASTCGI_LOG("%s___%d___%d\n",__FUNCTION__,__LINE__,card);
	huayi_uci_commit(PKG_M_SYSTEM);
	*lock_time = HUAYI_SET_CARD;
	system("huayi-reset switch_sim");
	return FCGI_RESP_CODE_SUCCESS; 
}





FCGI_BOOL FastCgiHandleModemFunctionInit(void)
{	
	memset(&FastCgiHandleModemFunction,0,sizeof(MAX_MODEM_HYID*sizeof(int)));

	FastCgiHandleModemFunction[HYID_GET_NEWS_SMS_STATE] = cgi_get_new_sms_state;

	FastCgiHandleModemFunction[HYID_GET_BREAF_SMS_INFO] = cgi_get_breaf_sms_info;

	FastCgiHandleModemFunction[HYID_GET_SMS_SESSION_BY_NUMBER] = cgi_get_sms_session_by_number;

	FastCgiHandleModemFunction[HYID_DELETE_SMS_SESSION_BY_SID] = cgi_delete_sms_session_by_sid;

	FastCgiHandleModemFunction[HYID_DELETE_SMS_BY_CID] = cgi_delete_sms_by_cid;

	FastCgiHandleModemFunction[HYID_SEND_SMS_MESSAGES] = cgi_send_sms_messages;

	FastCgiHandleModemFunction[HYID_GET_PIN_STATE] = cgi_get_module_pin_state;

	FastCgiHandleModemFunction[HYID_SET_SIM_LOCK_SET_PIN] = cgi_set_sim_lock_set_pin;

	FastCgiHandleModemFunction[HYID_GET_PIN_CLCK] = cgi_get_sim_clck;

	FastCgiHandleModemFunction[HYID_SET_PIN_CLCK] = cgi_set_pin_clck;

	FastCgiHandleModemFunction[HYID_CHANGE_PIN_CODE] = cgi_change_module_pin_code;

	FastCgiHandleModemFunction[HYID_GET_APN_CONFIG] = cgi_get_module_apn_config;

	FastCgiHandleModemFunction[HYID_SET_APN_CONFIG] = cgi_set_module_apn_config;

	FastCgiHandleModemFunction[HYID_GET_OVERVIEW_CONFIG] = cgi_get_overview_info;

	FastCgiHandleModemFunction[HYID_GET_WAN_OR_WAN4G_CONFIG] = cgi_get_wan_or_wan4g_config;

	FastCgiHandleModemFunction[HYID_SET_WAN_OR_WAN4G_CONFIG] = cgi_set_wan_or_wan4g_config;

	FastCgiHandleModemFunction[HYID_GET_4G_SIGNAL] = cgi_get_4g_signal;

	FastCgiHandleModemFunction[HUID_GET_SIM_MESSAGE] =cgi_get_dev_message;

	FastCgiHandleModemFunction[HYID_GET_CARD]=cgi_get_card;
	
	FastCgiHandleModemFunction[HYID_SET_CARD]=cgi_set_card;
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

	if(!FastCgiHandleModemFunctionInit())
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
		
		if(HYID < HYID_GET_OVERVIEW_CONFIG || HYID >= MAX_MODEM_HYID)
		{
			rest_code = FCGI_RESP_CODE_ILLEGAL_HYID;
			
			goto v_out;
		}
		
#if defined(CONFIG_HUAYI_CGI_COOKIE_SUPPORT)
		
		if(HYID > HYID_GET_OVERVIEW_CONFIG && FCGI_RESP_CODE_UNAUTHORIZED == check_landing_status())
		{
			rest_code = FCGI_RESP_CODE_UNAUTHORIZED;
			goto v_out;
		}
		
#endif
		if(FastCgiHandleModemFunction[HYID] != NULL)
		{		
			rest_code = FastCgiHandleModemFunction[HYID](request_root,response_root,&lock_time);
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
