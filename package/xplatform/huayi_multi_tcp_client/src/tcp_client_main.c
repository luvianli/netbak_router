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

void client_product(); //保留
void client_info(cJSON *infocjson);
void write_client(int client_sockfd,int number);
void read_client(int client_sockfd);




#define SERVER_IP "192.168.1.1"  //客户端、服务端都在一台主机上，所以直接用本机IP地址
#define SERVER_PORT 6666


int client_socket_init()
{
   int client_sockfd;
   struct sockaddr_in server_address;
   
   client_sockfd=socket(AF_INET,SOCK_STREAM,0);
   memset(&server_address,0,sizeof(server_address));
   server_address.sin_family=AF_INET;
   server_address.sin_port=htons(SERVER_PORT);
   server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
   connect(client_sockfd,(struct sockaddr *)&server_address,sizeof(server_address));

   return client_sockfd;
}

void read_client(int client_sockfd)
{
	
	int msg_code,count=0;
	char str_msg_code[SMALL_STR_LEN]={0};
	memset(str_msg_code,0,sizeof(str_msg_code));
	
	write_client(client_sockfd,0);
	


	while(1)
	{
		memset(str_msg_code,0,sizeof(str_msg_code));
		read(client_sockfd,str_msg_code,sizeof(str_msg_code));
		msg_code=atoi(str_msg_code);
		

		
		switch (msg_code)
		{
			case huayi_heartbeat:
				write_client(client_sockfd,huayi_heartbeat);
				sleep(5);
				break;
				
			case huayi_switch_sim_before :
				system("killall -9 huayi_atcmd_server");
			    CON_LOG("killall -9 huayi_atcmd_server");
				write_client(client_sockfd,huayi_switch_sim_before);
				break;
			case huayi_switch_sim_after :
				system("huayi_atcmd_server /dev/ttyUSB1 &");
				CON_LOG("huayi_atcmd_server /dev/ttyUSB1 &");
				write_client(client_sockfd,huayi_switch_sim_after);
				break;
			case huayi_sim_info:
				//client_info(cjson);
				CON_LOG("huayi_sim_info");
				write_client(client_sockfd,huayi_sim_info);
				break;
			case huayi_product:
				//client_product();	
				CON_LOG("huayi_product");
				write_client(client_sockfd,huayi_product);
				break;
			case huayi_dial4g_init:
				system("/etc/init.d/huayi_dial4_init start");
				        
				CON_LOG("/etc/init.d/huayi_dial4_init start");
				write_client(client_sockfd,huayi_dial4g_init);
				break;
			default:
				sleep(5);
				write_client(client_sockfd,0);	
				close(client_sockfd);
				return;
		}
	}
}

void write_client(int client_sockfd,int number)
{

	char str_msg_code[SMALL_STR_LEN]={0};
	memset(str_msg_code,0,sizeof(str_msg_code));


	sprintf(str_msg_code,"%d",number);
	
	write(client_sockfd, str_msg_code,sizeof(str_msg_code));
	

}





int main(int argc, char *argv[])
{
	while(1)
	{
	   int client_sockfd;
	   client_sockfd=client_socket_init();
	   read_client(client_sockfd);
	}
   return 0;
}

