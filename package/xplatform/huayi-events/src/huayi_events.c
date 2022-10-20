/*=========================================================================*
 * FileName:        
 * Author:          Kyoko  Version: Ver 1.0  Date: 2011-10-6
 * Description:     
 * Version:         
 * Function List:   
 *                  1.
 * Modified History:         
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * $Author:         
 * $Time:           
 * $Version:        
 * $Desc��          
 *                  
 *=========================================================================*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <asm/atomic.h>
#include <net/netlink.h>

#include <huayi-universal/huayi_lock.h>
 
//#include <huayi-universal/huayi_log.h>
#include <huayi-universal/huayi_events.h>
#ifdef _HUAYI_EVENTS_DBG_
#include <huayi-universal/huayi_alarm.h>
#endif /* _HUAYI_EVENTS_DBG_ */
#include <huayi-universal/huayi_universal.h>

/* A_*** redefine in atheros wifi driver */


#define A_SPINLOCK_INIT     HUAYI_SPINLOCK_INIT
#define A_SPINLOCK_LOCK     HUAYI_SPINLOCK_LOCK
#define A_SPINLOCK_UNLOCK   HUAYI_SPINLOCK_UNLOCK


#define eventslog_l(level, fmt, arg...) do { \
;}while (0)


#define ASSERT_EID_FUNC(eid,pid,func) do { if (((eid >= EVENTS_SUM) || (eid<= ALT_EVENTS_UNREG-NLMSG_MIN_TYPE-1)) && (!pid || !func)) { \
    eventslog_l("recv msg from[PID=%u] eid=%d func=0x%p\n",pid,eid,func); \
    return EVENTS_FAIL; } \
    } while(0)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("HUAYI Events System");
MODULE_AUTHOR("gerry_li");

static int V = 0; /* 0: donot print debug info(1. send to eID,len 2. eID, PID==>PID) 
                   * otherwise: print those two debug info
                   */
module_param(V, int, S_IRUGO);

static struct sock* huayi_events_sk = 0;

struct netlink_kernel_cfg cfg;

static HUAYI_DEFINE_MUTEX(events_mutex); /* serialization of message processing */
static HUAYI_DEFINE_MUTEX(events_head_mutex); /* serialization of message processing */
static a_spinlock_t huayi_events_spinlockcurr;
static a_spinlock_t huayi_events_spinlockfree;
struct workqueue_struct *huayi_events_workqueue = NULL;
static struct work_struct huayi_events_work;

static int huayi_emit_msg(events_func_t func, HUAYI_EVENTS_TYPE eid, char*buf, uint32_t len);

typedef struct huayi_events_node {
    struct huayi_events_node* next;
    union {
        uint32_t pid;
        events_func_t func;
    }handle_u;
    uint32_t err;
#ifdef _HUAYI_EVENTS_DBG_
    uint32_t succ;
#endif /* _HUAYI_EVENTS_DBG_ */
    unsigned long timestamp;
}HUAYI_EVENTS_NODE;

#ifdef PRODUCT_C1N
#define EVENTS_SUM      64
#define EVENTS_MOD_SUM  64
#else
#define EVENTS_SUM      256
#define EVENTS_MOD_SUM  256
#endif /* PRODUCT_C1N */
#define EVENTS_MAX_ADDR (-1UL)

HUAYI_EVENTS_NODE* huayi_events_g[EVENTS_SUM] = {0};//all the head of the events type

