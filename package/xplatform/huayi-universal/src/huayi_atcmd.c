#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>  

#include <sys/stat.h>   
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <dlfcn.h>

#include "huayi_atcmd.h"
#include "huayi_log.h"
#include "huayi_uci_fun.h"

#if defined(CONFIG_PACKAGE_HUAYI_DIAL4G)
int debug = 0;
#define yprintf(fmt, args...) do { \
	if (debug){ printf("(%s:%d)=> " fmt, __FUNCTION__, __LINE__, ## args); } \
	}while(0)

	
int lock_fd = -1;
char ATret[AT_CMD_MAX_LEN]={0};

static int atcm_send(const char *atCmd, int sendLen)
{
	int result = -1;
	struct timeval tv_delay;
	int sockfd = 0;
	struct sockaddr_un socket_addr;  
	fd_set rset;
	int trans_size;
	char recvBuf[AT_VALUE_BUFF_LEN]={0};
	int rel = 0;
	unsigned long ul = 1;

	tv_delay.tv_sec = AT_TIMEOUT_SEC;
	tv_delay.tv_usec = AT_TIMEOUT_USEC;
	
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);  
	if (sockfd < 0)
	{
		result = -1;
		return result;
	}
	
	ioctl(sockfd, FIONBIO, &ul);
	
	socket_addr.sun_family = AF_UNIX;  
	strcpy(socket_addr.sun_path, ATCM_UNIX_DOMAIN);

	if(connect(sockfd, (struct sockaddr *)&socket_addr, sizeof(struct sockaddr_un)) == -1)
	{
		result = -2;
		yprintf("[cmhost %s:%d] Client socket connection failed. (Errno : 0x%02x)\n", __FUNCTION__, __LINE__, result);
		goto out;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_delay, sizeof(tv_delay)) == -1)
	{
		result = -3;
		yprintf("[cmhost %s:%d] Client setsocket failed. (Errno : 0x%02x)\n", __FUNCTION__, __LINE__, result);
		goto out;
	}
	
	if(setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_delay, sizeof(tv_delay)) == -1)
	{
		result = -4;
		yprintf("[cmhost %s:%d] Client setsocket failed. (Errno : 0x%02x)\n", __FUNCTION__, __LINE__, result);
		goto out;
	}

	//printf("AT cmd is [%s]\n", atCmd);
	trans_size = write(sockfd, atCmd, sendLen);
	if (trans_size < sendLen)
	{
		yprintf("[cmhost %s:%d] write have not complete. (trand_len : %d ?= %d)\n", __FUNCTION__, __LINE__, trans_size, sendLen);
		result = 0xf3;
		goto out;
	}
//	result = 0;
//	goto out;
	
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);

	rel = select(sockfd + 1, &rset, NULL, NULL, &tv_delay) ;
	if (rel < 0)
	{
		usleep(10000);
		yprintf("[cmhost %s:%d] select error. (Errno : 0x%02x)\n", __FUNCTION__, __LINE__, errno);
		goto out;
	}
	else if(rel == 0)
	{
		usleep(10000);
		yprintf("[cmhost %s:%d] select timeout. (Errno : 0x%02x)\n", __FUNCTION__, __LINE__, 0xff);
		goto out;
	}

	if(FD_ISSET(sockfd, &rset))
	{
		trans_size=read(sockfd, recvBuf, 16);
		//printf("at recv=[%s], len=[%d]\n", recvBuf, trans_size);
		if ( trans_size > 0 && strstr(recvBuf, "ok") )
			result = 0;
		else 
			goto out;
	}

out:
	//printf("result=[%d]\n", result);
	close(sockfd);
	return result;
}


int lte_lock_file(const char * lock_file)
{
	if (lock_fd < 0)
	{
		lock_fd = open(lock_file,O_WRONLY|O_CREAT);
		if (lock_fd < 0)
		{
			yprintf("the file open fail./n");
			return -1;
		}
	}

	if (flock(lock_fd,LOCK_EX) == 0)
    {
    	yprintf("the file was locked./n");
    }
	else
    {
    	yprintf("the file was not locked./n");
    }
	
	return 0;
}

int lte_unlock_file(const char * lock_file)
{
	if (flock(lock_fd,LOCK_UN) == 0)
    {
    	yprintf("the file was unlocked./n");
    }
	
	return 0;
}

int lte_f_exists(const char *path)	// note: anything but a directory
{
	struct stat st;
	return (stat(path, &st) == 0) && (!S_ISDIR(st.st_mode));
}


int utf8_to_ucs2(const unsigned char *in, int ilen, unsigned char *out, int olen) {
   unsigned char *inoffset = in;
   unsigned char *inend = in + ilen;
   int ret = 0;
   while (inoffset < inend && ret + 2 <= olen) {
      unsigned short temp1, temp2;
      unsigned char * one = inoffset;
      unsigned short ucs2_code = 0;
      if (0x00 == (*one & 0x80)) {
         /* 1 byte UTF-8 Charater.*/
         ucs2_code = (unsigned short) *one;
         inoffset++;
         memcpy(out + ret, &ucs2_code, 2);
         ret += 2;
         continue;
      } else if (0xc0 == (*one & 0xe0) && 0x80 == (*(one + 1) & 0xc0)) {
         /* 2 bytes UTF-8 Charater.*/
         temp1 = (unsigned short) (*one & 0x1f);
         temp1 <<= 6;
         temp1 |= (unsigned short) (*(one + 1) & 0x3f);
         ucs2_code = temp1;
         inoffset += 2;
         memcpy(out + ret, &ucs2_code, 2);
         ret += 2;
         continue;
      } else if (0xe0 == (*one & 0xf0) && 0x80 == (*(one + 1) & 0xc0) && 0x80 == (*(one + 2) & 0xc0)) {
         /* 3bytes UTF-8 Charater.*/
         temp1 = (unsigned short) (*one & 0x0f);
         temp1 <<= 12;
         temp2 = (unsigned short) (*(one + 1) & 0x3F);
         temp2 <<= 6;
         temp1 = temp1 | temp2 | (unsigned short) (*(one + 2) & 0x3F);
         ucs2_code = temp1;
         inoffset += 3;
         memcpy(out + ret, &ucs2_code, 2);
         ret += 2;
         continue;
      } else {
         break;
      }
   }
   return ret;
}

