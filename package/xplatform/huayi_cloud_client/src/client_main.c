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
	memcpy(heartContent, data, size*nmemb);
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
	if(strlen(post_fields) != 0) {
		// …Ë÷√http∑¢ÀÕµƒƒ⁄»›¿‡–ÕŒ™JSON  
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

/*void* heart_handle()
{
	char url[LIST_STR_LEN] ={ 0 }; 
	char heartData[HEART_REQ_DATALEN] = { 0 };
	CURL *curl = NULL;
	int ret= -1, interval = 0;
	int heartSize = 0;
	char registStatus[LONG_BUFF_LEN]={0};

	huayi_uci_get_int(PKG_SYSTEM,"server","interval", &interval);
	//printf("%s, %d, interval = %d !\n", __FUNCTION__, __LINE__, interval);
	gTimeWait = interval;
	while(1) {
		huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "registStatus", registStatus);
		if (strcmp(registStatus ,"connected") != 0) {
			CLOUD_CLIENT_LOG("%s, modem is not connected; registStatus is : %d\n",__FUNCTION__, registStatus);
			sleep(5);
			continue;
		}
		memset(heartData, 0, sizeof(heartData));
		memset(url, 0, sizeof(url));
		get_url(url);
		//printf("heart url:%s\n",url);
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
}*/

int client_socket(char *ip,int port)
{
   int client_sockfd;
   struct sockaddr_in server_address;
   
   client_sockfd=socket(AF_INET,SOCK_STREAM,0);
   if(client_sockfd < 0 )
   		return -1;
   memset(&server_address,0,sizeof(server_address));
   server_address.sin_family=AF_INET;
   server_address.sin_port=htons(port);
   server_address.sin_addr.s_addr = inet_addr(ip);
   if (connect(client_sockfd,(struct sockaddr *)&server_address,sizeof(server_address)) < 0)
   {
   		close(client_sockfd);
   		return -1;
   }
   return client_sockfd;
}

void* heart_handle()
{
	int wan_up = 0,wired_link = 0,cpe_link = 0;
	char ip[OPTION_STR_LEN] = { 0 },ip_address[IPV4_STR_LEN] = {0};
	int port;
	int server = 1;
	int serial_num = 0;
	char heartData[HEART_REQ_DATALEN] = { 0 };
	int heartSize = 0;
	int interval = 60;
	int status = 0;
	char buffer[1024] = {0},buf[1024] = {0};
	char *newbuf;
	int client_sockfd,written,dataLen;
	
	while(1)
	{
		huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",ip_address);
		huayi_uci_get_int(PKG_STATUS_BLEND, "wired", "link", &wired_link);
		huayi_uci_get_int(PKG_STATUS_BLEND, "cpe", "link", &cpe_link);
		if((wired_link != 1 && cpe_link != 1) || !strncmp(ip_address, "0.0.0.0", strlen("0.0.0.0")))
		{
			CLOUD_CLIENT_LOG("network error!\n");
			sleep(5);
			continue ;
		}
		server = get_server(ip,&port);
		if(server == -1)
		{
			sleep(5);
			continue;
		}
		CLOUD_CLIENT_LOG("%s%d,ip:%s,port:%d\n",__FUNCTION__,__LINE__,ip,port);

		memset(heartData, 0, sizeof(heartData));
		msg_content(heartData,&heartSize,serial_num);
		CLOUD_CLIENT_LOG("===  %s, %d, heartData = %s\n",__FUNCTION__, __LINE__, heartData);

		client_sockfd = -1;
		client_sockfd=client_socket(ip,port);
		if(client_sockfd == -1)
		{
			CLOUD_CLIENT_LOG("socket error!\n");
			sleep(2);
			continue;
		}else CLOUD_CLIENT_LOG("socket succes!\n");

		written = send(client_sockfd, heartData, heartSize,0);
    	if (written <= 0) {
        	CLOUD_CLIENT_LOG("Write error!");
			goto fail;
    	}else CLOUD_CLIENT_LOG("wirte succes!\n");

		//memset(buffer, 0, sizeof(buffer));	
		CLOUD_CLIENT_LOG("11111\n");	
		dataLen = read(client_sockfd,buffer,sizeof(buffer)-1);
		if (dataLen >= 0) {
			//buffer[dataLen] = '\0';
			strncpy(buf,buffer+1,sizeof(buffer)-1);
			newbuf = strchr(buf,'{');
		 	CLOUD_CLIENT_LOG("read:%s,len:%d",newbuf,dataLen);
			msg_parse(newbuf,&status);
			if(status != 0)//ËøîÂõûstatus‰∏çÁ≠â‰∫é0È©¨‰∏äÈáçÊñ∞‰∏äÊä•
			{
				close(client_sockfd);
				serial_num ++;
				continue;
			}
		} else{
		 	CLOUD_CLIENT_LOG("read error!\n");
			sleep(30);
		 	goto fail;
		}

out:
		close(client_sockfd);
		serial_num ++;
		huayi_uci_get_int(PKG_SYSTEM,"server","interval", &interval);
		CLOUD_CLIENT_LOG("gTimeWait = %d !\n",interval);
		CHECK_CLOUD_CLIENT_LOG();
		sleep(interval);
		continue;
fail:
		close(client_sockfd);
		sleep(5);
	}
	return ;
}

int main(int argc, char *argv[])
{
	/*pthread_t heartTid;
	int err;
	void *tret;
	char 	wan_type[OPTION_STR_LEN] = {0};
	
	(void) signal(SIGTERM, huayi_client_main_graceful_exit);

	curl_global_init(CURL_GLOBAL_ALL);

	pthread_create(&heartTid, NULL, heart_handle, NULL);
	
	err = pthread_join(heartTid,&tret);//◊Ë»˚µ»¥˝œﬂ≥ÃidŒ™tid1µƒœﬂ≥Ã£¨÷±µΩ∏√œﬂ≥ÃÕÀ≥ˆ  
    if(err!=0)  
    {   
        printf("can not join with clound client:%s\n",strerror(err));
        exit(-1);
    }       
    printf("clound client exit code %d\n",(int)tret);*/

	heart_handle();

	return 0;
}


