/******************************************************************************
 * @file  meig_ProtocolDL_fota_example.c
 * @brief
 *
 * User-space unit test application for the msm uart driver.
 *
 * -----------------------------------------------------------------------------
 * Copyright (c) 2019, MEIG Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * -----------------------------------------------------------------------------
 ******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <pthread.h>
#include <getopt.h>
#include <poll.h>
#include <unistd.h>


int g_is_Meig_BuffData_Process = 0;


#define OK                1
#define TIMETOUT         -1
#define UARTREADERROR     -2
#define SELECTERROR        -3
#define UARTWRITEERROR    -4

#define MAX_PATH 256

#define MEIG_UART_DDP_BUF_SIZE 1024*2

typedef struct _TDDPDataBuf
{
    char Data[MEIG_UART_DDP_BUF_SIZE];
    int  iRead;
    int  iWrite;
}TDDPDataBuf;

typedef signed short int16;


typedef enum
{
    MEIG_UART_SET_MIN = 0,
    MEIG_UART_SET_BAUDRATE = MEIG_UART_SET_MIN,
    MEIG_UART_SET_DATABITS,
    MEIG_UART_SET_STOPBIT,
    MEIG_UART_SET_PARITY,
    MEIG_UART_SET_FLOWCTRL,
    MEIG_UART_SET_MAX = MEIG_UART_SET_FLOWCTRL,
}MEIG_UART_SET_TYPE;

typedef unsigned int uint8_t;
typedef char s8;
typedef signed int s32;

enum Updata_status
{
    WT_WIRELESS_UPDATE_SUCCESSED = 0,
    WT_WIRELESS_UPDATE_FAILED,
    WT_WIRELESS_UPDATE_UPDATING,
    WT_WIRELESS_UPDATE_COPY_FILE,
    WT_WIRELESS_UPDATE_READY,
    WT_WIRELESS_UPDATE_INIT,
};


static uint8_t g_update_stat=0;
static uint8_t g_download_percent=0;
static uint8_t g_resetmoduleend=0;


static TDDPDataBuf g_tUARTReadBuf;

pthread_t thread_fd= (pthread_t)NULL;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                                            uart code region
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int g_pUartFD = -1;            //串口打开的文件描述符

static int g_ibaud = 115200;    //模块内部已经设置的波特率
static int g_ubaud = 115200;    //固件升级的波特率
static int g_nbaud = 115200;    //模块重启监控串口的波特率
static int g_iDataBits = 8;
static int g_iStopBit = 1;
static int g_iParity = 0;
static int g_iFlowCtrl = 1;
static int g_iDebugFlag = 7;

static int g_iCodeFlag = 15;

static FILE *g_logfileFD = NULL;    // ./log日志文件的文件流指针
static FILE *g_readFileFd = NULL;    //打开固件文件的文件流指针
static int g_fileTotalSize = 0;    //存储固件文件的大小
static int g_fileReadBytes = 0;    //已经从固体文件中读取的字节数
static int g_SendFilestart = 0;
static int g_sendTotalBytes = 0;    //已经发送的总字节数
static int g_ProtocolcmdEntry = 0;
static int g_FileDownLoadEnd = 0;    //更新结束的标志

static char MEIG_UART_DEV[64] = "/dev/ttyUSB0";

static char *g_pUARTSetNameMap[] = {
"BAUDRATE",
"DATABITS",
"STOPBIT",
"PARITY",
"FLOWCTRL"
};




#define MEIG_MAX_OPEN_UART_TRY_CNT 20

#define MEIG_IS_VALID_SET_FLAG(X) ((X >= MEIG_UART_SET_MIN) && (X <= MEIG_UART_SET_MAX))

#define MEIG_MAP_BAUD(X) {case X: (*pSpeed)=B##X;  break;}

unsigned char g_szDBGBuf[256];


#define MEIG_FOTA_DEBUG(x...)    \
do{ \
      if(g_iDebugFlag > 0) \
      { \
          if(g_iDebugFlag & 0x01) \
          { \
            printf(x); \
          } \
          if(g_iDebugFlag & 0x02) \
          { \
        /*add custmer function */    \
      } \
      } \
  } while(0);

int SLM700A_AT_is_channel_ready(char *command);

int Meig_MappingBaudRate(int baud, speed_t *pSpeed)
{
    if(NULL == pSpeed)
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] pSpeed is NULL![baud = %d]\n", baud);
        return -1;
    }
    
    switch (baud)
    {
        MEIG_MAP_BAUD(300);
        MEIG_MAP_BAUD(600);
        MEIG_MAP_BAUD(1200);
        MEIG_MAP_BAUD(2400);
        MEIG_MAP_BAUD(4800);
        MEIG_MAP_BAUD(9600);
        MEIG_MAP_BAUD(19200);
        MEIG_MAP_BAUD(38400);
        MEIG_MAP_BAUD(57600);
        MEIG_MAP_BAUD(115200);
        MEIG_MAP_BAUD(230400);
        MEIG_MAP_BAUD(460800);
        MEIG_MAP_BAUD(921600);
        MEIG_MAP_BAUD(1000000);
        MEIG_MAP_BAUD(1152000);
        MEIG_MAP_BAUD(1500000);
        MEIG_MAP_BAUD(2500000);
        MEIG_MAP_BAUD(3000000);
        MEIG_MAP_BAUD(3500000);
        MEIG_MAP_BAUD(4000000);
        case 3200000:
        {
            (*pSpeed) = B200;
            break;
        }
        default:
        {
           MEIG_FOTA_DEBUG("[Meig][UART_DDP] illegal baud rate![baud=%d]\n",baud);
           return -1;
        }
    }
    return 0;
}


