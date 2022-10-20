#include "huayi_sms.h"
#include "huayi_log.h"


VSBool gRunAgent;
HUAYI_SMS_MSG *sms_all = NULL;

void huayi_sms_init(struct SMSAgent *Agent)
{
	memset(Agent,0,sizeof(struct SMSAgent));
	gRunAgent = VS_TRUE;
	return;
}

void MainGracefulExit(int sig)
{
	gRunAgent = VS_FALSE;
	(void) signal(SIGTERM, SIG_IGN);
	return;
}


void huayiSqlite3ForEachCheck(sqlite3 *my_con)
{
	int ncol = 0,nrow = 0 ,i = 0;
	int ret = 0;
	char **result = NULL;
	char *zErrMsg = NULL;
	
	char query[MAX_SQLITE3_QUERY_LEN] = {0};
	
	if(my_con == NULL)
	{
		return;
	}

	snprintf(query,MAX_SQLITE3_QUERY_LEN,"select %s from `%s` where %s>0;",SQLITE3_SMSNUMBER_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMS_COUNT_KEY_NAME);

	ret = sqlite3_get_table(my_con,query,&result,&nrow,&ncol,&zErrMsg);
	if(NULL != zErrMsg)
	{
		printf("sqlite3_get_table fail\n");
		goto end;
	}

	if((!nrow) || (!result))
	{
		goto end;
	}

	if(ncol != 1)
	{
		goto end;
	}

	if(strcmp(result[0],SQLITE3_SMSNUMBER_KEY_NAME))
	{
		goto end;
	}

	for(i = 1;i < nrow + 1; i++)
	{
		if(!strlen(result[i]))
		{
			continue;
		}

		if(!huayiSqlite3CheckTableExist(my_con,result[i]))
		{
			snprintf(query,sizeof(query),"delete from `%s` where %s='%s';",SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,result[i]);
			huayiSqlite3Exec(my_con,query);
			continue;
		}	
	}

end:
	sqlite3_free_table(result);
	return;
}


VSBool huayi_sms_sqlite3_init(void)
{
	sqlite3 *my_con=NULL;
	char query[MAX_SQLITE3_QUERY_LEN]={0};
	VSBool ret = VS_FALSE;

	if(!huayiSqlite3Open(&my_con))
	{
		return VS_FALSE;
	}

	snprintf(query,sizeof(query),"create table if not exists %s ("
			"%s integer primary key autoincrement,"
			"%s varchar(17),"
			"%s varchar(1024),"
			"%s integer unsigned not null default 0,"
			"%s timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,"
			"%s integer unsigned not null default 0,"
			"%s integer unsigned not null,"
			"UNIQUE (%s)"
			");",SMS_HUAYI_TABLE,SQLITE3_SMSID_KEY_NAME,SQLITE3_SMSNUMBER_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
				SQLITE3_SMS_COUNT_KEY_NAME,SQLITE3_SMS_LASTTIME_KEY_NAME,SQLITE3_SMS_UNREAD_KEY_NAME,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMSNUMBER_KEY_NAME);

	if(!huayiSqlite3Exec(my_con,query))
	{
		ret = VS_FALSE;
		goto end;
	}
	huayiSqlite3ForEachCheck(my_con);
	
	huayiSqlite3Exec(my_con,"delete from `%s` where %s=0;",SMS_HUAYI_TABLE,SQLITE3_SMS_COUNT_KEY_NAME);

	ret = VS_TRUE;
end:
	huayiSqlite3Close(my_con);
	
	return ret;
}


void Char_to_hex(unsigned char str_i,int *temp)
{
	
	if((str_i>='0')&&(str_i<='9'))
	{
		*temp= str_i -'0';
	}else if((str_i>='a' && str_i<='f'))
  	{
  		*temp = str_i-'a'+10;
  	}else if((str_i>='A' && str_i<='F'))
  	{
  		*temp = str_i-'A'+10;
  	}
	return;
}

void Str_to_Hex (unsigned char * str, unsigned char * hex_16, int len)
{
	int i = 0;
	int a , b;
	memset(hex_16,0,512);
	for(i = 0; i < len; i++)
	{
		Char_to_hex(str[i*2],&a);
		Char_to_hex(str[i*2+1],&b);
		hex_16[i] =a*16 + b;
	}
	return;
}