static int huayi_events_add_node(HUAYI_EVENTS_TYPE eid, uint32_t pid, events_func_t func)
{
    struct huayi_events_node* pnode_new;
    struct huayi_events_node* pnode;
    uint16_t count = 1;
    
    ASSERT_EID_FUNC(eid,pid,func);
         
    pnode_new = HUAYI_ZMALLOC(sizeof(struct huayi_events_node)); 
    if (NULL == pnode_new) {
        eventslog_l(HUAYILOG_ERR,"can't alloc memory\n");
        return EVENTS_FAIL;
    }
    (pid > 0) ? (pnode_new->handle_u.pid = pid) :
        (pnode_new->handle_u.func = func);
    
    HUAYI_MUTEX_LOCK(&events_head_mutex);
    pnode = huayi_events_g[eid];
    while (pnode) {
#if 0   //no need check the same node
        if ((pnode->handle_u.pid == pid)//every node must be not zero
            || (pnode->handle_u.func == func)) {//if the pid already registered,do nothing
            HUAYI_MUTEX_UNLOCK(&events_head_mutex);
            HUAYI_FREE(pnode_new);
            return EVENTS_SUCC;
        }
#endif /* 0 */
        //calculate the node counter
        if(count++ > EVENTS_MOD_SUM) {
            HUAYI_MUTEX_UNLOCK(&events_head_mutex);
            HUAYI_FREE(pnode_new);
            eventslog_l(HUAYILOG_ERR,"EID[%d] registered reach the MAX[%d],PID[%u]\n",eid,EVENTS_MOD_SUM,pid);
            return EVENTS_FAIL;
        }
        pnode = pnode->next;
    }
    pnode = huayi_events_g[eid];
    huayi_events_g[eid] = pnode_new;//add it at first 
    pnode_new->next = pnode;
    
#ifdef _HUAYI_EVENTS_DBG_
    printk("%s[%d] add node eid =%d pid=%u func=0x%p success\n", __func__,__LINE__,eid, pnode_new->handle_u.pid,pnode_new->handle_u.func);
#endif /* _HUAYI_EVENTS_DBG_ */
    HUAYI_MUTEX_UNLOCK(&events_head_mutex);

    return EVENTS_SUCC;
}
static int huayi_events_del_node(HUAYI_EVENTS_TYPE eid, uint32_t pid, events_func_t func)
{    
    struct huayi_events_node** pnodepre;
    struct huayi_events_node* pnodecur;
    
    ASSERT_EID_FUNC(eid,pid,func);

#ifdef _HUAYI_EVENTS_DBG_
    printk("%s[%d] del node pid=%u func=0x%p\n", __func__,__LINE__,pid,func);
#endif /* _HUAYI_EVENTS_DBG_ */
    HUAYI_MUTEX_LOCK(&events_head_mutex);
    pnodecur = huayi_events_g[eid];
    pnodepre = &huayi_events_g[eid];
    while(pnodecur) {
        if ((pnodecur->handle_u.pid == pid)
            || (func == pnodecur->handle_u.func)) {
            *pnodepre = pnodecur->next;
            HUAYI_FREE(pnodecur);
#ifdef _HUAYI_EVENTS_DBG_
    printk("%s[%d] del node pid=%u func=0x%p success\n", __func__,__LINE__,pid,func);
#endif /* _HUAYI_EVENTS_DBG_ */
            break;
        }
        pnodepre = &pnodecur->next;
        pnodecur = pnodecur->next;
    }
    HUAYI_MUTEX_UNLOCK(&events_head_mutex);

    return EVENTS_SUCC;
}
static void huayi_events_del_all(void)
{
    struct huayi_events_node** pnodepre;
    struct huayi_events_node* pnodecur;
    HUAYI_EVENTS_TYPE eid;
    
    HUAYI_MUTEX_LOCK(&events_head_mutex);
    for (eid=0; eid<EVENTS_SUM; ++eid) {
        pnodecur = huayi_events_g[eid];
        pnodepre = &huayi_events_g[eid];
        while(pnodecur) {
            /*if ((pnodecur->handle_u.pid == pid)
                || (func == pnodecur->handle_u.func)) */{
                *pnodepre = pnodecur->next;
                HUAYI_FREE(pnodecur);
                //break;
            }
            pnodecur = *pnodepre;// = &pnodecur->next;
            //pnodecur = pnodecur->next;
        }
    }
    HUAYI_MUTEX_UNLOCK(&events_head_mutex);
}
static struct sk_buff*  huayi_events_make_reply_skb(int pid, int seq, int type, int done,
				 int multi, void *payload, uint32_t size) 
{
    struct sk_buff  *skb;
    struct nlmsghdr *nlh;
    //int     len = NLMSG_SPACE(size);
    void        *data;
    int     flags = multi ? NLM_F_MULTI : 0;
    int     t     = done  ? NLMSG_DONE  : type;

    skb = nlmsg_new(size, GFP_KERNEL);//alloc_skb(len, GFP_KERNEL);
    if (!skb)
        return NULL;

    nlh      = __nlmsg_put(skb, pid, seq, t, size, 0);
    nlh->nlmsg_flags = flags;
    data         = nlmsg_data(nlh);//NLMSG_DATA(nlh);
    memcpy(data, payload, size);

#ifdef _HUAYI_EVENTS_DBG_
    {
        char* pbuf = (char*)nlmsg_data(nlh);
        printk("[%d]: %s type=0x%x\n", __LINE__, pbuf, nlh->nlmsg_type);
    }
#endif /* _HUAYI_EVENTS_DBG_ */
    return skb;
}

