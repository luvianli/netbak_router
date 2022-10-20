#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
#include "huayi_handle_reload.h"

#define HUAYI_NAT_PREROUTING_CHAIN         "HUAYI_NAT_PREROUTING"
#define HUAYI_FILTER_INPUT_CHAIN           "HUAYI_FILTER_INPUT"
#define HUAYI_FILTER_FORWARD_CHAIN         "HUAYI_FILTER_FORWARD"
#define HUAYI_FILTER_OUTPUT_CHAIN          "HUAYI_FILTER_OUTPUT"
#define HUAYI_NAT_POSTROUTING_CHAIN		   "HUAYI_POSTROUTING_OUTPUT"

#define SPI_FILTER_INPUT_CHAIN			"spi_input"
#define SPI_FILTER_FORWARD_CHAIN        "spi_forward"
#define DMZ_CHAIN                       "huayi_dmz"
#define DMZ_FILTER_CHAIN                "huayi_dmz_filter"
#define MINIUPNPD_CHAIN                       "MINIUPNPD"
#define MINIUPNPD_FILTER_CHAIN                "MINIUPNPD"
#define TCPMSS_FORWARD_CHAIN            "huayi_tcpmss_forward"
#define NAT_MASQUERADE_CHAIN            "nat_masquerade"

#define URL_FILTER_CHAIN                "huayi_url_filter"
#define IPPORT_FILTER_CHAIN             "huayi_ipport_filter"
#define MAC_FILTER_CHAIN                "huayi_mac_filter"
#define USER_BLACKLIST_CHAIN            "huayi_user_blacklist"
#define CONNLIMIT_FILTER_CHAIN          "huayi_connlimit_filter"
#define WANPING_FILTER_CHAIN            "huayi_wanping_filter"
#define REMOTE_MANAGEMENT_FILTER_CHAIN  "huayi_remote_mgt_filter"
#define REMOTE_MANAGEMENT_CHAIN         "huayi_remote_management"
#define PORT_FORWARD_CHAIN              "huayi_port_forward"
#define PORT_FORWARD_FILTER_CHAIN       "huayi_port_forward_filter"
#define DROP_WIFIGUEST_FILTER_CHAIN     "huayi_wifiguest_filter"

#define IGMPPROXY_FORWARD_CHAIN     	"huayi_igmpproxy_forward"
#define IGMPPROXY_INPUT_CHAIN     		"huayi_igmpproxy_input"


#define PPP_FILTER_FINPUT_CHAIN					"huayi_ppp_input_filter"
#define PPP_FILTER_FORWARD_CHAIN				"huayi_ppp_forward_filter"
#define PPP_FILTER_OUTPUT_CHAIN					"huayi_ppp_output_filter"

#define PROTO_TCP						1
#define PROTO_UDP						2
#define PROTO_TCP_UDP					0


static void huayi_firewall_clear()
{
	char strCmdBuf[CMD_STR_LEN] = { 0 };

	//filter Destroying
	XCMD(strCmdBuf, "iptables -w -t filter -D INPUT -j %s ", HUAYI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -D FORWARD -j %s ", HUAYI_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", HUAYI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", HUAYI_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", HUAYI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", HUAYI_FILTER_FORWARD_CHAIN);

	//nat Destroying
	XCMD(strCmdBuf, "iptables -w -t nat -D PREROUTING -j %s ", HUAYI_NAT_PREROUTING_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -D POSTROUTING -j %s ", HUAYI_NAT_POSTROUTING_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -F %s ", HUAYI_NAT_PREROUTING_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -F %s ", HUAYI_NAT_POSTROUTING_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -X %s ", HUAYI_NAT_PREROUTING_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -X %s ", HUAYI_NAT_POSTROUTING_CHAIN);

	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", URL_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", IPPORT_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", MAC_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", USER_BLACKLIST_CHAIN);

#if defined(CONFIG_HUAYI_SUPPORT_VPN) || defined(CONFIG_PACKAGE_HUAYI_PPL2TP)
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", PPP_FILTER_FINPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", PPP_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", PPP_FILTER_OUTPUT_CHAIN);
#endif

	
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", CONNLIMIT_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", WANPING_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -F %s ", REMOTE_MANAGEMENT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", REMOTE_MANAGEMENT_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -F %s ", PORT_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", PORT_FORWARD_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -F %s ", DMZ_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", DMZ_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -F %s ", MINIUPNPD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", MINIUPNPD_FILTER_CHAIN);
	
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", IGMPPROXY_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", IGMPPROXY_INPUT_CHAIN);
	
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", SPI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", SPI_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", TCPMSS_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -F %s ", NAT_MASQUERADE_CHAIN);

	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", DROP_WIFIGUEST_FILTER_CHAIN);

	//url filter
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", URL_FILTER_CHAIN);
	
	//ipport filter
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", IPPORT_FILTER_CHAIN);
	
	//mac filter
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", MAC_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", USER_BLACKLIST_CHAIN);
	
	// connlimit	
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", CONNLIMIT_FILTER_CHAIN);
	
	//wan ping 
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", WANPING_FILTER_CHAIN);

	//pptpd xl2tpd
	XCMD(strCmdBuf, "iptables -w -t filter -X %s", PPP_FILTER_FINPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s", PPP_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s", PPP_FILTER_OUTPUT_CHAIN);
	
	//remote management
	XCMD(strCmdBuf, "iptables -w -t nat -X %s ", REMOTE_MANAGEMENT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", REMOTE_MANAGEMENT_FILTER_CHAIN);

	//port forward
	XCMD(strCmdBuf, "iptables -w -t nat -X %s ", PORT_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", PORT_FORWARD_FILTER_CHAIN);
	
	//dmz
	XCMD(strCmdBuf, "iptables -w -t nat -X %s ", DMZ_CHAIN);	
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", DMZ_FILTER_CHAIN);

	//miniupnpd
	XCMD(strCmdBuf, "iptables -w -t nat -X %s ", MINIUPNPD_CHAIN);	
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", MINIUPNPD_FILTER_CHAIN);

	//igmpproxy
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", IGMPPROXY_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", IGMPPROXY_INPUT_CHAIN);
	
	//spi 
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", SPI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", SPI_FILTER_FORWARD_CHAIN);
	
	//TCPMSS
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", TCPMSS_FORWARD_CHAIN);

	//masquerade
	XCMD(strCmdBuf, "iptables -w -t nat -X %s ", NAT_MASQUERADE_CHAIN);

	//wifi guest not accept lan net
	XCMD(strCmdBuf, "iptables -w -t filter -X %s ", DROP_WIFIGUEST_FILTER_CHAIN);
	
	return ;
}