int ucs2_to_utf8(const unsigned char *in, int ilen, unsigned char *out, int olen)
{
   int length = 0;
   if(!out) return length;
   char *start = NULL;
   char *pout = out;
   for(start = in; start != NULL && start < in+ilen-1; start +=2)
   {
      unsigned short ucs2_code = *(unsigned short *)start;
      if(0x0080 > ucs2_code)
      {
         /* 1 byte UTF-8 Character.*/
         if(length+1 > olen) return -1;

         *pout = (char)*start;
         length++;
         pout ++;
      }
      else if(0x0800 > ucs2_code)
      {
         /*2 bytes UTF-8 Character.*/
         if(length+2 > olen) return -1;
         *pout = ((char)(ucs2_code >> 6)) | 0xc0;
         *(pout+1) = ((char)(ucs2_code & 0x003F)) | 0x80;
         length += 2;
         pout += 2;
      }
      else
      {
         /* 3 bytes UTF-8 Character .*/
         if(length+3 > olen) return -1;

         *pout = ((char)(ucs2_code >> 12)) | 0xE0;
         *(pout+1) = ((char)((ucs2_code & 0x0FC0)>> 6)) | 0x80;
         *(pout+2) = ((char)(ucs2_code & 0x003F)) | 0x80;
         length += 3;
         pout += 3;
      }
   }

   return length;
}

int string2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int i = 0;
    for (i = 0; i < nSrcLength; i += 2)
    {
        if ((*pSrc >= '0') && (*pSrc <= '9'))
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }

        pSrc++;

        if ((*pSrc>='0') && (*pSrc<='9'))
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
            *pDst |= *pSrc - 'A' + 10;
        }

        pSrc++;
        pDst++;
    }
    return (nSrcLength / 2);
}

int bytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int i = 0;
    const char tab[]="0123456789ABCDEF"; 

    for (i = 0; i < nSrcLength; i++)
    {
        *pDst++ = tab[*pSrc >> 4];
        *pDst++ = tab[*pSrc & 0x0f];
        pSrc++;
    }

    *pDst = '\0';
    return (nSrcLength * 2);
}

int encode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int nSrc;        
    int nDst;        
    int nChar;        
    unsigned char nLeft;

    nSrc = 0;
    nDst = 0;

    while (nSrc < nSrcLength)
    {
        nChar = nSrc & 7;

        if(nChar == 0)
        {
            nLeft = *pSrc;
        }
        else
        {
            *pDst = (*pSrc << (8-nChar)) | nLeft;
            nLeft = *pSrc >> nChar;

            pDst++;
            nDst++;
        }

        pSrc++;
        nSrc++;
    }
    return nDst;
}

int decode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int nSrc;
    int nDst;
    int nByte;
    unsigned char nLeft;

    nSrc = 0;
    nDst = 0;
    
    nByte = 0;
    nLeft = 0;

    while(nSrc<nSrcLength)
    {
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
        nLeft = *pSrc >> (7-nByte);
        pDst++;
        nDst++;
        nByte++;
        if(nByte == 7)
        {
            *pDst = nLeft;

            pDst++;
            nDst++;

            nByte = 0;
            nLeft = 0;
        }
        pSrc++;
        nSrc++;
    }

    *pDst = '\0';
    return nDst;
}

int scanMultiBytes(const char *pSrc)
{
    int i;
    for (i = 0; i < strlen(pSrc); i ++)
    {
        if (*(pSrc+i) & 0x80)
        {
            return 1;
        }
    }
    return 0;
}

int encode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    memcpy(pDst, pSrc, nSrcLength);
    return nSrcLength;
}

int decode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    memcpy(pDst, pSrc, nSrcLength);
    *(pDst+nSrcLength) = '\0';
    return nSrcLength;
}

int encodeUcs2(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int i = 0;
    int nDstLength = 0;
    unsigned char ucs2[MAX_SMS_MSG_LEN] = {0};
    nDstLength = utf8_to_ucs2(pSrc, nSrcLength, ucs2, MAX_SMS_MSG_LEN);
    for(i = 0; i < nDstLength; i = i + 2)
    {
        *pDst++ = ucs2[i+1];
        *pDst++ = ucs2[i];
    }
    //FASTCGI_LOG("nDstLength: %d\n", nDstLength);
    return nDstLength;
}

int decodeUcs2(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int i = 0;
    int nDstLength;
    unsigned char utf8[MAX_SMS_MSG_LEN] = {0};
    for(i = 0; i < nSrcLength; i = i + 2)
    {
        utf8[i] = *(pSrc+i+1);
        utf8[i+1] = *(pSrc+i);
    }
    nDstLength = ucs2_to_utf8(utf8, nSrcLength, pDst, MAX_SMS_MSG_LEN);
    pDst[nDstLength] = '\0';
    return nDstLength;
}

// "8613851872468" --> "683158812764F8"
int invertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int i = 0;
    int nDstLength;
    char ch; 

    nDstLength = nSrcLength;
    for(i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;
        *pDst++ = *pSrc++;
        *pDst++ = ch;
    }
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';
        nDstLength++;
    }
    *pDst = '\0';
    return nDstLength;
}

// "683158812764F8" --> "8613851872468"
int serializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int i = 0;
    int nDstLength;
    char ch;

    nDstLength = nSrcLength;
    for(i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;
        *pDst++ = *pSrc++;
        *pDst++ = ch;
    }

    if(*(pDst-1) == 'F')
    {
        pDst--;
        nDstLength--;
    }
    *pDst = '\0';
    return nDstLength;
}

int rssi2dbm(char *rssi)
{
    int dbm = atoi(rssi);
    if(dbm == 99)
    {
        dbm = 0;
    }
    else
    {
        dbm = -113 + 2*dbm;
    }
    return dbm;
}

int rsrq2dbm(char *rsrq)
{
    int dbm = atoi(rsrq);
    if(dbm == 255)
    {
        dbm = 0;
    }
    else
    {
        dbm = -20 + 1/2*dbm;
    }
    return dbm;
}

int rsrp2dbm(char *rsrp)
{
    int dbm = atoi(rsrp);
    if(dbm == 255)
    {
        dbm = 0;
    }
    else
    {
        dbm = -140 + dbm;
    }
    return dbm;
}

int type2string(char *type, char *pDst)
{
    int itype = atoi(type);
    switch (itype)
    {
    case 0:
    case 1:
        strncpy(pDst, GSM, strlen(GSM));
        break;
    case 2:
        strncpy(pDst, UTRAN, strlen(UTRAN));
        break;
    case 3:
        strncpy(pDst, UTRAN, strlen(UTRAN));
        break;
    case 4:
    case 5:
    case 6:
        strncpy(pDst, UTRAN, strlen(UTRAN));
        break;
    case 7:
        strncpy(pDst, E_UTRAN, strlen(E_UTRAN));
        break;   
    case 10:
        strncpy(pDst, NSA, strlen(NSA));
        break;
    default:
        strncpy(pDst, GSM, strlen(GSM));
        break;
    }
    return 0;
}