int str_to_rts(unsigned char *inbuf,int len)
{
	int i = 0;
	unsigned char ptr,ptr2;
	
	for(i = 0; i < len; i++)
	{
		ptr = inbuf[i];
		ptr2 = inbuf[i];
		ptr = ptr<<4;
		ptr += ptr2>>4;
		inbuf[i] = ptr;
	}
	
	return 0;
}

int get_sender_by_raw_code(unsigned char *inbuf,int len,unsigned char *sender)
{
	int i = 0;
	int offset = 0;
	str_to_rts(inbuf,len);

	if(inbuf[0] == 0x86)
	{
		sender[0] = '+';
		offset++;
	}
	
	for(i= 0; i < len; i++)
	{
		offset += snprintf(sender+offset,32,"%02x",inbuf[i]);
	}

	if (sender[offset - 1] == 'f')
	{
		sender[offset - 1] = '\0';
	}
	
	return 0;
}

int huayi_retrieve_pdu_sms_message(unsigned char *text,
											  unsigned char *sender,
											  unsigned char *date,
											  unsigned char *time,
											  unsigned char *msg_type,
											  unsigned char *msg_id,
											  unsigned char *fragment_num,
											  unsigned char *msg)
{
	if((text == NULL) || (sender == NULL)||(date == NULL)||(time == NULL)||(msg_type == NULL)||(msg == NULL))
		return 0;

	SMS_STRUCTURE sms_info;
	int len = strlen(text);
	unsigned char hex_text[512] = { 0 };
	int offset = 0;
	int src_num_len;

	int pdu_len = 0;
	int data_len = 0;

	data_len = len/2 - 20;

	memset(&sms_info,0,sizeof(SMS_STRUCTURE));

	Str_to_Hex(text,hex_text,len);

	sms_info.center_len = hex_text[offset];
	offset++;

	/*no need sms center */
	memcpy(&sms_info.center,hex_text+offset,sms_info.center_len);
	offset+=sms_info.center_len;
 
	sms_info.code = hex_text[offset];
	offset++;

	sms_info.src_code_len = hex_text[offset];
	offset++;

	offset++;

	src_num_len = (sms_info.src_code_len/2) + (sms_info.src_code_len % 2);
	memcpy(&sms_info.src_code,hex_text+offset,src_num_len);
	offset+=src_num_len;

	sms_info.protocol_type = hex_text[offset];
	offset++;
	sms_info.encode_type = hex_text[offset];
	offset++;

	memcpy(&sms_info.date,hex_text+offset,6);
	offset+=6;

	sms_info.time_zone = hex_text[offset];
	offset++;

	sms_info.msg_len =hex_text[offset];
	offset++;
	
	if (sms_info.code&0X40)
	{
		snprintf(msg_id,32,"%02x%02x%02x%02x%02x%02x",sms_info.src_code[1],hex_text[offset],hex_text[offset+1],
			hex_text[offset+2],hex_text[offset+3],hex_text[offset+4]);
		*fragment_num = hex_text[offset+5];
		offset += 6;
		sms_info.msg_len -= 6;
	}
	else
	{
		snprintf(msg_id,32,"FFFFFFFFFFFF");
		*fragment_num = 0;
	}

	if (sms_info.encode_type == 0)
	{
		data_len = sms_info.msg_len%8 + sms_info.msg_len/8*7;
		sms_info.msg_len = data_len;
	}
	
	memcpy(&sms_info.msg,hex_text+offset,sms_info.msg_len);
	offset+=sms_info.msg_len;

	if(sms_info.encode_type == 8)
	{
		*msg_type = 1; /*chinese*/
		pdu_len = decodeUcs2(sms_info.msg, msg, sms_info.msg_len);
	}
	else if(sms_info.encode_type == 0)
	{
		*msg_type = 0; /*english*/
		pdu_len = decode7bit(sms_info.msg,msg,sms_info.msg_len);
	}
	else
	{
		*msg_type = 1; /*chinese*/
		pdu_len = decodeUcs2(sms_info.msg, msg, sms_info.msg_len);
	}

	str_to_rts(sms_info.date,6);

	get_sender_by_raw_code(sms_info.src_code,src_num_len,sender);

	snprintf(date,RESULT_STR_LEN,"20%02x/%02x/%02x",sms_info.date[0],sms_info.date[1],sms_info.date[2]);

	snprintf(time,RESULT_STR_LEN,"%02x:%02x:%02x",sms_info.date[3],sms_info.date[4],sms_info.date[5]);

	return pdu_len;
}


