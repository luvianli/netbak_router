#include "huayi_mqtt.h"

void my_connect_callback(struct mosquitto *mosq, void *obj, int rc)
{        
	printf("Call the function: on_connect\n");
	if(rc)
	{       
		// ���Ӵ����˳�����  
		MQTT_LOG("on_connect error!\n");    
		exit(1);       
	}else{
		// ��������   
		// �����������id�����ĵ����⡢qos 
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
#if 0
	char version[SMALL_STR_LEN] = {0};
	char imei[RESULT_STR_LEN]={0};
	huayi_uci_get_str(PKG_ABOUT,"about_software","fw_version", version);
	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", imei);
	sprintf(topic1,"iotv2/zyc100/%s/%s",version,imei);
	sprintf(topic2,"iotv2/zyc100/%s/push",version);
#else
	huayi_uci_get_str(PKG_SYSTEM,"mqtt","topicIn", topic1);
	huayi_uci_get_str(PKG_SYSTEM,"mqtt","topicOut", topic2);
#endif
	MQTT_LOG("topic1:%s,2:%s",topic1,topic2);
	return ;
}

int main(int argc,char *argv[])
{
	int ret = 0;
	char buff[MSG_MAX_SIZE] = {0};
	struct mosquitto *mosq = NULL;
	char host[RESULT_STR_LEN] = {0};
	int port = 0;
	char clientid[RESULT_STR_LEN] = {0},clientId[RESULT_STR_LEN] = {0};
	char username[RESULT_STR_LEN] = {0},password[RESULT_STR_LEN] = {0};

	get_topic();
	
	//��ʼ��libmosquitto��
	ret = mosquitto_lib_init();
	if(ret)
	{
		MQTT_LOG("Init lib error!\n");
        return -1;
	}

	huayi_uci_get_str(PKG_SYSTEM,"mqtt","clientId", clientid);
	sprintf(clientId, "sub%s", clientid);
	mosq = mosquitto_new(clientId,true,NULL);
	if(!mosq)
	{
		MQTT_LOG("create client failed..\n");
		mosquitto_lib_cleanup();
		return -1;
	}

	huayi_uci_get_str(PKG_SYSTEM,"mqtt","host", host);
	huayi_uci_get_int(PKG_SYSTEM,"mqtt","port", &port);
	huayi_uci_get_str(PKG_SYSTEM,"mqtt","username", username);
	huayi_uci_get_str(PKG_SYSTEM,"mqtt","password", password);
	
	mosquitto_connect_callback_set(mosq, my_connect_callback);	
	mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);      
	mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);    
	mosquitto_message_callback_set(mosq, my_message_callback);

#if 0
	mosquitto_username_pw_set(mosq, MQUSER, MQPWD);
	while(mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE))
#else
	mosquitto_username_pw_set(mosq, username, password);
	while(mosquitto_connect(mosq, host, port, KEEP_ALIVE))
#endif
	{
		MQTT_LOG("Sub connect server error!\n");
		sleep(5);
	}

	MQTT_LOG("Sub start!\n");

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
		/*������Ϣ*/
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