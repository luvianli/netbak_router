#ifndef __HUAYI_MSGQ_MODID__
#define __HUAYI_MSGQ_MODID__

#include <linux/netlink.h>

typedef enum msgq_modid{
    ALT_MSG_REG = NLMSG_MIN_TYPE+1,
    ALT_MSG_UNREG,
    ALT_MSG_TEST,   
    /*add ur type as fellow*/
    ALT_MSG_EVENT,
}MSGQ_MODID;

typedef enum huayi_events_type {
    ALT_EVENTS_START=NLMSG_MIN_TYPE,
    ALT_EVENTS_REG,// 17
    ALT_EVENTS_UNREG,
    /*add ur type as fellow*/
    HUAYI_EVENTS_SYS,
    HUAYI_EVENTS_NOTE,
}HUAYI_EVENTS_TYPE;

#define ALT_NETLINK_MSG     27  /* */
#define ALT_NETLINK_EVTS     28  /* */
#define ALT_MAX_PAYLOAD     1024

#endif /* __HUAYI_MSGQ_MODID__ */