static int huayi_events_notifychains(HUAYI_EVENTS_TYPE eid, uint32_t pid,
    events_func_t func, char*buf, uint32_t len) 
{
    struct huayi_events_node* pnode;
    struct huayi_events_node** ppre = NULL;
    struct sk_buff *skb;
    int ret = 0;
    
    if (((eid >= EVENTS_SUM) || (eid<= ALT_EVENTS_UNREG-NLMSG_MIN_TYPE-1))) { 
        eventslog_l(HUAYILOG_ERR,"recv msg from[PID=%u] eid=%d\n",pid,eid);
        return EVENTS_FAIL; 
    } 
    
#if	0
    printk("%s[%d] send message eid=%u len=%u\n", __func__,__LINE__,eid,len);
#endif /* _HUAYI_EVENTS_DBG_ */
    HUAYI_MUTEX_LOCK(&events_head_mutex);
    pnode = huayi_events_g[eid];
    ppre = &huayi_events_g[eid];
    while (pnode) {
#if	0
    printk("%s[%d] find node pid=%u:%u func=0x%p:0x%p success\n", __func__,__LINE__,
        pnode->handle_u.pid,pid,pnode->handle_u.func,func);
#endif 
        if ((pid == pnode->handle_u.pid) || (pnode->handle_u.func == func)) {
#if	0
            printk("%s[%d] find itself pid=%u:%u func=0x%p:0x%p \n", __func__,__LINE__,
                pnode->handle_u.pid,pnode->handle_u.func, pid, func);
#endif
            ppre = &pnode->next;
            pnode = pnode->next;
            continue;
        } else {
             if ((~pnode->handle_u.pid)//not every bit is 1 
                /*&& (pnode->handle_u.pid != pid)*/) {//send to user space,not send to itself
                skb = huayi_events_make_reply_skb(pnode->handle_u.pid, 0, 0, 1, 0,
                                 buf, len);
                if (skb!=NULL) {
                    int skblen = skb->len;
                    if (V)
                        eventslog_l(HUAYILOG_DEBUG,"eID=%u PID(%u)==>PID(%u)\n",eid+(ALT_EVENTS_REG), pid, pnode->handle_u.pid);

                    //msleep(500);
                    ret = netlink_unicast(huayi_events_sk, skb, pnode->handle_u.pid, /*0*/1);
                    if (ret != skblen) {
                        if (!pnode->timestamp) pnode->timestamp = jiffies; // first fail
                        pnode->err++;
                        eventslog_l(HUAYILOG_ERR,"Error(%d!=%u):eID=%u PID(%u)==>PID(%u) error times:%u !!!\n",
                            ret, skblen, eid+(ALT_EVENTS_REG), pid,pnode->handle_u.pid, pnode->err);

                        /* del the node if err>10 times in 10 mins,we assert this node is dead */
                        if ((pnode->err>=10) /*&& jiffies_to_msecs(jiffies-pnode->timestamp)/600000*/) {
                            eventslog_l(HUAYILOG_WARNING,"delete node:eID=%u PID(%u) whit errs(%u) in 10 mins!!!\n",
                                eid+(ALT_EVENTS_REG),pnode->handle_u.pid, pnode->err);
                            /* del current node directly */
                            *ppre = pnode->next;
                            HUAYI_FREE(pnode);
                            pnode = *ppre;
                            //ppre = &pnode->next;
                            continue;
                        }
                    } else {
#ifdef _HUAYI_EVENTS_DBG_
                        pnode->succ++;
#endif /* _HUAYI_EVENTS_DBG_ */
                        if (pnode->err){/* send success,clear the error counter and timestap */
                            pnode->err = 0;
                            pnode->timestamp = 0;
                        }
                    }
                    //kfree_skb(skb);
                }
            } else  /*if ((NULL != pnode->handle_u.func) 
               && (pnode->handle_u.func != func)) */{
                ret = pnode->handle_u.func(buf, len);//call the kernel callback routine
            } /*else {
                printk(KERN_ERR"unknow node for events pid=%u:%u func=0x%p:0x%p\n",
                    pnode->handle_u.pid,pnode->handle_u.func, pid, func);
            }*/
        }
        ppre = &pnode->next;
        pnode = pnode->next;
    }
    HUAYI_MUTEX_UNLOCK(&events_head_mutex);

    return ret;
}


