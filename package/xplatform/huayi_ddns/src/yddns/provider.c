/*==============================================================================
//
// Project:
//		userapp-ddns module
//
//------------------------------------------------------------------------------
// Description:
//
//          Support for DDNS
==============================================================================*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <time.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "ddns.h"

extern int SYS_wan_ip;
//==============================================================================
//                                    MACROS
//==============================================================================
#define BUFFER_SIZE		4*1024

//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================

//==============================================================================
//                               LOCAL VARIABLES
//==============================================================================

int DYNDNS_update_entry(struct userInfo_cxy *info);
static struct service_cxy DYNDNS_service = {
	DYNDNS_NAME,
	DYNDNS_update_entry,
	DYNDNS_DEFAULT_SERVER,
	DYNDNS_DEFAULT_PORT,
	DYNDNS_REQUEST
};

static struct service_cxy QDNS_service = {
	QDNS_NAME,
	DYNDNS_update_entry,
	QDNS_DEFAULT_SERVER,
	QDNS_DEFAULT_PORT,
	QDNS_REQUEST
};

int TZO_update_entry(struct userInfo_cxy *info);
static struct service_cxy TZO_service = {
  	TZO_NAME,           
  	TZO_update_entry,   
  	TZO_DEFAULT_SERVER, 
  	TZO_DEFAULT_PORT,   
  	TZO_REQUEST         
};

int NOIP_update_entry(struct userInfo_cxy *info);
static struct service_cxy NOIP_service = {
    NOIP_NAME,           
    NOIP_update_entry,   
    NOIP_DEFAULT_SERVER, 
    NOIP_DEFAULT_PORT,   
    NOIP_REQUEST         
};

int INTELBRAS_update_entry(struct userInfo_cxy *info);
static struct service_cxy INTELBRAS_service = {
	INTELBRAS_NAME,
	INTELBRAS_update_entry,
	INTELBRAS_DEFAULT_SERVER,
	INTELBRAS_DEFAULT_PORT,
	INTELBRAS_REQUEST
};
static struct service_cxy *service_list[]={
	&DYNDNS_service,
	&QDNS_service,
	&TZO_service,
	&NOIP_service,
	&INTELBRAS_service,
	NULL,
};

//==============================================================================
//                          LOCAL FUNCTION PROTOTYPES
//==============================================================================

//==============================================================================
//                              EXTERNAL FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
// 
//------------------------------------------------------------------------------
void usage(void)
{
	//printf("\r\nSample config: yddns -s service domainname/email username/hostname password/mac ip\r\n");
	//printf("eg:yddns -s qdns chenfei105105.3322.org chenfei105 18680311896 219.22.66.55\r\n");

	//printf("\r\nSample config: yddns -u service email hostname mac ip\r\n");	//for intelbras
	//printf("More config: yddns -m type domainname username password ip mx backmx trytime update_time\r\n");
}
//------------------------------------------------------------------------------
struct service_cxy *find_service(char *name)
{
	struct service_cxy *serv;
	int i;
	
	for(i=0; service_list[i]; i++)
	{
		serv = service_list[i];
		if(strcmp(serv->name, name) == 0)
			return serv;
	}
	return NULL;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
static sigjmp_buf jmpbuf;
static void alarm_func()
{
     siglongjmp(jmpbuf, 1);
}

struct hostent *gngethostbyname(char *HostName, int timeout)
{
     struct hostent *lpHostEnt;
 
     signal(SIGALRM, alarm_func);
     if(sigsetjmp(jmpbuf, 1) != 0)
     {
           alarm(0);//timout
           signal(SIGALRM, SIG_IGN);
           return NULL;
     }
	 
     alarm(timeout);//setting alarm
     lpHostEnt = gethostbyname(HostName);
     signal(SIGALRM, SIG_IGN);
 
     return lpHostEnt;
}

int do_connect(int *sock, char *host, unsigned short port)
{
	struct ifreq ifr;
	struct sockaddr_in addr;
	struct hostent *hostinfo;
	char if_name[IFNAMSIZ] = {0};
	int len,result;

	// set up the socket
	if((*sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return(-1);
    }
		
	// get the host address
	//hostinfo = gethostbyname(host);  
	hostinfo = gngethostbyname(host, 2); 
	if(!hostinfo)
	{
		printf("ddns gethostbyname timeout"); 
		close(*sock);
		return(-1);
    }

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","if_name",if_name);
	strncpy(ifr.ifr_name,if_name,IFNAMSIZ);
	if (setsockopt(*sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0)
	{
		perror("setsockopt");
		close(*sock);
		return(-1);
	}

	addr.sin_family = AF_INET; 
	addr.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
	addr.sin_port = htons(port);
	// connect the socket
	len = sizeof(addr);
	if((result=connect(*sock, (struct sockaddr *)&addr, len)) == -1) 
	{
		perror("connect");
		close(*sock);
		return(-1);
	}

	return 0;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
static void output(int fd, void *buf)
{
	fd_set writefds;
	int max_fd;
	struct timeval tv;
	int ret;

	FD_ZERO(&writefds);
	FD_SET(fd, &writefds);
	max_fd = fd;
	
	tv.tv_sec = 0;
    tv.tv_usec = 20000;          /* timed out every 20 ms */
    
	ret = select(max_fd + 1, NULL, &writefds, NULL, &tv);

	if(ret == -1)
	{
    	printf("[DDNS]: select error\n");
	}
	else if(ret == 0)
	{
		printf("select timeout\n");
	}
	else
	{
		/* if we woke up on client_sockfd do the data passing */
		if(FD_ISSET(fd, &writefds))
		{
			if(send(fd, buf, strlen(buf), 0) == -1)
				printf("error send()ing request\n");
		}
		else
			printf("socket was not exist!!\n");
    }
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int read_input(int fd, void *buf, int len)
{
	fd_set readfds;
	int max_fd;
	struct timeval tv;
	int ret;
	int bread = -1;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	max_fd = fd;
	
	tv.tv_sec = 9;
	tv.tv_usec = 0;          
	ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);

	if(ret == -1)
	{
		printf("[DDNS]: select error\n");
	}
	else if(ret == 0)
	{
		printf("select timeout\n");
	}
	else
	{
		/* if we woke up on client_sockfd do the data passing */
		if(FD_ISSET(fd, &readfds))
		{
			if((bread=recv(fd, buf, len, 0)) == -1)
			{
				printf("error recv()ing reply\n");
			}
		}
		else
			printf("socket was not exist!!\n");
	}

  return(bread);
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int DYNDNS_update_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];

	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("\r\nerror connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(bp, "GET %s?system=dyndns&", info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "hostname=%s&", info->host);
	bp += len;
	
	if(MY_IP)
	{
		len = sprintf(bp, "myip=%s&", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp, "wildcard=%s&", info->wildcard ? "ON" : "OFF");
	bp += len;
 	
	if(*(info->mx) != '\0')
	{
		len = sprintf(bp, "mx=%s&", info->mx);
		bp += len;
	}

	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"Authorization: Basic %s\r\n"
 						"User-Agent: %s\r\n"
 						"Host: %s\r\n\r\n",
 						auth,
						DEVICE_NAME,
						info->service->default_server);
 						
	bp += len;	
	*bp = '\0';	
	
	output(fd, buf);
	printf("\r\nsendmessage:\r\n%s",buf);
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';
	printf("\r\n%s\r\n",buf);
	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		printf("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "\ngood ") != NULL)
		{
			retval = UPDATERES_OK;
		}
		else
		{
			if(strstr(buf, "\nnohost") != NULL)
			{
				printf("invalid hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnotfqdn") != NULL)
			{
				printf("malformed hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\n!yours") != NULL)
			{
				printf("host is not under your control\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nabuse") != NULL)
			{
				printf("host has been blocked for abuse\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnochg") != NULL)
			{
				printf("your IP address has not changed since the last update\n");
				retval = UPDATERES_OK;
			}
			else if(strstr(buf, "\nbadauth") != NULL)
			{
				printf("authentication failure\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadsys") != NULL)
			{
				printf("invalid system parameter\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadagent") != NULL)
			{
				printf("this useragent has been blocked\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnumhost") != NULL)
			{
				printf("Too many or too few hosts found\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\ndnserr") != NULL)
			{
				printf("dyndns internal error\n");
				retval = UPDATERES_ERROR;
			}
			else
			{
				printf("error processing request\n");
				retval = UPDATERES_ERROR;
			}
		}
		break;

	case 401:
		printf("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int TZO_update_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char *p;

  	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("\r\nerror connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(buf,	"GET %s?TZOName=%s&Email=%s&TZOKey=%s&IPAddress=%s&"
						"HTTP/1.0\r\n"
						"User-Agent: %s-%s %s [%s]\r\n"
						"Host: %s\r\n\r\n",
						info->service->default_request,
						info->host,
						info->usrname,
	            		info->usrpwd,
	            		MY_IP_STR,
	            		DEVICE_NAME, FW_VERSION, FW_OS, AUTHOR,
	            		info->service->default_server);
	
	bp += len;
	
	*bp = '\0';
	output(fd, buf);       		
  
	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';

	//by chenfei 2012-7-25
	if( (p = strstr(buf, "TZO Name:")) == NULL )
	{
		printf("not find!\n");
		ret = 0;
		goto error;	
	}
	else
	{
		ret = 1;	
	}
	
  	switch(ret)
  	{
  		case TZO_SUCCESS:
  			printf("successfully!\n");
			retval = UPDATERES_OK;
  			break;
  			
  		default:
  			printf("authentication failure!\n");
  			retval = UPDATERES_SHUTDOWN;
			break;
  	}

error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int NOIP_update_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char *p;

  	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
  	bp = buf;
  	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("\r\nerror connecting to %s\n", info->service->default_server);
		retval = UPDATERES_ERROR;
		goto error;
	}
		
	len = sprintf(buf,"GET http://%s/update.php?username=%s&pass=%s&host=%s"
	            "&ip=%s HTTP/1.0\nUser-Agent: %s DUC %s\n\n",
	            info->service->default_request,
	            info->usrname,
	            info->usrpwd,
	            info->host,
	            MY_IP_STR,
	            DEVICE_NAME,
	            FW_VERSION);
  	
  	bp += len;
	
	*bp = '\0';
	output(fd, buf); 
	//printf("NOIP_update_entry: output buf:%s\n", buf);
  	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
  	//printf("\nNOIP_update_entry: buf:%s\n", buf);

	if( (p = strstr(buf, "status=")) == NULL )
	{
		printf("not find status!\n");
		goto error;	
	}
	else
	{
		//printf("get string!!!!! p:%s p[7]:%s\n", p, &p[7]);
		ret = atoi(&p[7]);	
	}
		
  	printf("\nret:%d\n", ret);
  	switch(ret)
  	{
  		case NOIP_SUCCESS:
  		case NOIP_ALREADYSET:
  			printf("successfully!\n");
			retval = UPDATERES_OK;
  			break;
  			
  		default:
  			printf("authentication failure!\n");
  			retval = UPDATERES_SHUTDOWN;
			break;
  	}
error:
	free(buf);
	return retval;
}

	
//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int INTELBRAS_update_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char keybuf[128]={0};

	intelbrasCrypt(info->devmac, keybuf);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;

	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("\r\nerror connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(bp, "GET http://%s%s?", info->service->default_server, info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "mac=%s", keybuf);
	bp += len;
	
	len = sprintf(bp, "&hostname=%s", info->usrname);
	bp += len;

	if(MY_IP)
	{
		len = sprintf(bp, "&MyIP=%s", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp, "&devmodel=%s&port=%d&hddmodel=%d&wifi=%s&voip=%s&swversion=%s&devserial=%d", 
				INTELBRAS_DEVMODEL,
				INTELBRAS_PORT,
				INTELBRAS_HDDMODEL,
				INTELBRAS_WIFI,
				INTELBRAS_VOIP,
				FW_VERSION,
				INTELBRAS_DEVSN);
	bp += len;

	len = sprintf(bp, "&email=%s&product=%s", info->host, INTELBRAS_PRODUCT);
	bp += len;
	
	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"User-Agent: %s\r\n"
 						"Host: %s\r\n\r\n",
						DEVICE_NAME,
						info->service->default_server); 						
	bp += len;	
	*bp = '\0';	
	
	output(fd, buf);
	//printf("\r\n[ddns dbg]sendmessage:\r\n%s",buf);
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	//printf("\r\n[ddns dbg] ret=%d\n", ret);
	switch(ret)
	{
	case -1:
		printf("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "\n901") != NULL)
		{
			printf("invalid hostname\n");
			//retval = UPDATERES_ERROR;
			retval = 901;
		}
 		else if(strstr(buf, "\n902") != NULL)
		{
			printf("invalid mac\n");
			//retval = UPDATERES_ERROR;
			retval = 902;
		}
		else if(strstr(buf, "\n903") != NULL)
		{
			printf("mac not in approved list\n");
			//retval = UPDATERES_ERROR;
			retval = 903;
		}
		else if(strstr(buf, "\n904") != NULL)
		{
			printf("invaild ip\n");
			//retval = UPDATERES_ERROR;
			retval = 904;
		}
		else if(strstr(buf, "\n905") != NULL)
		{
			printf("invaild dev model\n");
			//retval = UPDATERES_ERROR;
			retval = 905;
		}
		else if(strstr(buf, "\n906") != NULL)
		{
			printf("invaild port\n");
			//retval = UPDATERES_ERROR;
			retval = 906;
		}
		else if(strstr(buf, "\n907") != NULL)
		{
			printf("inuse\n");
			//retval = UPDATERES_ERROR;
			retval = 907;
		}
		else if(strstr(buf, "\n908") != NULL)
		{
			printf("nochng\n");
			//retval = UPDATERES_OK;
			retval = 908;
		}
		else if(strstr(buf, "\n909") != NULL)
		{
			printf("update with data change\n");
			//retval = UPDATERES_OK;
			retval = 909;
		}
		else if(strstr(buf, "\n910") != NULL)
		{
			printf("create\n");
			//retval = UPDATERES_OK;
			retval = 910;
		}
		else if(strstr(buf, "\n911") != NULL)
		{
			printf("flood\n");
			//retval = UPDATERES_ERROR;
			retval = 911;
		}
		else if(strstr(buf, "\n912") != NULL)
		{
			printf("server error\n");
			//retval = UPDATERES_ERROR;
			retval = 912;
		}
		else if(strstr(buf, "\n913") != NULL)
		{
			printf("invaild hdd model\n");
			//retval = UPDATERES_ERROR;
			retval = 913;
		}
		else if(strstr(buf, "\n914") != NULL)
		{
			printf("invaild wifi\n");
			//retval = UPDATERES_ERROR;
			retval = 914;
		}
		else if(strstr(buf, "\n915") != NULL)
		{
			printf("invaild voip\n");
			//retval = UPDATERES_ERROR;
			retval = 915;
		}
		else if(strstr(buf, "\n916") != NULL)
		{
			printf("invaild software vesion\n");
			//retval = UPDATERES_ERROR;
			retval = 916;
		}
		else if(strstr(buf, "\n917") != NULL)
		{
			printf("invaild dev serial\n");
			//retval = UPDATERES_ERROR;
			retval = 917;
		}
		else if(strstr(buf, "\n918") != NULL)
		{
			printf("invaild email\n");
			//retval = UPDATERES_ERROR;
			retval = 918;
		}
		/*
		else
		{
			printf("error processing request\n");
			//retval = UPDATERES_ERROR;
		}
		*/
		break;

	case 400:
		printf("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int INTELBRAS_update_wanip_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char keybuf[128]={0};

	intelbrasCrypt(info->devmac, keybuf);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;

	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("\r\nerror connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(bp, "GET http://%s%s?", info->service->default_server, "/update.aspx");
	bp += len;
	
	len = sprintf(bp, "mac=%s", keybuf);
	bp += len;
	
	if(MY_IP)
	{
		len = sprintf(bp, "&myip=%s", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp, "&port=%d", INTELBRAS_PORT);
	bp += len;

	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"User-Agent: %s\r\n"
 						"Host: %s\r\n\r\n",
						DEVICE_NAME,
						info->service->default_server); 						
	bp += len;	
	*bp = '\0';	
	
	output(fd, buf);
	printf("\r\n[ddns dbg]wanip_entry sendmessage:\r\n%s",buf);
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	printf("\r\n[ddns dbg]wanip_entry ret=%d\n", ret);
	switch(ret)
	{
	case -1:
		printf("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "\n701") != NULL)
		{
			printf("no record found\n");
			//retval = UPDATERES_ERROR;
			retval = 701;
		}
 		else if(strstr(buf, "\n801") != NULL)
		{
			printf("successful update\n");
			//retval = UPDATERES_OK;
			retval = 801;
		}
		else if(strstr(buf, "\n902") != NULL)
		{
			printf("invalid mac\n");
			//retval = UPDATERES_ERROR;
			retval = 902;
		}
		else if(strstr(buf, "\n903") != NULL)
		{
			printf("mac not allowed\n");
			//retval = UPDATERES_ERROR;
			retval = 903;
		}
		else if(strstr(buf, "\n904") != NULL)
		{
			printf("invaild ip\n");
			//retval = UPDATERES_ERROR;
			retval = 904;
		}
		else if(strstr(buf, "\n906") != NULL)
		{
			printf("invaild port\n");
			//retval = UPDATERES_ERROR;
			retval = 906;
		}
		else if(strstr(buf, "\n911") != NULL)
		{
			printf("flood detected\n");
			//retval = UPDATERES_ERROR;
			retval = 911;
		}
		else if(strstr(buf, "\n912") != NULL)
		{
			printf("server error\n");
			//retval = UPDATERES_ERROR;
			retval = 912;
		}
		else if(strstr(buf, "\n929") != NULL)
		{
			printf("expired\n");
			//retval = UPDATERES_ERROR;
			retval = 929;
		}
		/*
		else
		{
			printf("error processing request\n");
			retval = UPDATERES_ERROR;
		}
		*/
		break;

	case 400:
		printf("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}

int ping_test(char *srv,int count,char *ipaddr)
{
	FILE *fp;
 	char buf[128];
	int opmode;
	
	sprintf(buf, "ping %s -w %d | grep \"PING\"|cut -d \" \" -f3", srv, count);
    fp = popen(buf, "r");
	if (fp==NULL)
		return -1;

	if (NULL == fgets(buf, sizeof(buf),fp)) {
		pclose(fp);
		return -1;
	}
	pclose(fp);

	memcpy(ipaddr, buf+1, strlen(buf)-4);

	return 0;
}