// "13851872468"/"+8613851872468" -> "8613851872468"
int formatNumber(const char* pSrc, char* pDst, int nSrcLength)
{
    int i = 0;
    int nDstLength;
    char ch;

    ch = *pSrc;
    if(ch == '+')
    {
        for(i=1; i<nSrcLength;i++)
        {
            *pDst++ = *(pSrc+i);
        }
        nDstLength = nSrcLength - 1;
    }
    else if(ch == '8')
    {
        for(i=0; i<nSrcLength;i++)
        {
            *pDst++ = *pSrc++;
        }
        nDstLength = nSrcLength;
    }
    else
    {
        nDstLength = nSrcLength + 2;
        *pDst++ = '8';
        *pDst++ = '6';
        for(i=0; i<nSrcLength;i++)
        {
            *pDst++ = *pSrc++;
        }
        *pDst = '\0';
    }
    return nDstLength;
}


int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst)
{
    int nLength;
    int nDstLength;
    unsigned char buf[MAX_SMS_MSG_LEN];

    nLength = strlen(pSrc->SCA);
    buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;
    buf[1] = 0x91;
    nDstLength = bytes2String(buf, pDst, 2);
    nDstLength += invertNumbers(pSrc->SCA, &pDst[nDstLength], nLength); 

    nLength = strlen(pSrc->TPA);
    buf[0] = 0x11;
    buf[1] = 0;
    buf[2] = (char)nLength;
    buf[3] = 0x91;
    nDstLength += bytes2String(buf, &pDst[nDstLength], 4);
    nDstLength += invertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);

    nLength = strlen(pSrc->TP_UD);
    buf[0] = pSrc->TP_PID;
    buf[1] = pSrc->TP_DCS;
    buf[2] = 0;
    if(pSrc->TP_DCS == GSM_7BIT)    
    {
        buf[3] = nLength;
        nLength = encode7bit(pSrc->TP_UD, &buf[4], nLength+1) + 4;
    }
    else if(pSrc->TP_DCS == GSM_UCS2)
    {
        buf[3] = encodeUcs2(pSrc->TP_UD, &buf[4], nLength);
        nLength = buf[3] + 4;
    }
    else
    {
        buf[3] = encode8bit(pSrc->TP_UD, &buf[4], nLength);
        nLength = buf[3] + 4;
    }
    nDstLength += bytes2String(buf, &pDst[nDstLength], nLength);
    return nDstLength;
}

int gsmDecodePdu(const char* pSrc, SM_PARAM* pDst)
{
    int nDstLength;
    unsigned char tmp;
    unsigned char buf[MAX_SMS_MSG_LEN];

    string2Bytes(pSrc, &tmp, 2);
    tmp = (tmp - 1) * 2;
    pSrc += 4;
    serializeNumbers(pSrc, pDst->SCA, tmp);
    pSrc += tmp;
    string2Bytes(pSrc, &tmp, 2);
    pSrc += 2;
    string2Bytes(pSrc, &tmp, 2);
    if(tmp & 1) tmp += 1;
    pSrc += 4;
    serializeNumbers(pSrc, pDst->TPA, tmp);
    pSrc += tmp;
    string2Bytes(pSrc, (unsigned char*)&pDst->TP_PID, 2);
    pSrc += 2;
    string2Bytes(pSrc, (unsigned char*)&pDst->TP_DCS, 2);
    pSrc += 2;
    serializeNumbers(pSrc, pDst->TP_SCTS, 14);
    pSrc += 14;
    string2Bytes(pSrc, &tmp, 2);
    pSrc += 2;
    string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.TP_len, 2);
    pSrc += 2;
    string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.TP_udhi, 2);
    pSrc += 2;
    string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.TP_llen, 2);
    pSrc += 2;
    if(pDst->long_msg.TP_len == 0x06 && pDst->long_msg.TP_udhi == 0x08 && pDst->long_msg.TP_llen == 0x04)
    {
        string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.TP_id1, 2);
        pSrc += 2;
        string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.TP_id2, 2);
        pSrc += 2;
        string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.Pk_total, 2);
        pSrc += 2;
        string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.Pk_number, 2);
        pSrc += 2;
        tmp -= 7;
    }
    else if(pDst->long_msg.TP_len == 0x05 && pDst->long_msg.TP_udhi == 0x00 && pDst->long_msg.TP_llen == 0x03)
    {
        string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.TP_id1, 2);
        pSrc += 2;
        pDst->long_msg.TP_id2 = 0x00;
        string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.Pk_total, 2);
        pSrc += 2;
        string2Bytes(pSrc, (unsigned char*)&pDst->long_msg.Pk_number, 2);
        pSrc += 2;
        tmp -= 6;
    }
    else
    {
        pDst->long_msg.TP_len = 0;
        pDst->long_msg.TP_udhi = 0;
        pDst->long_msg.TP_llen = 0;
        pSrc -= 6;
    }
    
    if(pDst->TP_DCS == GSM_7BIT)    
    {
        nDstLength = string2Bytes(pSrc, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);
        decode7bit(buf, pDst->TP_UD, nDstLength);
        nDstLength = tmp;
    }
    else if(pDst->TP_DCS == GSM_UCS2)
    {
        nDstLength = string2Bytes(pSrc, buf, tmp * 2);
        nDstLength = decodeUcs2(buf, pDst->TP_UD, nDstLength);
    }
    else
    {
        nDstLength = string2Bytes(pSrc, buf, tmp * 2);
        nDstLength = decode8bit(buf, pDst->TP_UD, nDstLength);
    }
    return nDstLength;
}

int fill_sm_param(SM_PARAM *pSrc, char* sca)
{
    char    tpa[MAX_SMS_VALUE_LEN] = {0};
	
    memcpy(tpa, pSrc->TPA, sizeof(pSrc->TPA));
    formatNumber(sca, pSrc->SCA, strlen(sca));
    formatNumber(tpa, pSrc->TPA, strlen(tpa));
	
    if(scanMultiBytes(pSrc->TP_UD) == 1)
    {
        pSrc->TP_DCS = GSM_UCS2;
    }
    else
    {
        pSrc->TP_DCS = GSM_7BIT;
    }
    
    return 0;
}


