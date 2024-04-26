#ifndef _HUAYI_CLOUD_CLIENT_HEADER_
#define _HUAYI_CLOUD_CLIENT_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/timerfd.h>
#include "huayi_uci_fun.h"
//#include "libutils.h"
#define SKEY "6113c78aaf33865387f1c70bdbba97ac"
#define clientId "1101"


#define  IMAGE_PATH_STOR   "/tmp/firmware.img"
/*
 *  Uboot image header format
 *  (ripped from mkimage.c/image.h)
 */
#define IH_MAGIC    0x27051956
#define IH_NMLEN	(32-4)		/* Image Name Length		*/
typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
#ifndef	IMG_WITH_CHIPID
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
#else
	uint8_t		ih_name[IH_NMLEN-1];	/* Image Name		*/
	uint8_t		ih_chipid;	/* Chip ID: 0x50 for RT3050, 0x52 for RT3052, etc. */
#endif
	uint32_t	ih_ksz;		/* Kernel Part Size		*/
} image_header_t;


#endif