int Meig_SetUartOptions(int fd, MEIG_UART_SET_TYPE iSetFlag, int iVal)
{
    speed_t speed = B115200;
    struct termios options;
    int iRet = -1;

    if(fd < 0)
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] fd is illegal![fd=%d,]\n", fd);
        return -1;
    }

    if(!MEIG_IS_VALID_SET_FLAG(iSetFlag))
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] fd %d invalid set flag: %d\n", fd, iSetFlag);
        return -1;
    }

    MEIG_FOTA_DEBUG("[Meig][UART_DDP][%s] set %d to %s\n", __FUNCTION__, iVal, g_pUARTSetNameMap[iSetFlag]);

    iRet = tcgetattr(fd, &options);
    if(iRet < 0) 
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] failed to tcgetattr![iRet=%d]\n",iRet);
        return(-1);
    }

    //set wait char and wait time
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 0;

    options.c_cflag |= (CLOCAL | CREAD);

    switch(iSetFlag)
    {
        case MEIG_UART_SET_DATABITS:
        {
            options.c_cflag &= ~CSIZE;
            switch(iVal)
            {
                case 5: options.c_cflag |= CS5; break;
                case 6: options.c_cflag |= CS6; break;
                case 7: options.c_cflag |= CS7; break;
                case 8: options.c_cflag |= CS8; break;
                default: 
                {
                    MEIG_FOTA_DEBUG("[Meig][UART_DDP] illegal data bits![iDataBits=%d]\n",iVal);
                    return -1;
                }
            }
            break;
        }
        case MEIG_UART_SET_PARITY:
        {
            switch(iVal)
            {
                case 0:  //no parity
                {
                    if(g_iCodeFlag & 0x01)
                    {
                        MEIG_FOTA_DEBUG("[Meig][UART_DDP] set parity:c_cflag = IGNPAR\n"); //same withe tmp open
                        options.c_iflag = IGNPAR;
                    }
                    else
                    {
                        MEIG_FOTA_DEBUG("[Meig][UART_DDP] set parity:c_cflag &= ~PARENB\n");
                        options.c_cflag &= ~PARENB;
                        options.c_iflag &= ~INPCK;
                    }
                    break;
                }
                case 1:  //odd 
                {
                    options.c_cflag |= (PARENB | PARODD);
                    options.c_iflag |= (INPCK | ISTRIP);
                    break;
                }
                case 2: //even
                {
                    options.c_iflag |= (INPCK | ISTRIP);
                    options.c_cflag |= PARENB;
                    options.c_cflag &= ~PARODD;
                    break;
                }
                default:
                {
                    MEIG_FOTA_DEBUG("[Meig][UART_DDP] illegal parity![iParity=%d]\n", iVal);
                    return -1;
                }
            }
            break;
        }
        case MEIG_UART_SET_STOPBIT:
        {
            switch (iVal)
            {
                case 0:
            #if 0
                {
                    MEIG_FOTA_DEBUG("[Meig][UART_DDP] use default stop bit\n");
                    break;
                }
            #endif
                case 1:
                {
                    options.c_cflag &= ~CSTOPB; 
                    break;
                }
                case 2:    
                {
                    options.c_cflag |= CSTOPB;
                    break;
                }
                default:
                {
                    MEIG_FOTA_DEBUG("[Meig][UART_DDP] illegal stop bit![iParity=%d]\n", iVal);
                    return -1;
                }
            }
            break;
        }
        case MEIG_UART_SET_FLOWCTRL:
        {
            if(1 == iVal)
            {
                options.c_cflag &= ~CRTSCTS;
            }
            else
            {
                options.c_cflag |= CRTSCTS;
            }
            break;
        }
        case MEIG_UART_SET_BAUDRATE:
        {
            iRet = Meig_MappingBaudRate(iVal, &speed);
            if(0 != iRet)
            {
                MEIG_FOTA_DEBUG("[Meig][UART_DDP] illegal baud rate![baud=%d]\n",iVal);
                return -1;
            }

            MEIG_FOTA_DEBUG("[Meig][UART_DDP] used specified baund rate : %d\n",iVal);
            cfsetispeed(&options, speed);
            cfsetospeed(&options, speed);   
            break;
        }
    }

    //ignore control char
    options.c_lflag = 0;
    options.c_oflag = 0;
    MEIG_FOTA_DEBUG("[Meig][UART_DDP] clr c_lfag & c_oflag\n");

    iRet = tcflush(fd, TCIOFLUSH); 
    if(iRet < 0)
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] failed to TCIOFLUSH after baund setting![iRet=%d]\n",iRet);
    }
    
    iRet = tcsetattr(fd, TCSANOW, &options);
    if(iRet < 0)
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] failed to set band rate![iRet=%d]\n",iRet);
        return -1;
    }
    
    return 0;
}


int Meig_SetAllUartOptions(int fd, int baud, int iDataBits, int iStopBit, int iParity, int iFlowCtrl)
{
    MEIG_UART_SET_TYPE iSetType = MEIG_UART_SET_MIN;
    int iParam[MEIG_UART_SET_MAX+1] = {0};
    int iRet = 0;

    iParam[MEIG_UART_SET_BAUDRATE] = baud;
    iParam[MEIG_UART_SET_DATABITS] = iDataBits;
    iParam[MEIG_UART_SET_STOPBIT] = iStopBit;
    iParam[MEIG_UART_SET_PARITY] = iParity;
    iParam[MEIG_UART_SET_FLOWCTRL] = iFlowCtrl;

    for(iSetType = MEIG_UART_SET_MIN; iSetType <= MEIG_UART_SET_MAX; iSetType++)
    {
        iRet = Meig_SetUartOptions(fd, iSetType, iParam[iSetType]);
        if(iRet != 0)
        {
            MEIG_FOTA_DEBUG("[Meig][UART_DDP][%s] ERROR: set uart option failed!\n",__FUNCTION__);
            return iRet;
        }
    }
    return 0;
}

