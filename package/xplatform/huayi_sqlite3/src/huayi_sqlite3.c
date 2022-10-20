
#include "huayi_sqlite3.h"


/*notice: please free hex_string after using*/
/*char *huayiSqlite3HexString(char *string)
{
	int from_len = 0;
	char *hex_string = NULL;
	if(string == NULL)
		return NULL;

	from_len = strlen(string);
	if(from_len == 0)
		return NULL;

	hex_string=(char *)malloc(strlen(string)*2+1);
	
	mysql_hex_string(hex_string,string,from_len);

	return hex_string;
}*/
char *huayiSqlite3HexString(char *string,int from_len)
{
	char *hex_string = NULL;
	if(string == NULL || from_len == 0)
		return NULL;

	hex_string=(char *)malloc(from_len*2+1);
	
	huayi_hex_string(hex_string,string,from_len);

	return hex_string;
}

HUAYI_SQLITE3_BOOL huayiSqlite3Open(sqlite3 **my_con)
{
	int ret = -1;

	sqlite3 *sql_db = NULL;
	ret = sqlite3_open(SQLITE3_DB, &sql_db);
	if(0 != ret){
		fprintf(stderr, "%s can't open database: %s\n", __FUNCTION__, sqlite3_errmsg(sql_db));	
		sqlite3_close(sql_db);	
		return ret;
	}

	*my_con = sql_db;
	return HUAYI_SQLITE3_TRUE;
}

HUAYI_SQLITE3_BOOL huayiSqlite3Close(sqlite3 *my_con)
{
	if(my_con) 
	{
		sqlite3_close(my_con);
		my_con = NULL;
		return HUAYI_SQLITE3_TRUE;
	}
	return HUAYI_SQLITE3_FALSE;
}

int huayiSqlite3GetStr(sqlite3 *my_con,const char *query,char *value,int len)
{
	int ret;
	char *zErrMsg = NULL;
	char **result = NULL;
	int nrow = 0,ncol = 0;

	ret = sqlite3_get_table(my_con,query,&result,&nrow,&ncol,&zErrMsg);

	if(NULL != zErrMsg)
	{
		printf("%s sql fail,error code:%d, erro:%s\n",__FUNCTION__, ret, zErrMsg);	
		ret = 0;
		goto end;
	}

	if(nrow == 0)
	{
		ret = 0;
		goto end;
	}

	snprintf(value,len,"%s",result[1]);
	ret = strlen(result[1]);

end:
	sqlite3_free_table(result);
	
	return ret;
	/*SQLITE3_RES *res	= NULL;
	int line;
	SQLITE3_ROW row;
	int i, k,field;

	if((!my_con)||(!query)||(!value))
	{
		return 0;
	}
	
	if(!huayiSqlite3Exec(my_con,query))
	{
		return 0;
	}
	
	res = mysql_store_result(my_con);
	
	if(res)
	{
		if((mysql_num_fields(res) != 1) || (mysql_field_count(my_con)!=1))
		{
			huayiSqlite3ErrorLog(my_con,"huayiSqlite3GetStr:result incorrect");
		}else
		{
			row = mysql_fetch_row(res);
			if(row)
			{
				mysql_free_result(res);
				res = NULL;
				if(row[0])
				{
					snprintf(value,len,"%s",row[0]);
					return strlen((char *)&row[0]);
				}
				
			}
			
		}
	}
	
	if(res)
	{	
		mysql_free_result(res);
	}
	return 0;
	*/
}

void huayiSqlite3GetInt(sqlite3 *my_con,const char *query,int *value)
{
	char value_str[MAX_SQLITE3_VALUE_LEN];

	if((!my_con)||(!query)||(!value))
	{
		return;
	}

	memset(value_str,0,MAX_SQLITE3_VALUE_LEN);
	
	if (0 == huayiSqlite3GetStr(my_con,query,value_str,MAX_SQLITE3_VALUE_LEN))
	{
		return;
	}

	*value = atoi(value_str);
	
	return;
}

HUAYI_SQLITE3_BOOL huayiSqlite3Exec(sqlite3 *my_con,const char *fmt,...)
{
	char query[MAX_SQLITE3_QUERY_LEN]={0};
	int ret = -1;
	char *zErrMsg = NULL;
	va_list argptr;

	va_start(argptr, fmt);

	vsprintf(query,fmt,argptr);

	va_end(argptr);

	ret = sqlite3_exec(my_con,query, NULL, NULL, &zErrMsg);
	if(ret != SQLITE_OK)
	{
		printf("%s sql fail,error code:%d, erro:%s\n",__FUNCTION__, ret, zErrMsg);	
		return HUAYI_SQLITE3_FALSE;
	}
	
	return HUAYI_SQLITE3_TRUE;
}

HUAYI_SQLITE3_BOOL huayiSqlite3GetTable(sqlite3 *my_con,char *query,unsigned long *nrow,unsigned int *ncol, void **result)
{
	int ret;
	int i,j;
	
	if(nrow==NULL || ncol==NULL)
		goto __err;
	
	ret = huayiSqlite3Exec(my_con,query);
	
	if(ret == HUAYI_SQLITE3_FALSE)
		goto __err;

	*nrow = 0;
	*ncol = 0;
	*result = NULL;
	
	/**result = mysql_store_result(my_con);
	
	if(*result) 
	{
		*nrow = mysql_num_rows(*result);
		*ncol = mysql_num_fields(*result);
	}*/
	
	return HUAYI_SQLITE3_TRUE;

__err:
	*nrow = 0;
	*ncol = 0;
	*result = NULL;
	return HUAYI_SQLITE3_FALSE;
	
}


HUAYI_SQLITE3_BOOL huayiSqlite3CheckTableExist(sqlite3 *my_con,char *table_name)
{	
	HUAYI_SQLITE3_BOOL ret;	
	char **result = NULL;
	char *zErrMsg = NULL;
	char query[MAX_SQLITE3_QUERY_LEN] = {0};
	int nrow = 0,ncol = 0; 

	if((!my_con)||(!table_name))	
	{		
		return HUAYI_SQLITE3_FALSE;	
	}		
	
	sprintf(query,"select * from `%s`;",table_name);
	ret = sqlite3_get_table(my_con,query,&result,&nrow,&ncol,&zErrMsg);
	if(NULL != zErrMsg)
	{
		ret = HUAYI_SQLITE3_FALSE;
		goto end;
	}

	if(nrow)	
	{			
		ret = HUAYI_SQLITE3_TRUE;	
	}
	else	
	{
		ret = HUAYI_SQLITE3_FALSE;	
	}
end:
	sqlite3_free_table(result);

	return ret;
}

