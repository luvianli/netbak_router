#ifndef __FASTCGI_UPDOWNLOAD_CGI_H__
#define __FASTCGI_UPDOWNLOAD_CGI_H__

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fastcgi_common.h"
#include <ctype.h>


#ifdef	IMG_WITH_CHIPID

/*start of Modify for several type of product ID by zengxianzhu at 20110228*/
#if	defined(CONFIG_RALINK_RT5350_1T1R)
#define	IH_CHIPID	0x53
#elif	defined(CONFIG_RALINK_RT3352_2T2R)
#define	IH_CHIPID	0x33
#elif	defined(CONFIG_RALINK_RT3050_1T1R)
#define	IH_CHIPID	0x50
#elif defined(CONFIG_RALINK_RT3052_2T2R)
#define	IH_CHIPID	0x52
#elif defined(CONFIG_RALINK_MT7620)
#define	IH_CHIPID	0x76
#endif
/*end of Modify for several type of product ID by zengxianzhu at 20110228*/

#endif
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



#define UPLOAD_AP_NAME_FILE				"/tmp/upload_apname_file"

#define HUAYI_AP_NAME_FILE				"/tmp/huayi_apname_file"

#define UPLOAD_PORTAL_PICTURE			"/tmp/upload_portal_picture"

#define HUAYI_PORTAL_PICTURE			"/tmp/bg.png"


#define RFC_ERROR "RFC1867 error"

void do_download_supermode(char *super_file);
void do_download_syslog();
void do_backup_system_config();
int do_restore_system_config(int *lock_time);
int do_system_upgrade(int *lock_time);
int uploade_multipart_file(int * lock_time);
int do_disposition_config(int *lock_time);

#endif
