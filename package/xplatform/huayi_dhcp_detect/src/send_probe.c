#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include <errno.h>
#include <pthread.h>

#include <sys/ioctl.h>   
#include <linux/if.h> 
#include <fcntl.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_ether.h>
#include <sys/select.h>
#include <sys/time.h>

#include "jobqueen.h"
#include "send_probe.h"
#include <signal.h>
#include <setjmp.h>

#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"
/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_SUBNET		0x01
#define DHCP_TIME_OFFSET	0x02
#define DHCP_ROUTER		0x03
#define DHCP_TIME_SERVER	0x04
#define DHCP_NAME_SERVER	0x05
#define DHCP_DNS_SERVER		0x06
#define DHCP_LOG_SERVER		0x07
#define DHCP_COOKIE_SERVER	0x08
#define DHCP_LPR_SERVER		0x09
#define DHCP_HOST_NAME		0x0c
#define DHCP_BOOT_SIZE		0x0d
#define DHCP_DOMAIN_NAME	0x0f
#define DHCP_SWAP_SERVER	0x10
#define DHCP_ROOT_PATH		0x11
#define DHCP_IP_TTL		0x17
#define DHCP_MTU		0x1a
#define DHCP_BROADCAST		0x1c
#define DHCP_NTP_SERVER		0x2a
#define DHCP_WINS_SERVER	0x2c
#define DHCP_REQUESTED_IP	0x32
#define DHCP_LEASE_TIME		0x33
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_T1			0x3a
#define DHCP_T2			0x3b
#define DHCP_VENDOR		0x3c
#define DHCP_CLIENT_ID		0x3d
#define DHCP_FQDN		0x51
#define DHCP_END		0xFF

char *ifName;
JOB_BOOL gRun;

struct dhcpMessage {
	u_int8_t op;
	u_int8_t htype;
	u_int8_t hlen;
	u_int8_t hops;
	u_int32_t xid;
	u_int16_t secs;
	u_int16_t flags;
	u_int32_t ciaddr;
	u_int32_t yiaddr;
	u_int32_t siaddr;
	u_int32_t giaddr;
	u_int8_t chaddr[16];
	u_int8_t sname[64];
	u_int8_t file[128];
	u_int32_t cookie;
	u_int8_t options[308]; /* 312 - cookie */ 
};

struct udp_dhcp_packet {
	struct iphdr ip;
	struct udphdr udp;
	struct dhcpMessage data;
};
struct ethhdr mac;

u_int16_t checksum(void *addr, int count)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	register int32_t sum = 0;
	u_int16_t *source = (u_int16_t *) addr;

	while (count > 1)  {
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) {
		/* Make sure that the left-over byte is added correctly both
		 * with little and big endian hosts */
		u_int16_t tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

unsigned long random_xid(void)
{
	static int initialized;
	if (!initialized) {
		int fd;
		unsigned long seed;

		fd = open("/dev/urandom", 0);
		if (fd < 0 || read(fd, &seed, sizeof(seed)) < 0) {
			seed = time(0);
		}
		if (fd >= 0) close(fd);
		srand(seed);
		initialized++;
	}
	return rand();
}

void init_packet(struct dhcpMessage *packet)
{
	int fd;
	struct ifreq ifr;
	//struct sockaddr_in *our_ip;
	
	memset(packet, 0, sizeof(struct dhcpMessage));
	memset(&ifr, 0, sizeof(ifr));
	
	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name)); 
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
		close(fd);
		return ;
	}
	memcpy(packet->chaddr, ifr.ifr_hwaddr.sa_data, 6);
	close(fd);
	/*printf("my mac %02x:%02x:%02x:%02x:%02x:%02x\n",
			packet->chaddr[0], 
			packet->chaddr[1], 
			packet->chaddr[2], 
			packet->chaddr[3], 
			packet->chaddr[4], 
			packet->chaddr[5]);
	*/		
	packet->op = 1;
	packet->htype = 1;
	packet->hlen = 6;
	packet->cookie = htonl(0x63825363);
	packet->xid =random_xid();
	//add option 0x35
	packet->options[0] = 0x35;
	packet->options[1] = 0x01;
	packet->options[2] = 0x01;
	//add option 0x32
	packet->options[3] = 0x32;
	packet->options[4] = 0x04;
	packet->options[5] = 0x00;
	packet->options[6] = 0x00;
	packet->options[7] = 0x00;
	packet->options[8] = 0x00;
		//add option 0x3d
	packet->options[9] = 0x3d;
	packet->options[10] = 0x07;
	packet->options[11] = 0x01;
	packet->options[12] = 0xb8;
	packet->options[13] = 0x55;
	packet->options[14] = 0x10;
	packet->options[15] = 0xd2;
	packet->options[16] = 0x44;
	packet->options[17] = 0x88;
	//add option 0x39
	packet->options[18] = 0x39;
	packet->options[19] = 0x02;
	packet->options[20] = 0x02;
	packet->options[21] = 0x40;
	//add option 0x37
	packet->options[22] = 0x37;
	packet->options[23] = 0x07;
	packet->options[24] = 0x01;
	packet->options[25] = 0x03;
	packet->options[26] = 0x06;
	packet->options[27] = 0x0c;
	packet->options[28] = 0x0f;
	packet->options[29] = 0x1c;
	packet->options[30] = 0x2a;
	packet->options[31] = 0xff;

	
}

