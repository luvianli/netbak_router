#ifndef __HUAYI_NETLINK_HEADER__
#define __HUAYI_NETLINK_HEADER__

#include <linux/netlink.h>
#include "huayi_core_base.h"
#include "huayi_handle_events.h"


#define HUAYI_PORT_NETLINK_MAX_MSG	 	256
#define HUAYI_GPIO_NETLINK_MAX_MSG	 	256

struct huayi_port_netlink_rec
{
    struct nlmsghdr hdr;
   	char buff[HUAYI_PORT_NETLINK_MAX_MSG];
};

struct huayi_gpio_netlink_rec
{
    struct nlmsghdr hdr;
   	char buff[HUAYI_GPIO_NETLINK_MAX_MSG];
};

VTBool huayi_events_init(struct VcoreAgent *Agent);
void huayi_events_destory(struct VcoreAgent *Agent);
void huayi_events_receive(struct VcoreAgent *Agent);

VTBool huayi_port_event_init(struct VcoreAgent *Agent);
void huayi_port_event_receive(struct VcoreAgent *Agent);
VTBool huayi_port_event_destory(struct VcoreAgent *Agent);

VTBool huayi_gpio_event_init(struct VcoreAgent *Agent);
void huayi_gpio_event_receive(struct VcoreAgent *Agent);
VTBool huayi_gpio_event_destory(struct VcoreAgent *Agent);

#if defined(CONFIG_HUAYI_SUPPORT_GPIO_EVENT)
VTBool hook_gpio_init(void);
#endif


#endif