int huayi_masqueradenat()
{
	char strCmdBuf[CMD_STR_LEN] = { 0 };

	XCMD(strCmdBuf, "iptables -w -t nat -F %s ", NAT_MASQUERADE_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -A %s -j MASQUERADE", NAT_MASQUERADE_CHAIN);
	
	return VT_TRUE;
}


VTBool huayi_reload_igmpproxy(void * arg,cJSON *input_root)
{
	char strcmdbuf[CMD_STR_LEN] = { 0 };
	int  enable = 0;

	huayi_uci_get_int(PKG_IGMP_PROXY,"@igmpproxy[0]","enable",&enable);

	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", IGMPPROXY_FORWARD_CHAIN);
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", IGMPPROXY_INPUT_CHAIN);

	if (enable == 0)
	{
		return VT_TRUE;
	}

	XCMD(strcmdbuf, "iptables -t mangle -I PREROUTING -d 224.0.0.0/4 -p udp -j TTL --ttl-set 2");
	
	XCMD(strcmdbuf, "iptables -t filter -A %s -p udp -d 224.0.0.0/4 -j ACCEPT", IGMPPROXY_FORWARD_CHAIN);

	XCMD(strcmdbuf, "iptables -t filter -A %s -p igmp -j ACCEPT", IGMPPROXY_INPUT_CHAIN);
	
	return VT_TRUE;
}


VTBool huayi_spifirewall(void * arg, cJSON *input_root)
{	
	struct VcoreAgent *Agent= (struct VcoreAgent *)arg;
	char strCmdBuf[CMD_STR_LEN]  = { 0 };
	
	XCMD(strCmdBuf, "iptables -w -A %s -m state --state INVALID -j DROP", SPI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -A %s -i br-lan -m state --state NEW -j ACCEPT", SPI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -A %s -i lo -m state --state NEW -j ACCEPT", SPI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -A %s -m state --state ESTABLISHED,RELATED -j ACCEPT", SPI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -A %s -j DROP", SPI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -A %s -m state --state INVALID -j DROP", SPI_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -A %s -i br-lan -m state --state NEW -j ACCEPT", SPI_FILTER_FORWARD_CHAIN);

	XCMD(strCmdBuf, "iptables -w -A %s -m state --state ESTABLISHED,RELATED -j ACCEPT", SPI_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -A %s -j DROP", SPI_FILTER_FORWARD_CHAIN);

	return VT_TRUE;
}


VTBool huayi_firewall_init()
{
	char strCmdBuf[256] = { 0 };
	
	huayi_firewall_clear();

	//filter init
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", HUAYI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A INPUT -j %s ", HUAYI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", HUAYI_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A FORWARD -j %s ", HUAYI_FILTER_FORWARD_CHAIN);	

	//nat init
	XCMD(strCmdBuf, "iptables -w -t nat -N %s ", HUAYI_NAT_PREROUTING_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -A PREROUTING -j %s ", HUAYI_NAT_PREROUTING_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -N %s ", HUAYI_NAT_POSTROUTING_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -A POSTROUTING -j %s ", HUAYI_NAT_POSTROUTING_CHAIN);

	//url filter
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", URL_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, URL_FILTER_CHAIN);
	
	//ipport filter
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", IPPORT_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, IPPORT_FILTER_CHAIN);
	
	//mac filter
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", MAC_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, MAC_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", USER_BLACKLIST_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, USER_BLACKLIST_CHAIN);

	//connlimit	
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", CONNLIMIT_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, CONNLIMIT_FILTER_CHAIN);

	//wan ping
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", WANPING_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_INPUT_CHAIN, WANPING_FILTER_CHAIN);

	//pptpd xl2tpd
#if defined(CONFIG_HUAYI_SUPPORT_VPN) || defined(CONFIG_PACKAGE_HUAYI_PPL2TP)
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", PPP_FILTER_FINPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_INPUT_CHAIN, PPP_FILTER_FINPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", PPP_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, PPP_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", PPP_FILTER_OUTPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_OUTPUT_CHAIN, PPP_FILTER_OUTPUT_CHAIN);
#endif

	//remote management
	XCMD(strCmdBuf, "iptables -w -t nat -N %s ", REMOTE_MANAGEMENT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -A %s -j %s ", HUAYI_NAT_PREROUTING_CHAIN, REMOTE_MANAGEMENT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", REMOTE_MANAGEMENT_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_INPUT_CHAIN, REMOTE_MANAGEMENT_FILTER_CHAIN);

	//wifi guest not accept lan net
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", DROP_WIFIGUEST_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_INPUT_CHAIN, DROP_WIFIGUEST_FILTER_CHAIN);
	
	//port forward
	XCMD(strCmdBuf, "iptables -w -t nat -N %s ", PORT_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -A %s -j %s ", HUAYI_NAT_PREROUTING_CHAIN, PORT_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", PORT_FORWARD_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, PORT_FORWARD_FILTER_CHAIN);
	
	//dmz
	XCMD(strCmdBuf, "iptables -w -t nat -N %s ", DMZ_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -A %s -j %s ", HUAYI_NAT_PREROUTING_CHAIN, DMZ_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", DMZ_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, DMZ_FILTER_CHAIN);

	//miniupnpd
	XCMD(strCmdBuf, "iptables -w -t nat -N %s ", MINIUPNPD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -A %s -j %s ", HUAYI_NAT_PREROUTING_CHAIN, MINIUPNPD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", MINIUPNPD_FILTER_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, MINIUPNPD_FILTER_CHAIN);
	
	//tcpmss
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", TCPMSS_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, TCPMSS_FORWARD_CHAIN);	

	//igmpproxy
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", IGMPPROXY_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, IGMPPROXY_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", IGMPPROXY_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_INPUT_CHAIN, IGMPPROXY_INPUT_CHAIN);
	
	//spi 
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", SPI_FILTER_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_FORWARD_CHAIN, SPI_FILTER_FORWARD_CHAIN);	
	XCMD(strCmdBuf, "iptables -w -t filter -N %s ", SPI_FILTER_INPUT_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t filter -A %s -j %s ", HUAYI_FILTER_INPUT_CHAIN, SPI_FILTER_INPUT_CHAIN);

	//masquerade
	XCMD(strCmdBuf, "iptables -w -t nat -N %s ", NAT_MASQUERADE_CHAIN);
	XCMD(strCmdBuf, "iptables -w -t nat -A %s  -j %s ", HUAYI_NAT_POSTROUTING_CHAIN, NAT_MASQUERADE_CHAIN);
	
	return VT_TRUE;
}

VTBool huayi_reload_remote_manage(void * arg,cJSON *input_root)
{
	int remote_enable = 0;
	char strcmdbuf[CMD_STR_LEN]   = { 0 };	
	char wanifname[SHORT_STR_LEN]  = { 0 };	
	int port;
	
	XCMD(strcmdbuf, "iptables -w -t nat -F %s ", REMOTE_MANAGEMENT_CHAIN);	
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", REMOTE_MANAGEMENT_FILTER_CHAIN);

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"remote","remote_enable",&remote_enable);
	
	if ( remote_enable == 0 ) {
		return VT_TRUE;	
	}
	
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan", "ifname", wanifname);
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"remote","port",&port);
	if ( port  != 80 ) {
		XCMD(strcmdbuf, "iptables -w -t nat -A %s -i %s -p tcp --dport %d -j REDIRECT --to-port 80 ", REMOTE_MANAGEMENT_CHAIN, wanifname,  port);	
		XCMD(strcmdbuf, "iptables -w -A %s -i %s -p tcp --dport 80 -j ACCEPT ", REMOTE_MANAGEMENT_FILTER_CHAIN, wanifname);	
	} else {		
		XCMD(strcmdbuf, "iptables -w -A %s -i %s -p tcp --dport 80 -j ACCEPT ", REMOTE_MANAGEMENT_FILTER_CHAIN, wanifname);
	}	
	
	return VT_TRUE;
}

