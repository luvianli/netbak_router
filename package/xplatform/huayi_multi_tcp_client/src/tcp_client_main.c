#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>
#include<sys/socket.h> 
#include<netinet/in.h> 
#include<arpa/inet.h>
#include"huayi_uci_fun.h"
#include"cJSON.h"

enum choose
{
	huayi_heartbeat=200,
	huayi_switch_sim_before,
	huayi_switch_sim_after,
	huayi_sim_info,
	huayi_product,
	huayi_dial4g_init,
	max_mgs_code
}ch;

enum res_choose
{
	res_huayi_heartbeat=300,
	res_huayi_switch_sim_before,
	res_huayi_switch_sim_after,
	res_huayi_sim_info,
	res_huayi_product,
	res_huayi_dial4g_init,
	res_max_mgs_code
}res_ch;


void client_product(); //保留
void client_info(cJSON *infocjson);
int write_client(int client_sockfd,int number);
int read_client(int client_sockfd);




#define SERVER_IP "192.168.1.1"  //客户端、服务端都在一台主机上，所以直接用本机IP地址
#define SERVER_PORT 6666


int client_socket_init()
{
   int client_sockfd;
   struct sockaddr_in server_address;
   
   client_sockfd=socket(AF_INET,SOCK_STREAM,0);
   if(client_sockfd < 0 )
   		return -1;
   memset(&server_address,0,sizeof(server_address));
   server_address.sin_family=AF_INET;
   server_address.sin_port=htons(SERVER_PORT);
   server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
   if (connect(client_sockfd,(struct sockaddr *)&server_address,sizeof(server_address)) < 0)
   {
   		close(client_sockfd);
   		return -1;
   }
   return client_sockfd;
}

int read_client(int client_sockfd)
{
	
	int msg_code,count=0, ret = 0;
	char str_msg_code[SMALL_STR_LEN]={0};
	memset(str_msg_code,0,sizeof(str_msg_code));
	
	ret = write_client(client_sockfd, huayi_dial4g_init);
	if(ret == -1 )
		return ret;
	
	while(1)
	{
		memset(str_msg_code,0,sizeof(str_msg_code));
		read(client_sockfd,str_msg_code,sizeof(str_msg_code));
		msg_code=atoi(str_msg_code);
		
		switch (msg_code)
		{
			case huayi_heartbeat:
				ret = write_client(client_sockfd,huayi_heartbeat);
				sleep(5);
				break;			
			case huayi_switch_sim_before :
				system("killall -9 huayi_atcmd_server");
			    CON_LOG("killall -9 huayi_atcmd_server");
				ret = write_client(client_sockfd, res_huayi_switch_sim_before);
				break;
			case huayi_switch_sim_after :
				system("huayi_atcmd_server /dev/ttyUSB1 &");
				CON_LOG("huayi_atcmd_server /dev/ttyUSB1 &");
				ret = write_client(client_sockfd, res_huayi_switch_sim_after);
				break;
			case huayi_sim_info:
				//client_info(cjson);
				CON_LOG("huayi_sim_info");
				ret = write_client(client_sockfd, res_huayi_sim_info);
				break;
			case huayi_product:
				//client_product();	
				CON_LOG("huayi_product");
				ret = write_client(client_sockfd, res_huayi_product);
				break;
			case huayi_dial4g_init:
				system("/etc/init.d/huayi_dial4_init restart");
				        
				CON_LOG("/etc/init.d/huayi_dial4_init restart");
				ret = write_client(client_sockfd, res_huayi_dial4g_init);
				break;
			default:
				sleep(5);
				ret = write_client(client_sockfd,0);	
				close(client_sockfd);
				return 0;
		}

		if(ret < 0)
			return -1;
	}

	return 0;
}

int write_client(int client_sockfd,int number)
{
	char str_msg_code[SMALL_STR_LEN]={0};
	int ret = 0;
	
	memset(str_msg_code,0,sizeof(str_msg_code));
	sprintf(str_msg_code,"%d",number);
	ret = write(client_sockfd, str_msg_code,sizeof(str_msg_code));
	if(ret != sizeof(str_msg_code))
		return -1;
	
	return 0;
}

int main(int argc, char *argv[])
{
	while(1)
	{
	   int client_sockfd;
	   client_sockfd=client_socket_init();
	   if(client_sockfd == -1)
	   {
	   		sleep(2);
			continue;
	   }
	   if(read_client(client_sockfd) < 0)
	   {
			sleep(2);
			continue;
	   }
	}
   return 0;
}

