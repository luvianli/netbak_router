#ifndef _STD_C_INCLUDE__
#define _STD_C_INCLUDE__
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <endian.h>
#include <pthread.h>
#include <sys/types.h>
#include "global.h"
#include "mg_log.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#define min(x,y)                    (((x) < (y)) ? (x) : (y))
#define KERNEL_VERSION(j,n,p)       ((j)*1000000 + (n)*1000 + (p))

#if __BYTE_ORDER == __LITTLE_ENDIAN
#ifndef le32toh
static __inline uint16_t __bswap16(uint16_t __x)
{
        return __x<<8 | __x>>8;
}

#define le16toh(x) (uint16_t)(x)
#define le32toh(x) (uint32_t)(x)
#define be16toh(x) __bswap16(x)
#endif
#endif


extern uint32_t chartoint(char* date);
extern uint16_t chartoshort(char* date);
extern void inttochar(char* date,uint32_t iddr);
extern void shorttochar(char* str,uint16_t iddr);

extern const char *get_time();
extern int dloader_main(int usbfd, FILE *fw_fp, const char *modem_name, int *errRebootCnt);


#endif


