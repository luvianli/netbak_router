/**
 * @testlib.c
 * this file is used to test msgQ work!
 * the user send a string to kernel and then kernel reversal the string and 
 * send to user process
 *
 * if want to use the sample file,u need to turn on _MSGQ_DBG_ macro and 
 * modify the Mikefile line 34(remove the '#')
 */

#include <stdlib.h> 
#include <errno.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <sys/stat.h> 

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>

#include <huayi-universal/huayi_events.h>
#include <huayi-universal/huayi_log.h>

#define eventslog_l(level, fmt, arg...) do { \
 ; }while (0)

/**
 * send a string to kernel and reversal it 
 * 
 *
 * @return 
 *  NULL
 */
int main(int argc, char* argv[])
{
#ifdef _HUAYI_EVENTS_DBG_
    int i,sk,len;
    char buf[ALT_MAX_PAYLOAD];
    HUAYI_EVENTS_TYPE eid[3] = {HUAYI_EVENTS_SYS/*,HUAYI_EVENTS_SYS2,HUAYI_EVENTS_SYS3*/};

    if (argc!=2) {
        printf("this binary file just a demo for how to use msgq,it will reversal the <string>\n"
               "if u want to use it,u must open the macro which defined huayi_msgq.h\n"
               "usage:\n\ttestlib <string>\n");
        return 0;
    }
    
    sk = huayi_events_create();
    huayi_events_add(eid,3,sk);
        
    eventslog("create smgq(%d)\n", sk);
    
    //sprintf(buf,"%s",argv[1]);
    for (i=0; 1; i++) {
        sprintf(buf,"%si=%d",argv[1],i);

        len = huayi_events_send(sk, HUAYI_EVENTS_SYS, buf, ALT_MAX_PAYLOAD);
        //len = huayi_events_send(sk, HUAYI_EVENTS_SYS2, buf, ALT_MAX_PAYLOAD);
        //len = huayi_events_send(sk, HUAYI_EVENTS_SYS3, buf, ALT_MAX_PAYLOAD);

        
        printf("main loop[i=%d]  len:%d %s\n",i,len, buf);
        
        //len = huayi_events_recv(sk,-1, buf, ALT_MAX_PAYLOAD);
        if (len<0) {
            printf("error occur when receive message\n");
        } else if (0 == len)
            printf("time out when receive message\n");
        else {
            printf("receive %d bytes\n",len);
            printf("%s\n",buf);
        }
        sleep(1);
    }

    huayi_events_del(eid,3,sk);
    huayi_events_destroy(sk);

#else
#if  0
    //printf("please turn on macro '_HUAYI_EVENTS_DBG_' and rebuild again\n"); 
    struct msgtype msg; 
    int msgid[HUAYI_MSG_KEYMAX-HUAYI_MSG_KEY1];
    HUAYI_MSG_KEY key;
    int sk,len;
    char buf[BUF_SIZE];

    for (key=HUAYI_MSG_KEY1; key<HUAYI_MSG_KEYMAX; ++key) {
        if(-1 == (msgid[key-HUAYI_MSG_KEY1] =
            msgget(key, S_IRUSR|S_IWUSR|IPC_CREAT|IPC_EXCL))) {
            fprintf(stderr, "Creat Message Error:%s\n", strerror(errno));
            exit(1);
        }
    }
    
     while(1) {
        sk = huayi_events_register(HUAYI_EVENTS_SYS);
        len = huayi_events_recv(sk,-1, buf, BUF_SIZE);

        msg.mtype = 1;
        memcpy(msg.mtext, buf, BUF_SIZE);
        msgsnd(msgid, &msg, sizeof(struct msgtype), 0);   
     }
#endif /* 0 */
#endif /* _HUAYI_EVENTS_DBG_ */
    return 0;
}

