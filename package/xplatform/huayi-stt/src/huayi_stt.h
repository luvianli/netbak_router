/**
 * @file huayi_stt.h
 * 
 * HUAYI STT
 * 
 * HUAYI STT Module for HUAYI platform
 *  
 * 
 *  
 */

#ifndef HUAYI_STT_H
#define HUAYI_STT_H
#include <linux/mm.h>
#include <asm-generic/cputime.h>

#define STT_LOG(level, fmt, arg...) do { \
         } while(0)

#define K(x) ((x) << (PAGE_SHIFT - 10))
#define STT_MAX_NAME    32

#define STT_ABOUT            	"about"
#define STT_SYS_STATS        	"status_system"
#define STT_NET_STATS        	"status_network"

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
#define STT_MODEM_STATS        	"status_modem"
#endif

#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY) || defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
#define STT_BLEND_STATS        	"status_blend"
#endif


typedef struct stt_proc_entry {
    struct proc_dir_entry *proc;
    uint8_t    index;
    char       name[STT_MAX_NAME];
} STT_PROC_ENTRY;


typedef struct stt {
    struct proc_dir_entry *proc_dir_stats;
	STT_PROC_ENTRY about;
	STT_PROC_ENTRY sys_stats;
	STT_PROC_ENTRY net_stats;
	STT_PROC_ENTRY module_stats;
#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
	STT_PROC_ENTRY modem_stats;
#endif
#if defined(CONFIG_PACKAGE_HUAYI_WIRED_PRIORITY) || defined(CONFIG_PACKAGE_HUAYI_REP4G_PRIORITY)
	STT_PROC_ENTRY blend_stats;
#endif
} STT;



#define MEM_TYPE "DDR2"


#define QCA_MIB_ITEM_NUMBER	41
#define MIB_RX_BROAD 0
#define MIB_RX_PAUSE 1
#define MIB_BX_MULTI 2
#define MIB_RX_FCSERR 3
#define MIB_RX_ALIGNERR 4
#define MIB_RX_RUNT 5
#define MIB_RX_FRAGMENT 6
#define MIB_RX_64BYTE 7
#define MIB_RX_128BYTE 8
#define MIB_RX_256BYTE 9
#define MIB_RX_512BYTE 10
#define MIB_RX_1024BYTE 11
#define MIB_RX_1518BYTE 12
#define MIB_RX_MAXBYTE 13
#define MIB_RX_TOOLONG 14
#define MIB_RX_GOODBYTE 15

#define MIB_RX_BADBYTE 16
#define MIB_RX_OVERFLOW 17
#define MIB_FILTERED 18
#define MIB_TX_BROAD 19
#define MIB_TX_PAUSE 20
#define MIB_TX_MULTI 21
#define MIB_TX_UNDERRUN 22
#define MIB_TX_64BYTE 23
#define MIB_TX_128BYTE 24
#define MIB_TX_256BYTE 25
#define MIB_TX_512BYTE 26
#define MIB_TX_1024BYTE 27
#define MIB_TX_1518BYTE 28
#define MIB_TX_MAXBYTE 29
#define MIB_TX_OVERSIZE 30
#define MIB_TX_BYTE 31
#define MIB_TX_COLLISION 32
#define MIB_TX_ABORTCOL 33
#define MIB_TX_MULTICOL 34
#define MIB_TX_SINGLECOL 35
#define MIB_TX_EXCDEFER 36
#define MIB_TX_DEFER 37
#define MIB_TX_LATECOL 38
#define MIB_RX_UNICAST 39
#define MIB_TX_UNICAST 40

#define NOT_READY 0

#define MAC_PROC_STA 50

#endif /* HUAYI_STT_H */


