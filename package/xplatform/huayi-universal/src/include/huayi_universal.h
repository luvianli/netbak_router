#ifndef __HUAYI_UNIVERSAL_H__
#define __HUAYI_UNIVERSAL_H__

#include "huayi_proc.h"
#include "huayi_types.h"
#include "huayi_lock.h"


typedef enum HUAYI_STATUS{
    HUAYI_ERROR = -1,
    HUAYI_OK = 0
}HUAYI_STATUS;

#define HUAYI_BASE_NOISE_FLOOR -95

#define A_MAX(a,b) (((a) > (b)) ? (a) : (b))

#define A_MIN(a,b) (((a) > (b)) ? (b) : (a))

#define BZERO(s,l)     	memset(s,0,l)

#define BCOPY(s,d,l)	        memcpy(d,s,l)

#define MAC_ADDR_COPY(d, s) BCOPY(s,d,6)

#ifdef __KERNEL__
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif
#endif
#include <linux/types.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/hardirq.h>
#define HUAYI_MALLOC(size)         (in_interrupt()?kmalloc(size, GFP_ATOMIC):kmalloc(size, GFP_KERNEL))
#define HUAYI_ZMALLOC(size)        (in_interrupt() ? \
                                        kmalloc(size, GFP_ATOMIC|__GFP_ZERO) : \
                                        kmalloc(size, GFP_KERNEL|__GFP_ZERO))
#define HUAYI_FREE(ptr)            kfree(ptr)
#else
#include <stdlib.h>
#define HUAYI_MALLOC(size)          malloc(size)
#define HUAYI_FREE(ptr)             free(ptr)
#endif 


static inline unsigned int A_INET_ADDR(const char *str)
{ 
    unsigned char a,b,c,d; 
    char arr[4]; 
    sscanf(str,"%hhu.%hhu.%hhu.%hhu",&a,&b,&c,&d);
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d; 
    return *(unsigned int*)arr; 
} 
#define NIPQUAD_FMT "%u.%u.%u.%u"
#define NIPQUAD(addr) ((unsigned char *)&addr)[0],((unsigned char *)&addr)[1],((unsigned char *)&addr)[2],((unsigned char *)&addr)[3]

#define MAC_FMT "%02X:%02X:%02X:%02X:%02X:%02X"

#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2],((u8*)(x))[3],((u8*)(x))[4],((u8*)(x))[5]
  



#endif /* __HUAYI_UNIVERSAL_H__ */
