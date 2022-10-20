

#ifndef PA_COMMON_H
#define PA_COMMON_H

/* Include files. */
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/version.h>
#include <linux/netfilter/nf_conntrack_common.h>

#define WAN_NUM					(1)
#define QOS_LEVEL_DEFAULT		100	 

/*
	¡À¡§??¡¤??¨°
*/
typedef enum  {
	PKT_DIR_DOWN = 0,
	PTK_DIR_UP,
	PTK_DIR_MAX,
}e_pkt_dir;
#if 0
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"

#endif
#define str_safe_cpy(dest, src, size) \
do { \
	strncpy((dest), (src), (size)); \
	dest[size-1] = '\0'; \
} while (0)



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
#endif

