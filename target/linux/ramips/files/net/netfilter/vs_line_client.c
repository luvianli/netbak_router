#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/tcp.h>
#include <net/ip.h>
#include <net/sock.h>
#include <net/netlink.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("line list");
/* Include files. */
#include <net/netfilter/vs_line_client.h>
/* Type definitions. */


/* Global variable declarations. */
static struct kmem_cache *online_ip_cache __read_mostly;

/*
	IP ³¬Ê±Ä¬ÈÏ60  Ãë
*/
static u_int32_t	ttl = 60;							

u_int32_t	online_ip_hash_size = 299;		
EXPORT_SYMBOL_GPL(online_ip_hash_size);
struct list_head	*online_ip_hash;								
EXPORT_SYMBOL_GPL(online_ip_hash);
DEFINE_RWLOCK(online_ip_lock);			
EXPORT_SYMBOL_GPL(online_ip_lock);
unsigned int online_ip_count = 0;
EXPORT_SYMBOL_GPL(online_ip_count);

DEFINE_RWLOCK(qos_user_lock);
EXPORT_SYMBOL_GPL(qos_user_lock);
/*************************************/

static inline void online_ip_onoff_timer_on(online_ip_hash_node_t *node,
										    u_int32_t timeout)
{
	node->expires.expires = jiffies + timeout * HZ;
	add_timer(&(node->expires));
}

static inline void online_ip_onoff_timer_off(online_ip_hash_node_t *node)
{
	node->expires.expires = 0;
	del_timer(&(node->expires));
}

static inline void online_ip_onoff_timer_refresh(online_ip_hash_node_t *node, 
											     u_int32_t timeout)
{
	mod_timer(&(node->expires), jiffies + timeout * HZ);
}


e_ret check_ip_addr(u_int32_t ipaddr)
{
	int mask = -1;

	mask = ipaddr & 0x000000FF;
	
	if (!ipaddr) {
		return eRET_FAILURE;
	}
	else if ((mask >=0x000000E0) || (mask ==0)){
		//printk("in addr %u.%u.%u.%u\n",NIPQUAD(ipaddr));
		return eRET_FAILURE;
	}
	else
		return eRET_SUCCESS;
}
EXPORT_SYMBOL_GPL(check_ip_addr);


static inline unsigned int get_online_ip_hash(u_int32_t ipaddr)
{
	return ((ipaddr & 0xFFFF0000) % online_ip_hash_size);
}