VTBool huayi_reload_macfilter(void * arg,cJSON *input_root)
{
	int  i = 0;
	int  mac_filter = 0;
	char section[OPTION_STR_LEN] = { 0 };
	char mac_addr[MAC_STR_LEN] = { 0 };
	char strcmdbuf[CMD_STR_LEN] = { 0 };

	printf("huayi_reload_macfilter start\n");

	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", MAC_FILTER_CHAIN);
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","mac_filter",&mac_filter);
	if(0 == mac_filter) {
		return VT_TRUE;
	}
	
	while(1)
	{
		memset(section, 0, sizeof(section));
		snprintf(section, sizeof(section),"@macfilter[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"mac_addr",mac_addr))
		{
			break;
		}
		memset(strcmdbuf, 0, sizeof(strcmdbuf));
		XCMD(strcmdbuf, "iptables -w -t filter -A %s -m mac --mac-source %s  -j DROP", MAC_FILTER_CHAIN,  mac_addr);
		i++;
	}
	
	return VT_TRUE;
}

VTBool huayi_reload_urlfilter(void * arg,cJSON *input_root)
{
	
	int  url_filter = 0;
	char section[OPTION_STR_LEN] = {0};
	char url[OPTION_STR_LEN] = {0};
	int i = 0;
	char strcmdbuf[CMD_STR_LEN] = { 0 };

	printf("huayi_reload_urlfilter start\n");

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","url_filter",&url_filter);
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", URL_FILTER_CHAIN);
	if (0 == url_filter) {
		return VT_TRUE;
	}
	
	while(1)
	{
		snprintf(section, sizeof(section),"@urlfilter[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"url",url))
		{
			break;
		}
		XCMD(strcmdbuf, "iptables -w -A %s -m string --string %s --algo bm -j DROP ", URL_FILTER_CHAIN, url);
		i++;
	}
	
	return VT_TRUE;	
}

VTBool huayi_MakeIPPortFilterRule(char *src_ip, int proto, int port_start, int port_end)
{
	char strcmdbuf[CMD_STR_LEN] = { 0 };
	char strAction[OPTION_STR_LEN]     = { 0 };
	char strport[TEMP_STR_LEN] = { 0 };
	char strproto[OPTION_STR_LEN] = { 0 };

	strcpy(strAction, "-j DROP");
	
//iptables -w -A IPPORT_FILTER -s s_ip -p proto --dport port_start:port_end
	
	if(port_start == port_end) {
		snprintf(strport, sizeof(strport),  "--dport %d", port_start);
	} else {
		snprintf(strport, sizeof(strport), "--dport %d:%d", port_start, port_end);
	}

	if (proto == PROTO_TCP) {
		strcpy(strproto,  "-p tcp ");
	} else if (proto == PROTO_UDP) {
		strcpy(strproto,  "-p udp ");
	} else {   //tcp+udp
		snprintf(strcmdbuf, sizeof(strcmdbuf), "iptables -w -A %s -s %s -p tcp %s %s",IPPORT_FILTER_CHAIN, src_ip, strport, strAction);
		system(strcmdbuf);
		snprintf(strcmdbuf, sizeof(strcmdbuf), "iptables -w -A %s -s %s -p udp %s %s",IPPORT_FILTER_CHAIN, src_ip, strport, strAction);
		system(strcmdbuf);
		
		return VT_TRUE;
	}
	snprintf(strcmdbuf, sizeof(strcmdbuf), "iptables -w -A %s -s %s %s %s %s",IPPORT_FILTER_CHAIN, src_ip, strproto, strport, strAction);
	system(strcmdbuf);
	
	return VT_TRUE;
}


VTBool huayi_reload_ipportfilter(void * arg,cJSON *input_root)
{
	int  ip_port_filter = 0;
	char section[OPTION_STR_LEN] = {0};
	char src_ip[IPV4_STR_LEN] = {0};
	char strcmdbuf[CMD_STR_LEN] = { 0 };
	int proto	 = 0, port_start = 0, port_end	= 0;
	int i = 0;

	printf("huayi_reload_ipportfilter start\n");
	
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", IPPORT_FILTER_CHAIN);
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","ip_port_filter",&ip_port_filter);
	if(0 == ip_port_filter) {
		return VT_TRUE;
	}
	while(1)
	{
		snprintf(section,sizeof(section),"@ipportfilter[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"src_ip",src_ip))
		{
			break;
		}
		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"proto",&proto);
		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"port_start",&port_start);
		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"port_end",&port_end);
		if((port_start <= 0) || (port_start > 65535) || (port_end <= 0) || (port_end > 65535)) {
			i++;
			continue;
		}
		huayi_MakeIPPortFilterRule(src_ip, proto, port_start, port_end);	
		i++;
	}
	return VT_TRUE;
}

