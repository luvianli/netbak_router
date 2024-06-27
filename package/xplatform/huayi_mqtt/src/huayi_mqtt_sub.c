#include "huayi_mqtt.h"

void my_connect_callback(struct mosquitto *mosq, void *obj, int rc)
{        
	printf("Call the function: on_connect\n");
	if(rc)
	{       
		// 连接错误，退出程序  
		MQTT_LOG("on_connect error!\n");    
		exit(1);       
	}else{
		// 订阅主题   
		// 参数：句柄、id、订阅的主题、qos 
		if(mosquitto_subscribe(mosq, NULL, topic1, 2))
		{     
			MQTT_LOG("Set the topic error!\n"); 
			exit(1);        
		}        
	}
}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{ 
	//MQTT_LOG("Call the function: my_disconnect_callback\n");      
	running = 0;
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{       
	//MQTT_LOG("Call the function: on_subscribe\n");
}

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{       
	//MQTT_LOG("Call the function: on_message\n"); 
	MQTT_LOG("Recieve a message of %s : %s", (char *)msg->topic, (char *)msg->payload);  
	if(0 == strcmp(msg->payload, "quit"))
	{           
		mosquitto_disconnect(mosq);      
	}

	FILE *file = fopen("/tmp/log/mqtt_sub_cont", "w");
	if (file == NULL) {
		MQTT_LOG("Error opening file");
    }
 
    fprintf(file, "%s", msg->payload);
    fclose(file);
	sleep(2);
	system("huayi_mqtt_setsub &");
}

void get_topic()
{
	char version[SMALL_STR_LEN] = {0};
	char imei[RESULT_STR_LEN]={0};
	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version", version);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", imei);
	sprintf(topic1,"iotv2/zyc100/%s/%s",version,imei);
	sprintf(topic2,"iotv2/zyc100/%s/push",version);
	MQTT_LOG("topic1:%s,2:%s",topic1,topic2);

	return ;
}

int main(int argc,char *argv[])
{
	int ret = 0;
	char buff[MSG_MAX_SIZE] = {0};
	struct mosquitto *mosq = NULL;
	get_topic();
	
	//初始化libmosquitto库
	ret = mosquitto_lib_init();
	if(ret)
	{
		MQTT_LOG("Init lib error!\n");
        return -1;
	}

	mosq = mosquitto_new("sub_test",true,NULL);
	if(!mosq)
	{
		MQTT_LOG("create client failed..\n");
		mosquitto_lib_cleanup();
		return -1;
	}

	// 设置回调函数       
	// 参数：句柄、回调函数       
	mosquitto_connect_callback_set(mosq, my_connect_callback);	
	mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);      
	mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);    
	mosquitto_message_callback_set(mosq, my_message_callback);

	mosquitto_username_pw_set(mosq, MQUSER, MQPWD);		
	if(mosquitto_connect_async(mosq, HOST, PORT, KEEP_ALIVE))
	{
		MQTT_LOG("Connect server error!\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();

		return -1;
	}

	MQTT_LOG("Start!\n");

	//mosquitto_loop_start作用是开启一个线程，在线程里不停的调用 mosquitto_loop() 来处理网络信息
	int loop = mosquitto_loop_start(mosq);

	if(loop)
	{
		printf("mosquitto loop error\n");
		mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
		return -1;
	}
	
	while(1)
    {
		//mosquitto_loop(mosq, -1, 1);
        sleep(10);
     }

	#if 0
	while(fgets(buff, MSG_MAX_SIZE, stdin) != NULL)
	{
		/*发布消息*/
		mosquitto_publish(mosq,NULL,"topic1",strlen(buff)+1,buff,0,0);
		memset(buff,0,sizeof(buff));
	}
	#endif

	mosquitto_loop_stop(mosq, false);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	printf("End!\n");

	return 0;
}