int send_sms(char* phone_number, char *center_number, char* sms_msg)
{
    int     ret = -1;
    int     len = -1;
    int     nPduLength;
    int     index = 0;
	char 	cmd[AT_CMD_BUFF_LEN] = {0};
    char    pdu[MAX_SMS_MSG_LEN] = {0};
	char 	*ptr = NULL;
    unsigned char nSmscLength;
    SM_PARAM pSrc;
	
    bzero(&pSrc, sizeof(SM_PARAM));
	
	memcpy(pSrc.TPA, phone_number, strlen(phone_number));
	memcpy(pSrc.TP_UD, sms_msg, strlen(sms_msg));
	
    pSrc.TP_DCS = GSM_UCS2;
   
   
	if(strlen(center_number) < MAX_SMS_VALUE_LEN && strlen(center_number) > 0)
	{
        fill_sm_param(&pSrc, center_number);
        nPduLength = gsmEncodePdu(&pSrc, pdu);
        strcat(pdu, "\x01a");
        string2Bytes(pdu, &nSmscLength, 2);
        nSmscLength++;
		
		snprintf(cmd, AT_CMD_BUFF_LEN, "AT+CMGF=0");//0:pdu mode    1:txt mode
		if ( Atcmd(cmd, strlen(cmd), 1) == LTE_FALSE )
		{
			return -1;
		}
		
		snprintf(cmd, AT_CMD_BUFF_LEN, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);
		
		AtcmdSendSms(cmd,strlen(cmd),pdu,strlen(pdu));
		
		ptr = ATret;
		
		if(strstr(ptr, "+CMS ERROR") != NULL)
        {
			return -1;
        }
    }
    else
    {
        return -1;
    }
    
    return 0;
}


/*
src1="key : val \n"
src2="key : val,"
*/
void at_value_get(char *val, char *src, int size, char end )
{
	//printf("src=%s. end=%c.\n", src, end);
    char *s1=val;
    char *s2=src;
    if ( strchr(src, ':') == NULL )
        goto noval;

    memset(val, 0, size);
    while (*s2 != ':')
		s2++;
    s2++;
    while (*s2 == ' ')
		s2++;
    while (*s2 != end)
    {
        *s1 = *s2;
        s1++;
        s2++;
    }
	
noval:		
    *s1 = '\0';

	//printf("val=%s.\n", val);
    return;
}

/*
action
	set  - 1 : usleep 500ms
	get - 0 :usleep 100ms
*/
LTE_BOOL Atcmd(char *cmd, int len, int action)
{
	int fd,ret = 1;
	int wait_times = AT_TIMEOUT_SEC*10;
	int use_apn = 0;
	
	ret = lte_lock_file(AT_CMD_LOCK_FILE);
	if (ret < 0)
	{
		return LTE_FALSE;
	}
	
	unlink(AT_RESULT_FILE);
	memset(ATret, '\0', sizeof(ATret));
	
	//printf("AT cmd is [%s]\n", cmd);
	if ( !lte_f_exists(DEFAULT_DEV_ATC) ) {
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}

	if ( atcm_send(cmd, len) < 0 ) {
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}
	huayi_uci_get_int(PKG_MODEM,"status","mode",&use_apn);
	if(use_apn == 1)
		sleep(5);
	else		
		usleep(10*1000);
	//printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	fd = open(AT_RESULT_FILE, O_RDONLY);
	if (fd == -1){
		yprintf("open at result file error\n");
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}

	read(fd, ATret, sizeof(ATret));
	close(fd);

	//printf("result is \n[%s]\n", ATret);
	//syslog(LOG_INFO,">>> %s\n",ATret);
	lte_unlock_file(AT_CMD_LOCK_FILE);
	
	return LTE_TRUE;
}


LTE_BOOL AtcmdSendDelay(char *cmd, int len, int action)
{
	int fd = 0;
	int ret = 1;
	int i = 0;
	
	ret = lte_lock_file(AT_CMD_LOCK_FILE);
	if (ret < 0)
	{
		return LTE_FALSE;
	}
	
	unlink(AT_RESULT_FILE);
	memset(ATret, '\0', sizeof(ATret));
	
	if ( !lte_f_exists(DEFAULT_DEV_ATC) ) 
	{
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}

	if ( atcm_send(cmd, len) < 0 ) 
	{
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}

	usleep(500*1000);
	
	fd = open(AT_RESULT_FILE, O_RDONLY);
	if (fd == -1)
	{
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}
	
	ret = LTE_FALSE;
	
	for (i = 0; i < 10; i++)
	{
		read(fd, ATret, sizeof(ATret));

		if ( strstr(ATret, "OK") != NULL )
		{
			ret = LTE_TRUE;
			break;
		}
		usleep(500*1000);
	}
	
	close(fd);

	lte_unlock_file(AT_CMD_LOCK_FILE);
	
	return ret;
}


LTE_BOOL AtcmdSendSms(char *cmd ,int cmd_len, char *pdu, int pdu_len)
{
	int fd = 0;
	int ret = 1;
	int i = 0;
	
	ret = lte_lock_file(AT_CMD_LOCK_FILE);
	if (ret < 0)
	{
		return LTE_FALSE;
	}
	
	unlink(AT_RESULT_FILE);
	memset(ATret, '\0', sizeof(ATret));
	
	if ( !lte_f_exists(DEFAULT_DEV_ATC) ) 
	{
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}
	
	if ( atcm_send(cmd, cmd_len) < 0 ) 
	{
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}

	usleep(500*1000);
	
	ret = LTE_FALSE;

	fd = open(AT_RESULT_FILE, O_RDONLY);
	if (fd == -1)
	{
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}
	
	for (i = 0; i < 10; i++)
	{
		read(fd, ATret, sizeof(ATret));

		if(strstr(ATret, ">") != NULL)
		{
			break;
		}
		usleep(500*1000);
	}

	if ( atcm_send(pdu, pdu_len) < 0 ) 
	{
		close(fd);
		lte_unlock_file(AT_CMD_LOCK_FILE);
		return LTE_FALSE;
	}

	for (i = 0; i < 10; i++)
	{
		read(fd, ATret, sizeof(ATret));

		if ( strstr(ATret, "OK") != NULL )
		{
			ret = LTE_TRUE;
			break;
		}
		usleep(500*1000);
	}
	
	close(fd);

	lte_unlock_file(AT_CMD_LOCK_FILE);
	
	return ret;
}