VTBool huayi_reload_dmz(void * arg,cJSON *input_root)
{
	int  dmz_enable = 0;
	char strcmdbuf[CMD_STR_LEN] = { 0 }, wanip[SHORT_STR_LEN] = { 0 }, wanifname[SHORT_STR_LEN]  = { 0 };	
	char dest_ip[IPV4_STR_LEN] = {0};
	
	printf("huayi_reload_dmz start\n");
	
	XCMD(strcmdbuf, "iptables -w -t nat -F %s ", DMZ_CHAIN);	
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", DMZ_FILTER_CHAIN);

	huayi_uci_get_int(PKG_HUAYI_FIREWALL, "dmz", "dmz_enable", &dmz_enable);
	if ( dmz_enable == 0 )	{
		return VT_TRUE;	
	}

	huayi_uci_get_str(PKG_HUAYI_FIREWALL,"dmz","dest_ip",dest_ip);
	if ( strlen(dest_ip)==0 ){		
		return VT_TRUE;	
	}	
	
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan", "ifname", wanifname);
	huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", wanip);
		
	XCMD(strcmdbuf, "iptables -w -t nat -A %s -i %s -j DNAT -d %s -p ALL --to %s", DMZ_CHAIN, wanifname, wanip, dest_ip);	
	XCMD(strcmdbuf, "iptables -w -t filter -A %s  -i %s -d %s -p ALL -j ACCEPT", DMZ_FILTER_CHAIN, wanifname, dest_ip);		

	return VT_TRUE;
}

