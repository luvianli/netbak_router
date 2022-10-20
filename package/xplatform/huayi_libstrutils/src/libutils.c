/* MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
 *
 * $Id: md5c.c,v 1.3 2007-02-01 07:41:01 winfred Exp $
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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "libutils.h"
#include <errno.h>


static void Transform ();
static unsigned char PADDING[64] = {
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/* F, G and H are basic MD5 functions: selection, majority, parity */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z))) 
/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
{(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) \
{(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) \
{(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) \
{(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
}
void MD5Init (MD5_CTX *mdContext)
{
	mdContext->i[0] = mdContext->i[1] = (UINT4)0;
	/* Load magic initialization constants.
	 */
	mdContext->buf[0] = (UINT4)0x67452301;
	mdContext->buf[1] = (UINT4)0xefcdab89;
	mdContext->buf[2] = (UINT4)0x98badcfe;
	mdContext->buf[3] = (UINT4)0x10325476;
}
void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen)
{
	UINT4 in[16];
	int mdi;
	unsigned int i, ii;
	/* compute number of bytes mod 64 */
	mdi = (int)((mdContext->i[0] >> 3) & 0x3F);
	/* update number of bits */
	if ((mdContext->i[0] + ((UINT4)inLen << 3)) < mdContext->i[0])
		mdContext->i[1]++;
	mdContext->i[0] += ((UINT4)inLen << 3);
	mdContext->i[1] += ((UINT4)inLen >> 29);
	while (inLen--) {
		/* add new character to buffer, increment mdi */
		mdContext->in[mdi++] = *inBuf++;
		/* transform if necessary */
		if (mdi == 0x40) {
			for (i = 0, ii = 0; i < 16; i++, ii += 4)
				in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
					(((UINT4)mdContext->in[ii+2]) << 16) |
					(((UINT4)mdContext->in[ii+1]) << 8) |
					((UINT4)mdContext->in[ii]);
			Transform (mdContext->buf, in);
			mdi = 0;
		}
	}
}
void MD5Final (MD5_CTX *mdContext)
{
	UINT4 in[16];
	int mdi;
	unsigned int i, ii;
	unsigned int padLen;
	/* save number of bits */
	in[14] = mdContext->i[0];
	in[15] = mdContext->i[1];
	/* compute number of bytes mod 64 */
	mdi = (int)((mdContext->i[0] >> 3) & 0x3F);
	/* pad out to 56 mod 64 */
	padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
	MD5Update (mdContext, PADDING, padLen);
	/* append length in bits and transform */
	for (i = 0, ii = 0; i < 14; i++, ii += 4)
		in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
			(((UINT4)mdContext->in[ii+2]) << 16) |
			(((UINT4)mdContext->in[ii+1]) << 8) |
			((UINT4)mdContext->in[ii]);
	Transform (mdContext->buf, in);
	/* store buffer in digest */
	for (i = 0, ii = 0; i < 4; i++, ii += 4) {
		mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
		mdContext->digest[ii+1] =
			(unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
		mdContext->digest[ii+2] =
			(unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
		mdContext->digest[ii+3] =
			(unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
	}
}
/* Basic MD5 step. Transform buf based on in.
 */
static void Transform (UINT4 *buf, UINT4 *in)
{
	UINT4 a = buf[0], b = buf[1], c = buf[2], d = buf[3];
	/* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
	FF ( a, b, c, d, in[ 0], S11, 3614090360UL); /* 1 */
	FF ( d, a, b, c, in[ 1], S12, 3905402710UL); /* 2 */
	FF ( c, d, a, b, in[ 2], S13,  606105819UL); /* 3 */
	FF ( b, c, d, a, in[ 3], S14, 3250441966UL); /* 4 */
	FF ( a, b, c, d, in[ 4], S11, 4118548399UL); /* 5 */
	FF ( d, a, b, c, in[ 5], S12, 1200080426UL); /* 6 */
	FF ( c, d, a, b, in[ 6], S13, 2821735955UL); /* 7 */
	FF ( b, c, d, a, in[ 7], S14, 4249261313UL); /* 8 */
	FF ( a, b, c, d, in[ 8], S11, 1770035416UL); /* 9 */
	FF ( d, a, b, c, in[ 9], S12, 2336552879UL); /* 10 */
	FF ( c, d, a, b, in[10], S13, 4294925233UL); /* 11 */
	FF ( b, c, d, a, in[11], S14, 2304563134UL); /* 12 */
	FF ( a, b, c, d, in[12], S11, 1804603682UL); /* 13 */
	FF ( d, a, b, c, in[13], S12, 4254626195UL); /* 14 */
	FF ( c, d, a, b, in[14], S13, 2792965006UL); /* 15 */
	FF ( b, c, d, a, in[15], S14, 1236535329UL); /* 16 */
	/* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
	GG ( a, b, c, d, in[ 1], S21, 4129170786UL); /* 17 */
	GG ( d, a, b, c, in[ 6], S22, 3225465664UL); /* 18 */
	GG ( c, d, a, b, in[11], S23,  643717713UL); /* 19 */
	GG ( b, c, d, a, in[ 0], S24, 3921069994UL); /* 20 */
	GG ( a, b, c, d, in[ 5], S21, 3593408605UL); /* 21 */
	GG ( d, a, b, c, in[10], S22,   38016083UL); /* 22 */
	GG ( c, d, a, b, in[15], S23, 3634488961UL); /* 23 */
	GG ( b, c, d, a, in[ 4], S24, 3889429448UL); /* 24 */
	GG ( a, b, c, d, in[ 9], S21,  568446438UL); /* 25 */
	GG ( d, a, b, c, in[14], S22, 3275163606UL); /* 26 */
	GG ( c, d, a, b, in[ 3], S23, 4107603335UL); /* 27 */
	GG ( b, c, d, a, in[ 8], S24, 1163531501UL); /* 28 */
	GG ( a, b, c, d, in[13], S21, 2850285829UL); /* 29 */
	GG ( d, a, b, c, in[ 2], S22, 4243563512UL); /* 30 */
	GG ( c, d, a, b, in[ 7], S23, 1735328473UL); /* 31 */
	GG ( b, c, d, a, in[12], S24, 2368359562UL); /* 32 */
	/* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
	HH ( a, b, c, d, in[ 5], S31, 4294588738UL); /* 33 */
	HH ( d, a, b, c, in[ 8], S32, 2272392833UL); /* 34 */
	HH ( c, d, a, b, in[11], S33, 1839030562UL); /* 35 */
	HH ( b, c, d, a, in[14], S34, 4259657740UL); /* 36 */
	HH ( a, b, c, d, in[ 1], S31, 2763975236UL); /* 37 */
	HH ( d, a, b, c, in[ 4], S32, 1272893353UL); /* 38 */
	HH ( c, d, a, b, in[ 7], S33, 4139469664UL); /* 39 */
	HH ( b, c, d, a, in[10], S34, 3200236656UL); /* 40 */
	HH ( a, b, c, d, in[13], S31,  681279174UL); /* 41 */
	HH ( d, a, b, c, in[ 0], S32, 3936430074UL); /* 42 */
	HH ( c, d, a, b, in[ 3], S33, 3572445317UL); /* 43 */
	HH ( b, c, d, a, in[ 6], S34,   76029189UL); /* 44 */
	HH ( a, b, c, d, in[ 9], S31, 3654602809UL); /* 45 */
	HH ( d, a, b, c, in[12], S32, 3873151461UL); /* 46 */
	HH ( c, d, a, b, in[15], S33,  530742520UL); /* 47 */
	HH ( b, c, d, a, in[ 2], S34, 3299628645UL); /* 48 */
	/* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
	II ( a, b, c, d, in[ 0], S41, 4096336452UL); /* 49 */
	II ( d, a, b, c, in[ 7], S42, 1126891415UL); /* 50 */
	II ( c, d, a, b, in[14], S43, 2878612391UL); /* 51 */
	II ( b, c, d, a, in[ 5], S44, 4237533241UL); /* 52 */
	II ( a, b, c, d, in[12], S41, 1700485571UL); /* 53 */
	II ( d, a, b, c, in[ 3], S42, 2399980690UL); /* 54 */
	II ( c, d, a, b, in[10], S43, 4293915773UL); /* 55 */
	II ( b, c, d, a, in[ 1], S44, 2240044497UL); /* 56 */
	II ( a, b, c, d, in[ 8], S41, 1873313359UL); /* 57 */
	II ( d, a, b, c, in[15], S42, 4264355552UL); /* 58 */
	II ( c, d, a, b, in[ 6], S43, 2734768916UL); /* 59 */
	II ( b, c, d, a, in[13], S44, 1309151649UL); /* 60 */
	II ( a, b, c, d, in[ 4], S41, 4149444226UL); /* 61 */
	II ( d, a, b, c, in[11], S42, 3174756917UL); /* 62 */
	II ( c, d, a, b, in[ 2], S43,  718787259UL); /* 63 */
	II ( b, c, d, a, in[ 9], S44, 3951481745UL); /* 64 */
	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

char *MD5_file (char *path, int md5_len)
{
	FILE *fp = fopen (path, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	char *file_md5;
	int i;
	if (fp == NULL) {
		fprintf (stderr, "fopen %s failed\n", path);
		return NULL;
	}

	MD5Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, fp)) != 0)
	{
		MD5Update (&mdContext, data, bytes);
	}
	MD5Final(&mdContext);

	file_md5 = (char *)malloc((md5_len + 1) * sizeof(char));
	if(file_md5 == NULL)
	{
		fprintf(stderr, "malloc failed.\n");
		return NULL;
	}
	memset(file_md5, 0, (md5_len + 1));

	if(md5_len == 16)
	{
		for(i=4; i<12; i++)
		{
			sprintf(&file_md5[(i-4)*2], "%02x", mdContext.digest[i]);
		}
	}
	else if(md5_len == 32)
	{
		for(i=0; i<16; i++)
		{
			sprintf(&file_md5[i*2], "%02x", mdContext.digest[i]);
		}
	}
	else
	{
		fclose(fp);
		free(file_md5);
		return NULL;
	}
	fclose (fp);
	return file_md5;
}

int get_file_size(char *filename)
{
	int file_len = 0;
    FILE *fp = NULL;
	
    fp = fopen(filename,"r");
	if(NULL == fp)
	{
		printf("file open faild\n");
		return 0;
	}

	fseek(fp,0L,SEEK_END);
	
	file_len = ftell(fp) + 1;

	fclose(fp);
	
	return file_len;
}


char *md5str(char *srcstr, int srclen,int md5_len)
{
	MD5_CTX mdContext;
	char *str_md5;
	int i =0;

	MD5Init(&mdContext);
	MD5Update(&mdContext, srcstr, srclen); 
	MD5Final(&mdContext);

	str_md5 = (char *)malloc((md5_len + 1) * sizeof(char));
	if(str_md5 == NULL)
	{   
		fprintf(stderr, "malloc failed.\n");
		return NULL;
	}   
	memset(str_md5, 0, (md5_len + 1));

	if(md5_len == 16) 
	{   
		for(i=4; i<12; i++)
		{   
			sprintf(&str_md5[(i-4)*2], "%02x", mdContext.digest[i]);
		}
	}
	else if(md5_len == 32)
	{
		for(i=0; i<16; i++)
		{
			sprintf(&str_md5[i*2], "%02x", mdContext.digest[i]);
		}
	}
	else
	{
		free(str_md5);
		return NULL;
	}
	return str_md5;

}


/*CRC*/

static const unsigned int crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};
 
unsigned int crc32( const unsigned char *buf, unsigned int size)
{
     unsigned int i, crc;
     crc = 0xFFFFFFFF;
 
 
     for (i = 0; i < size; i++)
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
 
     return crc^0xFFFFFFFF;
} 