static void huayi_events_receive_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
    uint32_t    pid;
    uint32_t    size,i;
    void*       pmsg;
    HUAYI_EVENTS_TYPE  eid;
    HUAYI_EVENTS_TYPE* peid;

	if(skb->len >= nlmsg_total_size(0))
    {
        nlh = nlmsg_hdr(skb);
		pmsg = NLMSG_DATA(nlh);
		pid  = NETLINK_CREDS(skb)->pid;
		size = nlh->nlmsg_len;
		eid = nlh->nlmsg_type;
    }else{
		return;
	}
	
    switch (eid) {
    case ALT_EVENTS_REG:
        peid = (HUAYI_EVENTS_TYPE*)pmsg;
        size /= sizeof(HUAYI_EVENTS_TYPE);
        //eid = ntohl(eid)
        for (i=0; i<size; ++i,++peid) {
            eventslog_l(HUAYILOG_NOTICE,"recv msg from[PID=%d] eventID=%u size=%u(reg)\n",NETLINK_CB(skb).creds.pid,*peid, size);
            huayi_events_add_node(*peid-(ALT_EVENTS_REG),NETLINK_CB(skb).creds.pid, NULL);
        }
        break;

    case ALT_EVENTS_UNREG:
        peid= (HUAYI_EVENTS_TYPE*)pmsg;
        size /= sizeof(HUAYI_EVENTS_TYPE);
        //eid = ntohl(eid)
        for (i=0; i<size; ++i,++peid) {
            eventslog_l(HUAYILOG_NOTICE,"recv msg from[PID=%d] eventID=%u size=%u(unreg)\n",NETLINK_CB(skb).pid,*peid, size);
            huayi_events_del_node(*peid-(ALT_EVENTS_REG),NETLINK_CB(skb).creds.pid, NULL);
        }
        break;
    default:
        eid -= (ALT_EVENTS_REG);
        //eid = ntohl(eid)
#ifdef _HUAYI_EVENTS_DBG_
        eventslog_l(HUAYILOG_NOTICE,"recv msg from eventID=%u LEN=%d\n",eid, nlmsg_len(nlh));
#endif /* _HUAYI_EVENTS_DBG_ */
        huayi_events_notifychains(eid, NETLINK_CB(skb).creds.pid, NULL, (char*)pmsg, size);
        break;
    }
    eventslog_l(HUAYILOG_DEBUG,"======= recv msg end from PID=%u eid=%d =======\n",pid,eid);
    return;//no need ack
}

/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     netlink receive routine
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           
 * Output:          
 * Return:          
 * Others:          
 *-------------------------------------------------------------------------*/
static void huayi_events_rcv(struct sk_buff *skb) {
#ifdef _HUAYI_EVENTS_DBG_
    printk("%s[%d] \n", __func__,__LINE__);
#endif /* _HUAYI_EVENTS_DBG_ */
    HUAYI_MUTEX_LOCK(&events_mutex);
//    msgq_netlinkrecv(skb, &msgq_receive_msg);
    netlink_rcv_skb(skb, &huayi_events_receive_msg);
    HUAYI_MUTEX_UNLOCK(&events_mutex);
#ifdef _HUAYI_EVENTS_DBG_
    printk("%s[%d] \n", __func__,__LINE__);
#endif /* _HUAYI_EVENTS_DBG_ */
}
int huayi_events_create(events_func_t func)
{
    int ret = (int)(EVENTS_MAX_ADDR-(unsigned long)func);

    return ret;
}
int huayi_events_destroy(int fd)
{
    return 0;
}


/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     for kernel space register a handle routine
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           eid:register events type; func:the callback routine
 * Output:          
 * Return:          
 * Others:          
 *-------------------------------------------------------------------------*/