VSBool sms_insert_into_db(struct SMSAgent *Agent,SM_PARAM *sms,int sms_msg_size)
{
	char timestamp[15] ={0};
	char str_number[17] = {0};
	char *number=NULL;
	sqlite3 *my_con = NULL;
	int sid = 0;
	char query[MAX_SQLITE3_QUERY_LEN]={0};
	VSBool ret = VS_FALSE;
	char sms_msg_escape_characters[MAX_SMS_ESCAPE_CHARACTER_LEN] = {0};
	char *sql_hex = NULL;

	char **result = NULL;
	char *zErrMsg = NULL;
	int nrow = 0,ncol = 0; 
	int c_sid = 0;
	int sms_count = 0;
	int unread_count = 0;
	char date_str[MAX_SQLITE3_VALUE_LEN] = {0};
	char data_msg[MAX_SQLITE3_QUERY_LEN] = {0};

	

	snprintf(timestamp,sizeof(timestamp),"%s",sms->TP_SCTS);

	memset(str_number,0,sizeof(str_number));

	strncpy(str_number,sms->TPA,MAX_SMS_VALUE_LEN);

	if(strncmp(str_number,"86",2) ==0)
	{
		number=str_number+2;
	}else if(strncmp(str_number,"+86",3) ==0)
	{
		number=str_number + 3;
	}
	else
	{
		number=str_number;
	}
		
	if(!huayiSqlite3Open(&my_con))
	{
		return VS_FALSE;
	}

	if(!huayiSqlite3CheckTableExist(my_con,number))
	{
		snprintf(query,sizeof(query),"create table if not exists `%s` ("
			"%s integer primary key autoincrement,"
			"%s varchar(%d),"
			"%s timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,"
			"%s int unsigned not null default 0,"
			"UNIQUE (%s)"
			");",number,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,MAX_SMS_MSG_LEN,
			SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,SQLITE3_SMS_SRC_KEY_NAME,SQLITE3_SMS_CHILD_ID_KEY_NAME);
	
		if(!huayiSqlite3Exec(my_con,query))
		{
			ret = VS_FALSE;
			goto end;
		}
	}

	ret = huayiHanldeEscapeCharacter(sms->TP_UD,'\'',data_msg);
	if(ret == 0)
	{
		ret = VS_FALSE;
		goto end;
	}

	/*sql_hex = huayiSqlite3HexString(sms->TP_UD,sms_msg_size);
	printf("sql_hex= %p\n",sql_hex);
	if(NULL == sql_hex)
	{
		ret = VS_FALSE;
		goto end;
	}*/

	memset(query,0,sizeof(query));
	snprintf(query,sizeof(query),"insert into `%s` (%s,%s) values('%s',datetime('now','localtime'));",number,SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,data_msg);
	//printf("query = %s\n",query);
	ret = sqlite3_exec(my_con,query, NULL, NULL, &zErrMsg);
	if(ret != SQLITE_OK)
	{
		printf("%s sql fail,error code:%d, erro:%s\n",__FUNCTION__, ret, zErrMsg);	
		ret = VS_FALSE;
		goto end;
	}

	memset(query,0,sizeof(query));
	snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",SQLITE3_SMSID_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,number);
	//printf("query = %s\n",query);

	huayiSqlite3GetInt(my_con,query,&sid);
	if(sid == 0)
	{
		if(!huayiSqlite3Exec(my_con,"insert into `%s` (%s,%s,%s,%s,%s,%s) select %s,%s,1,%s,1,%s from `%s` order by %s desc limit 1;",SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,
																				SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_COUNT_KEY_NAME,SQLITE3_SMS_LASTTIME_KEY_NAME,
																				SQLITE3_SMS_UNREAD_KEY_NAME,SQLITE3_SMS_CHILD_ID_KEY_NAME,
																				number,SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,
																				SQLITE3_SMS_CHILD_ID_KEY_NAME,number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME))
		{
			ret = VS_FALSE;
			goto end;
		}
		
	}else
	{	
		memset(query,0,sizeof(query));
		snprintf(query,sizeof(query),"select %s,%s,%s from `%s` order by %s desc limit 1;",
													SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
													SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME);
		sqlite3_get_table(my_con,query,&result,&nrow,&ncol,&zErrMsg);
		if(NULL != zErrMsg || nrow < 1 || !ncol)
		{
			ret = VS_FALSE;
			goto end;
		}

		c_sid = atoi(result[ncol]);
		sprintf(date_str,result[ncol+2]);

		memset(query,0,sizeof(query));
		snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",SQLITE3_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,number);
		huayiSqlite3GetInt(my_con,query,&sms_count);

		memset(query,0,sizeof(query));
		snprintf(query,sizeof(query),"select %s from `%s` where %s='%s';",SQLITE3_SMS_UNREAD_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMSNUMBER_KEY_NAME,number);
		huayiSqlite3GetInt(my_con,query,&unread_count);

		memset(query,0,sizeof(query));
		snprintf(query,sizeof(query),"update `%s` set %s=%d,%s='%s',%s=%d,%s=%d,%s='%s' where %s=%d;",
												SMS_HUAYI_TABLE,
												SQLITE3_SMS_CHILD_ID_KEY_NAME,c_sid,
												SQLITE3_SMS_MSG_KEY_NAME,data_msg,
												SQLITE3_SMS_COUNT_KEY_NAME,sms_count+1,
												SQLITE3_SMS_UNREAD_KEY_NAME,unread_count+1,
												SQLITE3_SMS_LASTTIME_KEY_NAME,date_str,
												SQLITE3_SMSID_KEY_NAME,sid);

		ret = sqlite3_exec(my_con,query, NULL, NULL, &zErrMsg);
		if(ret != SQLITE_OK)
		{
			printf("%s sql fail,error code:%d, erro:%s\n",__FUNCTION__, ret, zErrMsg);	
			ret = VS_FALSE;
			goto end;
		}

		/*snprintf(query,sizeof(query),"update `%s` inner join(select %s,%s,%s from `%s` order by %s desc limit 1) c set %s.%s=c.%s,%s.%s=c.%s,%s.%s=%s.%s+1,%s.%s=%s.%s+1,%s.%s=c.%s where %s.%s=%d;",
																			SMS_HUAYI_TABLE,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
																			SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,number,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_CHILD_ID_KEY_NAME,SQLITE3_SMS_CHILD_ID_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_MSG_KEY_NAME,SQLITE3_SMS_MSG_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_COUNT_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMS_COUNT_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_UNREAD_KEY_NAME,SMS_HUAYI_TABLE,SQLITE3_SMS_UNREAD_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMS_LASTTIME_KEY_NAME,SQLITE3_SMS_RECEIVE_TIME_KEY_NAME,
																			SMS_HUAYI_TABLE,SQLITE3_SMSID_KEY_NAME,sid);
		printf("query = %s\n",query);
		if(!huayiSqlite3Exec(my_con,query))
		{
			ret = VS_FALSE;
			goto end;
		}*/
	}
	ret = VS_TRUE;

end:
	sqlite3_free_table(result);
	huayiSqlite3Close(my_con);
	return ret;
}



