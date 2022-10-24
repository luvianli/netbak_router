/**
 * @file huayi_stt.c
 * 
 * 
 */
//#include <asm/cpu-info.h>
//#include <asm/cpu.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/kernel.h>
#include <linux/string.h> 
#include <linux/kernel_stat.h>
#include <linux/swap.h>
#include <linux/ethtool.h>
#include <linux/math64.h>
#include <huayi-universal/huayi_runstate.h>
#include <huayi-universal/huayi_universal.h>
#include <huayi-universal/huayi_dal.h>
#include "huayi_stt.h"
#include <asm/cputime.h>
#include <linux/tick.h>


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HUAYI STT Module");
MODULE_AUTHOR("xxx");


static struct stt s_stt;


static int status_about_proc(struct seq_file *s, void *v)
{
    struct huayi_runstate *s_runstate = HUAYI_RUNSTATE_GET();
	struct sysinfo sys_info;
	
	si_meminfo(&sys_info);
	
    seq_printf(s, "config 'about' 'about_hardware'\n");
    seq_printf(s, "        option 'system_type' '%s'\n",s_runstate->about.system_type);
	
	seq_printf(s, "        option 'cpu_model' '%s'\n",s_runstate->about.cpu_model);
	
	seq_printf(s, "        option 'cpu_cores' '%u'\n",s_runstate->about.cpu_cores);
	
	seq_printf(s, "        option 'cpu_processors' '%u'\n",s_runstate->about.cpu_processors);

	seq_printf(s, "        option 'flash_size' '%u'\n",s_runstate->about.flash_size);

	seq_printf(s, "        option 'mem_type' '%s'\n",MEM_TYPE);
	
	seq_printf(s, "        option 'mem_size' '%lu'\n",K(sys_info.totalram));
	
    seq_printf(s, "\n");
	
    seq_printf(s, "config 'about' 'about_software'\n");
	
    seq_printf(s, "        option 'fw_version' '%s'\n", s_runstate->about.fw_version);

	seq_printf(s, "        option 'buildtime' '%s'\n",s_runstate->about.buildtime);
	
    seq_printf(s, "        option 'uboot_version' '%s'\n",s_runstate->about.uboot_version);

	seq_printf(s, "        option 'webui_version' '%s'\n",s_runstate->about.webui_version);

    seq_printf(s, "\n");
	
    seq_printf(s, "config 'about' 'about_system'\n");
	
	seq_printf(s, "        option 'product_name' '%s'\n",s_runstate->about.product_name);

	seq_printf(s, "        option 'product_model' '%s'\n",s_runstate->about.product_model);

	seq_printf(s, "        option 'product_sn' '%s'\n",s_runstate->about.product_sn);
	
    seq_printf(s, "        option 'product_desc' '%s'\n",s_runstate->about.product_desc);
	
    seq_printf(s, "        option 'company_name' '%s'\n",s_runstate->about.company_name);
	
    seq_printf(s, "        option 'enterprise_oid' '%u'\n",s_runstate->about.enterprise_oid);

    seq_printf(s, "        option 'company_website' '%s'\n",s_runstate->about.company_website);
	
    seq_printf(s, "        option 'company_contacts' '%s'\n",s_runstate->about.company_contacts);
	
    seq_printf(s, "        option 'company_address' '%s'\n", s_runstate->about.company_address);

	seq_printf(s, "        option 'company_copyright' '%s'\n", s_runstate->about.company_copyright);
	
	seq_printf(s, "        option 'domain_name' '%s'\n",s_runstate->about.domain_name);
	
    seq_printf(s, "\n");
	
    return HUAYI_OK;

}


static int about_single_open(struct inode *inode, struct file *file)
{
   return(single_open(file,status_about_proc, NULL));
}