int huayi_events_add(HUAYI_EVENTS_TYPE* peid,uint16_t neid, int fd)
{
    int ret;
    HUAYI_EVENTS_TYPE eid;
    HUAYI_EVENTS_TYPE* peid_new = peid;
    uint16_t i, isdel = 0;
    events_func_t func = (events_func_t)(EVENTS_MAX_ADDR-(unsigned long)(fd));

    if (!peid || !neid || !(func)) {
        eventslog_l(HUAYILOG_ERR, "Register event Error,wrong input para:0x%p %u 0x%p\n",
            peid, neid, func);
        ret = EVENTS_FAIL;
    }

    for (i=0; i<neid; ++i) {//register all the eid,if one eid register fail,then del the alreadly registered eid
        eid = *peid_new++ - (ALT_EVENTS_REG);
        ASSERT_EID_FUNC(eid,0,func);
        if (EVENTS_FAIL == huayi_events_add_node(eid, 0, func)) {
            isdel = 1;
            eventslog_l(HUAYILOG_ERR, "Register event Error,eid=%d neid=%u 0x%p\n",
                eid, neid, func);
            break;
        }
    }

    if (isdel) {// del the alreadly registered eid
        for (; peid<peid_new; ) {
            eid = *peid++ - (ALT_EVENTS_REG);
            huayi_events_del_node(eid, 0, func);
        }
        ret = EVENTS_FAIL;
    }

    //printk("%s[%d]: 0x%x 0x%p 0x%lx\n",__func__,__LINE__, ret, func,EVENTS_MAX_ADDR);    

    return ret;
}

/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     for kernel space unregister a handle routine
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           eid:unregister events type; func:the registered routine
 * Output:          
 * Return:          
 * Others:          
 *-------------------------------------------------------------------------*/
int huayi_events_del(HUAYI_EVENTS_TYPE* peid,uint16_t neid, int fd)
{
    uint16_t i;
    HUAYI_EVENTS_TYPE eid;
    HUAYI_EVENTS_TYPE* peid_new = peid;
    events_func_t func = (events_func_t)(EVENTS_MAX_ADDR-(unsigned long)(fd));

    
    //printk("%s[%d]: 0x%p 0x%lx fd=0x%x\n",__func__,__LINE__, func, EVENTS_MAX_ADDR,fd);    
    if (!peid || !neid || !(func)) {
        eventslog_l(HUAYILOG_ERR, "Register event Error,wrong input para:0x%p %u 0x%p\n",
            peid, neid, func);
        return EVENTS_FAIL;
    }

    for (i=0; i<neid; i++) {
        eid = *peid_new++ - (ALT_EVENTS_REG);
        ASSERT_EID_FUNC(eid,0,func);
        huayi_events_del_node(eid, 0, func);
    }

    return EVENTS_SUCC;
}

/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     for kernel space send a events
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           fd: the handle that returned by call huayi_events_register()
                    eid:events type; buf:events bufffer; len:buffer length 
 * Output:          
 * Return:          send lens
 * Others:          
 *-------------------------------------------------------------------------*/
int huayi_events_send(int fd, HUAYI_EVENTS_TYPE eid, char*buf, uint32_t len)
{
    int ret;
    events_func_t func = (events_func_t)(EVENTS_MAX_ADDR-(unsigned long)(fd));
    

    //printk("%s[%d]: 0x%p 0x%lx fd=0x%x\n",__func__,__LINE__, func, EVENTS_MAX_ADDR,fd);  
    if (V)  
        eventslog_l(HUAYILOG_DEBUG,"send to eID=%u len=%u\n",eid,len);
    eid -= (ALT_EVENTS_REG);
    if (((eid >= EVENTS_SUM) || (eid<= ALT_EVENTS_UNREG-NLMSG_MIN_TYPE-1))) { 
        eventslog_l(HUAYILOG_ERR,"recv msg from eid=%d\n",eid);
        return EVENTS_FAIL; 
    } 
    
    if ((NULL == buf) || (!len)) {
        eventslog_l(HUAYILOG_ERR,"can't send buf<0x%p> and len=%u to eid=%d\n", buf,len,eid);

        return EVENTS_FAIL;
    }
    ret = huayi_emit_msg(func, eid, buf, len);
    //huayi_events_notifychains(eid, 0, func, buf, len);

    return ret;
}