void huayi_sms_agent_loop(struct SMSAgent *Agent)
{
	char cmd[CMD_STR_LEN] = {0};
	char *ptr = NULL, *pKey = NULL;
	char registStatus[LONG_BUFF_LEN]={0};
	char used[SMALL_STR_LEN]={0},maximum[SMALL_STR_LEN]={0};
	unsigned char sender[RESULT_STR_LEN]={0},date[RESULT_STR_LEN]={0},time[RESULT_STR_LEN]={0};
	unsigned char text[LONG_BUFF_LEN]={0},pdu_text[MAX_SMS_MSG_LEN]={0};
	unsigned char msg_id[RESULT_STR_LEN]={0};
	unsigned char msg_type[TEMP_STR_LEN] = {0},fragment_num = 0;
	int i=0,k=0,j=0;
	int maximum_t=0;
	SM_PARAM sms_info;
	int sms_save = 0;
	int sms_size = 0;
	int sms_pdu_len = 0;
	int sms_num = 0;
	
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","registStatus",registStatus);
	if (strcmp(registStatus ,"connected") == 0)
	{
		if (Agent->init_modem_status == 0)
		{
			init_set_modem_status();
			Agent->init_modem_status = 1;
		}
		
		snprintf(cmd, CMD_STR_LEN, "AT+CPMS?");//message storage
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return;
		}
		ptr = ATret;
		if ((pKey = strstr(ptr, "+CPMS:")) != NULL )
		{
			huayi_get_nth_value(1, ptr, ',', used, sizeof(used));
			huayi_get_nth_value(2, ptr, ',', maximum, sizeof(maximum));
		}

		if (sms_all == NULL)
		{
			sms_size = atoi(maximum)*sizeof(HUAYI_SMS_MSG);
			sms_all = malloc(sms_size);
			if (sms_all == NULL)
			{
				return;
			}

			memset(sms_all,0,sms_size);
		}
		
		snprintf(cmd, CMD_STR_LEN, "AT+CMGF=0");//0:pdu mode    1:txt mode
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return;
		}

		snprintf(cmd, CMD_STR_LEN, "AT+CSMP=17,167,0,8");
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return;
		}

		snprintf(cmd, CMD_STR_LEN, "AT+CSCS=\"UCS2\"");
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return;
		}
		
		maximum_t=atoi(maximum);
		for (k=0; k < maximum_t; k++)
		{	
			snprintf(cmd, CMD_STR_LEN, "AT+CMGR=%d", k);//read message
			if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
			{
				break;
			}
			
			if ((pKey = strstr(ptr, "+CMGR:")) != NULL )
			{
				memset(text,0,LONG_BUFF_LEN);
				huayi_get_nth_value(2, ptr, '\r\n', text, sizeof(text));
				
				memset(pdu_text,0,sizeof(pdu_text));
				sms_pdu_len = huayi_retrieve_pdu_sms_message(text,sender,date,time,msg_type,msg_id,&fragment_num,pdu_text);
				
				if(strlen(pdu_text) == 0 || strlen(sender) == 0 || strlen(date) == 0 || strlen(time) == 0)
				{
					continue;
				}
				
				//replace_char(Date,date,'/','_');
				//replace_char(Time,time,':','_');

				if (strcmp(msg_id,"FFFFFFFFFFFF") == 0)
				{
					memset(sms_info.TP_UD,0,MAX_SMS_MSG_LEN);
					snprintf(sms_info.TPA,MAX_SMS_VALUE_LEN,sender);
					snprintf(sms_info.TP_SCTS,MAX_SMS_VALUE_LEN,date);
					memcpy(sms_info.TP_UD,pdu_text,MAX_SMS_MSG_LEN);
					sms_insert_into_db(Agent,&sms_info,sms_pdu_len);
					sms_save = 1;
				}
				else
				{
					for (i = 0; i < maximum_t; i++)
					{
						if (sms_all[i].is_used == 1)
						{
							continue;
						}
						sms_all[i].fragment_num = fragment_num;
						sms_all[i].is_used = 1;
						sms_all[i].delay_sec = 0;
						sms_all[i].pdu_len = sms_pdu_len;
						snprintf(sms_all[i].sender,MAX_SMS_VALUE_LEN,sender);
						snprintf(sms_all[i].msg_id,MAX_SMS_VALUE_LEN,msg_id);
						memcpy(sms_all[i].msg,pdu_text,MAX_SMS_MSG_LEN);
						break;
					}
				}
				
				snprintf(cmd, CMD_STR_LEN, "AT+CMGD=%d", k);
				Atcmd(cmd, strlen(cmd), 1);
			}
			
		}

		for (i = 0; i < maximum_t; i++)
		{
			if (sms_all[i].is_used == 0 || sms_all[i].fragment_num != 1)
			{
				continue;
			}

			if (sms_all[i].delay_sec < 6)
			{
				sms_all[i].delay_sec++;
				continue;
			}

			sms_size = 0;
			memset(pdu_text,0,sizeof(pdu_text));

			memcpy(pdu_text+sms_size,sms_all[i].msg,sms_all[i].pdu_len);
			sms_size+=sms_all[i].pdu_len;

			sms_num = atoi(sms_all[i].msg_id + 10);
			
			for (j = 0; j < sms_num; j++)
			{
				for (k = 0; k < maximum_t; k++)
				{
					if (sms_all[k].is_used == 0 || (sms_all[k].fragment_num != j+2))
					{
						continue;
					}

					if (strcmp(sms_all[i].msg_id,sms_all[k].msg_id) == 0)
					{
						memcpy(pdu_text+sms_size,sms_all[k].msg,sms_all[k].pdu_len);
						sms_size+=sms_all[k].pdu_len;
						sms_all[k].is_used = 0;
					}
				}
			}
			
			sms_all[i].is_used = 0;
			
			snprintf(sms_info.TPA,MAX_SMS_VALUE_LEN,sms_all[i].sender);
			memcpy(sms_info.TP_UD,pdu_text,MAX_SMS_MSG_LEN);

			sms_insert_into_db(Agent,&sms_info,sms_size);
			sms_save = 1;
		}

		if (sms_save == 1)
			system("huayi-reset save_database");
	}
}