VTBool huayi_MakePortForwardRule(char *ip_addr, int proto, int internal_port, int external_port)
{
	char strcmdbuf[CMD_STR_LEN] = { 0 }, wanip[SHORT_STR_LEN] = { 0 }, wanifname[SHORT_STR_LEN]  = { 0 };
	char strproto[SHORT_STR_LEN] = { 0 }, strexport[SHORT_STR_LEN] = { 0 } , strinport[OPTION_STR_LEN] = { 0 };
	
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan", "ifname", wanifname);
	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan", "ip_address", wanip);
	snprintf(strexport, sizeof(strexport),  "--dport %d ", external_port);
	if (proto == PROTO_TCP) {
		strcpy(strproto,  "-p tcp ");
	} else {
		strcpy(strproto,  "-p udp ");
	}
	
	snprintf(strinport, sizeof(strinport), "--to %s:%d", ip_addr, internal_port);
	
	XCMD(strcmdbuf, "iptables -w -t nat -A %s  %s -d %s  %s -j DNAT %s", PORT_FORWARD_CHAIN, strproto, wanip, strexport,  strinport);
	
	return VT_TRUE;
}

VTBool huayi_MakePortForwardFilterRule(char *ip_addr, int proto, int internal_port)
{
	char strproto[SHORT_STR_LEN] = { 0 };
	char strcmdbuf[CMD_STR_LEN] = { 0 }, wanifname[SHORT_STR_LEN]  = { 0 };
	char strinport[OPTION_STR_LEN] = { 0 };

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan", "ifname", wanifname);
	if (proto == PROTO_TCP) {
		strcpy(strproto,  "-p tcp ");
	} else {
		strcpy(strproto,  "-p udp ");
	}
	
	snprintf(strinport, sizeof(strinport), "-d %s --dport %d", ip_addr,  internal_port);
	
	XCMD(strcmdbuf, "iptables -w -t filter -A %s  -i %s %s %s -j ACCEPT ", PORT_FORWARD_FILTER_CHAIN, wanifname, strproto, strinport);

	return VT_TRUE;
}

VTBool huayi_reload_port_forward(void * arg,cJSON *input_root)
{	
	int  port_forward = 0;
	char section[OPTION_STR_LEN] = {0};
	char ip_addr[IPV4_STR_LEN] = {0};
	char strcmdbuf[CMD_STR_LEN]   = { 0 };
	int  proto = 0,  internal_port=0, external_port=0, i = 0;

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","port_forward",&port_forward);
	
	XCMD(strcmdbuf, "iptables -w -t nat -F %s ", PORT_FORWARD_CHAIN);
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", PORT_FORWARD_FILTER_CHAIN);
	
	if(port_forward == 0)
	{	
		return VT_TRUE;
	}
	
	while(1)
	{
		snprintf(section,sizeof(section),"@port_forward[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"ip_addr",ip_addr))
		{
			break;
		}
		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"proto",&proto);
		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"internal_port",&internal_port);
		huayi_uci_get_int(PKG_HUAYI_FIREWALL,section,"external_port",&external_port);

		switch (proto)
		{
			case PROTO_TCP:
			case PROTO_UDP:
				huayi_MakePortForwardRule(ip_addr, proto, internal_port, external_port);
				huayi_MakePortForwardFilterRule(ip_addr, proto, internal_port);
				break;
			case PROTO_TCP_UDP:
				huayi_MakePortForwardRule(ip_addr, PROTO_TCP, internal_port, external_port);
				huayi_MakePortForwardFilterRule(ip_addr, PROTO_TCP, internal_port);
				huayi_MakePortForwardRule(ip_addr, PROTO_UDP, internal_port, external_port);
				huayi_MakePortForwardFilterRule(ip_addr, PROTO_UDP, internal_port);
				break;
			default:
				break;
		}
		i++;
	}
	
	return VT_TRUE;	
}

VTBool huayi_reload_conntrack_limit(void * arg,cJSON *input_root)
{	
	int  conn_limit = 0;
	char section[OPTION_STR_LEN] = { 0 }, strcmdbuf[CMD_STR_LEN] = { 0 };
	char connlimit_num[OPTION_STR_LEN] = { 0 };
	char ip_start[IPV4_STR_LEN] = { 0 }, ip_end[IPV4_STR_LEN] = { 0 };
	int i = 0;

	printf("huayi_reload_conntrack_limit start\n");
	
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", CONNLIMIT_FILTER_CHAIN);
	/*we should discuss about this feature*/
	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"global","conn_limit",&conn_limit);

	if(0 == conn_limit) {
		return VT_TRUE;
	}
	
	while(1)
	{
		memset(section, 0, sizeof(section));
		snprintf(section,sizeof(section),"@connlimit[%d]",i);
		if(!huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"connlimit_num", connlimit_num))
		{
			break;
		}
		huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"ip_start", ip_start);
		huayi_uci_get_str(PKG_HUAYI_FIREWALL,section,"ip_end", ip_end);
		memset(strcmdbuf, 0, sizeof(strcmdbuf));
		XCMD(strcmdbuf, "iptables -w -A %s  -p tcp -m iprange --src-range %s-%s -m connlimit --connlimit-above %s -j DROP", CONNLIMIT_FILTER_CHAIN, ip_start, ip_end, connlimit_num);
		i++;
	}
	
	return VT_TRUE;	
}