EXPORT_SYMBOL(huayi_events_create);
EXPORT_SYMBOL(huayi_events_destroy);
EXPORT_SYMBOL(huayi_events_add);
EXPORT_SYMBOL(huayi_events_del);
EXPORT_SYMBOL(huayi_events_send);

//#define HUAYI_USE_DYN_MEM

#define MAX_MSG_LEN     256
#define MAX_MSG_NUM     512

struct huayi_events_data {
    struct huayi_events_data* next;
    events_func_t func;
    HUAYI_EVENTS_TYPE eid;
    uint16_t len;
    uint8_t free;   //need free ?
    uint8_t reserve;
#ifdef HUAYI_USE_DYN_MEM
    char* buf;
#else
    char buf[MAX_MSG_LEN];
#endif /* HUAYI_USE_DYN_MEM */
};

#ifndef HUAYI_USE_DYN_MEM
struct huayi_events_data huayi_evts_data[MAX_MSG_NUM];
#endif /* HUAYI_USE_DYN_MEM */

struct huayi_events_data* pcurr = NULL; //piont to the begin structure which need to tx
struct huayi_events_data* pfree = NULL; //piont to the last available structure

static int huayi_emit_msg(events_func_t func, HUAYI_EVENTS_TYPE eid, char*buf, uint32_t len)
{
    uint32_t i;
    struct huayi_events_data* ptmp;

    len = (len<MAX_MSG_LEN) ? (len) : (MAX_MSG_LEN-1);
    i = len;
    
    A_SPINLOCK_LOCK(&huayi_events_spinlockfree);
    ptmp = pfree;
    if (pfree) pfree = pfree->next;
    A_SPINLOCK_UNLOCK(&huayi_events_spinlockfree);

    if ( !ptmp ) { /* dynamic allocate */
        ptmp = kmalloc(sizeof(struct huayi_events_data), GFP_ATOMIC);
        if (!ptmp) {
            eventslog_l(HUAYILOG_CRIT,"workqueue can't malloc mem\n");
            return EVENTS_FAIL;
        } 
        memset(ptmp, 0, sizeof(struct huayi_events_data));
#ifdef HUAYI_USE_DYN_MEM
        ptmp->buf = kmalloc(MAX_MSG_LEN, GFP_KERNEL);
        if (!ptmp->buf) {
            kfree(ptmp);
            eventslog_l(HUAYILOG_CRIT,"workqueue can't malloc mem\n");
            ptmp = NULL;
            return EVENTS_FAIL;
        }
#endif /* HUAYI_USE_DYN_MEM */
        ptmp->free = 1;
    }

    ptmp->func = func;
    ptmp->eid = eid;
    ptmp->len = (uint16_t)len;
    ptmp->buf[i] = 0;
    while (i--) {
        ptmp->buf[i] = buf[i];
    }
    
    A_SPINLOCK_LOCK(&huayi_events_spinlockcurr);
    ptmp->next = pcurr;
    pcurr = ptmp;
    A_SPINLOCK_UNLOCK(&huayi_events_spinlockcurr);
    
    queue_work(huayi_events_workqueue, &huayi_events_work);
    
    return len;
}
static int huayi_tx_msg_work(struct work_struct *work)
{
    struct huayi_events_data *phead,*pcurrent,*plast;

    spin_lock_bh(&huayi_events_spinlockcurr);
    phead = pcurr;
    pcurr = NULL;
    spin_unlock_bh(&huayi_events_spinlockcurr);
    
    pcurrent = phead;
    plast = phead = NULL;
    while  (pcurrent) {
        struct huayi_events_data *ptmp;

        /* send msg */
        huayi_events_notifychains(pcurrent->eid, 0, 
            pcurrent->func, pcurrent->buf, pcurrent->len);

        /* remove current node,add it in phead list or del it? */
        ptmp = pcurrent;
        pcurrent = pcurrent->next;
            
        if (ptmp->free) {
            kfree(ptmp);
        } else {
            ptmp->next = phead;
            phead = ptmp;
            if (!ptmp->next) plast = ptmp; // tail node
        }
    }
#ifdef HUAYI_USE_DYN_MEM 
    struct huayi_events_data **ppre;

    ppre = &phead;
    pcurrent = phead;
    while  (pcurrent) {
        struct huayi_events_data *ptmp;

        /* send msg */
        huayi_events_notifychains(pcurrent->eid, 0, 
            pcurrent->func, pcurrent->buf, pcurrent->len);

        ptmp = pcurrent;
        pcurrent = pcurrent->next;
            
        if (ptmp->free) { 
            /* pre node next/head pointer point to tmp node next */
            *ppre = ptmp->next;
            kfree(ptmp);
        } else { /* pre pointer point to current next's addr  */
            ppre = &ptmp->next;
        }
    }
    if (!phead) return EVENTS_SUCC;
    plast = container_of(ppre, struct huayi_events_data, next);
#if 0
    for (plast=pcurrent=phead; pcurrent; ) {
        int needmove = 1;
        if (pcurrent->free) {
            if (pcurrent == phead) {
                plast = phead = pcurrent->next;
                needmove = 0;
            } else {
                plast->next = pcurrent->next;
            }
            kfree(pcurrent->buf);
            kfree(pcurrent);
            pcurrent = plast;
        } 
        if (needmove) {
            plast = pcurrent;
            pcurrent = pcurrent->next;
        }

    }
    
    phead = kmalloc(sizeof(struct huayi_events_data), GFP_KERNEL);
    if (!phead) {
        eventslog_l(HUAYILOG_CRIT,"workqueue can't malloc mem\n");
    } else {
        memset(phead, 0, sizeof(struct huayi_events_data));
        phead->buf = kmalloc(MAX_MSG_LEN, GFP_KERNEL);
        if (!phead->buf) {
            kfree(phead);
            eventslog_l(HUAYILOG_CRIT,"workqueue can't malloc mem\n");
            phead = NULL;
        }
    }
    plast = phead;
#endif /* 0 */
#endif /* HUAYI_USE_DYN_MEM */
    if (!plast || !phead)
        eventslog_l(HUAYILOG_CRIT,"free buffer have soem problem\n");
    
    spin_lock_bh(&huayi_events_spinlockfree);
    if (plast)
        plast->next = pfree;
    if (phead)
        pfree = phead;
    spin_unlock_bh(&huayi_events_spinlockfree);

    return EVENTS_SUCC;
}