int read_interface(char *interface, int *ifindex, u_int32_t *addr,char *arp)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *our_ip;

	memset(&ifr, 0, sizeof(struct ifreq));
	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, interface);

		if (addr) 
		{ 
			if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
				our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
				*addr = our_ip->sin_addr.s_addr;
			} else {
				close(fd);
				return -1;
			}
		}

		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			*ifindex = ifr.ifr_ifindex;
		} else {
			close(fd);
			return -1;
		}	

		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {

		} else {
			close(fd);
			return -1;
		}
			
		close(fd);
	} 
	else
	{
		return -1;
	}
	return 0;
}

int send_discover()
{
	int fd;
	int result;
	//unsigned char h_dest[6]={0xff,0xff,0xff,0xff,0xff,0xff},h_source[6]={0xF4,0x28,0x53,0xA9,0x30,0x8F};
	struct sockaddr_ll dest;
	struct udp_dhcp_packet packet;
	struct dhcpMessage payload;
	int ifindex;
	
	if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		printf("socket call failed: ");
		return -1;
	}
	
	memset(&dest, 0, sizeof(dest));
	memset(&packet, 0, sizeof(packet));
	
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_IP);
	read_interface(ifName,&ifindex,NULL,"\0\0\0\0\0\0");
	dest.sll_ifindex = ifindex;
	dest.sll_halen = 6;
	memcpy(dest.sll_addr, MAC_BCAST_ADDR, 6);
	
	if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0) {
		printf("bind call failed: ");
		close(fd);
		return -1;
	}
	
	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = INADDR_ANY;
	packet.ip.daddr = INADDR_BROADCAST;
	packet.udp.source = htons(68);
	packet.udp.dest = htons(67);
	packet.udp.len = htons(sizeof(packet.udp) + sizeof(struct dhcpMessage)); /* cheat on the psuedo-header */
	packet.ip.tot_len = packet.udp.len;
	init_packet(&payload);
	memcpy(&(packet.data), &payload, sizeof(struct dhcpMessage));
	packet.udp.check = checksum(&packet, sizeof(struct udp_dhcp_packet));

	packet.ip.tot_len = htons(sizeof(struct udp_dhcp_packet));
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = checksum(&(packet.ip), sizeof(packet.ip));
	
	result = sendto(fd, &packet, sizeof(struct udp_dhcp_packet), 0, (struct sockaddr *) &dest, sizeof(dest));
	if (result <= 0) {
		printf("write on socket failed:%s", strerror(errno));
		return -1;
	}
	close(fd);
	return 0;
}

int raw_socket(int ifindex)
{
	int fd;
	struct sockaddr_ll sock;

	//printf("Opening raw socket on ifindex %d\n", ifindex);
	if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
		printf("socket call failed: %s", strerror(errno));
		return -1;
	}
	
	sock.sll_family = AF_PACKET;
	sock.sll_protocol = htons(ETH_P_IP);
	sock.sll_ifindex = ifindex;
	if (bind(fd, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
		printf("bind call failed: %s", strerror(errno));
		close(fd);
		return -1;
	}

	return fd;
}