online_ip_hash_node_t * find_online_ip(u_int32_t ipaddr)
{
	online_ip_hash_node_t *node;
	unsigned int index;

	index = get_online_ip_hash(ipaddr);
	list_for_each_entry(node, &(online_ip_hash[index]), h_node){
		if(node->data.ip == ipaddr){
			return node;
		}
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(find_online_ip);

static void online_ip_timeout(unsigned long ptr)
{
	online_ip_hash_node_t *node = NULL;

	write_lock_bh(&online_ip_lock);
	node = (online_ip_hash_node_t *)ptr;
     if(NULL != node)
     {
		if (node->status.active == ONLINE_IP_NO_PACKTE
		    && (atomic_read(&(node->ref)) == 0)
		    && list_empty(&node->status.session_list)
		){
			//del_timer( &(node->expires));
			if(!list_empty(&node->h_node)){
				list_del(&(node->h_node));
				online_ip_count--;
			}			
			user_write_lock();
			if (!list_empty(&node->qos.global_user_node)) {
				list_del_init(&node->qos.global_user_node);
			}
			user_write_unlock();
			kmem_cache_free(online_ip_cache, node);
			node = NULL;
			goto done;
		}
		node->status.active = ONLINE_IP_NO_PACKTE;
	     	
		online_ip_onoff_timer_refresh(node, ttl);
	}
	
done:
	write_unlock_bh(&online_ip_lock);
	return;
}

online_ip_hash_node_t * add_online_ip(struct sk_buff *skb,u_int32_t ipaddr,unsigned int *index)
{
	online_ip_hash_node_t *node;
	int i,j;
	struct ethhdr *hdr = NULL;
	
	get_for_mac_header(skb, hdr);
	if (NULL == hdr)
		return NULL;
	
	node = kmem_cache_zalloc(online_ip_cache, GFP_ATOMIC);
	if (node == NULL) {
		printk("add_online_ip: Can't alloc.\n");
		return NULL;
	}
	setup_timer(&(node->expires), online_ip_timeout, (unsigned long)(node));
	node->data.ip= ipaddr;
	
	node->data.real_dev = skb->real_dev_mark;
	printk("dev name:%s real:%d\n",skb->dev->name,node->data.real_dev);
	memcpy(node->data.mac, hdr->h_source, ETH_ALEN);
	node->data.create_jiffies = get_jiffies_64();
	INIT_LIST_HEAD(&(node->h_node));
	INIT_LIST_HEAD(&(node->status.session_list));
	rwlock_init(&node->status.lock);
	INIT_LIST_HEAD(&(node->qos.global_user_node));
	for (i = 0; i <DIR_CNT; i++){
		for (j = 0; j < QDISC_MAX; j++){
			skb_queue_head_init(&(node->qos.qdisc[i][j]));
		}
	}
	*index = get_online_ip_hash(ipaddr);
	online_ip_onoff_timer_refresh(node, ttl);
	return node;
}
EXPORT_SYMBOL_GPL(add_online_ip);

#if 1  /*show client info in huayi_stt*/
static void *online_ip_get_first(struct seq_file *seq)
{
	struct online_ip_iter_state *st = seq->private;
	
	for (st->bucket = 0;st->bucket < online_ip_hash_size;st->bucket++){
		 if(!list_empty(&(online_ip_hash[st->bucket]))){
		 	st->head = &(online_ip_hash[st->bucket]);
			return online_ip_hash[st->bucket].next;
		 }
	}
	return NULL;
}

static void *online_ip_get_next(struct seq_file *seq,
				     void *head)
{
	struct online_ip_iter_state *st = seq->private;
	struct list_head * node = (struct list_head *)head;

	node = node->next;
	if (node != st->head){
		return node;
	}
	else{
		st->bucket++;
		for (;st->bucket < online_ip_hash_size;st->bucket++) {
			if(!list_empty(&(online_ip_hash[st->bucket]))){
				st->head = &(online_ip_hash[st->bucket]);
				return  online_ip_hash[st->bucket].next;
			}
		}
		return NULL;
	}
}

static void *online_ip_get_idx(struct seq_file *seq, loff_t pos)
{
	void *head = online_ip_get_first(seq);

	if (head)
		while (pos && (head = online_ip_get_next(seq, head)))
				pos--;

	return pos ? NULL : head;
}

static void *online_ip_seq_start(struct seq_file *seq, loff_t *pos)
{
	read_lock_bh(&online_ip_lock);
	if (*pos == 0){
		return SEQ_START_TOKEN;
	}
	
	return online_ip_get_idx(seq, *pos - 1);
}

static void *online_ip_seq_next(struct seq_file *s, void *v, loff_t *pos)
{	
	(*pos)++;
	if (v == SEQ_START_TOKEN)
		return online_ip_get_idx(s, 0);

	return online_ip_get_next(s, v);
}

static void online_ip_seq_stop(struct seq_file *s, void *v)
{
	read_unlock_bh(&online_ip_lock);
}



static int online_ip_seq_show(struct seq_file *s, void *v)
{
	online_ip_hash_node_t * node;
	char mac[32] = {0};
	char ip[32] = {0};
	char interfaces[8] = {0};
	u_int64_t tmp_jiffies;
	unsigned int ref ;
	
	if (v == SEQ_START_TOKEN) {
		/*seq_printf(s, "%d\n",online_ip_count);
		seq_printf(s, "%-15s %-17s %-10s %-12s %-5s %-12s %-11s %-12s %-12s %-12s %-12s\n",
					"ipaddr","macaddr","timeout","livetime(s)","connect","active","upBytes","downBytes","uprate(KB/s)","downrate(KB/s)","interface"
					);*/
		return 0;
	}
	node =(online_ip_hash_node_t *)v;
	
	sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",
		   node->data.mac[0],node->data.mac[1],node->data.mac[2],
		   node->data.mac[3],node->data.mac[4],node->data.mac[5]);

	sprintf(ip,"%u.%u.%u.%u",NIPQUAD(node->data.ip));
	tmp_jiffies = get_jiffies_64()  - node->data.create_jiffies;
	do_div(tmp_jiffies,HZ);
	ref =  atomic_read(&(node->ref));

	if(node->data.real_dev == WIRE_LAN_MARK) {
	#if defined(CONFIG_GE2_RGMII_AN) || defined(CONFIG_GE2_INTERNAL_GPHY)
		strcpy(interfaces,GMAC2_LAN_DEV_NAME);
	#else
		strcpy(interfaces,WIRE_LAN_DEV_NAME);
	#endif
	}
	else if(node->data.real_dev == WIRELESS_MARK) {
		strcpy(interfaces,WIRELESS_DEV_NAME);
	}
	else if(node->data.real_dev == WIRELESS_5G_MARK) {
		strcpy(interfaces,WIRELESS_5G_DEV_NAME);
	}
	else if(node->data.real_dev == WIRELESS_GUEST_MARK) {
		strcpy(interfaces,WIRELESS_GUEST_DEV_NAME);
	}
	else if(node->data.real_dev == WIRELESS_GUEST_5G_MARK) {
		strcpy(interfaces,WIRELESS_GUEST_5G_DEV_NAME);
	}
	seq_printf(s,"%s %s %u %llu %u %u %llu %llu %llu %llu %s\n",
			ip,
			mac,
			timer_pending(&node->expires) ? (unsigned int)(node->expires.expires - jiffies)/HZ : 0,
			tmp_jiffies,
			ref,
			node->status.active,
			node->status.up_bytes,
			node->status.down_bytes,
			node->status.up_rate,
			node->status.down_rate,
			interfaces);

	return 0;
}

static const struct seq_operations online_ip_seq_ops = {
	.start = online_ip_seq_start,
	.next  = online_ip_seq_next,
	.stop  = online_ip_seq_stop,
	.show  = online_ip_seq_show
};

static int online_ip_open(struct inode *inode, struct file *file)
{
	struct seq_file *seq;
	struct online_ip_iter_state *iter;
	int err;

	iter = kzalloc(sizeof(*iter), GFP_KERNEL);
	if (!iter)
		return -ENOMEM;

	err = seq_open(file, &online_ip_seq_ops);
	if (err) {
		kfree(iter);
		return err;
	}

	seq = file->private_data;
	seq->private = iter;
	return 0;
}

static const struct file_operations online_ip_fops = {
	.owner  	= THIS_MODULE,
	.open    	= online_ip_open,
	.read     	= seq_read,
	.llseek   	= seq_lseek,
	.release 	= seq_release_private,
};


#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,22)
static e_ret init_online_ip_procfs(void)
#else
static e_ret init_online_ip_procfs(struct net *net)
#endif
{
	struct proc_dir_entry *pde;

	pde = proc_create_data("client_list", 0, net->proc_net,
			 &online_ip_fops, NULL);
	if (!pde) {
		return eRET_FAILURE;
	}
	return eRET_SUCCESS;
}

#endif

 int update_rate_info(void) 
{
	online_ip_hash_node_t *node;
	u_int32_t i;
	u_int64_t up_tmp = 0UL,down_tmp = 0UL;

	write_lock_bh(&online_ip_lock);
	for (i = 0; i < online_ip_hash_size; i++) {
		if (!list_empty(&(online_ip_hash[i]))) {
			list_for_each_entry(node, &(online_ip_hash[i]), h_node) {
				
				up_tmp = (node->status.up_bytes - node->status.last_up_bytes);//*1000000;
				down_tmp = (node->status.down_bytes - node->status.last_down_bytes);//*1000000;
				node->status.last_up_bytes =  node->status.up_bytes;
				node->status.last_down_bytes = node->status.down_bytes;
				
				do_div( up_tmp,3 );
				do_div( down_tmp,3);
				node->status.up_rate = up_tmp;
				node->status.down_rate = down_tmp;
			}
		}
	}
	write_unlock_bh(&online_ip_lock);
	
	return 1;
}

EXPORT_SYMBOL_GPL(update_rate_info);

e_ret init_online_ip(struct net *net)
{
	unsigned int i;
	int ret = eRET_SUCCESS;
	online_ip_cache = kmem_cache_create("online_ip_cache",
						sizeof(online_ip_hash_node_t),
						0, 0, NULL);
	if (!online_ip_cache) {
		ret = eRET_FAILURE;
		return ret;
	}
	online_ip_hash = kzalloc(sizeof(struct list_head) * online_ip_hash_size,GFP_KERNEL);
	if(NULL == online_ip_hash){
		kmem_cache_destroy(online_ip_cache);
		ret = eRET_FAILURE;
		return ret;
	}
	for(i = 0; i < online_ip_hash_size; i ++){
		INIT_LIST_HEAD(&(online_ip_hash[i]));
	}
#if 1 /*show client info in huayi_stt*/
	init_online_ip_procfs(net);
#endif
	return ret;
}
EXPORT_SYMBOL_GPL(init_online_ip);


static int __init onlineip_init(void)
{
	printk("onlineinit\n");
	return 0;
}

static void __exit onlineip_fini(void)
{
	printk("onlinefini\n");
}

module_init(onlineip_init);
module_exit(onlineip_fini);