#ifdef _HUAYI_EVENTS_DBG_
int testkernelevents(char*buf, uint32_t len)
{
    printk("%s[%d]:rx[%u]:%s\n", __func__,__LINE__,len,buf);
    return EVENTS_SUCC;    
}
#endif /* _HUAYI_EVENTS_DBG_ */


static void netlink_cfg_init(struct netlink_kernel_cfg *cfg)
{	
	memset(cfg,0,sizeof(cfg));
	
	cfg->groups = 0;   /*unicast*/
	
	cfg->input = huayi_events_receive_msg;
	return;
}



/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     module init function
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           
 * Output:          
 * Return:          
 * Others:          
 *-------------------------------------------------------------------------*/
static int __init huayi_events_init(void) 
{
    int i;
	//netlink_set_nonroot(ALT_NETLINK_EVTS, NL_NONROOT_RECV);
	//huayi_events_sk = netlink_kernel_create(&init_net, ALT_NETLINK_EVTS, 0,
	//				   huayi_events_rcv, /*&events_mutex*/NULL, THIS_MODULE);
	netlink_cfg_init(&cfg);
	
	huayi_events_sk = netlink_kernel_create(&init_net, ALT_NETLINK_EVTS,&cfg);
    if ( !huayi_events_sk ) {
        printk("netlink fd create fail11!\n");
        return EVENTS_FAIL;
    }

	A_SPINLOCK_INIT(&huayi_events_spinlockcurr);
	A_SPINLOCK_INIT(&huayi_events_spinlockfree);
    /* ����workqueue */
    huayi_events_workqueue = create_singlethread_workqueue ("huayi_events");
    if ( !huayi_events_workqueue ) {
        printk("Create workqueu failed in events system\n");
        return EVENTS_FAIL;
    }
    //init 2 queues
    spin_lock_bh(&huayi_events_spinlockfree);
#ifdef HUAYI_USE_DYN_MEM
    for (i=0; i<MAX_MSG_NUM; ++i) {
        struct huayi_events_data* pcur; 
        
        pcur = kmalloc(sizeof(struct huayi_events_data), GFP_KERNEL);
        if (!pcur) {
            eventslog_l(HUAYILOG_CRIT,"workqueue can't malloc mem\n");
        } else {
            memset(pcur, 0, sizeof(struct huayi_events_data));
            pcur->buf = kmalloc(MAX_MSG_LEN, GFP_KERNEL);
            if (!pcur->buf) {
                kfree(pcur);
                eventslog_l(HUAYILOG_CRIT,"workqueue can't malloc mem\n");
                pcur = NULL;
                continue;
            }
            pcur->next = pfree;
            pfree = pcur;
        }
    }
#else
    for (i=1; i<MAX_MSG_NUM; ++i) {
        memset(&huayi_evts_data[i-1], 0, sizeof(struct huayi_events_data));
        huayi_evts_data[i-1].next = &huayi_evts_data[i];
    }
    memset(&huayi_evts_data[i-1], 0, sizeof(struct huayi_events_data));;
    pfree = huayi_evts_data;
#endif /* HUAYI_USE_DYN_MEM */
    spin_unlock_bh(&huayi_events_spinlockfree);
    pcurr = NULL;
    
    INIT_WORK(&huayi_events_work, huayi_tx_msg_work);
    
#ifdef _HUAYI_EVENTS_DBG_
    {
        HUAYI_EVENTS_TYPE eid[2] = {HUAYI_EVENTS_SYS,HUAYI_EVENTS_SYS2};
    	int func = huayi_events_register(eid, 2,testkernelevents);
        huayi_events_send(func, HUAYI_EVENTS_SYS, "abc", 4);
    }
#endif /* _HUAYI_EVENTS_DBG_ */
	printk("huayi events init\n");
    return EVENTS_SUCC;
}