void neoway_sms_agent_loop(struct SMSAgent *Agent)
{
	char cmd[CMD_STR_LEN] = {0};
	char *ptr = NULL, *pKey = NULL;
	char registStatus[LONG_BUFF_LEN]={0};
	char used[SMALL_STR_LEN]={0},maximum[SMALL_STR_LEN]={0};
	unsigned char sender[RESULT_STR_LEN]={0},date[RESULT_STR_LEN]={0},time[RESULT_STR_LEN]={0};
	unsigned char text[LONG_BUFF_LEN]={0},pdu_text[MAX_SMS_MSG_LEN]={0};
	unsigned char msg_id[RESULT_STR_LEN]={0};
	unsigned char msg_type[TEMP_STR_LEN] = {0},fragment_num = 0;
	int i=0,k=0,j=0;
	int maximum_t=0;
	SM_PARAM sms_info;
	int sms_save = 0;
	int sms_size = 0;
	int sms_pdu_len = 0;
	int sms_num = 0;
	
	huayi_uci_get_str(PKG_STATUS_MODEM,"modem","registStatus",registStatus);
	if (strcmp(registStatus ,"connected") == 0)
	{
		if (Agent->init_modem_status == 0)
		{
			neoway_init_set_modem_status();
			Agent->init_modem_status = 1;
		}
		
		snprintf(cmd, CMD_STR_LEN, "AT+CPMS?");//message storage
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return;
		}
		ptr = ATret;
		if ((pKey = strstr(ptr, "+CPMS:")) != NULL )
		{
			huayi_get_nth_value(1, ptr, ',', used, sizeof(used));
			huayi_get_nth_value(2, ptr, ',', maximum, sizeof(maximum));
		}

		if (sms_all == NULL)
		{
			sms_size = atoi(maximum)*sizeof(HUAYI_SMS_MSG);
			sms_all = malloc(sms_size);
			if (sms_all == NULL)
			{
				return;
			}

			memset(sms_all,0,sms_size);
		}
		
		snprintf(cmd, CMD_STR_LEN, "AT+CMGF=0");//0:pdu mode    1:txt mode
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return;
		}

		snprintf(cmd, CMD_STR_LEN, "AT+CSMP=17,167,0,8");
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return;
		}

		snprintf(cmd, CMD_STR_LEN, "AT+CSCS=\"UCS2\"");
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return;
		}
		
		maximum_t=atoi(maximum);
		for (k=0; k < maximum_t; k++)
		{	
			snprintf(cmd, CMD_STR_LEN, "AT+CMGR=%d", k);//read message
			if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
			{
				continue;
			}
			
			if ((pKey = strstr(ptr, "+CMGR:")) != NULL )
			{
				memset(text,0,LONG_BUFF_LEN);
				huayi_get_nth_value(1, ptr, '\r\n', text, sizeof(text));
				
				memset(pdu_text,0,sizeof(pdu_text));
				sms_pdu_len = huayi_retrieve_pdu_sms_message(text,sender,date,time,msg_type,msg_id,&fragment_num,pdu_text);
				
				if(strlen(pdu_text) == 0 || strlen(sender) == 0 || strlen(date) == 0 || strlen(time) == 0)
				{
					continue;
				}
				
				//replace_char(Date,date,'/','_');
				//replace_char(Time,time,':','_');

				if (strcmp(msg_id,"FFFFFFFFFFFF") == 0)
				{
					memset(sms_info.TP_UD,0,MAX_SMS_MSG_LEN);
					snprintf(sms_info.TPA,MAX_SMS_VALUE_LEN,sender);
					snprintf(sms_info.TP_SCTS,MAX_SMS_VALUE_LEN,date);
					memcpy(sms_info.TP_UD,pdu_text,MAX_SMS_MSG_LEN);
					sms_insert_into_db(Agent,&sms_info,sms_pdu_len);
					sms_save = 1;
				}
				else
				{
					for (i = 0; i < maximum_t; i++)
					{
						if (sms_all[i].is_used == 1)
						{
							continue;
						}
						sms_all[i].fragment_num = fragment_num;
						sms_all[i].is_used = 1;
						sms_all[i].delay_sec = 0;
						sms_all[i].pdu_len = sms_pdu_len;
						snprintf(sms_all[i].sender,MAX_SMS_VALUE_LEN,sender);
						snprintf(sms_all[i].msg_id,MAX_SMS_VALUE_LEN,msg_id);
						memcpy(sms_all[i].msg,pdu_text,MAX_SMS_MSG_LEN);
						break;
					}
				}
				
				snprintf(cmd, CMD_STR_LEN, "AT+CMGD=%d", k);
				Atcmd(cmd, strlen(cmd), 1);
			}
			
		}

		for (i = 0; i < maximum_t; i++)
		{
			if (sms_all[i].is_used == 0 || sms_all[i].fragment_num != 1)
			{
				continue;
			}

			if (sms_all[i].delay_sec < 6)
			{
				sms_all[i].delay_sec++;
				continue;
			}

			sms_size = 0;
			memset(pdu_text,0,sizeof(pdu_text));

			memcpy(pdu_text+sms_size,sms_all[i].msg,sms_all[i].pdu_len);
			sms_size+=sms_all[i].pdu_len;

			sms_num = atoi(sms_all[i].msg_id + 10);
			
			for (j = 0; j < sms_num; j++)
			{
				for (k = 0; k < maximum_t; k++)
				{
					if (sms_all[k].is_used == 0 || (sms_all[k].fragment_num != j+2))
					{
						continue;
					}

					if (strcmp(sms_all[i].msg_id,sms_all[k].msg_id) == 0)
					{
						memcpy(pdu_text+sms_size,sms_all[k].msg,sms_all[k].pdu_len);
						sms_size+=sms_all[k].pdu_len;
						sms_all[k].is_used = 0;
					}
				}
			}
			
			sms_all[i].is_used = 0;
			
			snprintf(sms_info.TPA,MAX_SMS_VALUE_LEN,sms_all[i].sender);
			memcpy(sms_info.TP_UD,pdu_text,MAX_SMS_MSG_LEN);

			sms_insert_into_db(Agent,&sms_info,sms_size);
			sms_save = 1;
		}

		if (sms_save == 1)
			system("huayi-reset save_database");
	}
}


VSBool huayi_client_ipc_destory(struct SMSAgent *Agent)
{
	close(Agent->sms_sock.sms_ipc_fd);

	return VS_TRUE;
}

VSBool huayi_sms_exit(struct SMSAgent *Agent)
{
	huayi_client_ipc_destory(Agent);
	
	if (sms_all)
		free(sms_all);
	
	return VS_TRUE;
}