LTE_BOOL atcmd_get_cpin(int *value)
{
    char *ptr = NULL;
    
    if ( Atcmd("AT+CPIN?", strlen("AT+CPIN?"), ATCMD_GET) == LTE_FALSE )
    {
        *value = MODEM_SIN_ERROR;
        return LTE_FALSE;
    }
	
    ptr = ATret;
    yprintf("ATret[%s]\n", ATret);
    if(strstr(ptr, "READY") != NULL)
    {
        yprintf("+CPIN: READY\n");
        *value = MODEM_SIN_READY;
    }
    else if( strstr(ptr, "SIM busy") != NULL )
    {
        yprintf("SIM busy\n");
        *value = MODEM_SIN_BUSY;
    }
	else if( strstr(ptr, "ERROR") != NULL )
    {
        yprintf("SIM ERROR\n");
        *value = MODEM_SIN_ERROR;
    }
    else if( strstr(ptr, "SIM not inserted") != NULL )
    {
        yprintf("SIM not inserted\n");
        *value = MODEM_SIN_NOT_INSERTED;
    }
    else if(strstr(ptr, "SIM PIN") != NULL)
    {
        yprintf("SIM PIN\n");
        *value = MODEM_SIN_PIN;
    }
    else if(strstr(ptr, "SIM PUK") != NULL)
    {
        yprintf("SIM PUK!\n");
        *value = MODEM_SIN_PUK;
    }   
    else
    {
        yprintf("SIM Unknow\n");
        return LTE_FALSE;
    }
	
    return LTE_TRUE;
}

LTE_BOOL atcmd_sim_is_exist()
{
    char *ptr = NULL;
    
    if ( Atcmd("AT+CPIN?", strlen("AT+CPIN?"), ATCMD_GET) == LTE_FALSE )
    {
        return LTE_FALSE;
    }
	
    ptr = ATret;
    yprintf("ATret[%s]\n", ATret);
    if(strstr(ptr, "ERROR: 10") != NULL)
    {
        yprintf("+CME ERROR: 10\n");
		return LTE_FALSE;
    }  
	
    return LTE_TRUE;
}


LTE_BOOL atcmd_set_cpin(char *pin)
{
	int status = 0;
	char *ptr = NULL;
	char cmd[AT_CMD_BUFF_LEN]={0};
	
	snprintf(cmd, AT_CMD_BUFF_LEN, "AT+CPIN=\"%s\"", pin);
	if ( AtcmdSendDelay(cmd, strlen(cmd), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	return LTE_TRUE;
}


LTE_BOOL atcmd_get_cpinr(int *pin_num, int *puk_num)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char *sim_pin = "+QPINC:";
	char cmd[AT_CMD_BUFF_LEN]={0};
	char infoValue[AT_VALUE_BUFF_LEN] = {0};
	char vlaue[AT_VALUE_BUFF_LEN] = {0};
	
	snprintf(cmd, AT_CMD_BUFF_LEN, "AT+QPINC?");
	if ( Atcmd(cmd, strlen(cmd), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}

	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}

	if ((pKey = strstr(ptr, sim_pin)) != NULL )
    {
        at_value_get(infoValue, pKey, sizeof(infoValue), '\n');
		huayi_get_nth_value(1, infoValue, ',', vlaue, AT_VALUE_BUFF_LEN);
		*pin_num = atoi(vlaue);
		huayi_get_nth_value(2, infoValue, ',', vlaue, AT_VALUE_BUFF_LEN);
		*puk_num = atoi(vlaue);
    }
	else
	{
		return LTE_FALSE;
	}
	
	return LTE_TRUE;
}


LTE_BOOL atcmd_get_clck(int *sim_clck)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char *sim_pin = "+CLCK:";
	char cmd[AT_CMD_BUFF_LEN]={0};
	char infoValue[AT_VALUE_BUFF_LEN] = {0};
	char vlaue[AT_VALUE_BUFF_LEN] = {0};
	
	snprintf(cmd, AT_CMD_BUFF_LEN, "AT+CLCK=\"SC\",2");
	if ( Atcmd(cmd, strlen(cmd), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}

	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}

	if ((pKey = strstr(ptr, sim_pin)) != NULL )
    {
        at_value_get(infoValue, pKey, sizeof(infoValue), '\n');
		*sim_clck = atoi(infoValue);
    }
	else
	{
		return LTE_FALSE;
	}
	
	return LTE_TRUE;
}