struct file_operations about_proc_fops = {
        .open           = about_single_open,
	 	.write			= NULL,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#ifdef arch_idle_time

static cputime64_t get_idle_time(int cpu)
{
	cputime64_t idle;

	idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	if (cpu_online(cpu) && !nr_iowait_cpu(cpu))
		idle += arch_idle_time(cpu);
	return idle;
}

static cputime64_t get_iowait_time(int cpu)
{
	cputime64_t iowait;

	iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
	if (cpu_online(cpu) && nr_iowait_cpu(cpu))
		iowait += arch_idle_time(cpu);
	return iowait;
}

#else

static u64 get_idle_time(int cpu)
{
	u64 idle, idle_time = -1ULL;

	if (cpu_online(cpu))
		idle_time = get_cpu_idle_time_us(cpu, NULL);

	if (idle_time == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.idle */
		idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	else
		idle = usecs_to_cputime64(idle_time);

	return idle;
}

static u64 get_iowait_time(int cpu)
{
	u64 iowait, iowait_time = -1ULL;

	if (cpu_online(cpu))
		iowait_time = get_cpu_iowait_time_us(cpu, NULL);

	if (iowait_time == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.iowait */
		iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
	else
		iowait = usecs_to_cputime64(iowait_time);

	return iowait;
}

#endif



static int status_sys_proc(struct seq_file *s, void *v)
{
    struct huayi_runstate *s_runstate = HUAYI_RUNSTATE_GET();
	struct timex  txc;
    struct timespec tp;
    struct sysinfo sys_info;
    int i ;
    long mem_used, mem_free, mem_cached = 0;
	u64 user, nice, system, idle, iowait, irq, softirq, steal;
	u64 guest;

	user = nice = system = idle = iowait =
    irq = softirq = steal = 0;
    guest = 0;

	for_each_possible_cpu(i) {
		user += kcpustat_cpu(i).cpustat[CPUTIME_USER];
		nice += kcpustat_cpu(i).cpustat[CPUTIME_NICE];
		system += kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
		idle += get_idle_time(i);
		iowait += get_iowait_time(i);
		irq += kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
		softirq += kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
		steal += kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
		guest += kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
	}



    si_meminfo(&sys_info);
	
    mem_cached = global_page_state(NR_FILE_PAGES) - sys_info.bufferram;

	if (mem_cached < 0)
        mem_cached = 0;
	
    mem_free = sys_info.freeram;
	
    mem_used = sys_info.totalram - mem_free;
	
    do_gettimeofday(&(txc.time));
	
    ktime_get_ts(&tp);
	
  
    seq_printf(s, "config 'sys' 'sys'\n");
	
    seq_printf(s, "       option 'uptime' '%ld'\n",tp.tv_sec + (tp.tv_nsec ? 1 : 0));

	seq_printf(s, "       option 'rec_uptime' '%u'\n",s_runstate->system.rec_uptime);
	
	seq_printf(s, "       option 'time_of_day'  '%ld'\n", txc.time.tv_sec );

	seq_printf(s, "       option 'mem_total' '%lu'\n",K(sys_info.totalram));

	seq_printf(s, "       option 'mem_used' '%lu'\n",K(mem_used));

	seq_printf(s, "       option 'mem_cached' '%lu'\n",K(mem_cached));

	seq_printf(s, "       option 'mem_free' '%lu'\n",K(mem_free));
	
    seq_printf(s, "       option 'mem_rt_usage' '%lu'\n", ((mem_used) * 100) / sys_info.totalram);

	seq_printf(s, "       option 'mem_avg_usage' '%u'\n", s_runstate->system.mem_avg_usage);
    
	seq_printf(s, "       option 'cpu_total_time' '%llu'\n", (user + nice + system + idle +iowait + irq + softirq + steal + guest));
	
	seq_printf(s, "       option 'cpu_idle_time' '%llu'\n", idle);
	
    seq_printf(s, "       option 'cpu_rt_usage' '%u'\n", s_runstate->system.cpu_rt_usage);

	seq_printf(s, "       option 'cpu_avg_usage' '%u'\n", s_runstate->system.cpu_avg_usage);

    seq_printf(s, "       option 'flash_usage' '%u'\n", s_runstate->system.flash_usage);

	seq_printf(s, "       option 'upgrade_keep_setting' '%u'\n", s_runstate->system.upgrade_keep_setting);

	seq_printf(s, "       option 'usb_storage' '%u'\n", s_runstate->system.usb_storage);

	seq_printf(s, "       option 'ping_detect' '%u'\n", s_runstate->system.ping_detect);

	seq_printf(s, "       option 'tracert_detect' '%u'\n", s_runstate->system.tracert_detect);

	seq_printf(s, "       option 'dhcp_detect' '%u'\n", s_runstate->system.dhcp_detect);
#if defined(CONFIG_PACKAGE_HUAYI_DDNS)
	seq_printf(s, "       option 'ddns_state' '%u'\n", s_runstate->system.ddns_state);

	seq_printf(s, "       option 'ddns_ip'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->system.ddns_ip));
#endif
	seq_printf(s, "       option 'wan_proto' '%u'\n", s_runstate->system.wan_proto);
	
	seq_printf(s, "       option 'scan_state' '%u'\n", s_runstate->system.scan_state);

	seq_printf(s, "       option 'rep_24g_status' '%u'\n", s_runstate->system.rep_24g_status);
	
	seq_printf(s, "       option 'rep_58g_status' '%u'\n", s_runstate->system.rep_58g_status);

	seq_printf(s, "       option 'wifi_24g_num' '%u'\n", s_runstate->system.wifi_24g_num);
	
	seq_printf(s, "       option 'wifi_58g_num' '%u'\n", s_runstate->system.wifi_58g_num);
#if defined(CONFIG_PACKAGE_HUAYI_CAPWAP_CLIENT)
	seq_printf(s, "       option 'ac_ipaddr'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->system.ac_ipaddr));

	seq_printf(s, "       option 'capwap_client_status'  '%d'\n", s_runstate->system.capwap_client_status);

	seq_printf(s, "       option 'ac_assoc_time'  '%ld'\n", s_runstate->system.ac_assoc_time);
#endif

#if defined(CONFIG_HUAYI_BUTTON_WIFI_SUPPORT)	
	seq_printf(s, "       option 'wifi_button'  '%ld'\n", s_runstate->system.wifi_button);
#endif

#if defined(CONFIG_HUAYI_SUPPORT_VPN)
	seq_printf(s, "       option 'pptp_link' '%u'\n", s_runstate->system.pptp_link);

	seq_printf(s, "       option 'pptp_ip'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->system.pptp_ip));

	seq_printf(s, "       option 'xl2tp_link' '%u'\n", s_runstate->system.xl2tp_link);

	seq_printf(s, "       option 'xl2tp_ip'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->system.xl2tp_ip));
#endif

	seq_printf(s, "       option 'arp_flush' '%u'\n", s_runstate->system.arp_flush);

	seq_printf(s, "       option 'wifi_off' '%u'\n", s_runstate->system.wifi_off);
	
    seq_printf(s, "\n");
    return HUAYI_OK;

}


static int status_sys_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file,status_sys_proc, NULL));
}

struct file_operations status_sys_proc_fops = {
        .open           = status_sys_single_open,
	 	.write			= NULL,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};


static void* status_network_get(struct dal_net_stats *dev_stats, struct net_device *dev) 
{
	struct rtnl_link_stats64 netdevstats;
	struct huayi_runstate *s_runstate = HUAYI_RUNSTATE_GET();
	
    if (0==strcmp(dev->name, "eth1") || 0==strcmp(dev->name, "eth0.2") || 0==strcmp(dev->name, "apcli0"))  
	{
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
		if (0 != strcmp(dev->name, s_runstate->network.wan_device))
		{
			if (strlen(s_runstate->network.wan_device) == 0 || 
				(strstr(s_runstate->network.wan_device,"pppoe") != NULL &&
				strcmp(dev->name, "eth0.2") == 0))
			{
				;
			}
			else
			{
				return NULL;
			}
		}
#endif
		BZERO(&dev_stats->wan_stats, sizeof(struct dal_wan_stats));
		MAC_ADDR_COPY(&dev_stats->wan_stats.mac_address, dev->dev_addr);
		dev_stats->wan_stats.if_mtu = dev->mtu;
		snprintf(dev_stats->wan_stats.device_id, DAL_MAX_IFNAME, "%s",s_runstate->network.wan_device);
		snprintf(dev_stats->wan_stats.desc, DAL_MAX_DEVICE_DESC, "%s", "wan");
		dev_stats->wan_stats.ipv4_addr = s_runstate->network.wan_ipv4;
		dev_stats->wan_stats.ipv4_mask = s_runstate->network.wan_netmask;
		dev_stats->wan_stats.gateway = s_runstate->network.wan_gateway;
		dev_stats->wan_stats.link = s_runstate->network.wan_link;
		dev_stats->wan_stats.wan_up= s_runstate->network.wan_up;
		dev_stats->wan_stats.wan_uptime= s_runstate->network.wan_uptime;
		memcpy(&dev_stats->wan_stats.dns_server,&s_runstate->network.wan_dns_server,sizeof(u_int32_t)*CSTATUS_MAX_DNS_SERVER);
		dev_stats->wan_stats.rx_bps = s_runstate->network.wan_rx_bps;
		dev_stats->wan_stats.tx_bps = s_runstate->network.wan_tx_bps;
		dev_get_stats((struct net_device*)dev,&netdevstats);
		dev_stats->wan_stats.rx_bytes		= 	netdevstats.rx_bytes;
		dev_stats->wan_stats.rx_packets 	= netdevstats.rx_packets;
		dev_stats->wan_stats.tx_bytes	= netdevstats.tx_bytes;
		dev_stats->wan_stats.tx_packets	= netdevstats.tx_packets;
		dev_stats->wan_stats.rx_dropped 	= netdevstats.rx_dropped;
		dev_stats->wan_stats.tx_dropped 	= netdevstats.tx_dropped;
		dev_stats->wan_stats.rx_errors	= netdevstats.rx_errors;
		dev_stats->wan_stats.tx_errors 	= netdevstats.tx_errors;
		dev_stats->wan_stats.multicast 	= netdevstats.multicast;
        return dev;
		
    }else if(0==strcmp(dev->name, "eth0"))
    {
    	BZERO(&dev_stats->lan_stats, sizeof(struct dal_lan_stats));
		MAC_ADDR_COPY(&dev_stats->lan_stats.mac_address, dev->dev_addr);
		dev_stats->lan_stats.if_mtu = dev->mtu;
		snprintf(dev_stats->lan_stats.device_id, DAL_MAX_IFNAME, "%s",s_runstate->network.lan_device);
		snprintf(dev_stats->lan_stats.desc, DAL_MAX_DEVICE_DESC, "%s", "lan");	
		dev_stats->lan_stats.ipv4_addr = s_runstate->network.lan_ipv4;
		dev_stats->lan_stats.ipv4_mask = s_runstate->network.lan_netmask;
		dev_stats->lan_stats.gateway = s_runstate->network.lan_gateway;
		dev_stats->lan_stats.linkport = s_runstate->network.linkport;
		return dev;
    }
   	return NULL;
}

void network_status_show(struct seq_file *s, struct dal_net_stats *common_stats)
{
	int i = 0;
	
	seq_printf(s, "config 'interface' 'wan'\n");
	seq_printf(s, "       option 'ifname' '%s'\n",common_stats->wan_stats.device_id);
    seq_printf(s, "       option 'mac_address' '"MAC_FMT"'\n", MAC_ARG(common_stats->wan_stats.mac_address));
	seq_printf(s, "       option 'ip_address'  '"NIPQUAD_FMT"'\n", NIPQUAD(common_stats->wan_stats.ipv4_addr));
	seq_printf(s, "       option 'netmask'  '"NIPQUAD_FMT"'\n", NIPQUAD(common_stats->wan_stats.ipv4_mask));
	seq_printf(s, "       option 'gateway'  '"NIPQUAD_FMT"'\n", NIPQUAD(common_stats->wan_stats.gateway));
	for(i=0; i < CSTATUS_MAX_DNS_SERVER; i++) 
	{
        if (common_stats->wan_stats.dns_server[i] == 0)
		{
            break;
        }
    	seq_printf(s, "       list 'dns_server' '"NIPQUAD_FMT"'\n",NIPQUAD(common_stats->wan_stats.dns_server[i]));
    }
	seq_printf(s, "       option 'if_mtu' '%u'\n",  common_stats->wan_stats.if_mtu);
    seq_printf(s, "       option 'description' '%s'\n", common_stats->wan_stats.desc);
	seq_printf(s, "       option 'rx_packets' '%llu'\n", common_stats->wan_stats.rx_packets);
	seq_printf(s, "       option 'tx_packets' '%llu'\n", common_stats->wan_stats.tx_packets);
	seq_printf(s, "       option 'rx_bytes' '%llu'\n", common_stats->wan_stats.rx_bytes);
	seq_printf(s, "       option 'tx_bytes' '%llu'\n", common_stats->wan_stats.tx_bytes);
	seq_printf(s, "       option 'rx_errors' '%llu'\n", common_stats->wan_stats.rx_errors);
	seq_printf(s, "       option 'tx_errors' '%llu'\n", common_stats->wan_stats.tx_errors);
	seq_printf(s, "       option 'rx_dropped' '%llu'\n", common_stats->wan_stats.rx_dropped);
	seq_printf(s, "       option 'tx_dropped' '%llu'\n", common_stats->wan_stats.tx_dropped);
	seq_printf(s, "       option 'link' '%u'\n", common_stats->wan_stats.link);
	seq_printf(s, "       option 'wan_up' '%u'\n", common_stats->wan_stats.wan_up);
	seq_printf(s, "       option 'wan_uptime' '%ld'\n", common_stats->wan_stats.wan_uptime);
	seq_printf(s, "       option 'rx_bps' '%u'\n", common_stats->wan_stats.rx_bps);
	seq_printf(s, "       option 'tx_bps' '%u'\n", common_stats->wan_stats.tx_bps);
	seq_printf(s, "\n");

	seq_printf(s, "config 'interface' 'lan'\n");
	seq_printf(s, "       option 'ifname' '%s'\n",common_stats->lan_stats.device_id);
    seq_printf(s, "       option 'mac_address' '"MAC_FMT"'\n", MAC_ARG(common_stats->lan_stats.mac_address));
	seq_printf(s, "       option 'ip_address'  '"NIPQUAD_FMT"'\n", NIPQUAD(common_stats->lan_stats.ipv4_addr));
	seq_printf(s, "       option 'netmask'  '"NIPQUAD_FMT"'\n", NIPQUAD(common_stats->lan_stats.ipv4_mask));
	seq_printf(s, "       option 'gateway'  '"NIPQUAD_FMT"'\n", NIPQUAD(common_stats->lan_stats.gateway));
	seq_printf(s, "       option 'if_mtu' '%u'\n",  common_stats->lan_stats.if_mtu);
	seq_printf(s, "       option 'linkport' '%u'\n",  common_stats->lan_stats.linkport);
    seq_printf(s, "       option 'description' '%s'\n", common_stats->lan_stats.desc);
	seq_printf(s, "\n");

	return;
}


static int status_net_proc(struct seq_file *s, void *v)
{
	struct dal_net_stats network_status;
	struct net_device *dev;
	struct net *net = seq_file_net(s);

	memset(&network_status,0,sizeof(struct dal_net_stats));
	
	read_lock(&dev_base_lock);

	dev = net_device_entry(&net->dev_base_head);
	
	for_each_netdev(net, dev) {
		if (strstr(dev->name, "eth1") == NULL && strstr(dev->name, "br-lan") == NULL &&
			strstr(dev->name, "eth0") == NULL && strstr(dev->name, "apcli") == NULL)
		{
			continue;
		}
		
		if (status_network_get(&network_status, dev) == NULL)
		{
			continue;
		}
	}
	
	read_unlock(&dev_base_lock);
	
	network_status_show(s,&network_status);

	
	return HUAYI_OK;

}

static int status_net_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file,status_net_proc, NULL));
}

struct file_operations status_net_proc_fops = {
        .open           = status_net_single_open,
	 	.write			= NULL,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
static int status_modem_proc(struct seq_file *s, void *v)
{
    struct huayi_runstate *s_runstate = HUAYI_RUNSTATE_GET();
	
    seq_printf(s, "config 'status' 'modem'\n");
    seq_printf(s, "        option 'modemModel' '%s'\n",s_runstate->modem.modemModel);

	seq_printf(s, "        option 'sim_ready' '%u'\n",s_runstate->modem.sim_ready);

	seq_printf(s, "        option 'network' '%s'\n",s_runstate->modem.network);

	seq_printf(s, "        option 'operator' '%s'\n",s_runstate->modem.operator);

	seq_printf(s, "        option 'mobile_type' '%s'\n",s_runstate->modem.mobile_type);
	
	seq_printf(s, "        option 'registStatus' '%s'\n",s_runstate->modem.registStatus);
	
	seq_printf(s, "        option 'imei' '%s'\n",s_runstate->modem.imei);
	
	seq_printf(s, "        option 'imsi' '%s'\n",s_runstate->modem.imsi);

	seq_printf(s, "        option 'phoneNumber' '%s'\n",s_runstate->modem.phoneNumber);

	seq_printf(s, "        option 'center_number' '%s'\n",s_runstate->modem.center_number);

	seq_printf(s, "        option 'bandLte' '%s'\n",s_runstate->modem.bandLte);
	
	seq_printf(s, "        option 'signal' '%s'\n",s_runstate->modem.signal);

	seq_printf(s, "        option 'lteMode' '%s'\n",s_runstate->modem.lteMode);

	seq_printf(s, "        option 'iccid' '%s'\n",s_runstate->modem.iccid);

	seq_printf(s, "        option 'lac' '%s'\n",s_runstate->modem.lac);

	seq_printf(s, "        option 'ci' '%s'\n",s_runstate->modem.ci);

	seq_printf(s, "        option 'dialresetcount' '%s'\n",s_runstate->modem.dialresetcount);
	
    seq_printf(s, "\n");
	
    return HUAYI_OK;

}


static int modem_single_open(struct inode *inode, struct file *file)
{
   return(single_open(file,status_modem_proc, NULL));
}

struct file_operations modem_proc_fops = {
        .open           = modem_single_open,
	 	.write			= NULL,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY)
static int status_blend_proc(struct seq_file *s, void *v)
{
    struct huayi_runstate *s_runstate = HUAYI_RUNSTATE_GET();
	int i = 0;

	seq_printf(s, "config 'interface' 'wired'\n");
	seq_printf(s, "       option 'device' '%s'\n",s_runstate->blend.wired_device_id);
	seq_printf(s, "       option 'ipv4'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.wired_ipv4_addr));
	seq_printf(s, "       option 'netmask'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.wired_ipv4_mask));
	seq_printf(s, "       option 'gateway'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.wired_gateway));
	for(i=0; i < CSTATUS_MAX_DNS_SERVER; i++) 
	{
        if (s_runstate->blend.wired_dns_server[i] == 0)
		{
            break;
        }
    	seq_printf(s, "       list 'dns_server' '"NIPQUAD_FMT"'\n",NIPQUAD(s_runstate->blend.wired_dns_server[i]));
    }
	seq_printf(s, "       option 'link' '%u'\n", s_runstate->blend.wired_link);
	seq_printf(s, "\n");

	seq_printf(s, "config 'interface' 'cpe'\n");
	seq_printf(s, "       option 'device' '%s'\n",s_runstate->blend.cpe_device_id);
	seq_printf(s, "       option 'ipv4'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.cpe_ipv4_addr));
	seq_printf(s, "       option 'netmask'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.cpe_ipv4_mask));
	seq_printf(s, "       option 'gateway'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.cpe_gateway));
	for(i=0; i < CSTATUS_MAX_DNS_SERVER; i++) 
	{
        if (s_runstate->blend.cpe_dns_server[i] == 0)
		{
            break;
        }
    	seq_printf(s, "       list 'dns_server' '"NIPQUAD_FMT"'\n",NIPQUAD(s_runstate->blend.cpe_dns_server[i]));
    }
	seq_printf(s, "       option 'link' '%u'\n", s_runstate->blend.cpe_link);
	seq_printf(s, "\n");
	
    return HUAYI_OK;

}


static int blend_single_open(struct inode *inode, struct file *file)
{
   return(single_open(file,status_blend_proc, NULL));
}

struct file_operations blend_proc_fops = {
        .open           = blend_single_open,
	 	.write			= NULL,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#if defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
static int status_blend_proc(struct seq_file *s, void *v)
{
    struct huayi_runstate *s_runstate = HUAYI_RUNSTATE_GET();
	int i = 0;

	seq_printf(s, "config 'interface' 'wisp'\n");
	seq_printf(s, "       option 'device' '%s'\n",s_runstate->blend.wired_device_id);
	seq_printf(s, "       option 'ipv4'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.wired_ipv4_addr));
	seq_printf(s, "       option 'netmask'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.wired_ipv4_mask));
	seq_printf(s, "       option 'gateway'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.wired_gateway));
	for(i=0; i < CSTATUS_MAX_DNS_SERVER; i++) 
	{
        if (s_runstate->blend.wired_dns_server[i] == 0)
		{
            break;
        }
    	seq_printf(s, "       list 'dns_server' '"NIPQUAD_FMT"'\n",NIPQUAD(s_runstate->blend.wired_dns_server[i]));
    }
	seq_printf(s, "       option 'link' '%u'\n", s_runstate->blend.wired_link);
	seq_printf(s, "\n");

	seq_printf(s, "config 'interface' 'cpe'\n");
	seq_printf(s, "       option 'device' '%s'\n",s_runstate->blend.cpe_device_id);
	seq_printf(s, "       option 'ipv4'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.cpe_ipv4_addr));
	seq_printf(s, "       option 'netmask'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.cpe_ipv4_mask));
	seq_printf(s, "       option 'gateway'  '"NIPQUAD_FMT"'\n", NIPQUAD(s_runstate->blend.cpe_gateway));
	for(i=0; i < CSTATUS_MAX_DNS_SERVER; i++) 
	{
        if (s_runstate->blend.cpe_dns_server[i] == 0)
		{
            break;
        }
    	seq_printf(s, "       list 'dns_server' '"NIPQUAD_FMT"'\n",NIPQUAD(s_runstate->blend.cpe_dns_server[i]));
    }
	seq_printf(s, "       option 'link' '%u'\n", s_runstate->blend.cpe_link);
	seq_printf(s, "\n");
	
    return HUAYI_OK;

}


static int blend_single_open(struct inode *inode, struct file *file)
{
   return(single_open(file,status_blend_proc, NULL));
}

struct file_operations blend_proc_fops = {
        .open           = blend_single_open,
	 	.write			= NULL,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif


HUAYI_STATUS create_stt_proc(void)
{
    if(huayi_proc == NULL) {
        STT_LOG(HUAYILOG_CRIT, "Initialize /proc/%s failed\n", HUAYI_PROC_ROOT);
        return HUAYI_ERROR;
    }

	
    s_stt.proc_dir_stats = huayi_proc;

/*-------------------- create file about-------------------------------------------*/
	snprintf(s_stt.about.name, STT_MAX_NAME, "%s", STT_ABOUT);

	s_stt.about.proc = proc_create_data(s_stt.about.name,0, s_stt.proc_dir_stats, &about_proc_fops, NULL);
	if (s_stt.about.proc == NULL) 
	{
		printk("Initialize /proc/%s/%s\n failed",HUAYI_PROC_ROOT,s_stt.about.name);
		return HUAYI_ERROR;
	}
	
/*-------------------- create file about-------------------------------------------*/

/*-------------------- create file status_system-------------------------------------------*/
	snprintf(s_stt.sys_stats.name, STT_MAX_NAME, "%s", STT_SYS_STATS);
	s_stt.sys_stats.proc = proc_create_data(s_stt.sys_stats.name,0, s_stt.proc_dir_stats, &status_sys_proc_fops, NULL);
	if (s_stt.sys_stats.proc == NULL) {
	    printk("Initialize /proc/%s/%s\n failed",HUAYI_PROC_ROOT, s_stt.sys_stats.name);
	    return HUAYI_ERROR;
	}
/*-------------------- create file status_system-------------------------------------------*/

/*-------------------- create file status_network-------------------------------------------*/
	snprintf(s_stt.net_stats.name, STT_MAX_NAME, "%s", STT_NET_STATS);
	s_stt.net_stats.proc = proc_create_data(s_stt.net_stats.name,0, s_stt.proc_dir_stats, &status_net_proc_fops, NULL);
	if (s_stt.net_stats.proc == NULL) {
		printk("Initialize /proc/%s/%s\n failed",HUAYI_PROC_ROOT, s_stt.net_stats.name);
		return HUAYI_ERROR;
	}
/*-------------------- create file status_network-------------------------------------------*/

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
/*-------------------- create file status_modem-------------------------------------------*/
		snprintf(s_stt.modem_stats.name, STT_MAX_NAME, "%s", STT_MODEM_STATS);
		s_stt.modem_stats.proc = proc_create_data(s_stt.modem_stats.name,0, s_stt.proc_dir_stats, &modem_proc_fops, NULL);
		if (s_stt.modem_stats.proc == NULL) {
			printk("Initialize /proc/%s/%s\n failed",HUAYI_PROC_ROOT, s_stt.modem_stats.name);
			return HUAYI_ERROR;
		}
/*-------------------- create file status_modem-------------------------------------------*/
#endif

#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY) || defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
/*-------------------- create file status_blend-------------------------------------------*/
		snprintf(s_stt.blend_stats.name, STT_MAX_NAME, "%s", STT_BLEND_STATS);
		s_stt.blend_stats.proc = proc_create_data(s_stt.blend_stats.name,0, s_stt.proc_dir_stats, &blend_proc_fops, NULL);
		if (s_stt.blend_stats.proc == NULL) {
			printk("Initialize /proc/%s/%s\n failed",HUAYI_PROC_ROOT, s_stt.blend_stats.name);
			return HUAYI_ERROR;
		}
/*-------------------- create file status_blend-------------------------------------------*/
#endif

    return HUAYI_OK;
}


HUAYI_STATUS remove_stt_proc(void) 
{

/*--------------------remove file about-------------------------------------------*/
	if (s_stt.about.proc != NULL) {
        remove_proc_entry(s_stt.about.name, s_stt.proc_dir_stats);
        s_stt.about.proc = NULL;
    }
/*--------------------remove file status_system-------------------------------------------*/
	 // sys stats
    if (s_stt.sys_stats.proc != NULL) {
        remove_proc_entry(s_stt.sys_stats.name, s_stt.proc_dir_stats);
        s_stt.sys_stats.proc = NULL;
    }
/*--------------------remove file status_system-------------------------------------------*/

/*--------------------remove file status_network-------------------------------------------*/
	if (s_stt.net_stats.proc != NULL) {
		remove_proc_entry(s_stt.net_stats.name, s_stt.proc_dir_stats);
		s_stt.net_stats.proc = NULL;
	}
/*--------------------remove file status_network-------------------------------------------*/

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
/*--------------------remove file status_modem-------------------------------------------*/
	if (s_stt.modem_stats.proc != NULL) {
		remove_proc_entry(s_stt.modem_stats.name, s_stt.proc_dir_stats);
		s_stt.modem_stats.proc = NULL;
	}
/*--------------------remove file status_modem-------------------------------------------*/
#endif
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY) || defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
/*--------------------remove file status_blend-------------------------------------------*/
	if (s_stt.modem_stats.proc != NULL) {
		remove_proc_entry(s_stt.blend_stats.name, s_stt.proc_dir_stats);
		s_stt.modem_stats.proc = NULL;
	}
/*--------------------remove file status_blend-------------------------------------------*/
#endif
	
    return HUAYI_OK;
}

void stt_init(void) {
    BZERO(&s_stt, sizeof(struct stt));
    create_stt_proc();
    return;
}

void stt_deinit(void) {
    remove_stt_proc();
    return;
}

static int32_t __init module_huayi_stt_init(void){
	stt_init();
	//printk("huayi stt module init\n");
    return 0;
}
module_init(module_huayi_stt_init);

static void __exit module_huayi_stt_exit(void){
	stt_deinit();
	//printk("huayi stt module exit\n");
    return;
}
module_exit(module_huayi_stt_exit);