int Meig_UARTOpen(void)
{  
    int iRet = -1;
    int iTryCnt = 0;

    while(iTryCnt < MEIG_MAX_OPEN_UART_TRY_CNT)
    {
        g_pUartFD = open(MEIG_UART_DEV, O_RDWR|O_NOCTTY|O_NDELAY);

        if (-1 == g_pUartFD)
        {
            iTryCnt++;
            MEIG_FOTA_DEBUG("[Meig][UART_DDP] failed to open uart dev[dev:%s,ret:%d,iTryCnt=%d]\n", MEIG_UART_DEV, g_pUartFD, iTryCnt);
            sleep(1);
            continue;
        }
        else
        {
            break;
        }
    }
    if(-1 == g_pUartFD)
    {
        return -1;
    }

    iRet = Meig_SetAllUartOptions(g_pUartFD, g_ibaud, g_iDataBits, g_iStopBit, g_iParity, g_iFlowCtrl);
    if(-1 == iRet)
    {
        close(g_pUartFD);
        g_pUartFD = -1;
        return -1;
    }
    
    if(g_iCodeFlag & 0x04)
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] set uart port to be NONE block state\n");
    }
    else
    {
        fcntl(g_pUartFD, F_SETFL, 0);
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] set uart port to be block state\n");
    }
    return 0;
}  

void Meig_UARTClose(void)
{
    if(g_pUartFD > 0)
    {
        close(g_pUartFD);
        g_pUartFD = -1;
    }
}

void Meig_SendfileClose(void)
{
    if(g_readFileFd > 0)
    {
    fclose(g_readFileFd);
    g_readFileFd = NULL;
    }
}


int Meig_PDLFotaExit(int exit_No)
{
    MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: Meig PDL fota exit (%d)\r\n", exit_No);
    Meig_UARTClose();
    Meig_SendfileClose();

    return exit_No;
}

void Meig_TermSignalProcess(int iSigNo)
{
    switch(iSigNo)
    {
        case SIGTERM:
        case SIGINT:
        case SIGQUIT:
        case SIGHUP:
        case SIGKILL:
        {
            MEIG_FOTA_DEBUG("[Meig][UART_DDP][%s] have detected termination sig. exit !\n",__FUNCTION__);
            Meig_PDLFotaExit(-10);
            break;
        }
        default:
        {
            return ;
        }
    }
    return ;
}

typedef struct _TUartDDPSigHdlrType
{
    int iSigNo;
    void (*pFunc)(int);
}TUartDDPSigHdlrType;

const TUartDDPSigHdlrType UartDDPSigTabl[] = {
    {SIGTERM ,Meig_TermSignalProcess},
    {SIGINT ,Meig_TermSignalProcess},
    {SIGQUIT ,Meig_TermSignalProcess},
    {SIGHUP ,Meig_TermSignalProcess},
    {SIGKILL ,Meig_TermSignalProcess}
};
#define MEIG_UART_DDP_SIG_TABL_SIZE (sizeof(UartDDPSigTabl)/sizeof(UartDDPSigTabl[0]))

void Meig_UartDDPSigInit(void)
{
    int i = 0;

    for(i=0; i<MEIG_UART_DDP_SIG_TABL_SIZE; i++)
    {
        signal(UartDDPSigTabl[i].iSigNo, UartDDPSigTabl[i].pFunc);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                                            main code region
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int openFile(char *fileName)
{
    //int i,readlen,readBytes=0;

    g_readFileFd = fopen(fileName, "rb");
    if( NULL != g_readFileFd )
    {
        MEIG_FOTA_DEBUG("Download file open success !!!\n");
    }
    else 
    {
       MEIG_FOTA_DEBUG("Download file open failed, plase check the is exist !!!!!\n");
       return -1;
    }
    return 0;
}

#if 0
void openlogFile(char *fileName)
{
    g_logfileFD = fopen(fileName, "wb");
    if( NULL == g_logfileFD )
        {
    MEIG_FOTA_DEBUG("Meig open log file failed!!\r\n");   
    }
}
#endif

int Meig_ParseParams(s8 file_applod[])
{
   // int i;
    struct stat statbuf;
    //获取文件的属性
    if(0 == stat(file_applod,&statbuf))
    {
        //得到文件的大小
        g_fileTotalSize = statbuf.st_size;
        MEIG_FOTA_DEBUG("Download file g_fileTotalSize=%d \n", g_fileTotalSize);
        //打开固件文件
            if( 0 != openFile(file_applod))
            {
                MEIG_FOTA_DEBUG("openFile Error!!! \n");
                 return -1;
            }
    }
    else
    {
        MEIG_FOTA_DEBUG("Download file not found!!!!!!!\n");
        return -1;
    }
    return 0;
}

int Meig_GetEmptySizeInBuf(TDDPDataBuf *pData)
{
    if(NULL == pData)
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] pData is NULL\n");
        return 0;
    }

    return (sizeof(pData->Data) - pData->iWrite);
}

int Meig_GetDataSizeInBuf(TDDPDataBuf *pData)
{
    if(NULL == pData)
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] pData is NULL\n");
        return 0;
    }

    if(pData->iRead > pData->iWrite)
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP] ERROR: iRead is more than iWrite[iRead=%d,iWrite=%d]\n", pData->iRead, pData->iWrite);
        return 0;
    }

    return pData->iWrite - pData->iRead;
}