uint16_t my_checksum(void *addr, int count)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	int32_t sum = 0;
	uint16_t *source = (uint16_t *) addr;

	while (count > 1)  {
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) {
		/* Make sure that the left-over byte is added correctly both
		 * with little and big endian hosts */
		uint16_t tmp = 0;
		*(uint8_t*)&tmp = *(uint8_t*)source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

#define offsetof(type, member) ((long) &((type *) 0)->member)

int get_raw_packet(struct dhcpMessage *payload, int fd)
{
	int bytes;
	struct udp_dhcp_packet packet;
	unsigned char buf[1024]={0}; 
	//u_int32_t dest;
	u_int16_t check;

	memset(&buf, 0, sizeof(buf));
	bytes = read(fd, &buf, sizeof(buf));
	if (bytes < 0) {
		printf("couldn't read on raw listening socket -- ignoring");
		usleep(500000); /* possible down interface, looping condition */
		return -1;
	}
	memcpy(&mac,buf,sizeof(mac));
	memcpy(&packet,buf+sizeof(mac),sizeof(packet));
	if (packet.ip.protocol != IPPROTO_UDP || packet.ip.version != IPVERSION ||
	    packet.ip.ihl != sizeof(packet.ip) >> 2 || packet.udp.dest != htons(68) ||
	    ntohs(packet.udp.len) != (short) (bytes - sizeof(packet.ip)-sizeof(mac))) {
	    	return -2;
	}
	/* verify IP checksum */
	check = packet.ip.check;
	packet.ip.check = 0;
	if (check != my_checksum(&packet.ip, sizeof(packet.ip))) {
		printf("Bad IP header checksum, ignoring");
		return -2;
	}

	/* verify UDP checksum. IP header has to be modified for this */
	memset(&packet.ip, 0, offsetof(struct iphdr, protocol));
	/* ip.xx fields which are not memset: protocol, check, saddr, daddr */
	packet.ip.tot_len = packet.udp.len; /* yes, this is needed */
	check = packet.udp.check;
	packet.udp.check = 0;
	if (check && check != my_checksum(&packet.ip, bytes-sizeof(mac))) {
		printf("packet with bad UDP checksum received, ignoring");
		return -2;
	}

	memcpy(payload,&packet.data,sizeof( struct dhcpMessage));

	return bytes;
	
}
#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2
#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2

uint8_t *get_option(struct dhcpMessage *packet, int code)
{
	int i, length;
	uint8_t *optionptr;
	int over = 0;
	int curr = OPTION_FIELD;

	optionptr = packet->options;
	i = 0;
	length = sizeof(packet->options);
	while (1) {
		if (i >= length) {
			printf("bogus packet, option fields too long");
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				printf("bogus packet, option fields too long");
				return NULL;
			}
			return optionptr + i + 2;
		}
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				printf("bogus packet, option fields too long");
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && (over & FILE_FIELD)) {
				optionptr = packet->file;
				i = 0;
				length = sizeof(packet->file);
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && (over & SNAME_FIELD)) {
				optionptr = packet->sname;
				i = 0;
				length = sizeof(packet->sname);
				curr = SNAME_FIELD;
			} else
				return NULL;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}

int record_dhcpserverip(struct lisener_agent* agent,struct dhcpMessage* packet)
{
	FILE * fp=NULL;
	uint8_t * option=NULL;
	int i = 0;
	
	if(NULL==(option=get_option(packet,0x36)))
	{
		printf("no dhcp 54 option\n");
		fclose(fp);
		return -1;
	}

	for(i=0;i<agent->server_num;i++)
	{
		if((!memcmp(option,agent->server[i].ip,4))&&(!memcmp(&mac,agent->server[i].mac,6)))
		{
			return 0;
		}
	}

	if(agent->server_num<128)
	{

		memcpy(agent->server[agent->server_num].mac,&mac,6);

		memcpy(agent->server[agent->server_num].ip,option,4);

		agent->server_num++;
	}
	
	
	fp=fopen(DHCP_DETECT_RESULT, "a+");
	
	if(NULL==fp)
	{
		printf("fopen error \n");
		return -1;
	}

	fprintf(fp,"config dhcp_detect\n");
	fprintf(fp,"	option id '%d'\n",agent->server_num);
	fprintf(fp,"	option ip '%d.%d.%d.%d'\n",option[0],option[1],option[2],option[3]);
	fprintf(fp,"	option mac '%02X:%02X:%02X:%02X:%02X:%02X'\n",mac.h_source[0],mac.h_source[1],mac.h_source[2],mac.h_source[3],mac.h_source[4],mac.h_source[5]);
	fprintf(fp,"\n");
	//fprintf(fp,"ip=%d.%d.%d.%d,mac=%02x:%02x:%02x:%02x:%02x:%02x\n",option[0],option[1],option[2],option[3],mac.h_source[0],mac.h_source[1],mac.h_source[2],mac.h_source[3],mac.h_source[4],mac.h_source[5]);

	fclose(fp);

	
	return 0;
}

char *xstrdup(const char *s)
{    
	register char *ret = strdup(s);    
	if (!ret)	
		printf("strdup failed\n");    
	return ret;
}
void usage(void)
{    
	fprintf(stderr, "Usage: huayi dhcp detect [options]I:V:h\n");    
	fprintf(stderr, "\n\n");
}

