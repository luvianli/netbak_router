/*===========================================================================
FILE:
   QMI.h

DESCRIPTION:
   Qualcomm QMI driver header
   
FUNCTIONS:
   Generic QMUX functions
      ParseQMUX
      FillQMUX
   
   Generic QMI functions
      GetTLV
      ValidQMIMessage
      GetQMIMessageID

   Get sizes of buffers needed by QMI requests
      QMUXHeaderSize
      QMICTLGetClientIDReqSize
      QMICTLReleaseClientIDReqSize
      QMICTLReadyReqSize
      QMIWDSSetEventReportReqSize
      QMIWDSGetPKGSRVCStatusReqSize
      QMIDMSGetMEIDReqSize
      QMICTLSyncReqSize

   Fill Buffers with QMI requests
      QMICTLGetClientIDReq
      QMICTLReleaseClientIDReq
      QMICTLReadyReq
      QMIWDSSetEventReportReq
      QMIWDSGetPKGSRVCStatusReq
      QMIDMSGetMEIDReq
      QMICTLSetDataFormatReq
      QMICTLSyncReq
      
   Parse data from QMI responses
      QMICTLGetClientIDResp
      QMICTLReleaseClientIDResp
      QMIWDSEventResp
      QMIDMSGetMEIDResp

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

#pragma once

/*=========================================================================*/
// Definitions
/*=========================================================================*/

extern int debug;
// DBG macro
#define DBG( format, arg... ) do { \
   if (debug == 1)\
   { \
      printk( KERN_INFO "GobiNet::%s " format, __FUNCTION__, ## arg ); \
   } }while(0)

#if 0
#define VDBG( format, arg... ) do { \
   if (debug == 1)\
   { \
      printk( KERN_INFO "GobiNet::%s " format, __FUNCTION__, ## arg ); \
   } } while(0)
#else
#define VDBG( format, arg... ) do { } while(0)
#endif

// QMI Service Types
#define QMICTL 0
#define QMIWDS 1
#define QMIDMS 2
#define QMINAS 3
#define QMIUIM 11
#define QMIWDA 0x1A

#define u8        unsigned char
#define u16       unsigned short
#define u32       unsigned int
#define u64       unsigned long long

#define bool      u8
#define true      1
#define false     0

#define ENOMEM    12
#define EFAULT    14
#define EINVAL    22
#ifndef ENOMSG
#define ENOMSG    42
#endif
#define ENODATA   61

#define TLV_TYPE_LINK_PROTO 0x10


#ifdef WDS_GET_CLIENT_IP_PERF
#define  QMI_WDS_PKT_SRVC_IND  0x0022
#define QMIWDS_GET_RUNTIME_SETTINGS_REQ       0x002D
#define QMIWDS_GET_RUNTIME_SETTINGS_RESP      0x002D




#define TLV_PKT_IND_CONNECTED 0x02
#define TLV_PKT_IND_DISCONNECTED 0x01

#endif

/*=========================================================================*/
// Struct sQMUX
//
//    Structure that defines a QMUX header
/*=========================================================================*/
typedef struct sQMUX
{
   /* T\F, always 1 */
   u8         mTF;

   /* Size of message */
   u16        mLength;

   /* Control flag */
   u8         mCtrlFlag;
   
   /* Service Type */
   u8         mQMIService;
   
   /* Client ID */
   u8         mQMIClientID;

}__attribute__((__packed__)) sQMUX;

/*=========================================================================*/
// Generic QMUX functions
/*=========================================================================*/

// Remove QMUX headers from a buffer
int ParseQMUX(
   u16 *    pClientID,
   void *   pBuffer,
   u16      buffSize );

// Fill buffer with QMUX headers
int FillQMUX(
   u16      clientID,
   void *   pBuffer,
   u16      buffSize );

/*=========================================================================*/
// Generic QMI functions
/*=========================================================================*/

// Get data buffer of a specified TLV from a QMI message
int GetTLV(
   void *   pQMIMessage,
   u16      messageLen,
   u8       type,
   void *   pOutDataBuf,
   u16      bufferLen );

// Check mandatory TLV in a QMI message
int ValidQMIMessage(
   void *   pQMIMessage,
   u16      messageLen );

// Get the message ID of a QMI message
int GetQMIMessageID(
   void *   pQMIMessage,
   u16      messageLen );

/*=========================================================================*/
// Get sizes of buffers needed by QMI requests
/*=========================================================================*/

// Get size of buffer needed for QMUX
u16 QMUXHeaderSize( void );

// Get size of buffer needed for QMUX + QMICTLGetClientIDReq
u16 QMICTLGetClientIDReqSize( void );

// Get size of buffer needed for QMUX + QMICTLReleaseClientIDReq
u16 QMICTLReleaseClientIDReqSize( void );

// Get size of buffer needed for QMUX + QMICTLReadyReq
u16 QMICTLReadyReqSize( void );

// Get size of buffer needed for QMUX + QMIWDSSetEventReportReq
u16 QMIWDSSetEventReportReqSize( void );

