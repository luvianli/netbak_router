#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h> //struct sockadd_in 结构体的头文件
#include<sys/ioctl.h>
#include<errno.h>
#include<sys/socket.h>
#include"huayi_uci_fun.h"
#include"huayi_atcmd.h"
#include <fcntl.h>
#include <sys/epoll.h>

struct DataPacket datapacket;
struct DataPacket receivePackets;

int client_socket_init()
{
   int client_sockfd;
   struct sockaddr_in server_address;

   if ((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        SERVER_SOCKET_LOG("Socket creation failed");
        return -1;
    }
   
   memset(&server_address,0,sizeof(server_address));
   server_address.sin_family=AF_INET;
   server_address.sin_port=htons(6666);
   server_address.sin_addr.s_addr = inet_addr("192.168.64.1");


   if (connect(client_sockfd,(struct sockaddr *)&server_address,sizeof(server_address)) == -1){
   		close(client_sockfd);
        SERVER_SOCKET_LOG("Failed to connect to server");
   		return -1;
   }
   SERVER_SOCKET_LOG("connect to server ...");
   return client_sockfd;
}

_BOOL RepeatWriteServer(int fd){
    
    int written = send(fd, &datapacket, sizeof(datapacket), 0);
    if (written == -1) {
    } else if (written < sizeof(datapacket)){
        SERVER_SOCKET_LOG("Only partial data was written");
    } else {
        SERVER_SOCKET_LOG("Write#fd:%d# choose:%d# cmdBuf:%s# returnValue:%s#",datapacket.sockfd,datapacket.choose,datapacket.cmdBuf,datapacket.returnValue);
        return _true;
    }
    return _false;
}

_BOOL RepeatReceivingResultJudgment(int fd){

    memset(&receivePackets, 0, sizeof(receivePackets));
    int dataLen = recv(fd, &receivePackets, sizeof(receivePackets), 0);
    if (dataLen > 0) {
            SERVER_SOCKET_LOG("###read######fd:%d #choose:%d#cmdBuf:%s# returnValue:%s",receivePackets.sockfd,datapacket.choose,receivePackets.cmdBuf,receivePackets.returnValue);
            return _true;
        } else if (dataLen == 0) {
            SERVER_SOCKET_LOG("Connection closed\n");
        } else if (dataLen == -1) {
            SERVER_SOCKET_LOG("Failed to receive data");
        }
    return _false;
}
_BOOL RepeatPerformServerTransfer(int fd){
    int ret = 0;
    char resultbuf[LONG_BUFF_LEN] = {0};
    datapacket.choose=TcpStoCmd;
    datapacket.sockfd=fd;
    strncpy(datapacket.cmdBuf,"heartbeat", sizeof(datapacket.cmdBuf));
    strncpy(datapacket.returnValue,"", sizeof(datapacket.returnValue));
    if ( LTE_TRUE != atcmd_check()){
		system("etc/init.d/huayi_dial4_init restart");
        SERVER_SOCKET_LOG("one etc/init.d/huayi_dial4_init restart");
	}
    SERVER_SOCKET_LOG("====");
    RepeatWriteServer(fd);
   do{
    SERVER_SOCKET_LOG("====");
        ret = RepeatReceivingResultJudgment(fd);
        if(receivePackets.choose == TcpStoCmd){
            SERVER_SOCKET_LOG("read %s resultbuf:%s",receivePackets.cmdBuf,resultbuf);
            SERVER_SOCKET_LOG("====");
            memset(resultbuf,0,sizeof(resultbuf));
            SERVER_SOCKET_LOG("====");
            memset(datapacket.returnValue,0,sizeof(datapacket.returnValue));
            memset(datapacket.cmdBuf,0,sizeof(datapacket.cmdBuf));
            if(strstr(receivePackets.cmdBuf,"&") == NULL){
                            SERVER_SOCKET_LOG("====");
                get_cmd_result(receivePackets.cmdBuf, resultbuf, sizeof(resultbuf));
                SERVER_SOCKET_LOG("====");
                strncpy(datapacket.returnValue,resultbuf, sizeof(datapacket.returnValue));
                strncpy(datapacket.cmdBuf,"", sizeof(datapacket.returnValue));
            }else{
                            SERVER_SOCKET_LOG("====");
                system(receivePackets.cmdBuf);
                            SERVER_SOCKET_LOG("====");
            }
            SERVER_SOCKET_LOG("exe %s resultbuf:%s",receivePackets.cmdBuf,resultbuf);
            ret = RepeatWriteServer(fd);
            SERVER_SOCKET_LOG("====");
        }
    } while(ret == _true);
    return _false;
}


int main(int argc, char *argv[])
{
	while(1)
	{
	   int client_sockfd;
	   client_sockfd=client_socket_init();
	   if(client_sockfd == -1)
	   {
            SERVER_SOCKET_LOG("====");
	   		sleep(2);
			continue;
	   }
	   if(RepeatPerformServerTransfer(client_sockfd) == _false){
            sleep(2);
            close(client_sockfd);
            SERVER_SOCKET_LOG("====");
            continue;
	   }
	}
   return 0;
}

