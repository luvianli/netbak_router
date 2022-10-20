/* MD5.H - header file for MD5C.C
 *
 * $Id: md5.h,v 1.3 2007-02-01 07:41:01 winfred Exp $
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>


#ifndef _h_MD5
#define _h_MD5 1


/* typedef a 32 bit type */
typedef unsigned long int UINT4;
/* Data structure for MD5 (Message Digest) computation */
typedef struct {
	UINT4 i[2];                   /* number of _bits_ handled mod 2^64 */
	UINT4 buf[4];                                    /* scratch buffer */
	unsigned char in[64];                              /* input buffer */
	unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

void MD5Init ();
void MD5Update ();
void MD5Final ();
char *md5str(char *srcstr, int srclen,int md5_len);
char *MD5_file (char *path, int md5_len);

int get_file_size(char *filename);

unsigned int crc32( const unsigned char *buf, unsigned int size);

#endif /* _h_MD5 */