int Meig_check_version(char * version)
{
    int try_times;
    MEIG_FOTA_DEBUG(" ==== Meig_check_version ====\n");

    //处理各种信号的对应处理方式
    Meig_UartDDPSigInit();

    //打开MCU和模块的串口通道
    if(0 != Meig_UARTOpen())
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP]: failed to open uart dev\n");

        g_update_stat = WT_WIRELESS_UPDATE_FAILED;
        //*ret = -2;
        Meig_PDLFotaExit(-2);
        return -1;
    }
    else
    {
        try_times = 0;
        while(SLM700A_AT_is_channel_ready("AT+SGSW\r\n") != OK)
        {
            usleep(2*1000*1000);
            try_times++;
            if (try_times >= 5)//升级的第二阶段重启做异常处理
            {
                //读取是否已经在升级
                Meig_UARTClose();
                return -2;
            }
        }
    }

    return 0;

}



/* retrun 0, It is ok
   Fatal error, retrun a negative
*/

int Meig_SendfileData(int sendlen)
{
    char tmp_buf[512];//使用460800升级的时候，buf的长度为512
    int iRetryTimes = 0, readBytes = 0;
    int ReadFileBytes = 0;
    int UartWriteBytes = 0 , WriteBytes = 0 ;
    fd_set writefds;
    struct timeval tv;
    int MaxFd = 0,z;

    int i_print = 0;
     
    do
    {
        memset(tmp_buf,0x00, sizeof(tmp_buf));
        if(sendlen >= sizeof(tmp_buf))
        {        
               ReadFileBytes = ((g_fileTotalSize - g_fileReadBytes) > sizeof(tmp_buf))? sizeof(tmp_buf):(g_fileTotalSize - g_fileReadBytes);
        }
        else
        {
            ReadFileBytes = ((g_fileTotalSize - g_fileReadBytes) > sendlen)? sendlen:(g_fileTotalSize - g_fileReadBytes);
        }  
               
        readBytes = fread(tmp_buf,1,ReadFileBytes,g_readFileFd);
        if(ReadFileBytes != readBytes )
        {
            MEIG_FOTA_DEBUG("[Meig][ProtocolDL]:Read file failed !!!(ReadFileBytes=%d,readBytes=%d)\r\n",ReadFileBytes,readBytes);
            return -7;
        }
        g_fileReadBytes = g_fileReadBytes + ReadFileBytes;
        UartWriteBytes = 0;
        do
        {
            FD_ZERO( &writefds );
            if(    g_pUartFD >= 0)
                FD_SET(g_pUartFD,&writefds);
            MaxFd = g_pUartFD + 1;
            tv.tv_sec = 0;
            tv.tv_usec = 500000;
            do
            {
                 z = select( MaxFd,0, &writefds, 0, &tv);
            }while( z==-1 && errno==EINTR );
            if( z == -1 )
                  MEIG_FOTA_DEBUG("select(2)\n");

            retry:
            iRetryTimes++; 
            if(g_pUartFD>=0 && FD_ISSET(g_pUartFD, &writefds))
            {
                
                WriteBytes = write(g_pUartFD,tmp_buf+UartWriteBytes,readBytes);
                if( 0 > WriteBytes)
                {
                    MEIG_FOTA_DEBUG("#####[Meig][ProtocolDL]: Send Raw data to module failed!! WriteBytes=%d, erron=%d \r\n", WriteBytes,errno);
                    usleep(1000000);
                    if( iRetryTimes > 30 )
                    {return -8;}
                    goto retry;
                }
                UartWriteBytes = WriteBytes;
                if(UartWriteBytes != readBytes)
                {
                    MEIG_FOTA_DEBUG("######[Meig][ProtocolDL]: UartWriteBytes=%d,readBytes=%d \r\n",UartWriteBytes,readBytes);
                    usleep(800000);
                }

                i_print++;
                if (i_print%200 == 0) MEIG_FOTA_DEBUG("Send_data=%d | ", UartWriteBytes);
                
                readBytes = readBytes - UartWriteBytes;
                usleep(10*1000);
            }
        }while(readBytes);
        g_sendTotalBytes = g_sendTotalBytes + ReadFileBytes;
        sendlen = sendlen - ReadFileBytes;

        if (i_print%200 == 0)  MEIG_FOTA_DEBUG("need sendlen=%d, sendTotal=%d,fileTotal=%d\r\n ",sendlen,g_sendTotalBytes,g_fileTotalSize);
    }while((0 != sendlen) && (g_sendTotalBytes != g_fileTotalSize));

    MEIG_FOTA_DEBUG("[Meig][ProtocolDL] dd need sendlen=%d, g_sendTotalBytes=%d,g_fileTotalSize=%d\r\n",sendlen,g_sendTotalBytes,g_fileTotalSize);

#if 1
    if ((0 == sendlen) && (g_sendTotalBytes == g_fileTotalSize))
    {
        //MEIG_FOTA_DEBUG("[Meig][ProtocolDL] ============ FOTA START1\r\n");
        usleep(3*1000*1000);
        MEIG_FOTA_DEBUG("[Meig][ProtocolDL] ============ FOTA START write AT+FOTA=diff\r\n");
        WriteBytes = write(g_pUartFD,"AT+FOTA=diff\r\n", 14);

        //usleep(3*1000*1000);
        //SLM700A_AT_is_channel_ready("AT+FOTA=diff\r\n");

        usleep(3*1000*1000);
        Meig_PDLFotaExit(0);

        MEIG_FOTA_DEBUG("********** MEIG Protocol Install Start        ************\r\n");
        MEIG_FOTA_DEBUG("********** The FOTA takes 2 minutes           ************\r\n");

#if 1
        for(i_print = 0; i_print< 130; i_print+=5)
        {
            usleep(5*1000*1000);
            MEIG_FOTA_DEBUG("%d\r\n", i_print);
        }

        //
        //TODO: 重启模组  
        MEIG_FOTA_DEBUG("********** Restart the SLM770U               ************\r\n");
        system("gpio l 1 4000 0 0 0 4000");
        usleep(5*1000*1000);
        system("gpio l 1 0 4000 0 0 4000");
        //
        //
        //

        usleep(20*1000*1000); //等待模组开机初始化，首次开机20s
        if (Meig_check_version("_C01_U09")!= 0)
        {
            MEIG_FOTA_DEBUG("******************* FOTA Failed ***************************\r\n");
        }

        g_FileDownLoadEnd=1;
        g_resetmoduleend=1;
        Meig_PDLFotaExit(0);
        return 0;
#endif
            /* The FOTA takes 2 minutes. Restart the module after 2 minutes*/

        

        
    }
    else
    {
        MEIG_FOTA_DEBUG("[Meig][ProtocolDL] ============ FOTA ERROR :send file");
    
        Meig_PDLFotaExit(99);
    }
#endif



    return 0;
}


