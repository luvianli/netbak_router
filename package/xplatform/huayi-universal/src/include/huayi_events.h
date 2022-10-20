#ifndef __HUAYI_EVENTS_H__
#define __HUAYI_EVENTS_H__

#ifdef __KERNEL__
#include <linux/socket.h>
#include <linux/netlink.h>
#else
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/net.h>
#define uint32_t u_int32_t
#endif /* __KERNEL__ */
#include <huayi-universal/huayi_modid.h>    

/* Comment by Kyoko at 2011-10-6 16:19 **
typedef struct huayi_events_head {
    HUAYI_EVENTS_TYPE eid;
    uint32_t    evt_len;    //include the head
}HUAYI_EVENTS_HEAD;

typedef enum huayi_events_nlktype{
    ALT_EVENTS_REG = NLMSG_MIN_TYPE+1,
    ALT_EVENTS_UNREG,
    ALT_EVENTS_OTHERS,
}HUAYI_EVENTS_NLKTYPE; **/




#ifdef __KERNEL__
typedef int (*events_func_t)(char*buf, uint32_t len);
int huayi_events_create(events_func_t func);
int huayi_events_destroy(int fd);
int huayi_events_add(HUAYI_EVENTS_TYPE* peid,uint16_t neid, int fd);
int huayi_events_del(HUAYI_EVENTS_TYPE* peid,uint16_t neid, int fd);
int huayi_events_send(int fd,HUAYI_EVENTS_TYPE eid, char*buf, uint32_t len);
#else
#if 0
typedef enum huayi_msg_key {
    HUAYI_MSG_KEY1 = 0x30,
    HUAYI_MSG_KEY2,
    
    HUAYI_MSG_KEYMAX,
}HUAYI_MSG_KEY;

#define BUF_SIZE 200
struct msgtype {
    long mtype;  
    char mtext[BUF_SIZE];
};
#endif /* 0 */
int huayi_events_create(void);
int huayi_events_destroy(int fd);
int huayi_events_add(HUAYI_EVENTS_TYPE* peid, unsigned short neid,int fd);
int huayi_events_del(HUAYI_EVENTS_TYPE* peid, unsigned short neid, int fd);
int huayi_events_send(int fd, HUAYI_EVENTS_TYPE eid, char*buf, uint32_t len);
int huayi_events_recv(int fd, int32_t secs, char* buf, uint32_t len);
#endif /* __KERNEL__ */

#define EVENTS_SUCC 0
#define EVENTS_FAIL -1

#endif /* __HUAYI_EVENTS_H__ */
