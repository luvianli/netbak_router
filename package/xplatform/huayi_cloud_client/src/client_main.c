/*******************************************************************************************
 * Project:  huayi_cloud_client                                                           *
 *                                                                                         *
 * Author :  rex									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20220112 add by rex  Version 0.0 													   *
 ******************************************************************************************/

#include "huayi_cloud_client.h"
#include <signal.h>
#include <pthread.h>
#include <wait.h>


char heartContent[HEART_RES_DATALEN] = {0};
int heartContentLen = 0;
//unit: ms.  defautl is two min
int gTimeWait = 120000; //ms
char gurldomain[CMD_STR_LEN];
int need_upgrade = 0;


void huayi_client_main_graceful_exit(int sig)
{
	(void) signal(SIGTERM, SIG_IGN);

	return;
}

static int heart_call_back(char *data, size_t size, size_t nmemb,
		char *writerData)
{
	char Content[HEART_RES_DATALEN] = {0};
	memset(Content,0,sizeof(Content));
	memcpy(Content, data, size*nmemb);
	strcat(heartContent,Content);
	heartContentLen = size*nmemb;
	
	return heartContentLen;
}

int init_curl(CURL **curl, char *url, char *post_fields, int post_size ,callback_func *action)
{
	CURLcode code;
	*curl = curl_easy_init();

	if(*curl == NULL) {
		CLOUD_CLIENT_LOG("can't create curl connection\n");
		return -1;
	}
	code = curl_easy_setopt(*curl,CURLOPT_URL,url);
	if(code != CURLE_OK) {
		CLOUD_CLIENT_LOG("set url fail\n");
		return -1;
	}

	code=curl_easy_setopt(*curl,CURLOPT_SSL_VERIFYPEER,false);
	if(code != CURLE_OK) {
		CLOUD_CLIENT_LOG("set CURLOPT_SSL_VERIFYPEER fail\n");
		return -1;
	}
	
	if(strlen(post_fields) != 0) {
		// 设置http发送的内容类型为JSON  
		struct curl_slist *plist = NULL;
        plist = curl_slist_append(plist, "Content-Type:application/json;charset=UTF-8");  
        curl_easy_setopt(*curl, CURLOPT_HTTPHEADER, plist); 
		code = curl_easy_setopt(*curl, CURLOPT_POSTFIELDS, post_fields);
		//printf("post size:%d\n",post_size);
		curl_easy_setopt(*curl, CURLOPT_POSTFIELDSIZE, post_size);
		curl_easy_setopt(*curl, CURLOPT_CONNECTTIMEOUT, 60);
		if(code != CURLE_OK) {
			CLOUD_CLIENT_LOG("set post field fail\n");
			return -1;
		}
	}
	code = curl_easy_setopt(*curl, CURLOPT_WRITEFUNCTION, action);
	if(code != CURLE_OK) {
		CLOUD_CLIENT_LOG("set writefunction fail\n");
		return -1;
	}

	return 0;
}
#define TEST 0
#if TEST
char req_teststr[1024] = "{\"opname\":\"46011\",\"dbm\":100,\"iccid\":\"89860321247553074190\",\"cell_id\":\
\"770DE33\",\"lac\":\"7754\",\"version\":\"122\",\"sn\":\" \",\"ip\":\"192.168.43.1\",\"speed_downlink\":0,\
\"speed_uplink\":0,\"ssid\":\"HUAYI_E340_2G\",\"password\":\"12345678\",\"count\":0,\"hidden\":0}";

char res_teststr[1024] = "{\"msg\":\"seccuss\",\"code\":0,\"data\":{\"switchSim\":0,\"autoSlot\":1,\"ssid\":\"admin\",\
\"password\":\"87654321\",\"hidden\":0,\"rebot\":0,\"speed_downlink\":0,\"speed_uplink\":0,\"nextRptTime\":\
\"60000\",\"host\":\"http://8.129.188.169:8080/admin/ylapi/yYUploadData\",\"firmware_url\":\"\",\"firmware_md5\":\"\"}}";
#endif

