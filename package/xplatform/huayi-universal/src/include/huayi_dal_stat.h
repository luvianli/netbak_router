#ifndef __HUAYI_DAL_STAT_H__
#define __HUAYI_DAL_STAT_H__

#include "huayi_dal.h"
#include <stdbool.h>


#define IEEE80211_ADDR_LEN  6
#define CSTATUS_MAX_DNS_SERVER 10

struct dal_wan_stats
{
	u_int8_t   	device_id[DAL_MAX_DEVICE_ID];
    u_int8_t   	mac_address[IEEE80211_ADDR_LEN];
	u_int32_t  	ipv4_addr;
	u_int32_t   ipv4_mask;
	u_int32_t   gateway;
	u_int8_t	link;
	u_int8_t    wan_up;
	u_int64_t 	wan_uptime;
	u_int32_t   dns_server[CSTATUS_MAX_DNS_SERVER];
	u_int8_t 	desc[DAL_MAX_DEVICE_DESC];
    u_int32_t   if_mtu;
	u_int32_t 	rx_bps;
	u_int32_t 	tx_bps;
	u_int64_t	rx_packets;
	u_int64_t	tx_packets;
	u_int64_t	rx_bytes;
	u_int64_t	tx_bytes;
	u_int64_t	rx_errors;
	u_int64_t	tx_errors;
	u_int64_t	rx_dropped;
	u_int64_t	tx_dropped;
	u_int64_t	multicast;
};

struct dal_lan_stats
{
	u_int8_t   	device_id[DAL_MAX_DEVICE_ID];
    u_int8_t   	mac_address[IEEE80211_ADDR_LEN];
	u_int32_t  	ipv4_addr;
	u_int32_t   ipv4_mask;
	u_int32_t   gateway;
	u_int8_t 	desc[DAL_MAX_DEVICE_DESC];
    u_int32_t   if_mtu;
	u_int8_t    linkport;
};

typedef struct dal_net_stats
{
	
    struct dal_lan_stats lan_stats;
	struct dal_wan_stats wan_stats;
}Dal_device_stats;

#endif /* __HUAYI_DAL_STAT_H__ */