LTE_BOOL atcmd_set_clck(int sim_clck, char *sim_pin)
{
	char *ptr = NULL;
	char cmd[AT_CMD_BUFF_LEN]={0};
	
	snprintf(cmd, AT_CMD_BUFF_LEN, "AT+CLCK=\"SC\",%d,\"%s\"",sim_clck,sim_pin);
	if ( AtcmdSendDelay(cmd, strlen(cmd), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	return LTE_TRUE;
}

LTE_BOOL atcmd_set_cpwd(char *old_pin, char *new_pin)
{
	char *ptr = NULL;
	char cmd[AT_CMD_BUFF_LEN]={0};
	
	snprintf(cmd, AT_CMD_BUFF_LEN, "AT+CPWD=\"SC\",\"%s\",\"%s\"",old_pin,new_pin);
	if ( AtcmdSendDelay(cmd, strlen(cmd), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	return LTE_TRUE;
}


LTE_BOOL atcmd_get_signal(int *value)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char tmpbuf[AT_CMD_BUFF_LEN] = {0};

	// 0~99, not dBm
	{
		if ( Atcmd("AT+CSQ", strlen("AT+CSQ"), 0) == LTE_FALSE )
		{
			*value = 0;
			return LTE_FALSE;
		}
		ptr = ATret;
		
		if ((pKey = strstr(ptr, "+CSQ:")) != NULL )
		{
			//printf("===RSSI=%s========\n", pKey); //+CSQ: 15,99
			at_value_get(tmpbuf, pKey, sizeof(tmpbuf), ',');
			*value = atoi(tmpbuf);
			if ( 0 <= *value && *value <= 31 )
				*value = *value*100/31;
			else if ( 100 <= *value && *value < 199 )//+CSQ: 159,199
				*value = *value - 100;
			else
				*value = 0;
		}
		
	}
	
	return LTE_TRUE;
}

/*cloud server signal rule :
 * >= -80  excellent
 * [-90, -80)  good
 * [-100, -90) generally
 * [-120, -100) difference
 * < -120   no signal
 */
LTE_BOOL atcmd_get_signal_for_cloud(int *value)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char tmpbuf[AT_CMD_BUFF_LEN] = {0};

	if ( Atcmd("AT+CSQ", strlen("AT+CSQ"), 0) == LTE_FALSE )
	{
		*value = -121;
		return LTE_FALSE;
	}
	ptr = ATret;
	
	if ((pKey = strstr(ptr, "+CSQ:")) != NULL )
	{
		//printf("===RSSI=%s========\n", pKey); //+CSQ: 15,99
		at_value_get(tmpbuf, pKey, sizeof(tmpbuf), ',');
		*value = atoi(tmpbuf);

		if ( 1 < *value && *value <= 31 ) 
		{
			if (*value == 31)
			{
				*value = -51;
			}
			else if (24 <= *value && *value <= 30)
			{
			 	*value = (*value)*2 -113;
			}
			else if (16 <= *value && *value <= 23)
			{ 
			 	*value = (*value)-106;
			}
			else if (8 <= *value && *value <= 15)
			{ 
			 	*value = (*value)-108;
			}
			else if (1 <= *value && *value <= 7)
			{ 
			 	*value = (*value)-109;
			}
			else
			{
				*value = -121;
			}		
		}
		else if ( 101 <= *value && *value < 199 )//+CSQ: 159,199
		{
			*value = *value-216;

		}
		else
		{
			*value = -121;
		}
			
	}
	else
	{
		*value = -121;
	}
	
	return LTE_TRUE;
}


// at+qcfg=\"usbnet\",0    ->GobiNet
// at+qcfg=\"usbnet\",1    ->ECM
LTE_BOOL atcmd_get_usbnet(int *value)
{
	char *ptr = NULL;
	char *pKey = NULL;
	
	if ( Atcmd("at+qcfg=\"usbnet\"?", strlen("at+qcfg=\"usbnet\"?"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	if ((pKey = strstr(ptr, ",")) != NULL )
	{
		*(pKey+2)=0;
		*value = atoi(pKey+1);
		return LTE_TRUE;
	}
	
	return LTE_FALSE;
}

LTE_BOOL atcmd_get_qsimdet(int *value)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char tmpbuf[AT_CMD_BUFF_LEN] = {0};
	
	if ( Atcmd("at+qsimdet?", strlen("at+qsimdet?"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	if ((pKey = strstr(ptr, "+QSIMDET:")) != NULL )
	{
		at_value_get(tmpbuf, pKey, sizeof(tmpbuf), '\n');

		*value = atoi(tmpbuf);
		return LTE_TRUE;
	}
	
	return LTE_FALSE;
}

LTE_BOOL atcmd_get_qsimstat(int *value)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char tmpbuf[AT_CMD_BUFF_LEN] = {0};
	
	if ( Atcmd("at+qsimstat?", strlen("at+qsimstat?"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	if ((pKey = strstr(ptr, "+QSIMSTAT:")) != NULL )
	{
		at_value_get(tmpbuf, pKey, sizeof(tmpbuf), ',');
		*value = atoi(tmpbuf);
		return LTE_TRUE;
	}
	
	return LTE_FALSE;
}


/**
return:
	1 : found
	0 : not found
	-1 : error 
*/
LTE_BOOL atcmd_get_imsi(char *imsi)
{
	int i = 0;
	char *ptr = NULL;

	if ( Atcmd("AT+CIMI", strlen("AT+CIMI"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}
	
	for ( i=0; i < AT_CMD_MAX_LEN-16; i++ )
	{
		if ( ptr[i] && isdigit( ptr[i] ) )
		{
			strncpy(imsi, ptr+i, 15); //copy imsi
			return LTE_TRUE;
		}
	}
		
	return LTE_FALSE;
}

/**
return:
	1 : found
	0 : not found
	-1 : error 
*/
LTE_BOOL atcmd_get_mccmnc(char *ccmnc)
{
	int i = 0;
	char imsi[AT_VALUE_BUFF_LEN] = { 0 };
	char *ptr = NULL;
	
	if ( Atcmd("AT+CIMI", strlen("AT+CIMI"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	
	for ( i=0; i < AT_CMD_MAX_LEN-16; i++ )
	{
		if ( ptr[i] && isdigit( ptr[i] ) )
		{
			strncpy(imsi, ptr+i, 15); //copy imsi
			strncpy(ccmnc, ptr+i, 6); //copy MCC:MNC
			
			return LTE_TRUE;
		}
	}
		
	return LTE_FALSE;
}

LTE_BOOL atcmd_get_4gmode(char *netmode)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char sysinfo[AT_VALUE_BUFF_LEN] = {0};
	char sys_mode[AT_VALUE_BUFF_LEN] = {0};
	int i_mode = 0;

	strcpy(netmode, "ERROR");
	if ( Atcmd("AT^SYSINFO", strlen("AT^SYSINFO"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}

	ptr = ATret;
	//^SYSINFO: 2,3,0,X,1
    if ((pKey = strstr(ptr, "^SYSINFO:")) != NULL )
    {
        at_value_get(sysinfo, pKey, sizeof(sysinfo), '\n');
		if((huayi_get_nth_value(3, sysinfo, ',', sys_mode, sizeof(sys_mode)) == -1))
		{
			return LTE_FALSE;
		}
		
		i_mode = atoi(sys_mode);
		if ( 9 == i_mode )
			strcpy(netmode, "4G");
		else if ( 4 == i_mode || 5 == i_mode || 8 == i_mode || 11 == i_mode )
			strcpy(netmode, "3G");
		else if ( 2 == i_mode || 3 == i_mode )
			strcpy(netmode, "2G");
		else if ( 0 == i_mode )
			strcpy(netmode, "No Service");
		else
			strcpy(netmode, "ERROR");
			
    }
		
	return LTE_TRUE;
}


LTE_BOOL atcmd_check(void)
{
	char *ptr = NULL;
	char *pKey = NULL;

	if ( Atcmd("AT", strlen("AT"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	
	if ((pKey = strstr(ptr, "OK")) != NULL )
    {
		return LTE_TRUE;
    }
	
	return LTE_FALSE;
}

LTE_BOOL atcmd_get_quectel_bandLte(char *vlaue)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char infoValue[AT_CMD_BUFF_LEN] = {0};

	if ( AtcmdSendDelay("AT+QENG=\"SERVINGCELL\"", strlen("AT+QENG=\"SERVINGCELL\""), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}

	ptr = ATret;
	
    if ((pKey = strstr(ptr, "+QENG")) != NULL )
    {
        at_value_get(infoValue, pKey, sizeof(infoValue), '\n');
		huayi_get_nth_value(2, infoValue, ',', vlaue, AT_VALUE_BUFF_LEN);
		return LTE_TRUE;
    }
	
	return LTE_FALSE;
}

LTE_BOOL atcmd_get_neoway_bandLte(char *vlaue)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char infoValue[AT_LONG_BUFF_LEN] = {0};

	if ( AtcmdSendDelay("AT+NETDMSG", strlen("AT+NETDMSG"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}

	ptr = ATret;
	
    if ((pKey = strstr(ptr, "+NETDMSG")) != NULL )
    {
        at_value_get(infoValue, pKey, sizeof(infoValue), '\n');
		huayi_get_nth_value(5, infoValue, ',', vlaue, AT_VALUE_BUFF_LEN);
		return LTE_TRUE;
    }
	
	return LTE_FALSE;
}


LTE_BOOL atcmd_get_center_number(char *value)
{
	int     len = -1;
    int     index = 0;
    char    *ptr = NULL;

	if ( AtcmdSendDelay("AT+CSCS=\"GSM\"", strlen("AT+CSCS=\"GSM\""), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}

  	if ( AtcmdSendDelay("AT+CSCA?", strlen("AT+CSCA?"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}
    
    ptr = strstr(ptr, "+CSCA:");
    if(ptr == NULL)
    {
        return LTE_FALSE;
    }
	
    ptr = strtok(ptr, "\"");
	while (ptr != NULL) 
    {
        if(index == 1)
        {
            len = strlen(ptr);
            strncpy(value, ptr, len);
            break;
        }
        index++;
        ptr = strtok(NULL, "\"");
	}
    
    return LTE_TRUE;

}


LTE_BOOL atcmd_get_EC20_imei(char *value)
{
	int i = 0;
	char *ptr = NULL;

	if ( AtcmdSendDelay("AT+CGSN", strlen("AT+CGSN"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}

	for ( i=0; i < AT_CMD_MAX_LEN-16; i++ )
	{
		if ( ptr[i] && isdigit( ptr[i] ) )
		{
			strncpy(value, ptr+i, 15);
			
			return LTE_TRUE;
		}
	}

	return LTE_FALSE;
}

LTE_BOOL atcmd_get_EC20_phoneNumber(char *value)
{
	int i = 0;
	char *ptr = NULL;
	
	if ( AtcmdSendDelay("AT+CNUM", strlen("AT+CNUM"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}
	
	for ( i=0; i < AT_CMD_MAX_LEN-16; i++ )
	{
		if ( ptr[i] && isdigit( ptr[i] ) )
		{
			strncpy(value, ptr+i, 13);

			return LTE_TRUE;
		}
	}
	
	return LTE_FALSE;
}


LTE_BOOL read_sms_center_number(char* center_number, int *center_len)
{
    int     len = -1;
    int     index = 0;
    char    *ptr = NULL;

  	if ( Atcmd("AT+CSCA?", strlen("AT+CSCA?"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}
    
    ptr = strstr(ptr, "+CSCA:");
    if(ptr == NULL)
    {
        return LTE_FALSE;
    }
	
    ptr = strtok(ptr, "\"");
	while (ptr != NULL) 
    {
        if(index == 1)
        {
            len = strlen(ptr);
            if(*center_len < len)
            {
                return LTE_FALSE;
            }
            *center_len = len;
            strncpy(center_number, ptr, len);
            break;
        }
        index++;
        ptr = strtok(NULL, "\"");
	}
    
    return LTE_TRUE;
}


LTE_BOOL restart_4gradio(void)
{
	char * ptr = NULL;
	char *pKey = NULL;
	
retry_stop:
    if ( Atcmd("AT+CFUN=0", strlen("AT+CFUN=0"), 0) == LTE_FALSE )
    {
        return LTE_FALSE;
    }

	ptr = ATret;

    if ((pKey = strstr(ptr, "OK")) != NULL )
    {
        sleep(3);
		
    }else
    {
    	sleep(1);
     	goto retry_stop;
    }

retry_start:	
	if ( Atcmd("AT+CFUN=1", strlen("AT+CFUN=1"), 0) == LTE_FALSE )
    {
        return LTE_FALSE;
    }
	
	ptr = ATret;
	
	if ((pKey = strstr(ptr, "OK")) != NULL )
	{
		return LTE_TRUE;
	}
	else
	{
		sleep(1);
		goto retry_start;
	}
	
    return LTE_FALSE;
}

LTE_BOOL atcmd_get_iccid(char *iccid)
{
	int i = 0;
	char *ptr = NULL;

	if ( Atcmd("AT+QCCID", strlen("AT+QCCID"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}
	
	for ( i=0; i < AT_CMD_MAX_LEN-16; i++ )
	{
		if ( ptr[i] && isdigit( ptr[i] ) )
		{
			strncpy(iccid, ptr+i, 20); //copy iccid
			return LTE_TRUE;
		}
	}
		
	return LTE_FALSE;
}
LTE_BOOL lynq_get_iccid(char *iccid)
{
	int i = 0;
	char *ptr = NULL;

	if ( Atcmd("AT+ICCID", strlen("AT+ICCID"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	
	if ( strstr(ptr, "OK") == NULL )
	{
		return LTE_FALSE;
	}
	
	for ( i=0; i < AT_CMD_MAX_LEN-16; i++ )
	{
		if ( ptr[i] && isdigit( ptr[i] ) )
		{
			strncpy(iccid, ptr+i, 20); //copy iccid
			return LTE_TRUE;
		}
	}
		
	return LTE_FALSE;
}
LTE_BOOL lynq_get_4gmode(char *netmode)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char infoValue[AT_LONG_BUFF_LEN] = {0};

	if ( Atcmd("AT+CELLINFO?", strlen("AT+CELLINFO?"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	
	ptr = ATret;
	
    if ((pKey = strstr(ptr, "+CELLINFO")) != NULL )
    {
        at_value_get(infoValue, pKey, sizeof(infoValue), '\n');
		huayi_get_nth_value(1, infoValue, ',', netmode, AT_VALUE_BUFF_LEN);
		return LTE_TRUE;
    }

		
	return LTE_FALSE;
}

LTE_BOOL atcmd_get_lac(char *lac)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char creg[AT_VALUE_BUFF_LEN] = {0};
	char tmplac[AT_VALUE_BUFF_LEN] = { 0 };

retry:
	if ( Atcmd("AT+CREG=2", strlen("AT+CREG=2"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	sleep(3);
	ptr = ATret;
	if ((pKey = strstr(ptr, "OK")) == NULL )
    {
        sleep(3);
		goto retry;
    }

	if ( Atcmd("AT+CREG?", strlen("AT+CREG?"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	//+CREG: 2,1,"7754","D0E0932",7
	ptr = ATret;
	if ((pKey = strstr(ptr, "OK")) == NULL )
    {
        return LTE_FALSE;
    }
    if ((pKey = strstr(ptr, "+CREG:")) != NULL )
    {
        at_value_get(creg, pKey, sizeof(creg), '\n');
		if((huayi_get_nth_value(2, creg, ',', tmplac, sizeof(tmplac)) == -1))
		{
			return LTE_FALSE;
		}
		strcpy(lac, tmplac);
    }
		
	return LTE_TRUE;
}

LTE_BOOL atcmd_get_ci(char *ci)
{
		char *ptr = NULL;
	char *pKey = NULL;
	char creg[AT_VALUE_BUFF_LEN] = {0};
	char tmpci[AT_VALUE_BUFF_LEN] = { 0 };

retry:
	if ( Atcmd("AT+CREG=2", strlen("AT+CREG=2"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	sleep(3);
	ptr = ATret;
	if ((pKey = strstr(ptr, "OK")) == NULL )
    {
        sleep(3);
		goto retry;
    }

	if ( Atcmd("AT+CREG?", strlen("AT+CREG?"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	//+CREG: 2,1,"7754","D0E0932",7
	ptr = ATret;
	if ((pKey = strstr(ptr, "OK")) == NULL )
    {
        return LTE_FALSE;
    }
    if ((pKey = strstr(ptr, "+CREG:")) != NULL )
    {
        at_value_get(creg, pKey, sizeof(creg), '\n');
		if((huayi_get_nth_value(3, creg, ',', tmpci, sizeof(tmpci)) == -1))
		{
			return LTE_FALSE;
		}
		strcpy(ci, tmpci);
    }
		
	return LTE_TRUE;
}

LTE_BOOL atcmd_set_nat()
{
	char *ptr = NULL;
	char cmd[AT_CMD_BUFF_LEN]={0};
	
	snprintf(cmd, AT_CMD_BUFF_LEN, "AT+QCFG=\"%s\",%d", "nat", 1);
	if ( AtcmdSendDelay(cmd, strlen(cmd), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	//system("huayi_AT_cli AT+QCFG='nat',1");
	return LTE_TRUE;
}

LTE_BOOL atcmd_get_qnetdevctl(int *conntype, int *netcardstatus, int *netstate)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char tmpbuf[AT_CMD_BUFF_LEN] = {0};
	char tmpvalue[AT_VALUE_BUFF_LEN] = {0};
	
	if ( Atcmd("AT+QNETDEVCTL?", strlen("AT+QNETDEVCTL?"), 0) == LTE_FALSE )
	{
		*conntype = 0;
		return LTE_FALSE;
	}
	ptr = ATret;
	if ((pKey = strstr(ptr, "OK")) == NULL )
    {
        return LTE_FALSE;
    }
	if ((pKey = strstr(ptr, "+QNETDEVCTL:")) != NULL )
	{
		at_value_get(tmpbuf, pKey, sizeof(tmpbuf), '\n');
		*conntype = atoi(tmpbuf);
		if((huayi_get_nth_value(2, tmpbuf, ',', tmpvalue, sizeof(tmpvalue)) == -1))
		{
			return LTE_FALSE;
		}
		*netcardstatus = atoi(tmpvalue);
		if((huayi_get_nth_value(3, tmpbuf, ',', tmpvalue, sizeof(tmpvalue)) == -1))
		{
			return LTE_FALSE;
		}
		*netstate = atoi(tmpvalue);
	}
		
	
	return LTE_TRUE;
}

LTE_BOOL atcmd_get_cgact(int *actstate)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char tmpbuf[AT_CMD_BUFF_LEN] = {0};
	char tmpvalue[AT_VALUE_BUFF_LEN] = {0};
	
	if ( Atcmd("AT+CGACT?", strlen("AT+CGACT?"), 0) == LTE_FALSE )
	{
		*actstate = 0;
		return LTE_FALSE;
	}
	ptr = ATret;
	if ((pKey = strstr(ptr, "OK")) == NULL )
    {
        return LTE_FALSE;
    }
	if ((pKey = strstr(ptr, "+CGACT:")) != NULL )
	{
		at_value_get(tmpbuf, pKey, sizeof(tmpbuf), '\n');
		if((huayi_get_nth_value(1, tmpbuf, ',', tmpvalue, sizeof(tmpvalue)) == -1))
		{
			return LTE_FALSE;
		}
		*actstate = atoi(tmpvalue);
	}
	else
	{
		return LTE_FALSE;
	}
		
	
	return LTE_TRUE;
}

LTE_BOOL atcmd_get_cgpaddr(char *ipaddr)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char tmpbuf[AT_CMD_BUFF_LEN] = {0};
	char tmpvalue[AT_VALUE_BUFF_LEN] = {0};
	
	if ( Atcmd("AT+CGPADDR=1", strlen("AT+CGPADDR=1"), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	ptr = ATret;
	if ((pKey = strstr(ptr, "OK")) == NULL )
    {
        return LTE_FALSE;
    }
	if ((pKey = strstr(ptr, "+CGPADDR:")) != NULL )
	{
		at_value_get(tmpbuf, pKey, sizeof(tmpbuf), '\n');
		if((huayi_get_nth_value(1, tmpbuf, ',', tmpvalue, sizeof(tmpvalue)) == -1))
		{
			return LTE_FALSE;
		}
		/*ipaddr is 0.0.0.0 or ip is null*/
		if(strlen(tmpvalue) <= 7)
			return LTE_FALSE;
		else
			strcpy(ipaddr, tmpvalue);
	}
	else
	{
		return LTE_FALSE;
	}
		
	
	return LTE_TRUE;
}

LTE_BOOL atcmd_get_qeng(char *state)
{
	char *ptr = NULL;
	char *pKey = NULL;
	char tmpbuf[AT_CMD_BUFF_LEN] = {0};
	char tmpvalue[AT_VALUE_BUFF_LEN] = {0};
	
	if ( Atcmd("AT+QENG=\"servingcell\"", strlen("AT+QENG=\"servingcell\""), 0) == LTE_FALSE )
	{
		return LTE_FALSE;
	}
	ptr = ATret;
	if ((pKey = strstr(ptr, "OK")) == NULL )
    {
        return LTE_FALSE;
    }
	if ((pKey = strstr(ptr, "+QENG:")) != NULL )
	{
		at_value_get(tmpbuf, pKey, sizeof(tmpbuf), '\n');
		if((huayi_get_nth_value(1, tmpbuf, ',', tmpvalue, sizeof(tmpvalue)) == -1))
		{
			return LTE_FALSE;
		}
		strcpy(state, tmpvalue);
	}
	else
	{
		return LTE_FALSE;
	}
		
	
	return LTE_TRUE;
}


#endif

