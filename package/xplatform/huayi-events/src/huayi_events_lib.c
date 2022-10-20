/*=========================================================================*
 * FileName:        
 * Author:          Kyoko  Version: Ver 1.0  Date: 2011-10-6
 * Description:     
 * Version:         
 * Function List:   
 *                  1.
 * Modified History:         
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * $Author:         Wl
 * $Time:           2011-10-6
 * $Version:        1.0
 * $Desc£º          create this file
 *                  
 *=========================================================================*/
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/net.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <huayi-universal/huayi_universal.h>
#include <huayi-universal/huayi_events.h>
//#include <huayi-universal/huayi_log.h>

/* A_*** redefine in atheros wifi driver */

#define eventslog_l(level, fmt, arg...) do { \
 ; }while (0)
#define eventslog(fmt, arg...) do { \
 ; }while (0)

/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     send a events to kernel events handle module
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           sk:socket descriptor;   eid:events id
                    buf:msg buffer;         len:buffer length
 * Output:          
 * Return:          
 * Others:          
 *-------------------------------------------------------------------------*/
static int huayi_events_send2kernel(int sk, HUAYI_EVENTS_TYPE eid, char*buf, uint32_t len)
{
    int ret,recv;
    struct sockaddr_nl d_nl;
    struct nlmsghdr* nlh;

    if ((NULL == buf) || (len<=0))
	{
        eventslog("wrong input parameters\n");
        return EVENTS_FAIL;
    }

    if (ALT_MAX_PAYLOAD < len){
        eventslog("send msg too long\n");
        return EVENTS_FAIL;
    }
    
    memset(&d_nl, 0, sizeof(d_nl));

    d_nl.nl_family = AF_NETLINK;
    d_nl.nl_pid = 0; /* For Linux Kernel */
    d_nl.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr*)HUAYI_MALLOC(NLMSG_SPACE(len));
    if (NULL == nlh) 
	{
        return EVENTS_FAIL;
    }
	
    memcpy(NLMSG_DATA(nlh), buf, len);
	
    nlh->nlmsg_len = NLMSG_SPACE(len);//NLMSG_LENGTH(sizeof(AGC_NETLINK_MSG));
    nlh->nlmsg_pid = getpid();  /* self pid */
    nlh->nlmsg_flags = NLM_F_REQUEST;
	
    nlh->nlmsg_type = eid;
	
    if ((ALT_EVENTS_REG == eid) || (ALT_EVENTS_UNREG == eid)) {
        nlh->nlmsg_flags |= NLM_F_ACK;
    }
	
    ret = sendto(sk, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&d_nl, sizeof(struct sockaddr_nl));
    
    HUAYI_FREE(nlh);

    //for receive message
    
    return EVENTS_SUCC;
}
int huayi_events_create(void)
{
    int sk;
    struct sockaddr_nl l_nl;
    
    sk = socket(AF_NETLINK, SOCK_DGRAM, ALT_NETLINK_EVTS);
    if (!sk) {
        eventslog_l(HUAYILOG_ERR,"create msg Failed!\n");
        return EVENTS_FAIL;
    }

    memset(&l_nl, 0, sizeof(l_nl));

    l_nl.nl_family    = AF_NETLINK;
    l_nl.nl_pid       = getpid();
    l_nl.nl_groups    = 0;
    bind(sk, (struct sockaddr*)&l_nl, sizeof(l_nl));
    
    //len = huayi_events_send2kernel(sk, ALT_EVENTS_REG, (char*)peid, sizeof(HUAYI_EVENTS_TYPE)*neid);

    return sk;
}
int huayi_events_destroy(int fd)
{
    close(fd);
    
    return 1;
}
/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     for userspace registered a event handle
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           eid:the caller wants to receive event type
 * Output:          
 * Return:          a registered handle
 * Others:          the return handle must be stored in the caller process
 *-------------------------------------------------------------------------*/
int huayi_events_add(HUAYI_EVENTS_TYPE* peid,unsigned short neid,int fd)
{
    int ret,sk = fd;
    uint32_t len;    

    if (!peid || !neid) 
	{
        printf("Register event Error,wrong input para:0x%p %u\n", peid, neid);
        return EVENTS_FAIL;
    }
	
    len = (uint32_t)(sizeof(HUAYI_EVENTS_TYPE)*neid);
	
    return  huayi_events_send2kernel(sk, ALT_EVENTS_REG, (char*)peid, len);
}

/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     for userspace unregistered a event handle
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           eid:the caller don't wants to receive event type
                    fd:the handle that returned by call huayi_events_register()
 * Output:          
 * Return:          
 * Others:          
 *-------------------------------------------------------------------------*/