/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           
 * Output:          
 * Return:          
 * Others:          
 *-------------------------------------------------------------------------*/
static void __exit huayi_events_exit(void)
{
    struct huayi_events_node** pnodepre;
    struct huayi_events_node* pnodecur;
    HUAYI_EVENTS_TYPE eid = 0;
	
#ifdef _HUAYI_EVENTS_DBG_
    huayi_events_unregister(HUAYI_EVENTS_SYS, testkernelevents);
#endif /* _HUAYI_EVENTS_DBG_ */
    huayi_events_del_all();
    netlink_kernel_release(huayi_events_sk);
    HUAYI_MUTEX_LOCK(&events_head_mutex);
    for (eid=0; eid<EVENTS_SUM; ++eid) {
        pnodecur = huayi_events_g[eid];
        pnodepre = &huayi_events_g[eid];
        while (pnodecur) {
            *pnodepre = pnodecur->next;
            HUAYI_FREE(pnodecur);
            pnodecur = *pnodepre;
        }
    }
    HUAYI_MUTEX_UNLOCK(&events_head_mutex);
    {
        struct huayi_events_data*phead;
        struct huayi_events_data*pcurrent;
        
        spin_lock_bh(&huayi_events_spinlockcurr);
        phead = pcurr;
        pcurr = NULL;
        spin_unlock_bh(&huayi_events_spinlockcurr);
        while (phead) {
            pcurrent = phead;
            phead = phead->next;
            if(pcurrent->free) {
#ifdef HUAYI_USE_DYN_MEM
                kfree(pcurrent->buf);
#endif /* HUAYI_USE_DYN_MEM */
                kfree(pcurrent);
            }
        }
        
        spin_lock_bh(&huayi_events_spinlockfree);
        phead = pfree;
        pfree = NULL;
        spin_unlock_bh(&huayi_events_spinlockfree);
        while (phead) {
            pcurrent = phead;
            phead = phead->next;
            if(pcurrent->free) {
#ifdef HUAYI_USE_DYN_MEM
                kfree(pcurrent->buf);
#endif /* HUAYI_USE_DYN_MEM */
                kfree(pcurrent);
            }
        }
    }
    destroy_workqueue (huayi_events_workqueue);
	printk("huayi events exit\n");
}

module_init(huayi_events_init);
module_exit(huayi_events_exit);