/* retrun 0, It is ok
   Fatal error, retrun a negative
*/
int Meig_BuffData_Process(void)
{
    int iRet = 0, j =0 ,i = 0, sendBytes=0;
    int iReadAvailLen = 0;
    int ModuleWantBytes=0;
    char cmd_buf[25] ={0x00};
    g_is_Meig_BuffData_Process = 1;

    while(g_tUARTReadBuf.iRead < g_tUARTReadBuf.iWrite)
    {
        i=g_tUARTReadBuf.iRead;
        iReadAvailLen = Meig_GetDataSizeInBuf(&g_tUARTReadBuf);

        MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: iReadAvailLen=%d\r\n", iReadAvailLen);
        #if 0
        for(j=0;j<iReadAvailLen;j++)
        {
            MEIG_FOTA_DEBUG("Data[%d]=0x%x ",i+j,g_tUARTReadBuf.Data[i+j]);
        }
        MEIG_FOTA_DEBUG("\r\n");
        #endif

        for(i=g_tUARTReadBuf.iRead; i<iReadAvailLen; i++)
        {
            //MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: parse received data i=%d,iReadAvailLen=%d, iwrite=%d, iRead=%d\r\n",i, iReadAvailLen,g_tUARTReadBuf.iWrite,g_tUARTReadBuf.iRead);

            if(g_tUARTReadBuf.Data[i] == '>') 
            {
                MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: \"0x03\" cmd received i=%d, iwrite=%d, iRead=%d\r\n",i,g_tUARTReadBuf.iWrite,g_tUARTReadBuf.iRead);

                // Send Raw data to module now
                if( 0 == g_SendFilestart )
                {
                    MEIG_FOTA_DEBUG("[QuMeigec][ProtocolDL]:Protocol download send file now !!\r\n");
                    fseek(g_readFileFd,0L,SEEK_SET);            
                    g_sendTotalBytes = 0;
                    g_fileReadBytes = 0;
                    g_SendFilestart = 1;
                    g_ProtocolcmdEntry = 1;
                }
                ModuleWantBytes = g_fileTotalSize;
                MEIG_FOTA_DEBUG("[QuMeigec][ProtocolDL]: Module Want read data len =%d , i=%d\r\n",ModuleWantBytes,i);

                g_tUARTReadBuf.iRead = g_tUARTReadBuf.iWrite=0; 
                memset(g_tUARTReadBuf.Data,0x00,sizeof(g_tUARTReadBuf.Data));

                iRet = Meig_SendfileData(ModuleWantBytes);
                MEIG_FOTA_DEBUG("[QuMeigec][ProtocolDL]: ====== Meig_SendfileData iRet =%d ======\r\n",iRet);
                if( 0 != iRet)// Fatal error , exit it
                {
                    return iRet;
                }
                else
                {
                    #if 0
                    if (SLM700A_AT_is_channel_ready("AT+FOTA=diff\r\n") == OK)
                    {      
                        MEIG_FOTA_DEBUG("**********MEIG Protocol Install Start ************\r\n");

                        /* The FOTA takes 2 minutes. Restart the module after 2 minutes*/

                    }
                    else
                    {
                        Meig_PDLFotaExit(99);

                    }
                    #endif
                }
            }    
            else
            {
                g_tUARTReadBuf.iRead++;
            }
        }

        g_tUARTReadBuf.iRead++;
        if(g_tUARTReadBuf.iRead >= g_tUARTReadBuf.iWrite)
        {
            g_tUARTReadBuf.iRead = g_tUARTReadBuf.iWrite=0;    
            break;
        }
    }

#if 0
    if(strstr(g_tUARTReadBuf.Data, "OK") != NULL)
    {
        g_update_stat = WT_WIRELESS_UPDATE_UPDATING;
        MEIG_FOTA_DEBUG("********** Meig_SendfileData  END ************\r\n");
        

        if (SLM700A_AT_is_channel_ready("AT+FOTA=diff\r\n") == OK)
        {      
            MEIG_FOTA_DEBUG("**********MEIG Protocol Install Start ************\r\n");

            /* The FOTA takes 2 minutes. Restart the module after 2 minutes*/

            usleep(20*1000*1000);
            MEIG_FOTA_DEBUG("**********MEIG Protocol Install End ************\r\n");
            Meig_PDLFotaExit(0);

        }
        else
        {
            Meig_PDLFotaExit(99);

        }
    }
