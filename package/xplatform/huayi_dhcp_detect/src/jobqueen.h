/*******************************************************************************************
 * Project:  jobqueen                                                                      *
 *                                                                                         *
 * Author :  hpluo									                                       *  
 *  																					   * 
 * Changelist: 																		       *
 * 20161011 add by hpluo  Version 0.0 													   *
 ******************************************************************************************/
#ifndef __JOB_QUEEN_HEADER__
#define __JOB_QUEEN_HEADER__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/sysinfo.h>

/*______________________________________________________*/
/*  *******************___DEFINE___*******************  */
#define JOB_DEFAULT_IDLE_TIME 10
#define JOB_MIN_WAIT_TIME 1
#define JOB_SOCKET_LIST_SIZE 10
#define JOB_EVENT_TIMEOUT 0
#define JOB_SOCKET_NO_ACTIVITY 0
#define JOB_QUEEN_SIZE 255

typedef enum {
	J_FALSE = 0,
	J_TRUE = 1
} JOB_BOOL;

typedef enum {
	PTH_FALSE = 0,
	PTH_TRUE = 1
} PTH_BOOL;

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */
typedef enum {
	JOB_HEART_BEAT,
} JobQueen;

struct JobSockets{
	int socket;
	JOB_BOOL used;
	void (*processSocketFunc)(void*);
	void* argument;
};

struct{
	struct JobSockets socketArray[JOB_SOCKET_LIST_SIZE];
	long nextEventTime;
	int maxSocketValue;
	JOB_BOOL debug;
}JobSocketsList;

struct JOB_QUEEN {
    JobQueen job;
    JOB_BOOL isRunning;
	long nextEventTime; /*seconds*/
	JOB_BOOL (*processFunc)(void*);
	void (*printFunc)(void*);
	void* argumentPointer;
	struct JOB_QUEEN* pNextJob;
};

/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */
void JobQueueInit(void);
char* JobNameGet(JobQueen job);
void JobQueueInsertJobToQueue(struct JOB_QUEEN* newJob);
void JobQueueRemoveJobFromQueue(struct JOB_QUEEN* expiredJob);
void JobQueueAddJob(unsigned int secondOffset, JobQueen job, JOB_BOOL (*processFunc)(void*), void (*printFunc)(void*), void* argumentPointer);
void JobQueueDeleteFirstJobByType(JobQueen job);
void JobQueueDeleteAllJob(void);
void JobQueuePrintJob(void);
JOB_BOOL JobQueueRegisterSocket(int socket, void (*processFunc)(void*), void* argument);
JOB_BOOL JobQueueRemoveSocket(int socket);
void JobQueueMaxSocketValueUpdate(void);
void JobQueueNextJobEventTimeGet(long* nextEventTime);
void JobQueueExecutionLoop(void);
void JobQueueExcuteJob(void);
void WaitForNextJob(long timeout);
void JobQueueDebug(JOB_BOOL enable);
#endif
