#include "huayi_handle_reload.h"
//#include <errno.h>
//#include <sys/wait.h>


#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
#define FULL_SPEED "100000kbit"

VTBool huayi_reload_qos_iplimit(void * arg,cJSON *input_root)
{
	char tmp_buff[OPTION_STR_LEN]={0}, tmp_args1[OPTION_STR_LEN]={0}, tmp_args2[OPTION_STR_LEN]={0};
	char ip_range[OPTION_STR_LEN]={0}, tmp_args3[OPTION_STR_LEN]={0},tmp_args4[OPTION_STR_LEN]={0};
	int wan_pkt_mark=20, lan_pkt_mark=40, dut_pkt_mark=60, def_pkt_mark=62;
	char pInterface[OPTION_STR_LEN]={0}, pInterface2[OPTION_STR_LEN]={0};
	char section[OPTION_STR_LEN] = {0};
	char start_ip[IPV4_STR_LEN] = {0};	
	char end_ip[IPV4_STR_LEN] = {0};	
	char desc[OPTION_STR_LEN] = {0};
	int up_bw = 0,down_bw = 0;
	int i=0, QoS_Enabled=0;
	int QoS_Manual_Downlink=0, QoS_Manual_Uplink=0;
	int uplink_speed=102400, downlink_speed=102400;
	char br_range[OPTION_STR_LEN]={0};
	char br_ip[OPTION_STR_LEN]={0};
	char wan_ipaddr[OPTION_STR_LEN]={0};
	char proto[OPTION_STR_LEN]={0};
	unsigned char str_l7_filter[TEMP_STR_LEN]={0};
	char strcmdbuf[CMD_STR_LEN] = {0};
	
	XCMD(strcmdbuf, "iptables -F -t mangle");
	XCMD(strcmdbuf, "iptables -X -t mangle");
	XCMD(strcmdbuf, "iptables -Z -t mangle");
	
	huayi_uci_get_str(PKG_NETWORK,"wan","proto",proto);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ifname",pInterface);
	
	if (!strcmp(proto,HUAYI_NET_PROTO_PPPOE))
	{
		XCMD(strcmdbuf, "iptables -t mangle -A PREROUTING -i %s  -m ttl --ttl 0 -j TTL --ttl-set 128", PPPOE_DEV_WAN_IFNAME);

		XCMD(strcmdbuf, "iptables -t mangle -A PREROUTING -i %s  -m ttl --ttl 1 -j TTL --ttl-set 128", PPPOE_DEV_WAN_IFNAME);
		
		strcpy(pInterface,PPPOE_DEV_WAN_IFNAME);
	}

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ip_address",wan_ipaddr);
	if (strlen(wan_ipaddr) ==0)
	{
		printf("No wan ip currently!\n");
		return VT_TRUE;
	}
	
	huayi_uci_get_int(PKG_QOS,"qos","qos_mode",&QoS_Enabled);
	huayi_uci_get_int(PKG_QOS,"qos","total_upspeed",&QoS_Manual_Uplink);
	huayi_uci_get_int(PKG_QOS,"qos","total_dwspeed",&QoS_Manual_Downlink);
	
	XCMD(strcmdbuf, "tc qdisc del dev %s root",ETH_DEV_BR_LAN);
	
	if(strcmp(pInterface, PPPOE_DEV_WAN_IFNAME)==0)
	{
		strcpy(pInterface,ETH_DEV_WAN_IFNAME);
		strcpy(pInterface2,PPPOE_DEV_WAN_IFNAME);
	}
	
	XCMD(strcmdbuf, "tc qdisc del dev %s root",pInterface);
	XCMD(strcmdbuf, "tc qdisc del dev %s root",PPPOE_DEV_WAN_IFNAME);

	if(strlen(pInterface) == 0)
	{
		XCMD(strcmdbuf, "tc qdisc del dev %s root",pInterface);
		
		return VT_TRUE;
	}
	
	if(QoS_Enabled == 1)
	{
		uplink_speed = QoS_Manual_Uplink;
		if(uplink_speed < 100)
		{
			uplink_speed = 100;
		}
		
		downlink_speed = QoS_Manual_Downlink;
		if(downlink_speed < 100)
		{
			downlink_speed = 100;
		}
		
		XCMD(strcmdbuf, "tc qdisc add dev %s root handle 2:0 htb default 2 r2q 64",pInterface);
		
		sprintf(tmp_buff, "%dkbit", uplink_speed);
		XCMD(strcmdbuf, "tc class add dev %s parent 2:0 classid 2:1 htb rate %s ceil %s quantum 30000",pInterface,tmp_buff,tmp_buff);
		
		XCMD(strcmdbuf, "tc class add dev %s parent 2:1 classid 2:2 htb rate 1kbit ceil %s prio 256 quantum 30000",pInterface,tmp_buff);
		
		XCMD(strcmdbuf, "tc qdisc add dev %s parent 2:2 handle 102: sfq perturb 10",pInterface);

		if((pInterface2!=NULL)&&strcmp(pInterface2, PPPOE_DEV_WAN_IFNAME)==0)
		{
			XCMD(strcmdbuf, "tc qdisc add dev %s root handle 3:0 htb default 2 r2q 64",pInterface2);
			
			sprintf(tmp_buff, "%dkbit", uplink_speed);
			XCMD(strcmdbuf, "tc class add dev %s parent 3:0 classid 3:1 htb rate %s ceil %s quantum 30000",pInterface2,tmp_buff,tmp_buff);
			
			XCMD(strcmdbuf, "tc class add dev %s parent 3:1 classid 3:2 htb rate 1kbit ceil %s prio 256 quantum 30000",pInterface2,tmp_buff);
			
			XCMD(strcmdbuf, "tc qdisc add dev %s parent 3:2 handle 302: sfq perturb 10",pInterface2);
		}
		
		XCMD(strcmdbuf, "tc qdisc add dev %s root handle 5:0 htb default 2 r2q 64",ETH_DEV_BR_LAN);
		
		sprintf(tmp_buff, "%dkbit", downlink_speed);

		XCMD(strcmdbuf, "tc class add dev %s parent 5:0 classid 5:1 htb rate %s ceil %s quantum 30000",ETH_DEV_BR_LAN,FULL_SPEED,FULL_SPEED);
		
		XCMD(strcmdbuf, "tc class add dev %s parent 5:1 classid 5:2 htb rate %s ceil %s prio 1 quantum 30000",ETH_DEV_BR_LAN,tmp_buff,tmp_buff);
		
		XCMD(strcmdbuf, "tc class add dev %s parent 5:2 classid 5:3 htb rate 1kbit ceil %s prio 0 quantum 30000",ETH_DEV_BR_LAN,tmp_buff);
		
		XCMD(strcmdbuf, "tc qdisc add dev %s parent 5:3 handle 503: sfq perturb 10",ETH_DEV_BR_LAN);
		
		XCMD(strcmdbuf, "tc class add dev %s parent 5:1 classid 5:4 htb rate 1kbit ceil %s prio 6 quantum 30000",ETH_DEV_BR_LAN,FULL_SPEED);
		
		XCMD(strcmdbuf, "tc qdisc add dev %s parent 5:4 handle 504: sfq perturb 10",ETH_DEV_BR_LAN);
		
		sprintf(tmp_buff, "%d", def_pkt_mark);
		XCMD(strcmdbuf, "iptables -A POSTROUTING -t mangle -o %s -j MARK --set-mark %s",ETH_DEV_BR_LAN,tmp_buff);
		
		XCMD(strcmdbuf, "tc filter add dev %s parent 5:0 protocol ip prio 100 handle %s fw classid 5:3",ETH_DEV_BR_LAN,tmp_buff);
		
		while(1)
		{
			snprintf(section,sizeof(section),"@iplimit[%d]",i);
			if(!huayi_uci_get_str(PKG_QOS,section,"start_ip",start_ip))
			{
				break;
			}

			huayi_uci_get_str(PKG_QOS,section,"end_ip",end_ip);

			huayi_uci_get_int(PKG_QOS,section,"up_bw",&up_bw);

			huayi_uci_get_int(PKG_QOS,section,"down_bw",&down_bw);

			huayi_uci_get_str(PKG_QOS,section,"desc",desc);
			
			if(up_bw > 0)
			{
				sprintf(tmp_buff, "%d", wan_pkt_mark);
				
				sprintf((char*)str_l7_filter,"%s"," ");
				
				sprintf(ip_range, "%s-%s",start_ip, end_ip);
					
				XCMD(strcmdbuf, "iptables -A PREROUTING -t mangle -m iprange --src-range %s -j MARK --set-mark %s" ,ip_range,tmp_buff);
				
				sprintf(tmp_args1, "2:%d", wan_pkt_mark);
				sprintf(tmp_args2, "%dkbit", up_bw);
				sprintf(tmp_args3, "%dkbit", uplink_speed);
				sprintf(tmp_args4, "1%d:", wan_pkt_mark);
				
				XCMD(strcmdbuf, "tc class add dev %s parent 2:1 classid %s htb rate 1kbit ceil %s prio 2 quantum 30000",pInterface,tmp_args1,tmp_args2);

				XCMD(strcmdbuf, "tc qdisc add dev %s parent %s handle %s sfq perturb 10",pInterface,tmp_args1,tmp_args4);

				XCMD(strcmdbuf, "tc filter add dev %s parent 2:0 protocol ip prio 100 handle %s fw classid %s",pInterface,tmp_buff,tmp_args1);

				sprintf(tmp_args1, "3:%d", wan_pkt_mark);
				sprintf(tmp_args2, "%dkbit", up_bw);
				sprintf(tmp_args3, "%dkbit", uplink_speed);
				sprintf(tmp_args4, "3%d:", wan_pkt_mark);
				if((pInterface2 != NULL) && strcmp(pInterface2, PPPOE_DEV_WAN_IFNAME) == 0)
				{
					
					XCMD(strcmdbuf, "tc class add dev %s parent 3:1 classid %s htb rate 1kbit ceil %s prio 2 quantum 30000",pInterface2,tmp_args1,tmp_args2);
					
					XCMD(strcmdbuf, "tc qdisc add dev %s parent %s handle %s sfq perturb 10",pInterface2,tmp_args1,tmp_args4);
					
					XCMD(strcmdbuf, "tc filter add dev %s parent 3:0 protocol ip prio 100 handle %s fw classid %s",pInterface2,tmp_buff,tmp_args1);
				}

				wan_pkt_mark = wan_pkt_mark+1;
			}
			
			if(down_bw > 0)
			{
				sprintf(tmp_buff, "%d", lan_pkt_mark);
			
				sprintf(ip_range, "%s-%s",start_ip, end_ip);
				
				XCMD(strcmdbuf, "iptables -A POSTROUTING -t mangle -m iprange --dst-range %s -j MARK --set-mark %s",ip_range,tmp_buff);

				sprintf(tmp_args1, "5:%d", lan_pkt_mark);
				sprintf(tmp_args2, "%dkbit", down_bw);
				sprintf(tmp_args3, "%dkbit", downlink_speed);
				sprintf(tmp_args4, "5%d:", lan_pkt_mark);
				
				XCMD(strcmdbuf, "tc class add dev %s parent 5:2 classid %s htb rate 1kbit ceil %s prio 2 quantum 30000",ETH_DEV_BR_LAN,tmp_args1,tmp_args2);
				
				XCMD(strcmdbuf, "tc qdisc add dev %s parent %s handle %s sfq perturb 10",ETH_DEV_BR_LAN,tmp_args1,tmp_args4);
				
				XCMD(strcmdbuf, "tc filter add dev %s parent 5:0 protocol ip prio 100 handle %s fw classid %s",ETH_DEV_BR_LAN,tmp_buff,tmp_args1);
				
				lan_pkt_mark = lan_pkt_mark+1;
			}
			
			i++;
		}
		
		huayi_uci_get_str(PKG_STATUS_NETWORK,"lan","ip_address",br_ip);
		sprintf(br_range,"%s-%s",br_ip,br_ip);
		sprintf(tmp_buff, "%d", dut_pkt_mark);
		XCMD(strcmdbuf, "iptables -A POSTROUTING -t mangle -m iprange --src-range %s %s -j MARK --set-mark %s",br_range,str_l7_filter,tmp_buff);
		
		XCMD(strcmdbuf, "tc filter add dev %s parent 5:0 protocol ip prio 100 handle %s fw classid 5:4",ETH_DEV_BR_LAN,tmp_buff);
	}
	
	return VT_TRUE;
}
#endif