VTBool huayi_reload_acl(void * arg,cJSON *input_root)
{	
	int acl_count = 0;
	char acl_list[LONG_BUFF_LEN] = {0};
	char mac_address[MAC_STR_LEN] = {0};
	int  i = 0;
	char strcmdbuf[CMD_STR_LEN] = { 0 };

	printf("huayi_reload_acl start\n");

	huayi_uci_get_int(PKG_HUAYI_FIREWALL,"acl","acl_count",&acl_count);
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", USER_BLACKLIST_CHAIN);

	if(acl_count <= 0)
	{
		return VT_TRUE;
	}
	
	huayi_uci_get_str(PKG_HUAYI_FIREWALL,"acl","acl",acl_list);
	while(huayi_get_nth_value(i++, acl_list,' ', mac_address, sizeof(mac_address)))
	{
		XCMD(strcmdbuf, "iptables -w -t filter -A %s -m mac --mac-source %s  -j DROP", USER_BLACKLIST_CHAIN, mac_address);
	}
	
	return VT_TRUE;	
}


VTBool huayi_tcpmssforward()
{
	char strCmdBuf[CMD_STR_LEN] = { 0 };

	XCMD(strCmdBuf, "iptables -w -t filter -F %s ", TCPMSS_FORWARD_CHAIN);
	XCMD(strCmdBuf, "iptables -w -A %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu", TCPMSS_FORWARD_CHAIN);

	return VT_TRUE;
}

VTBool huayi_reload_wanping(void * arg, cJSON *input_root)
{
	char strcmdbuf[CMD_STR_LEN] = { 0 };
	int  wanping_en = 0;
	char wanifname[SHORT_STR_LEN]  = { 0 };

	printf("huayi_reload_wanping start\n");

	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", WANPING_FILTER_CHAIN);
	
	huayi_uci_get_int(PKG_HUAYI_FIREWALL, "wanping", "wanping_en", &wanping_en);

	if(0 == wanping_en) {
		return VT_FALSE;
	}

	huayi_uci_get_str(PKG_STATUS_NETWORK,"wan", "ifname", wanifname);
	
	XCMD(strcmdbuf, "iptables -w -A %s -i %s -p icmp -j ACCEPT", WANPING_FILTER_CHAIN, wanifname);
	
	return VT_TRUE;
}


int AvoidConflictionIpIndex(int *needFix)
{
	char strLanIp[SHORT_STR_LEN]      = { 0 };
	char strLanMask[SHORT_STR_LEN]    = { 0 };
	struct in_addr tWanIp;
	struct in_addr tWanMk;
	struct in_addr tLanIp;
	struct in_addr tLanMk;
	int i,  j;
	unsigned long int ulWanMaskVal;
	unsigned long int ulLanIpVal;
	unsigned long int ulLanMaskVal;
	unsigned long int ulMaskVal;
	char wanip[SHORT_STR_LEN] = { 0 }, wanmask[SHORT_STR_LEN] = { 0 };

	huayi_uci_get_str(PKG_NETWORK,"lan","ipaddr",strLanIp);
	huayi_uci_get_str(PKG_NETWORK,"lan","netmask",strLanMask);
	huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "ip_address", wanip);
	huayi_uci_get_str(PKG_STATUS_NETWORK, "wan", "netmask", wanmask);

	if(strcmp(wanip,"0.0.0.0")==0)
	{
		return VT_FALSE;
	}
	
	if (!inet_aton(wanip, &tWanIp)) {
	    return VT_FALSE;
	}
	
	if (!inet_aton(wanmask, &tWanMk)) {
	    return VT_FALSE;
	}
	
	ulWanMaskVal = tWanMk.s_addr;
	
	if (!inet_aton(strLanIp, &tLanIp)) {
		return VT_FALSE;
	}
	
	if ( !inet_aton(strLanMask, &tLanMk)) {
		return VT_FALSE;
	}
	
	ulLanIpVal   = tLanIp.s_addr;
	ulLanMaskVal = tLanMk.s_addr;
	memcpy(&ulMaskVal, ulLanMaskVal>ulWanMaskVal?&ulWanMaskVal:&ulLanMaskVal, sizeof(ulLanMaskVal));

	if ((tWanIp.s_addr & tWanMk.s_addr) == (tLanIp.s_addr & tLanMk.s_addr)) {    
	        for (i=0; i<32; i++) {
			if ((htonl(ulMaskVal) & (1<<i)) != 0) {
				break;
			}
		}
		
		if ((ulLanIpVal & (1<<i)) == 0) {
			ulLanIpVal = ulLanIpVal+(1<<(24-i));
		} else {
			ulLanIpVal = ulLanIpVal-(1<<(24-i));
		}
		
		tLanIp.s_addr = ulLanIpVal;
		tLanMk.s_addr = ulMaskVal;
		
		for (j=0; j<32; j++) {
			if((htonl(ulMaskVal) & (1<<j)) != 0) {
				break;
			}
		}
		huayi_uci_set_str(PKG_NETWORK, "lan", "ipaddr",  (char *)inet_ntoa(tLanIp));
		huayi_uci_set_str(PKG_NETWORK, "lan", "netmask", (char *)inet_ntoa(tLanMk));
		*needFix = 1;
	}

	return VT_TRUE;
}

