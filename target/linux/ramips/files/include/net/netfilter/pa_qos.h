#ifndef PA_QOS_H
#define PA_QOS_H

#include <net/pkt_sched.h>

#define Q_RTAB_NUM					(25)
#define PPPOE_MAX_IP				(5)	
#define QDISC_MAX                   (5)	      
typedef enum {
	IP_DIR_UP = 0,
	IP_DIR_DOWN,
	IP_DIR_MAX
}ip_dir;

typedef struct u2k_qos_rule_s{					
	u_int8_t 			share_mode;    		
	u_int8_t 			policy_mode;     		
	u_int8_t			priority;	
	u_int8_t			id;
	u_int8_t 			flg;					
	u_int32_t 		startip;      			
	u_int32_t 		endip;      			
	u_int32_t 		up;      				
	u_int32_t 		down;      			
}u2k_qos_rule_t;


typedef struct k_qos_rule_s{
	struct list_head  		list_node;							
	u2k_qos_rule_t			rule;								
	atomic_t 				ref;								
	u_int32_t               is_match;                           
	long					toks[IP_DIR_MAX];					
	psched_time_t 		last_time[IP_DIR_MAX];				   
	u_int32_t				rate_tab[IP_DIR_MAX][Q_RTAB_NUM];	
	struct sk_buff_head 	qdisc[IP_DIR_MAX][QDISC_MAX]; 		
	struct rcu_head		rcu;
}k_qos_rule_t;


#endif 