// Get size of buffer needed for QMUX + QMIWDSGetPKGSRVCStatusReq
u16 QMIWDSGetPKGSRVCStatusReqSize( void );

// Get size of buffer needed for QMUX + QMIDMSGetMEIDReq
u16 QMIDMSGetMEIDReqSize( void );

// Get size of buffer needed for QMUX + QMIWDASetDataFormatReq
u16 QMIWDASetDataFormatReqSize( void );

// Get size of buffer needed for QMUX + QMICTLSyncReq
u16 QMICTLSyncReqSize( void );

/*=========================================================================*/
// Fill Buffers with QMI requests
/*=========================================================================*/

// Fill buffer with QMI CTL Get Client ID Request
int QMICTLGetClientIDReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID,
   u8       serviceType );

// Fill buffer with QMI CTL Release Client ID Request
int QMICTLReleaseClientIDReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID,
   u16      clientID );

// Fill buffer with QMI CTL Get Version Info Request
int QMICTLReadyReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID );

// Fill buffer with QMI WDS Set Event Report Request
int QMIWDSSetEventReportReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

// Fill buffer with QMI WDS Get PKG SRVC Status Request
int QMIWDSGetPKGSRVCStatusReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

// Fill buffer with QMI DMS Get Serial Numbers Request
int QMIDMSGetMEIDReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

// Fill buffer with QMI WDA Set Data Format Request
int QMIWDASetDataFormatReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

int QMICTLSyncReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

/*=========================================================================*/
// Parse data from QMI responses
/*=========================================================================*/

// Parse the QMI CTL Get Client ID Resp
int QMICTLGetClientIDResp(
   void * pBuffer,
   u16    buffSize,
   u16 *  pClientID );

// Verify the QMI CTL Release Client ID Resp is valid
int QMICTLReleaseClientIDResp(
   void *   pBuffer,
   u16      buffSize );

// Parse the QMI WDS Set Event Report Resp/Indication or
//    QMI WDS Get PKG SRVC Status Resp/Indication
int QMIWDSEventResp(
   void *   pBuffer,
   u16      buffSize,
   u32 *    pTXOk,
   u32 *    pRXOk,
   u32 *    pTXErr,
   u32 *    pRXErr,
   u32 *    pTXOfl,
   u32 *    pRXOfl,
   u64 *    pTXBytesOk,
   u64 *    pRXBytesOk,
   bool *   pbLinkState,
   bool *   pbReconfigure );

// Parse the QMI DMS Get Serial Numbers Resp
int QMIDMSGetMEIDResp(
   void *   pBuffer,
   u16      buffSize,
   char *   pMEID,
   int      meidSize );

// Parse the QMI DMS Get Serial Numbers Resp
int QMIWDASetDataFormatResp(
   void *   pBuffer,
   u16      buffSize );

// Pasre the QMI CTL Sync Response
int QMICTLSyncResp(
   void *pBuffer,
   u16  buffSize );


#ifdef WDS_BIND_MUXID_PORT

u16 QMIWDSBindMuxDataPortReqSize( void );


u16 QMIWDSBindMuxDataPortReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );


#endif

#ifdef WDS_GET_CLIENT_IP_PERF


typedef struct _QCQMUX_MSG_HDR
{
   u16 QMICTLType;//qmi msg id
   u16 Length;//tlv len
} __attribute__ ((__packed__)) QCQMUX_MSG_HDR, *PQCQMUX_MSG_HDR;

typedef struct _QMICTL_SYNC_IND_MSG
{
   u8  CtlFlags;        // QMICTL_FLAG_INDICATION
   u16  TransactionId;
} __attribute__ ((__packed__)) QMICTL_SYNC_IND_MSG, *PQMICTL_SYNC_IND_MSG;


typedef struct _QMI_QCMUX_PACK_HDR
{
   sQMUX QCMUX_HEAD1;
   QMICTL_SYNC_IND_MSG QCMUX_HEAD2;
   QCQMUX_MSG_HDR QCMUX_HEAD3;

}__attribute__((__packed__)) QMI_QCMUX_PACK_HDR,*PQMI_QCMUX_PACK_HDR;

typedef struct _QMI_PKT_IND_TLV_01
{
  u8 tlvtype;
  u16 tlvlen;
  u8  conn;
  u8  brea;
}__attribute__((__packed__)) QMI_PKT_IND_TLV_01,*PQMI_PKT_IND_TLV_01;



typedef struct _QMI_PKT_IND_HDR
{
	sQMUX QCMUX_HEAD1;
	QMICTL_SYNC_IND_MSG QCMUX_HEAD2;
	QCQMUX_MSG_HDR QCMUX_HEAD3;
	QMI_PKT_IND_TLV_01 PTK_TLV;

}__attribute__((__packed__)) QMI_PKT_IND_HDR,*PQMI_PKT_IND_HDR;



u16 QMIWDSRuntimeReqSize( void );

int QMIWDSRuntimeReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );


#endif