VTBool huayi_lanwan_conflictionip()
{
	int needFix = 0;

	AvoidConflictionIpIndex(&needFix);
	if( VT_TRUE == needFix)
	{
		huayi_uci_commit(PKG_NETWORK);
		system("huayi-reset network");
	}
	
    return VT_TRUE;
}

/*
static int mask2len(struct in_addr *mask)
{
	int x, y;
	u_char *p;
	
	p = (u_char *)mask;
	for (x = 0; x < sizeof(*mask); x++) {
		if (p[x] != 0xff)
			break;
	}
	y = 0;
	if (x < sizeof(*mask)) {
		for (y = 0; y < 8; y++) {
			if ((p[x] & (0x80 >> y)) == 0)
				break;
		}
	}
	return x * 8 + y;
}
*/

VTBool huayi_reload_wifiguestfilter(void * arg, cJSON *input_root)
{
	int guest_enable = 0;
	char strcmdbuf[CMD_STR_LEN] = { 0 };
	int disabled_24G = 1;
#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	int disabled_58G = 1;
#endif
	
	XCMD(strcmdbuf, "iptables -w -t filter -F %s ", DROP_WIFIGUEST_FILTER_CHAIN);

	huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"disabled",&disabled_24G);

#if defined(CONFIG_HUAYI_WIRELESS_SUPPORT_5G)
	huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"disabled",&disabled_58G);
	if (disabled_24G == 1)
		disabled_24G = disabled_58G;
#endif

	if (disabled_24G == 1)
	{
		return VT_TRUE;
	}

	huayi_uci_get_int(PKG_WIRELESS,W24G_GUEST_IFACE,"local_network",&guest_enable); //only get 2.4g guest-ssid config. 2.4g and 5g same.
	if(0 == guest_enable) {
		XCMD(strcmdbuf, "iptables -w -I %s -p tcp -m mark --mark 5 -j DROP", DROP_WIFIGUEST_FILTER_CHAIN);
	}

	return VT_TRUE;
}

VTBool huayi_wan_up(void * arg, cJSON *input_root)
{
	VTBool ret = VT_TRUE;

	ret |= huayi_lanwan_conflictionip();
	ret |= huayi_reload_wanping(arg, input_root);
	ret |= huayi_reload_remote_manage(arg, input_root);
	ret |= huayi_reload_port_forward(arg, input_root);
	ret |= huayi_reload_dmz(arg, input_root);
	ret |= huayi_masqueradenat();
	ret |= huayi_reload_qos_iplimit(arg, input_root);

#if defined(CONFIG_PACKAGE_HUAYI_IGMPPROXY) 
	ret |= huayi_reload_igmpproxy(arg, input_root);
#endif

	return ret;
}

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
VTBool huayi_ppp_handler(PPP_HANDLE_TYPE type)
{
	VTBool ret = VT_TRUE;
	int pptpd_enable = 0;
	int xl2tpd_enable = 0;
	int ipsec_enable = 0;
	char cmd[CMD_STR_LEN] = {0};
	

	switch (type)
	{
		case PPP_HANDLE_PPTPD:
			XCMD(cmd,"iptables -w -D %s -p tcp --dport 1723 -j ACCEPT",PPP_FILTER_FINPUT_CHAIN);
			XCMD(cmd,"iptables -w -D %s -p 47 -j ACCEPT",PPP_FILTER_FINPUT_CHAIN);
			break;
		case PPP_HANDLE_XL2TPD:
			XCMD(cmd,"iptables -w -D %s -p udp --dport 1701 -j ACCEPT",PPP_FILTER_FINPUT_CHAIN);
			XCMD(cmd,"iptables -w -D %s -p udp --dport 4500 -j ACCEPT",PPP_FILTER_FINPUT_CHAIN);
			XCMD(cmd,"iptables -w -D %s -p udp --dport 500 -j ACCEPT", PPP_FILTER_FINPUT_CHAIN);
			XCMD(cmd,"iptables -w -D %s -p esp -j ACCEPT", PPP_FILTER_FINPUT_CHAIN);
			break;
		case PPP_HANDLE_ALL:
			XCMD(cmd,"iptables -w -F %s",PPP_FILTER_FINPUT_CHAIN);
			break;
		default:
			XCMD(cmd,"iptables -w -F %s",PPP_FILTER_FINPUT_CHAIN);
			break;
	}
	
	huayi_uci_get_int(PKG_PPTPD,"pptpd","enable",&pptpd_enable);
	huayi_uci_get_int(PKG_XL2TPD, "xl2tpd", "enable", &xl2tpd_enable);
	huayi_uci_get_int(PKG_XL2TPD, "xl2tpd", "ipsec_enable", &ipsec_enable);
	
	if(pptpd_enable)
	{
		XCMD(cmd,"iptables -w -A %s -p tcp --dport 1723 -j ACCEPT",PPP_FILTER_FINPUT_CHAIN);
		XCMD(cmd,"iptables -w -A %s -p 47 -j ACCEPT",PPP_FILTER_FINPUT_CHAIN);				
	}
	
	if(xl2tpd_enable)
	{
		XCMD(cmd,"iptables -w -A %s -p udp --dport 1701 -j ACCEPT", PPP_FILTER_FINPUT_CHAIN);
		if(ipsec_enable)
		{
			XCMD(cmd,"iptables -w -A %s -p udp --dport 4500 -j ACCEPT", PPP_FILTER_FINPUT_CHAIN);
			XCMD(cmd,"iptables -w -A %s -p udp --dport 500 -j ACCEPT", PPP_FILTER_FINPUT_CHAIN);
			XCMD(cmd,"iptables -w -A %s -p esp -j ACCEPT", PPP_FILTER_FINPUT_CHAIN);
		}
	}
	return ret;
}
#endif

