
/*notice :this module can not run as a service 
  this module just run one time !!!!!!!
*/

#include "huayi_init.h"
#include "huayi_uci_fun.h"

void huayi_tcp_conntrack_init()
{
	system("echo 20 > /proc/sys/net/netfilter/nf_conntrack_tcp_timeout_syn_sent");

	system("echo 86400 > /proc/sys/net/netfilter/nf_conntrack_tcp_timeout_established");

	system("echo 10 > /proc/sys/net/netfilter/nf_conntrack_tcp_timeout_time_wait");

	system("echo 10 > /proc/sys/net/netfilter/nf_conntrack_tcp_timeout_close");

	system("echo 60 > /proc/sys/net/netfilter/nf_conntrack_tcp_timeout_close_wait");

	system("echo 30 > /proc/sys/net/netfilter/nf_conntrack_udp_timeout");

	system("echo 180 > /proc/sys/net/netfilter/nf_conntrack_udp_timeout_stream");

	system("echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal");

	system("echo 28 > /proc/sys/net/netfilter/nf_conntrack_expect_max");

	system("echo 512 > /proc/sys/net/ipv4/tcp_max_orphans");

	system("echo 512 > /proc/sys/net/ipv4/tcp_max_tw_buckets");

	system("echo 120 > /proc/sys/net/ipv4/tcp_keepalive_time");
	
	system("echo 3 > /proc/sys/net/ipv4/tcp_keepalive_intvl");
	
	system("echo 2 > /proc/sys/net/ipv4/tcp_keepalive_probes");
		
	system("echo 256 > /proc/sys/vm/min_free_kbytes");
	
	//system("echo 600 > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established");
	
	//system("echo 4096 > /proc/sys/net/ipv4/netfilter/ip_conntrack_max");

	//system("echo 120 > /proc/sys/net/ipv4/netfilter/ip_conntrack_generic_timeout");
	
	//system("echo 15 > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_syn_recv");
	
	//system("echo 60 > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_fin_wait");
	
	//system("echo 2 > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_max_retrans");
	
	//system("echo 6 > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_syn_recv");
	
	system("echo 15 > /proc/sys/net/netfilter/nf_conntrack_icmp_timeout");
	
	system("echo 3000 > /proc/sys/net/core/netdev_max_backlog");
	
	system("echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse");
	
	system("echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle");
	
	system("echo 30 > /proc/sys/net/ipv4/tcp_fin_timeout");
	
	system("echo 8196 > /proc/sys/net/ipv4/tcp_max_syn_backlog");
	
	return;
}

#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
void huayi_firewall_start()
{
	system("huayi-reset firewall");
	
	return;
}
#endif

void set_simcarinfo()
{
	int status = 0,card_slot = 0;
	huayi_uci_get_int(PKG_SIMINFO, "system", "status", &status);
	if(status == 1)
	{
		int i,simnum = 0;
		char cmd[SHORT_STR_LEN] = {0};
		huayi_uci_get_int(PKG_M_SYSTEM, "system", "simnum", &simnum);
		char iccid[simnum][OPTION_STR_LEN],operat[simnum][1];
	    for (i = 0; i < simnum-1; i++)
	    {
			sprintf(cmd, "sim%d", i);
 			huayi_uci_get_str(PKG_SIMINFO, cmd, "iccid", iccid[i]);
			huayi_uci_get_str(PKG_SIMINFO, cmd, "operator", operat[i]);
			if(strlen(iccid[i]) > 9)
	        {
	        	huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid", iccid[i]);
				huayi_uci_set_str(PKG_M_SYSTEM, cmd, "operator", operat[i]);
			}
			else
				huayi_uci_set_str(PKG_M_SYSTEM, cmd, "iccid", "");
	    }
		huayi_uci_get_int(PKG_SIMINFO, "system", "card_slot", &card_slot);
		huayi_uci_set_int(PKG_M_SYSTEM, "usesiminfo", "card_slot", card_slot);
		huayi_uci_set_int(PKG_SIMINFO, "system", "status", 0);
		huayi_uci_commit(PKG_SIMINFO);
		huayi_uci_commit(PKG_M_SYSTEM);
	}

	return ;
}

int main(int argc, char** argv)
{
	int opmode = 0;
#if defined(CONFIG_HUAYI_SUPPORT_SPECIAL_INTERFACE)
	unsigned char sn[OPTION_STR_LEN]={ 0 };
	if (get_dev_sn(sn) < 0) 
	{
		strcpy(sn, " ");
	}
	if(strncmp(sn, "FFFFFFFFFFFF", strlen(sn)) == 0 ||
		strncmp(sn, "ASASA6793058", strlen(sn)) == 0)
	{
		strcpy(sn, " ");
	}
	if(strncmp(sn, " ", strlen(sn)) != 0)
		system("reboot");
#endif

	huayi_tcp_conntrack_init();

	system("watchdog &");
	
#if defined(CONFIG_HUAYI_GATEWAY_MODE_SUPPORT)
	int wan_port_num = 0;
	int wan_link = 0;
	
	huayi_uci_get_int(PKG_CAPABILITY,"cap","wan_port_num",&wan_port_num);
		
	wan_link = is_phyconnected(wan_port_num);
	if (wan_link == 1)
	{
		#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
		system("huayi_cmd set blend wired_link 1");
		#endif
		system("huayi_cmd set network wan_link 1");
	}

	huayi_uci_get_int(PKG_SYSTEM,"mode","opmode",&opmode);

	if((HUAYI_ROUTE_MODE==opmode)||(HUAYI_WISP_MODE==opmode))
	{
		huayi_firewall_start();	
	}
#endif

#if defined(CONFIG_HUAYI_AP_MODE_SUPPORT)
	char cmd_str[CMD_STR_LEN] = {0};
	int i = 0;
	int linkport = 0;

	for (i = 0; i < ETH_SWITCH_PORT_NUM; i++)
	{
		linkport |= is_phyconnected(i) << i;
	}
	
	snprintf(cmd_str,sizeof(cmd_str),"huayi_cmd set network linkport %d",linkport);
	system(cmd_str);
#endif
	set_simcarinfo();
	return 0;
}
