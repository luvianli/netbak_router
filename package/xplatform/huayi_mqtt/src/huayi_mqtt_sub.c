#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <string.h>

#define MSG_MAX_SIZE 512
#define KEEP_ALIVE 60

static int running = 1;

void my_connect_callback(struct mosquitto *mosq, void *obj, int rc)
{        
	printf("Call the function: on_connect\n");
	if(rc)
	{       
		// 连接错误，退出程序  
		printf("on_connect error!\n");    
		exit(1);       
	}else{
		// 订阅主题   
		// 参数：句柄、id、订阅的主题、qos 
		if(mosquitto_subscribe(mosq, NULL, "topic2", 2))
		{     
			printf("Set the topic error!\n"); 
			exit(1);        
		}        
	}
}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{ 
	printf("Call the function: my_disconnect_callback\n");      
	running = 0;
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{       
	printf("Call the function: on_subscribe\n");
}

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{       
	printf("Call the function: on_message\n"); 
	printf("Recieve a message of %s : %s\n", (char *)msg->topic, (char *)msg->payload);  
	if(0 == strcmp(msg->payload, "quit"))
	{           
		mosquitto_disconnect(mosq);      
	}
}


int main(int argc,char *argv[])
{
	int ret = 0;
	char buff[MSG_MAX_SIZE] = {0};
	struct mosquitto *mosq = NULL;

	//初始化libmosquitto库
	ret = mosquitto_lib_init();
	if(ret)
	{
		printf("Init lib error!\n");
        return -1;
	}

	mosq = mosquitto_new("sub_test",true,NULL);
	if(!mosq)
	{
		printf("create client failed..\n");
		mosquitto_lib_cleanup();
		return -1;
	}

	// 设置回调函数       
	// 参数：句柄、回调函数       
	mosquitto_connect_callback_set(mosq, my_connect_callback);	
	mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);      
	mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);    
	mosquitto_message_callback_set(mosq, my_message_callback);
	
	if(mosquitto_connect_async(mosq, "127.0.0.1", 1883, KEEP_ALIVE))
	{
		printf("Connect server error!\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();

		return -1;
	}

	printf("Start!\n");

	//mosquitto_loop_start作用是开启一个线程，在线程里不停的调用 mosquitto_loop() 来处理网络信息
	int loop = mosquitto_loop_start(mosq);

	if(loop)
	{
		printf("mosquitto loop error\n");
		mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
		return -1;
	}
	
	while(running)
    {
		//mosquitto_loop(mosq, -1, 1);
        sleep(1);
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