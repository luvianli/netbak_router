

#ifndef BM_ONLINE_IP_H
#define BM_ONLINE_IP_H

#include <linux/types.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/percpu.h>
#include <linux/netdevice.h>
#ifdef CONFIG_SYSCTL
#include <linux/sysctl.h>
#endif
#include <net/ip.h>
#include <linux/version.h>
#include <net/pkt_sched.h>
#include <linux/in.h>
#include <linux/version.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <net/netfilter/pa_qos.h>
#include <linux/seq_file.h>



/* Include files. */


/* Macro constant definitions. */

/* Type definitions. */
#define NETLINK_IP_LIMIT		23	
#define SET_TOTAL_DATA_LIMIT 1
#define SET_SINGLE_IP_FORBID 2
#define SET_BLOCKUSER_SWITCH 3
#define SET_BLOCKUSER_MAC 4
#define DEL_BLOCKUSER_MAC 5
#define CLEAR_BLOCKUSER 6
#define PRINT_BLOCKUSER 7
#define GET_TRAFFIC 8
#define CLEAR_ALL_IP_FORBID 9
#define SET_USER_DATA_LIMIT 10


#define AUTH_USER_SZ 64
#define MAC_LEN			6


enum DIR_DEF{
	DIR_UP = 0,   
	DIR_DOWN,	
	DIR_CNT
};

enum BLOCK_USER_TYPE{
    BLOCK_USER_OFF = 0,
    BLOCK_USER_BLACK,
    BLOCK_USER_WHITE
};

enum {
	ONLINE_IP_NO_PACKTE = 0,
	ONLINE_IP_PACKTE,
};

typedef enum {
	eRET_MIN = 0,
	eRET_SUCCESS = eRET_MIN,
	eRET_FAILURE,
	eRET_INVALID_ARG,
	eRET_INVALID_STATE,
	eRET_NO_RESOURCE,
	eRET_ALREADY_EXIST,
	eRET_NOT_EXIST,
	eRET_TIMEOUT,
	eRET_MAX,
	/* To use same size of unsigned int */
	eRET_PADDING = ((unsigned int)~0),
} e_ret;


/*
	每个IP的创建时需要赋值数据结构
*/
typedef struct online_ip_data_s{	
	u_int32_t			ip;				/*Ip地址,网络序*/
	u_int32_t 		max_conn;		/*Ip的最大连接数*/
	u_int64_t 		create_jiffies;	/*Ip的创建时间*/
	u_int32_t 		ttl;				/*Ip的生存时间,单位秒*/
	u_int8_t			mac[ETH_ALEN];	/*每个新连接都进行mac地址比较,整数比较,mac地址后4位*/
	u_int8_t			real_dev;
}online_ip_data_t;

/*
	每个IP的状态结构
*/
typedef struct online_ip_status_s{
	struct list_head			session_list;			/*用户的所有连接,保存连接跟踪的指针*/
	rwlock_t 					lock;				/*访问session_list 锁*/
	u_int32_t					active;				/*在一个定时器周期内,是否有报文*/
	u_int32_t					offline_num;
	u_int64_t  				last_packet_time;	/*用户最后一个报文的时间*/
	u_int64_t 				up_bytes;			/*报文累计上行字节*/
	u_int64_t 				last_up_bytes;		/*上一次汇报给应用层的报文累计下行字节*/
	u_int64_t 				down_bytes;			/*报文累计下行字节*/
	u_int64_t 				last_down_bytes;	/*上一次汇报给应用层的报文累计下行字节*/
	u_int64_t                             up_packets;               /*累计上行的报文数*/
	u_int64_t                             down_packets;          /*累计下行的报文数*/
	u_int64_t				up_rate;
	u_int64_t				down_rate;
	
}online_ip_status_t;

/*
	每个ip的规则结构
*/
typedef  struct online_ip_rule_s{	
	u_int8_t					forbid;				/*临时的行为管控*/
	u_int8_t					privilege;			/*是否为特权用户*/
}online_ip_rule_t;