void MainInit(struct lisener_agent *agent)
{
	gRun = J_TRUE;
	memset(agent,0,sizeof(struct lisener_agent));
	remove(DHCP_DETECT_RESULT);
	system("huayi_cmd set system dhcp_detect 1");
}

void MainGracefulExit(int sig)
{
	gRun = J_FALSE;
	(void) signal(SIGTERM, SIG_IGN);
}

int  UnicastSockInit(struct lisener_agent *agent)
{
	int ifindex;
	read_interface(ifName,&ifindex,NULL,"\0\0\0\0\0\0");
	agent->socket =raw_socket(ifindex);
	
	if(agent->socket<0)
	{
		printf("listen_socket error\n");
		return HB_FALSE;
	}
	return HB_TRUE;
}

void UnicastReceive(struct lisener_agent* agent)
{
	struct dhcpMessage packet;
	
	if(0 < get_raw_packet(&packet,agent->socket))
	{
		record_dhcpserverip(agent,&packet);
	}

	return;
}

void SendDiscover(struct lisener_agent* agent)
{
	agent->detect_count++;

	int fd;
	int result;
	//unsigned char h_dest[6]={0xff,0xff,0xff,0xff,0xff,0xff},h_source[6]={0xF4,0x28,0x53,0xA9,0x30,0x8F};
	struct sockaddr_ll dest;
	struct udp_dhcp_packet packet;
	struct dhcpMessage payload;
	int ifindex;
	
	if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		printf("socket call failed: ");
		return;
	}
	
	memset(&dest, 0, sizeof(dest));
	memset(&packet, 0, sizeof(packet));
	
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_IP);
	read_interface(ifName,&ifindex,NULL,"\0\0\0\0\0\0");
	dest.sll_ifindex = ifindex;
	dest.sll_halen = 6;
	memcpy(dest.sll_addr, MAC_BCAST_ADDR, 6);
	
	if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0) {
		printf("bind call failed: ");
		close(fd);
		goto end;
	}
	
	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = INADDR_ANY;
	packet.ip.daddr = INADDR_BROADCAST;
	packet.udp.source = htons(68);
	packet.udp.dest = htons(67);
	packet.udp.len = htons(sizeof(packet.udp) + sizeof(struct dhcpMessage));
	packet.ip.tot_len = packet.udp.len;
	init_packet(&payload);
	memcpy(&(packet.data), &payload, sizeof(struct dhcpMessage));
	packet.udp.check = checksum(&packet, sizeof(struct udp_dhcp_packet));

	packet.ip.tot_len = htons(sizeof(struct udp_dhcp_packet));
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = checksum(&(packet.ip), sizeof(packet.ip));
	
	result = sendto(fd, &packet, sizeof(struct udp_dhcp_packet), 0, (struct sockaddr *) &dest, sizeof(dest));
	if (result <= 0) {
		printf("write on socket failed:%s", strerror(errno));
	}
	close(fd);
end:
	if(agent->detect_count < 3)
		JobQueueAddJob(OVER_TIME_INTERVAL, JOB_HEART_BEAT,(void *)SendDiscover, NULL, agent);
	else
		gRun = J_FALSE;

	return;
}
int main(int argc, char** argv)
{
	int opt;
	struct lisener_agent agent;

	if(argc==1){
		usage();	    
		exit(1);
	}
	
	MainInit(&agent);
	(void) signal(SIGTERM,MainGracefulExit);

	while ((opt = getopt(argc, argv, "I:V:h")) > 0) {
		switch(opt) {	
			case 'I':	    
				ifName = xstrdup(optarg);break;	
			case 'V':	
			case 'h':	    
				usage();	    
				exit(0);	
			default:	    
				usage();	    
				exit(1);	
		}    
	}
	JobQueueInit();
	JobQueueDebug(J_FALSE);

	if(!UnicastSockInit(&agent))
	{
		printf("UnicastSockInit:Can't create unicast lisener socket\n");
		exit(1);
	}

	if(!JobQueueRegisterSocket(agent.socket,(void*)UnicastReceive,(void *)&agent))
	{
		printf("UnicastReceive:Can't register socket to job queue\n");
		exit(1);
	}

	SendDiscover(&agent);

	while(gRun == J_TRUE) {
		JobQueueExecutionLoop();
	}
	close(agent.socket);
	
	JobQueueDeleteAllJob();
	
	system("huayi_cmd set system dhcp_detect 2");
	return 0;
}