#endif

    if(strstr(g_tUARTReadBuf.Data, "+FOTADL: \"START\"") != NULL)
    {
        g_update_stat = WT_WIRELESS_UPDATE_UPDATING;
        MEIG_FOTA_DEBUG("**********MEIG Protocol Install Start ************\r\n");
        MEIG_FOTA_DEBUG("Installing...........\r\n");
    }
    if(strstr(g_tUARTReadBuf.Data, "+FOTADL: \"END\",0") != NULL)
    {
        g_FileDownLoadEnd = 1;
        g_resetmoduleend = 1;
        g_update_stat = WT_WIRELESS_UPDATE_SUCCESSED;
        Meig_UARTClose();
        MEIG_FOTA_DEBUG("**********MEIG Protocol Upgrade Success ************\r\n");
	Meig_PDLFotaExit(0);
        exit(0);
    }
    else if(strstr(g_tUARTReadBuf.Data, "+FOTADL: \"END\",801") != NULL)
    {
        g_update_stat = WT_WIRELESS_UPDATE_FAILED;
        MEIG_FOTA_DEBUG("**********MEIG Protocol Upgrade Failed 801************\r\n");
	Meig_PDLFotaExit(-2);
        exit(0);
    }
    else if(strstr(g_tUARTReadBuf.Data, "+FOTADL: \"FILEEND\"") != NULL)
    {
        g_update_stat = WT_WIRELESS_UPDATE_FAILED;
        MEIG_FOTA_DEBUG("**********MEIG Protocol download Fota File Failed************\r\n");
	Meig_PDLFotaExit(-2);
        exit(0);
    }
    return 0;
}
static int get_SLM700A_update_status()
{
    g_ibaud = g_nbaud;
    
    if(0 != Meig_UARTOpen())
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP]: failed to open uart dev\n");        
        g_update_stat = WT_WIRELESS_UPDATE_FAILED;
        //*ret = -2;
        Meig_PDLFotaExit(-2);
        return -1;
    }
    return 0;
}