#if defined(CONFIG_PACKAGE_HUAYI_IPV6) 
void huayi_ip6_firewall()
{
	char ifName[16]={0}, proto[16]={0};
	char cmd[CMD_STR_LEN] = {0};
	char ip6Addr[64]={0};
	if(!huayi_uci_get_str(PKG_NETWORK,"wan6","proto",proto)){
		return ;
	}
	huayi_uci_get_str(PKG_NETWORK,"wan6","ifname",ifName);
	XCMD(cmd,"ip6tables -t nat -F");
	XCMD(cmd,"ip6tables -t nat -A POSTROUTING -o %s -j MASQUERADE",ifName);
	
	if(!strcmp(proto,"static")){ // if WAN set as static mode, 
		if(!huayi_uci_get_str(PKG_NETWORK,"wan6","ip6addr",ip6Addr)){
			return ;
		}
		XCMD(cmd,"ifconfig %s %s",ifName,ip6Addr);
	}

}
#endif
VTBool huayi_firewall_all(void * arg,cJSON *input_root)
{
	VTBool ret = VT_TRUE;
	
	ret |= huayi_firewall_init();
	ret |= huayi_reload_conntrack_limit(arg, input_root);
	ret |= huayi_reload_urlfilter(arg, input_root);
	ret |= huayi_reload_ipportfilter(arg, input_root);
	ret |= huayi_reload_macfilter(arg, input_root);
	ret |= huayi_reload_acl(arg, input_root);
	ret |= huayi_lanwan_conflictionip();
#if defined(CONFIG_HUAYI_SUPPORT_VPN) 
	ret |= huayi_ppp_handler(PPP_HANDLE_ALL);
#endif
	ret |= huayi_reload_wanping(arg, input_root);
	ret |= huayi_reload_remote_manage(arg, input_root);
	ret |= huayi_reload_wifiguestfilter(arg, input_root);
	ret |= huayi_reload_port_forward(arg, input_root);
	ret |= huayi_reload_dmz(arg, input_root);
	ret |= huayi_spifirewall(arg, input_root);
	ret |= huayi_tcpmssforward();
	ret |= huayi_masqueradenat();
	ret |= huayi_reload_qos_iplimit(arg, input_root);

#if defined(CONFIG_PACKAGE_HUAYI_IGMPPROXY) 
	ret |= huayi_reload_igmpproxy(arg, input_root);
#endif

#if defined(CONFIG_PACKAGE_HUAYI_IPV6)
	huayi_ip6_firewall();
#endif

	return ret;
}

VTBool huayi_reload_firewall(void * arg,cJSON *input_root)
{
	huayi_firewall_all(arg, input_root);

	return VT_TRUE;
}
#ifdef CONFIG_PACKAGE_EASYCWMP
int huayi_reload_cwmp()
{
	char iface[16]={0};
	
	if(!huayi_uci_get_str(PKG_STATUS_NETWORK,"wan","ifname",iface)){
		return 0;
	}
	huayi_uci_set_str(PKG_EASYCWMP,"local","interface",iface);
	system("/etc/init.d/easycwmpd restart 2>/dev/null");
}
#endif

VTBool huayi_reload_wanup(void * arg,cJSON *input_root)
{
	huayi_wan_up(arg, input_root);
#if defined(CONFIG_PACKAGE_HUAYI_IPV6) 
	huayi_ip6_firewall();
#endif
	system("huayi-reset ntpd");

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	system("huayi-reset pptp");
	//system("huayi-reset pptpd");
	system("huayi-reset xl2tp");
	//system("huayi-reset xl2tpd");
#endif
	
#if defined(CONFIG_PACKAGE_HUAYI_IGMPPROXY) 
	system("/etc/init.d/igmpproxy restart");
#endif
#ifdef CONFIG_PACKAGE_EASYCWMP
	huayi_reload_cwmp();
#endif	
	return VT_TRUE;
}


VTBool huayi_reload_upnpd(void * arg,cJSON *input_root)
{
	system("/etc/init.d/miniupnpd restart");
	return VT_TRUE;
}

#endif
