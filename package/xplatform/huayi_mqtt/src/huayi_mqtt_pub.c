#include "huayi_mqtt.h"

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


void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	int i;  
	if(!result)
	{     
		/* Subscribe to broker information topics on successful connect. */ 
		MQTT_LOG("connect success");
		//mosquitto_subscribe(mosq, NULL, topic1, 2);  
	}else{    
		fprintf(stderr, "Connect failed\n");    
	}
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{  
	int i;   
	MQTT_LOG("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++)
	{       
		MQTT_LOG(", %d", granted_qos[i]);  
	}    

	printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{  
	/* Pring all log messages regardless of level. */    
	MQTT_LOG("%s\n", str);
}

int content(char *heartdata)
{
    FILE *file = fopen("/tmp/log/mqtt_pub_cont", "r");
    if (file == NULL) {
        return -1;
    }

	fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';

	fclose(file);
	while (length > 0 && (data[length-1] == '\0' || data[length-1] == '\n' || data[length-1] == '\r' || data[length-1] == ' ')) {
        data[--length] = '\0';
    }
	strcpy(heartdata, data);
    free(data);

	return 0;
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
	int ret = 0,failtime = 0;
	char buff[1024] = {0};
	struct mosquitto *mosq = NULL;
	int heartbeat = 0;
	char imei[RESULT_STR_LEN]={0};
	char host[RESULT_STR_LEN] = {0};
	int port = 0;
	char clientId[RESULT_STR_LEN] = {0};
	char username[RESULT_STR_LEN] = {0},password[RESULT_STR_LEN] = {0};
	
	get_topic();

	ret = mosquitto_lib_init();
	if(ret)
	{
		MQTT_LOG("Init lib error!\n");
        goto fail;
	}

	huayi_uci_get_str(PKG_STATUS_MODEM, "modem", "imei", imei);
	huayi_uci_get_str(PKG_SYSTEM,"mqtt","clientId", clientId);
	mosq = mosquitto_new(clientId,true,NULL);
	if(!mosq)
	{
		MQTT_LOG("create client failed..\n");
		mosquitto_lib_cleanup();
		goto fail;
	}

	MQTT_LOG("get mqtt config!\n");

	huayi_uci_get_str(PKG_SYSTEM,"mqtt","host", host);
	huayi_uci_get_int(PKG_SYSTEM,"mqtt","port", &port);
	huayi_uci_get_str(PKG_SYSTEM,"mqtt","username", username);
	huayi_uci_get_str(PKG_SYSTEM,"mqtt","password", password);

	//mosquitto_log_callback_set(mosq, my_log_callback);  
	mosquitto_connect_callback_set(mosq, my_connect_callback);  
	//mosquitto_message_callback_set(mosq, my_message_callback);   
	//mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);

#if 0
	mosquitto_username_pw_set(mosq, MQUSER, MQPWD);
	while(mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE))
#else
	mosquitto_username_pw_set(mosq, username, password);
	while(mosquitto_connect(mosq, host, port, KEEP_ALIVE))
#endif
	{
		MQTT_LOG("Pub connect server error!\n");
		sleep(5);
	}

	MQTT_LOG("Pub start!\n");

	int loop = mosquitto_loop_start(mosq);

	if(loop != MOSQ_ERR_SUCCESS)
	{
		MQTT_LOG("mosquitto loop error\n");
		goto fail;
	}
	
	while(1)
	{        
		if (mosquitto_loop(mosq, -1, 1) != MOSQ_ERR_SUCCESS)
        {
            MQTT_LOG("Connection lost, trying to reconnect...\n");
            while (mosquitto_reconnect(mosq) != MOSQ_ERR_SUCCESS)
            {
				failtime++;
                sleep(5);
				if(failtime > 20)
				{
					MQTT_LOG("reconnect 20 times,goto fail...\n");
					goto fail;
				}
            }
            MQTT_LOG("Reconnected successfully!\n");
        }
		failtime = 0;
		huayi_uci_get_int(PKG_SYSTEM,"mqtt","interval", &heartbeat);
		content(buff);
		if(strlen(buff)>0)
		{		
			MQTT_LOG("topic:%s,buff:%s\n",topic2,buff);
			ret = mosquitto_publish(mosq,NULL,topic2,strlen(buff)+1,buff,0,0);
			memset(buff,0,sizeof(buff));
		}
		usleep(heartbeat * 1000);
		CHECK_MQTT_LOG();
	}
	
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	MQTT_LOG("End!\n");

	return 0;

fail:
	system("/etc/init.d/hy_cloud_client start");
	return -1;
}