typedef struct online_ip_qos_s{	
	u_int8_t				is_match;
	u_int8_t				priority ;
	u_int8_t				limit_type ;
	u_int8_t 				share_mode;    						
	u_int8_t 				policy_mode;
	u_int32_t 				flg;	
	u_int32_t				sch_tokens; // 用于用户间公平调度
	u_int32_t				rate_tab[DIR_CNT][Q_RTAB_NUM];
	u_int32_t				tmp_rtab[DIR_CNT][Q_RTAB_NUM];
	long					toks[DIR_CNT];					
	u_int32_t				qlen;								
	psched_time_t 		last_time[DIR_CNT];				
	struct list_head  		global_user_node;					
	struct sk_buff_head 	qdisc[DIR_CNT][QDISC_MAX]; 		
}online_ip_qos_t;

/*
	每个IP的数据结构
*/
typedef struct online_ip_hash_node_s{
	struct list_head  				h_node;		/*hash表节点*/
	atomic_t						ref;			/*引用计算当前连接数*/
	//u_int32_t						ibms_finish;
	//u_int32_t						ibms_action;	
	online_ip_data_t				data; 		/*基础数据*/
	online_ip_status_t 				status; 		/*状态数据*/
	online_ip_qos_t				qos;
    //  online_ip_rule_t				rule;
	struct timer_list 				expires;		/*老化定时器*/
}online_ip_hash_node_t;

/*
	在线ip输出状态记录
*/
struct online_ip_iter_state {
	unsigned int bucket;
	void *head;
};


/*
	用户态下发的临时设置数据结构
*/
typedef struct temp_control_info_s{
	u_int32_t		              ip;                               /*Ip地址*/
	u_int8_t				forbid;                        /*是否启用禁止该ip访问网络*/
}temp_control_info_t;

typedef struct all_settings_control_s{
	u_int32_t access_data_switch;		/* 是否允许访问网络 0为可访问 1为禁止访问*/
	u_int32_t block_user_switch;		/* 黑白名单:0不开启 1白名单 2黑名单*/
	u_int32_t singleData_switch;		/* 是否开启单个IP数据流量限制 */
	u_int64_t totalTraffic;			/* 总流量大小 */
}total_control_switch;



#define get_for_mac_header(skb, p) \
do { \
		struct ethhdr *ts_hdr = eth_hdr(skb); \
		if (skb_mac_header(skb) == skb_network_header(skb)) { \
			unsigned char *t_data = NULL; \
			if (skb_mac_header(skb) - (skb)->head < 22) \
				(p) = NULL; \
			else { \
				t_data = skb_mac_header(skb) - 10; \
				if (ntohs(*((unsigned short *)t_data)) == ETH_P_PPP_SES  \
					|| (ntohs(*((unsigned short *)t_data)) == ETH_P_PPP_DISC)) { \
					ts_hdr = (struct ethhdr *)(t_data - 12);  \
					(p) = ts_hdr; \
					} else  \
						(p) = NULL; \
				} \
		} else \
			(p) = ts_hdr; \
}while(0)

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"


/* External function declarations. */

extern struct list_head *online_ip_hash;
extern rwlock_t online_ip_lock;
extern u_int32_t online_ip_count;
extern u_int32_t online_ip_hash_size;
extern rwlock_t qos_user_lock;

extern struct proc_dir_entry proc_root;

#define user_write_lock()  \
do {  \
	write_lock(&qos_user_lock);  \
} while (0)
#define user_write_unlock()  \
do {  \
	write_unlock(&qos_user_lock);  \
} while (0)

#define user_read_lock()  \
do {  \
	read_lock_bh(&qos_user_lock);  \
} while (0)
#define user_read_unlock()  \
do {  \
	read_unlock_bh(&qos_user_lock);  \
} while (0)


extern e_ret init_online_ip(struct net *net);
extern e_ret check_ip_addr(u_int32_t ipaddr);
extern online_ip_hash_node_t * find_online_ip(u_int32_t ipaddr);
extern online_ip_hash_node_t * add_online_ip(struct sk_buff *skb,u_int32_t ipaddr,unsigned int *index);

#endif