int huayi_events_del(HUAYI_EVENTS_TYPE* peid,unsigned short neid, int fd)
{
    int ret;
    uint32_t len;
    
    len = (uint32_t)(sizeof(HUAYI_EVENTS_TYPE)*neid);
    if (len ==
        huayi_events_send2kernel(fd, ALT_EVENTS_UNREG, (char*)peid, len)) {
        
        eventslog_l(HUAYILOG_NOTICE, "del message type success\n");
        ret = EVENTS_SUCC;
    } else {
        ret = EVENTS_FAIL;
        eventslog_l(HUAYILOG_NOTICE, "del message type Fail!\n");
    }
    
    return ret;
}

/*-------------------------------------------------------------------------*
 * Function:        
 * Description:     for user space receive a event from event handle module
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           fd:the handle that returned by call huayi_events_register()
                    secs: 0 no block,>0 block seconds,<0 block
                    buf: the buffer for receive message
                    len: buffer length
 * Output:          
 * Return:          <0:error occur; >0:receive success; =0:time out
 * Others:          
 *-------------------------------------------------------------------------*/
int huayi_events_recv(int fd, int32_t secs, char* buf, uint32_t len)
{
    fd_set rfds;
    struct timeval tv;
    int recv,ret = -1;
    char tmpbuf[ALT_MAX_PAYLOAD+100];
    struct sockaddr_nl d_nl;
    struct msghdr msg;
    struct nlmsghdr* nlh;
    struct iovec iov;
    int flags = MSG_DONTWAIT;

    if ((NULL == buf) || (len<=0)) {
        eventslog("wrong input parameters\n");
        return EVENTS_FAIL;
    }

    if (secs>0) {
        /* Watch stdin (fd SK) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        /* Wait up to secs seconds. */
        tv.tv_sec = secs;
        tv.tv_usec = 0;    

        recv = select(fd+1, &rfds, NULL, NULL, &tv);
        if (recv == -1) {
           perror("select()");
           return recv;
        } else if (0 == recv) {
            eventslog("No data within %u seconds.\n",secs);  
            return recv;
        /* FD_ISSET(0, &rfds) will be true. */
        } else {
            eventslog("Data is available now.\n");
        }
    }else if (secs<0)
        flags = MSG_WAITALL;
    

recv_agn:
    msg.msg_name = (void *)&(d_nl);
    msg.msg_namelen = sizeof(d_nl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    iov.iov_base = (void *)tmpbuf;
    iov.iov_len = sizeof(tmpbuf);
    recv = recvmsg(fd, &msg, flags);

#ifdef _HUAYI_EVENTS_DBG_
        printf("[%d]:user recv[%d] rv secs=%u\n", __LINE__,recv, secs);
#endif /* _HUAYI_EVENTS_DBG_ */
    if (recv > 0)
        for (nlh=(struct nlmsghdr *) tmpbuf ; NLMSG_OK (nlh, recv);
                nlh = NLMSG_NEXT (nlh, recv)) {
#ifdef _HUAYI_EVENTS_DBG_
    printf("[%d]:user recv[%d] rv nlh->nlmsg_type=%d\n", __LINE__,recv, nlh->nlmsg_type);
#endif /* _HUAYI_EVENTS_DBG_ */
            if (nlh->nlmsg_type == NLMSG_DONE) {
                /* Comment
                len = (nlh->nlmsg_len - NLMSG_ALIGN(NLMSG_HDRLEN))> len \
                    ? len : (nlh->nlmsg_len - NLMSG_ALIGN(NLMSG_HDRLEN)); **/
                ret = NLMSG_PAYLOAD(nlh,0)>len ? len : NLMSG_PAYLOAD(nlh,0);
#ifdef _HUAYI_EVENTS_DBG_
                printf("[%d]:user rv len=%d\n", __LINE__,ret);
#endif /* _HUAYI_EVENTS_DBG_ */
                memcpy(buf, NLMSG_DATA(nlh), ret);
                break;
            }
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                struct nlmsgerr *errmsg;
                errmsg = NLMSG_DATA(nlh);
                if (errmsg->error) {
                    eventslog("recvmsg error=%d\n",errmsg->error);
#ifdef _HUAYI_EVENTS_DBG_
            printf("[%d]:recvmsg error=%d\n", __LINE__,errmsg->error);
#endif /* _HUAYI_EVENTS_DBG_ */
                    //break;
                } //else
goto recv_agn;             
            }
        }
    return ret;//huayi_msgq_comm_kernel(modID, sk, buf, len);
}

/*-------------------------------------------------------------------------*
 * Function:        huayi_events_send
 * Description:     for user space send a event to event handle module
 * Calls:           
 * Called By:       
 * Table Accessed:  
 * Table Updated:   
 * Input:           sk:socket descriptor;   eid:events id
                    buf:msg buffer;         len:buffer length
 * Output:          
 * Return:          
 * Others:          
 *-------------------------------------------------------------------------*/
int huayi_events_send(int fd, HUAYI_EVENTS_TYPE eid, char*buf, uint32_t len)
{
    return huayi_events_send2kernel(fd, eid, buf, len);
}

