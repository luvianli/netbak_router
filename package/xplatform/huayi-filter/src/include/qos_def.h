#ifndef QOS_DEF_H
#define QOS_DEF_H
/* Include files. */
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <net/tcp.h>
//#include <net/pkt_sched.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/byteorder/generic.h>

#define WAN_CNT		1		/* wan口数量 */

typedef enum {
	SKB_DROP = -1,  /* skb丢弃 */
	SKB_ENQUEUE = 0,	/* skb入队成功 */
	SKB_PASS = 1		/* 不处理skb */
}e_qos_deal_type;

#endif