void* heart_handle()
{
	char url[LIST_STR_LEN] ={ 0 }; 
	char heartData[HEART_REQ_DATALEN] = { 0 };
	CURL *curl = NULL;
	int ret= -1, interval = 0;
	int heartSize = 0;
	char registStatus[LONG_BUFF_LEN]={0},imei[OPTION_STR_LEN] = {0};
	int  wan_up = 0;

	huayi_uci_get_int(PKG_SYSTEM,"server","interval", &interval);
	//printf("%s, %d, interval = %d !\n", __FUNCTION__, __LINE__, interval);
	gTimeWait = interval;
	while(1) {
		huayi_uci_get_int(PKG_STATUS_NETWORK,"wan","wan_up",&wan_up);
		if(wan_up != 1){
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "registStatus", registStatus);
		if (strcmp(registStatus ,"connected") != 0) {
			CLOUD_CLIENT_LOG("%s, modem is not connected; registStatus is : %d\n",__FUNCTION__, registStatus);
			sleep(10);
			continue;
		}}
		memset(heartData, 0, sizeof(heartData));
		memset(url, 0, sizeof(url));
		get_url(url);
		msg_content(heartData, &heartSize);
		CLOUD_CLIENT_LOG("===  %s, %d, heartData = %s, heartSize = %d\n",__FUNCTION__, __LINE__, heartData, heartSize);
#if TEST
		strcpy(heartContent, res_teststr);
		heartContentLen = strlen(res_teststr);
		printf("===  %s, %d, heartContent = %s, heartSize = %d\n",__FUNCTION__, __LINE__, heartContent, heartContentLen);
		msg_parse(heartContent, heartContentLen);
#else
		ret = init_curl(&curl, (char*)url, heartData, heartSize, heart_call_back);
		if(ret == -1) {
			CLOUD_CLIENT_LOG(" %s init curl fail\n",__FUNCTION__);
			if(curl != NULL) {
				curl_easy_cleanup(curl);
				curl = NULL;
			}	
			sleep(3);
			continue;
		}
		memset(heartContent,0,sizeof(heartContent));
		ret=curl_easy_perform(curl);
		if(ret == 0) { //parse html
			if(strlen(heartContent) == 0) {	
				CLOUD_CLIENT_LOG("content is null\n");
				goto out;
			}
			CLOUD_CLIENT_LOG("===  %s, %d, heartContent = %s, heartSize = %d\n",__FUNCTION__, __LINE__, heartContent, heartContentLen);
			msg_parse(heartContent, heartContentLen);
		}
		else {
			CLOUD_CLIENT_LOG("heart connect fail:%d\n",ret);
			if(curl != NULL) {
				curl_easy_cleanup(curl);
				curl = NULL;
			}	
			sleep(3);
			continue;
		}
out:
		curl_easy_cleanup(curl);
		curl = NULL;
#endif
		CLOUD_CLIENT_LOG("%s, %d, gTimeWait = %d !\n", __FUNCTION__, __LINE__, gTimeWait);
		CHECK_CLOUD_CLIENT_LOG();
		usleep(gTimeWait*1000); //us
	}
	return (void*)0;
}


int main(int argc, char *argv[])
{
	pthread_t heartTid;
	int err;
	void *tret;
	char 	wan_type[OPTION_STR_LEN] = {0};
	
	(void) signal(SIGTERM, huayi_client_main_graceful_exit);

	/*huayi_uci_get_str(PKG_SYSTEM,"mode","wan_type",wan_type);
	if (0 != strcmp(wan_type,WAN_TYPE_4G))
	{
		return 0;
	}*/
	curl_global_init(CURL_GLOBAL_ALL);

	pthread_create(&heartTid, NULL, heart_handle, NULL);
	
	err = pthread_join(heartTid,&tret);//阻塞等待线程id为tid1的线程，直到该线程退出  
    if(err!=0)  
    {   
        printf("can not join with clound client:%s\n",strerror(err));
        exit(-1);
    }       
    printf("clound client exit code %d\n",(int)tret);

	return 0;
}


