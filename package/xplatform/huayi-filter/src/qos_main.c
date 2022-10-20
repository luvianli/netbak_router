
#include <asm/bug.h>
#include <net/netfilter/vs_line_client.h>
#include "include/qos_def.h"
#include "include/vsf_hook.h"
#include "include/vsf_common.h"
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <linux/netfilter_ipv4.h>
#include <linux/icmp.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("rex.tian");
MODULE_DESCRIPTION("qos");


static struct timer_list rate_t;

extern int update_rate_info(void);
extern int (*qos_rate_process)(struct sk_buff *skb);

#if defined(CONFIG_NF_CONNTRACK_MARK)
static inline void set_wan_id(int dir, struct sk_buff *skb, struct nf_conn *qos_nfct)
{
	int wan_id = 0;
	unsigned long wan_mark = qos_nfct->mark;
	
	if ( wan_mark >= 0x300 )//wan
	{
		/*skip interface: lo, lan.
		wan:   0x300
		wan2: 0x400
		wan3: 0x500

		array: -1
		*/
		qos_nfct->wan_id = ( (wan_mark&0xff00)/256 ) - 2 - 1;
	}
	else
		qos_nfct->wan_id = 0;
	
		return ;
	}
#endif

e_qos_deal_type qos_handle (struct sk_buff *skb)
{	
	struct nf_conn *qos_nfct ;
	struct iphdr *iph;
	struct udphdr *udph;
	struct tcphdr * tcph = NULL;
	e_qos_deal_type ret = 1;
	int dir = DIR_TEST_BIT(skb, UP_STREAM) ? DIR_UP : DIR_DOWN;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,22)
	skb->tx_iif = skb->dev->ifindex;
#else
	skb->skb_tx_iif = skb->dev->ifindex;
#endif

	if (skb->nfct == NULL){
		ret = SKB_PASS;
		goto qos_out;
	}
	qos_nfct = (struct nf_conn *)skb->nfct;
	if (qos_nfct->online_ip == NULL){
		ret = SKB_PASS;
		goto qos_out;
	}

	if (!mask_test_bit(qos_nfct->flow_status, FLOW_FORWARD)) {
			ret = SKB_PASS;
			goto qos_out;
	}

	
	if ((iph = ip_hdr(skb)) == NULL){
		ret =  SKB_PASS;
		goto traffic_count;
	}

traffic_count:
	  /* 流量统计 */
	 if (SKB_PASS == ret) {
		 if (dir == DIR_UP) {
			qos_nfct->online_ip->status.up_bytes += skb->len;
			qos_nfct->online_ip->status.up_packets++;
		} else {
			qos_nfct->online_ip->status.down_bytes += skb->len;
			qos_nfct->online_ip->status.down_packets++;
		}
	}
qos_out:
	//skb->skb_xmit_flag = DEAL_DONE;
	//skb->cb[QOS_SKB_STATE] = DEAL_DONE;
	return ret;
	
}

void curr_rate(void)
{
	update_rate_info();
	mod_timer(&rate_t,jiffies +  3*HZ);
}

static void init_rate_timer(void)
{
	init_timer(&rate_t);
	rate_t.expires = jiffies +  2*HZ;
	rate_t.function = curr_rate;
	add_timer(&rate_t);
}


static void  qos_fini(void)
{
	qos_rate_process = NULL;
	del_timer(&rate_t);
	
	return ;
}

static int __init  qos_init(void)
{
	qos_rate_process = qos_handle;
	init_rate_timer();
	
	return 0;
}


module_init(qos_init);
module_exit(qos_fini);

