
#include "huayi_mysql.h"


/*notice: please free hex_string after using*/
/*char *huayiMysqlHexString(char *string)
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
char *huayiMysqlHexString(char *string,int from_len)
{
	char *hex_string = NULL;
	if(string == NULL || from_len == 0)
		return NULL;

	hex_string=(char *)malloc(from_len*2+1);
	
	mysql_hex_string(hex_string,string,from_len);

	return hex_string;
}
HUAYI_MYSQL_BOOL huayiMySqlOpen(MYSQL *my_con)
{
	unsigned char reconnect = 1;

	/*CLIENT_MULTI_STATEMENTS not support huayiMySqlGetStr*/
    if(!(mysql_real_connect(my_con,MYSQL_LOCALHOST, MYSQL_USERNAME,MYSQL_PASSWORD,MYSQL_DB, 0,MYSQL_SOCK,0)))
    {
		huayiMySqlClose(my_con);;

		return HUAYI_MYSQL_FALSE;
    }

	mysql_options(my_con, MYSQL_OPT_RECONNECT,&reconnect);

	return HUAYI_MYSQL_TRUE;
}

HUAYI_MYSQL_BOOL huayiMySqlClose(MYSQL *my_con)
{
	if(my_con) 
	{
		mysql_close(my_con);
		my_con = NULL;
		return HUAYI_MYSQL_TRUE;
	}
	return HUAYI_MYSQL_FALSE;
}

int huayiMySqlGetStr(MYSQL *my_con,const char *query,char *value,int len)
{
	MYSQL_RES *res	= NULL;
	int line;
	MYSQL_ROW row;
	int i, k,field;

	if((!my_con)||(!query)||(!value))
	{
		return 0;
	}
	
	if(!huayiMySqlExec(my_con,query))
	{
		return 0;
	}
	
	res = mysql_store_result(my_con);
	
	if(res)
	{
		if((mysql_num_fields(res) != 1) || (mysql_field_count(my_con)!=1))
		{
			huayiMySqlErrorLog(my_con,"huayiMySqlGetStr:result incorrect");
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
}

void huayiMySqlGetInt(MYSQL *my_con,const char *query,int *value)
{
	char value_str[MAX_MYSQL_VALUE_LEN];

	if((!my_con)||(!query)||(!value))
	{
		return;
	}

	memset(value_str,0,MAX_MYSQL_VALUE_LEN);
	
	if (0 == huayiMySqlGetStr(my_con,query,value_str,MAX_MYSQL_VALUE_LEN))
	{
		return;
	}

	*value = atoi(value_str);
	
	return;
}

HUAYI_MYSQL_BOOL huayiMySqlExec(MYSQL *my_con,const char *fmt,...)
{
	char query[MAX_MYSQL_QUERY_LEN]={0};
	int ret;
	va_list argptr;

	va_start(argptr, fmt);

	vsprintf(query,fmt,argptr);

	va_end(argptr);
	
	//MYSQL_LOG("query is %s\n",query);

	if(mysql_query(my_con,query))
	{
	
		huayiMySqlErrorLog(my_con,query);
		return HUAYI_MYSQL_FALSE;
	}
	
	return HUAYI_MYSQL_TRUE;
}

HUAYI_MYSQL_BOOL huayiMySqlKeepAlive(MYSQL *my_con)
{
	mysql_ping(my_con);

	return HUAYI_MYSQL_TRUE;
}

void huayiMySqlErrorLog(MYSQL *my_con,const char *action)
{
	//MYSQL_LOG("huayi mysql error: [%s] :%s\n",action,mysql_error(my_con));
	
	return;
}

HUAYI_MYSQL_BOOL huayiMysqlGetTable(MYSQL *my_con,char *query,unsigned long *nrow,unsigned int *ncol, void **result)
{
	int ret;
	int i,j;
	
	if(nrow==NULL || ncol==NULL)
		goto __err;
	
	ret = huayiMySqlExec(my_con,query);
	
	if(ret == HUAYI_MYSQL_FALSE)
		goto __err;

	*nrow = 0;
	*ncol = 0;
	*result = NULL;
	
	*result = mysql_store_result(my_con);
	
	if(*result) 
	{
		*nrow = mysql_num_rows(*result);
		*ncol = mysql_num_fields(*result);
	}
	
	return HUAYI_MYSQL_TRUE;

__err:
	*nrow = 0;
	*ncol = 0;
	*result = NULL;
	return HUAYI_MYSQL_FALSE;
	
}


HUAYI_MYSQL_BOOL huayiMysqlCheckTableExist(MYSQL *my_con,char *table_name)
{	
	HUAYI_MYSQL_BOOL ret;	
	MYSQL_RES *res	= NULL;	
	MYSQL_ROW row;		

	if((!my_con)||(!table_name))	
	{		
		return HUAYI_MYSQL_FALSE;	
	}	

	if(HUAYI_MYSQL_FALSE ==huayiMySqlExec(my_con,"select * from `%s`;",table_name))	
	{		
		return HUAYI_MYSQL_FALSE;	
	}	

	res = mysql_store_result(my_con);		

	if(res)	
	{		
		mysql_free_result(res);		
		ret = HUAYI_MYSQL_TRUE;	
	}
	else	
	{
		ret = HUAYI_MYSQL_FALSE;	
	}	

	return ret;
}

