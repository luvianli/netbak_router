/*
**  File:    vsf_hook.c
**  Author:  rex.tian
**  Date:    2/24/2018
**
**  History: 
**  <author>   <time>          <version >   <desc>
*/

#include <asm/bug.h>
#include "include/qos_def.h"
#include "include/vsf_common.h"
#include <net/netfilter/vs_line_client.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <linux/netfilter_ipv4.h>
#include "include/vsf_hook.h"
//#include "include/flow_analyze.h"



MODULE_LICENSE("GPL");
MODULE_AUTHOR("rex.tian");
MODULE_DESCRIPTION("vsf");

static u_int32_t vsf_hook(unsigned int hook,
						    struct sk_buff *pskb,
					           const struct net_device *in,
					           const struct net_device *out,
					           int (*okfn)(struct sk_buff *))
{
	struct nf_conn *ct = (struct nf_conn *)pskb->nfct;
	e_vsf_nf_action ret = VSF_NF_ACCEPT;
	//struct vsf_system_config *system_config = HUAYI_FILTER_CONFIG_GET(); 
	
	if (ct == NULL) {
        //printk("ct is NULL, protocol: %d\n", (int)ip_hdr(pskb)->protocol);
        goto done;
    }
    if (nf_ct_is_untracked(ct)) {
        //printk("untracked ct, protocol: %d\n", (int)ip_hdr(pskb)->protocol);
        goto done;
    }
	if (!(ct->online_ip)) {
        //printk("nos_track is NULL\n");
        goto done;
    }
	
	mask_set_bit(ct->flow_status, FLOW_FORWARD);

done:	
	return NF_ACCEPT;
}

static struct nf_hook_ops vsf_ops[] __read_mostly = {
	{
		.hook		= vsf_hook,
		.owner		= THIS_MODULE,
		.pf			= PF_INET,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_MANGLE + 1,/*让iptables 的mangle表先处理*/ 
	},
};

/*********************end hook****************************/

static void vsf_fini(void)
{
	nf_unregister_hooks(vsf_ops, ARRAY_SIZE(vsf_ops));
	printk("huayi_hook exit\n");
	return ;
}

static int __init vsf_init(void)
{
	nf_register_hooks(vsf_ops, ARRAY_SIZE(vsf_ops));
	printk("huayi_hook init\n");
	return 0;
}


module_init(vsf_init);
module_exit(vsf_fini);