/*****************************************************************************
 * 函 数 名  : SLM700A_AT_is_channel_ready
 * 创 建 者  : zhouliang
 * 创建日期  : 2022年1月11日
 * 函数功能  : 探测AT通道是否能够正常通信
 * 输入参数  : char *command  写入的指令
 * 输出参数  : 无
 * 返 回 值  : OK：正常
                TIMETOU：超时
                UARTREADERROR: AT指令回显错误
                SELECTERROR: select错误
                UARTWRITEERROR：写入指令失败
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
int SLM700A_AT_is_channel_ready(char *command)
{
    int readlen;
    int writelen;
    int s_ret = 0, ret = 0;
    int maxfd;
    fd_set read_fdset;
    struct timeval timeout;
    char read_buff[128];

    memset(read_buff, 0, sizeof(read_buff));
    writelen = write(g_pUartFD, command ,strlen((char *)command));
    MEIG_FOTA_DEBUG("writelen:%d-%s\n",writelen,command);
    if(writelen > 0)
    {
        do
        {
            FD_ZERO(&read_fdset);
            FD_SET(g_pUartFD,&read_fdset);
            timeout.tv_sec = 3;
            timeout.tv_usec = 0;
            maxfd = g_pUartFD + 1;
            s_ret = select(maxfd, &read_fdset, NULL, NULL, &timeout);
        }while(s_ret < 0);

        if(s_ret == 0)//无事件可读，超时
        {
            ret = TIMETOUT;
        }
        else if(s_ret > 0)//检测到一个可读事件
        {
            
            readlen = read(g_pUartFD, read_buff, sizeof(read_buff));
            MEIG_FOTA_DEBUG("g_pUartFD read:%d:%s\n", readlen, read_buff);
            if((readlen > 0)&&(strstr(read_buff,"OK") != NULL ))
            {
                ret = OK;
#if 0
                if((readlen > 0)&&(strstr(read_buff,"_C01_U09") != NULL ))
                {
                    MEIG_FOTA_DEBUG("***********************************************************\r\n");
                    MEIG_FOTA_DEBUG("******************* FOTA Success U09************************\r\n");
                    MEIG_FOTA_DEBUG("***********************************************************\r\n");
                    g_FileDownLoadEnd=1;
                    g_resetmoduleend=1;
                }

#endif
            }
            else
            {
                ret = UARTREADERROR;
            }
        }
        else
        {
            ret = SELECTERROR;
        }
    }
    else
    {
        ret = UARTWRITEERROR;
    }
    MEIG_FOTA_DEBUG("AT channel ret=%d, sret:%d\n",ret, s_ret);
    return ret;//wait_seconds==0 直接返回
}


#if 0
static int EC200A_module_is_updateing()
{
    int maxfd;
    int iRet = -1;
    int ret = 0;
    fd_set r_fds;
    struct timeval tv = {0,0};
    char read_buf[2048];
    int percent = 0;

    g_ibaud = g_ubaud;

    if(0 != Meig_UARTOpen())
    {
        printf("[Meig][UART_DDP]: failed to open uart dev\n");
        g_update_stat = WT_WIRELESS_UPDATE_FAILED;
        //*ret = -2;
        Meig_PDLFotaExit(-2);
        return -1;
    }
    memset(read_buf, 0, sizeof(read_buf));
    while(1)//加个超时时间等待
    {
        FD_ZERO(&r_fds);
        FD_SET(g_pUartFD, &r_fds);
        maxfd = g_pUartFD+1;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if((1 == g_resetmoduleend)&&(1 == g_FileDownLoadEnd))
        {    
            Meig_PDLFotaExit(0);
            printf("EC200A_module_is_updateing success\n");
            return 0;
        }
        iRet = select(maxfd, &r_fds, NULL, NULL, &tv);
        if(0 == iRet)
        {
            continue;
        }
        else if(0 > iRet)
        {
            MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: select return error![iRet=%d]\n", iRet);    
            printf("[Meig][ProtocolDL]: select return error![iRet=%d]\n", iRet);    
            //*ret = -4;            
            g_update_stat = WT_WIRELESS_UPDATE_FAILED;
            Meig_PDLFotaExit(-4);
            
            return -1;
        }

        if(FD_ISSET(g_pUartFD, &r_fds))
        {
            ret = read(g_pUartFD, read_buf, sizeof(read_buf));
            if(ret > 0)
            {
                //printf("read_buf:%s\n", read_buf);
                if(strstr(read_buf, "+FOTADL: \"END\",0") != NULL)
                {
                    g_FileDownLoadEnd = 1;
                    g_resetmoduleend = 1;
                    g_update_stat = WT_WIRELESS_UPDATE_UPDATING;
                    Meig_UARTClose();                        
                }
                if(strstr(read_buf, "+FOTADL: \"END\",801") != NULL)
                {
                    g_update_stat = WT_WIRELESS_UPDATE_FAILED;
                    return -1;
                }
                memset(read_buf, 0, sizeof(read_buf));
            }
        }
        
    }
}
#endif

//void main(int argc, char **argv)
void SLM700A_thread(void *lpin)
{
    int i;
    int try_times;
    int iRet = -1, icmdProcessRet = 0;
    int maxfd;
    int uart_rc = 0;
    int iWriteAvailLen = 0;
    int writeBytes = 0 ;
    char cmd_buff[128];
    char success_flag=1;
    char setbaud_flag = 1;
    char cmd[32];
    int readlen;
    int writelen;
    char read_buff[128];
    char *fileName = (char *)lpin;
    char path[128] = {0};

    fd_set r_fds;
    struct timeval tv = {0,0};

    MEIG_FOTA_DEBUG("SLM770U_thread success!!!\n");

    //处理各种信号的对应处理方式
    Meig_UartDDPSigInit();

    getcwd(path,100);
    strncat(path,fileName,strlen(fileName));
    
    MEIG_FOTA_DEBUG("My fileName %s,path is %s.\r\n",fileName,path);

    g_update_stat = WT_WIRELESS_UPDATE_COPY_FILE;
    //获取固件文件的大小和打开固件升级文件
    if( 0 != Meig_ParseParams(path))
    {
        MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: app params error!!!!!\n");
        //*ret = -1;
        g_update_stat = WT_WIRELESS_UPDATE_FAILED;//WT_WIRELESS_UPDATE_FILE_NOT_EXIST;
        Meig_PDLFotaExit(-1);
        return ;
    }

OPENUART:    
    //打开MCU和模块的串口通道
    if(0 != Meig_UARTOpen())
    {
        MEIG_FOTA_DEBUG("[Meig][UART_DDP]: failed to open uart dev\n");

        g_update_stat = WT_WIRELESS_UPDATE_FAILED;
        //*ret = -2;
        Meig_PDLFotaExit(-2);
        return ;
    }
    else
    {
        try_times = 0;
        while(SLM700A_AT_is_channel_ready("AT+SGSW\r\n") != OK)//AT指令探测
        {
            try_times++;
            if (try_times >= 5)//升级的第二阶段重启做异常处理
            {
                //读取是否已经在升级
                Meig_UARTClose();
            #if 0
                if(EC200A_module_is_updateing() == 0)
                {
                    g_ibaud = g_nbaud;
                    goto OPENUART;
                }
                
                g_update_stat = WT_WIRELESS_UPDATE_FAILED;
                Meig_PDLFotaExit(-3);
            #endif
                return ;
            }
        }

        MEIG_FOTA_DEBUG("=== g_resetmoduleend:%d g_FileDownLoadEnd:%d\n",g_resetmoduleend,g_FileDownLoadEnd);
        if((g_resetmoduleend == 1)&&(g_FileDownLoadEnd == 1))
        {
            g_update_stat = WT_WIRELESS_UPDATE_SUCCESSED;
            g_download_percent = 100;
            Meig_PDLFotaExit(0);
            return ;
        }
        if(setbaud_flag)
        {
            setbaud_flag = 0;
            memset(cmd,0x00,sizeof(cmd));
            memset(read_buff,0x00,sizeof(read_buff));
            snprintf((char *)cmd,sizeof(cmd),"AT+IPR=%d\r\n", g_ubaud);
            writelen = write(g_pUartFD,cmd,strlen((char *)cmd));
            printf("writelen:%d-%s\n",writelen,cmd);
            if(writelen > 0)
            {
                for(i = 0; i < 4; i++)
                {
                    usleep(300*1000);
                    readlen = read(g_pUartFD,read_buff,sizeof(read_buff));
                    MEIG_FOTA_DEBUG("update_setbaud:%d:%s\n", readlen, read_buff);
                    if((readlen > 0)&&(strstr(read_buff,"OK") != NULL ))
                    {
                        MEIG_FOTA_DEBUG("exit1\n");
                        break;
                    }
                }
                if(readlen <= 0)
                {
                    MEIG_FOTA_DEBUG("exit2\n");
                    g_update_stat = WT_WIRELESS_UPDATE_FAILED;
                    Meig_PDLFotaExit(-3);
                    return ;
                }
            }
            else
            {    
                MEIG_FOTA_DEBUG("exit3\n");
                g_update_stat = WT_WIRELESS_UPDATE_FAILED;
                Meig_PDLFotaExit(-3);
                return ;
            }
            MEIG_FOTA_DEBUG("exit4\n");
            Meig_UARTClose();
            g_ibaud = g_ubaud;
            goto OPENUART;
        }
    }
    g_update_stat = WT_WIRELESS_UPDATE_INIT;

    //send AT+FOTADL=<FILE:length>
    //send AT+FSDWNFILE="fota.pack",9769741
    memset(cmd_buff,0x00,sizeof(cmd_buff));
    memset(read_buff,0x00,sizeof(read_buff));

    //snprintf((char *)cmd_buff,sizeof(cmd_buff),"AT+FOTADL=\"FILE:%d\"\r\n",g_fileTotalSize);
    snprintf((char *)cmd_buff,sizeof(cmd_buff),"AT+FSDWNFILE=\"fota.pack\",%d\r\n",g_fileTotalSize);
    MEIG_FOTA_DEBUG("SEND:%s",cmd_buff);
    writeBytes = write(g_pUartFD,cmd_buff,strlen((char *)cmd_buff)); 

    if(writeBytes != strlen((char *)cmd_buff))
    {
        MEIG_FOTA_DEBUG("[Meig][ProtocolDL]:UART Send FOTADL cmd failed!!!!\n");
        //*ret = -3;
        g_update_stat = WT_WIRELESS_UPDATE_FAILED;
        Meig_PDLFotaExit(-3);
        return ;
    } 

    g_update_stat = WT_WIRELESS_UPDATE_UPDATING;
    //开始进行固件升级
    MEIG_FOTA_DEBUG("********************Meig Protocol download Fota start **********************\r\n");

uartdatainteraction:
    g_tUARTReadBuf.iWrite = 0;
    g_tUARTReadBuf.iRead = 0; 
    while(1)
    {
        FD_ZERO(&r_fds);
        FD_SET(g_pUartFD, &r_fds);
        maxfd = g_pUartFD+1;

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if((1 == g_resetmoduleend)&&(1 == g_FileDownLoadEnd))
        {    
            Meig_PDLFotaExit(0);
            g_update_stat = WT_WIRELESS_UPDATE_SUCCESSED;
            g_download_percent = 100;
            return ;
        }
        //MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: Wait data!! \r\n");

        iRet = select(maxfd, &r_fds, NULL, NULL, &tv);
        if(0 == iRet)
        {
            continue;
        }
        else if(0 > iRet)
        {
            MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: select return error![iRet=%d]\n", iRet);            
            //*ret = -4;            
            g_update_stat = WT_WIRELESS_UPDATE_FAILED;
            Meig_PDLFotaExit(-4);
            return ;
        }

        if(FD_ISSET(g_pUartFD, &r_fds))
        {
            iWriteAvailLen = Meig_GetEmptySizeInBuf(&g_tUARTReadBuf);
            MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: Meig_GetEmptySizeInBuf1 iWriteAvailLen  = %d \r\n",iWriteAvailLen);

            if (iWriteAvailLen == 0) usleep(1000*1000);
            if(iWriteAvailLen > 0)
            {
                memset(g_tUARTReadBuf.Data,0x00,sizeof(g_tUARTReadBuf.Data));

                //usleep(300*1000);
                usleep(1000*1000);

                uart_rc = read(g_pUartFD, g_tUARTReadBuf.Data+g_tUARTReadBuf.iWrite, iWriteAvailLen);
            
                MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: uart_rc  = %d \r\n",uart_rc);
                MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: data  = %s \r\n",g_tUARTReadBuf.Data+g_tUARTReadBuf.iWrite);
                if(uart_rc > 0)
                {
                    #if 0
                    if( 1 == g_FileDownLoadEnd )
                    {
                        // bypass the upgrade URC
                        MEIG_FOTA_DEBUG("%s",g_tUARTReadBuf.Data+g_tUARTReadBuf.iWrite);
                        if(( NULL != strstr((const char *)g_tUARTReadBuf.Data+g_tUARTReadBuf.iWrite,"100"))&&(success_flag == 1))
                        {
                            success_flag --;
                            MEIG_FOTA_DEBUG("\r\n!!!!!!!!FOTA UPGRADE SUCCEED!!!!!!!!!\r\n");
                            Meig_PDLFotaExit(0);
                            if(get_SLM700A_update_status()==0)
                            {
                                goto uartdatainteraction;
                            }
                        }
                        g_tUARTReadBuf.iWrite = 0;
                    }
                    #endif
                    g_tUARTReadBuf.iWrite += uart_rc;
                    MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: read uart data:[read_len=%d][g_tUARTReadBuf: iread=%d, iwrite=%d]\n", uart_rc,g_tUARTReadBuf.iRead, g_tUARTReadBuf.iWrite);
                }
                else
                {
                    MEIG_FOTA_DEBUG("[Meig][ProtocolDL]: failed to read from uart![ret=%d]\n",uart_rc);
                }
            }
        }

        if (g_is_Meig_BuffData_Process == 0)
        {
            icmdProcessRet = Meig_BuffData_Process();
            if( 0 != icmdProcessRet)
            {    
                Meig_PDLFotaExit(icmdProcessRet);    
                return ;
            }
        }

    }
    return ;
}

int main(int argc, char **argv)
//void SLM700A_WirelessFirmwareUpgrade(char *fileName,s32 *ret)
{
    MEIG_FOTA_DEBUG("in SLM770U_FirmwareUpgrade\n");
    
    char *fileName=NULL;
    fileName=malloc(MAX_PATH);

    if (!fileName)
        {
        MEIG_FOTA_DEBUG("fileName mallco NULL!!!\n");
        return -1;
}
    int opt;

    while ( -1 != (opt = getopt(argc, argv, "f:"))) {
        switch (opt) {
            case 'f':
                strncpy(fileName, optarg, MAX_PATH);
            break;
            default:
            break;
        }
    }

    MEIG_FOTA_DEBUG("pthread_create SLM770U!!!\n");

    //chan_close(&chan);//关闭idd层打开的串口通道
    if(pthread_create(&thread_fd, NULL, (void *)SLM700A_thread, (void*)(++fileName))<0)
    {    
        MEIG_FOTA_DEBUG("pthread_create failed!!!\n");
        //*ret = DDI_ERR;//线程创建失败
        exit(1);
    }
    g_update_stat = WT_WIRELESS_UPDATE_READY;
    g_download_percent = 0;

    pthread_join(thread_fd, NULL);
    return 0;
    
}

