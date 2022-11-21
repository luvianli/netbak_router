/**
 * @file huayi_log.h
 *
 * Define API for huayi_logs.
 *
 */
#include <stdio.h>
#include <stdlib.h>	
#include <sys/stat.h>


#ifndef __HUAYI_LOG_H__
#define __HUAYI_LOG_H__


#define HUAYI_UCI_LOG_SWITCH		0
/* UCI Log */
#define HUAYI_UCI_LOG(format, args...) \
	do{ \
		if(HUAYI_UCI_LOG_SWITCH)	\
		{\
			FILE *fp = fopen("/tmp/log/uci_log", "a"); \
			if(fp)\
			{\
				fprintf(fp, "%s@%s,%d] " format "\n", \
            	__FILE__, __func__,__LINE__, ##args ); \
			}\
			fclose(fp);\
		} \
	}while(0)

#define FASTCGI_LOG_SWITCH		1
			/* CGI Log */
#define FASTCGI_LOG(format, args...) \
	do{ \
		if(FASTCGI_LOG_SWITCH)	\
		{\
			FILE *fp = fopen("/tmp/log/fastcgi_log", "a"); \
			if(fp)\
			{\
				fprintf(fp, "%s@%s,%d] " format "\n", \
				__FILE__, __func__,__LINE__, ##args ); \
			}\
			fclose(fp);\
		} \
	}while(0)

#define CLOUD_CLIENT_LOG(format, args...) \
	do{ \
		int clientlog = 0; \
		huayi_uci_get_int(PKG_SYSTEM, "server", "clientlog", &clientlog); \
		if(1 == clientlog)	\
		{\
			FILE *fp = fopen("/tmp/log/cloud_client_log", "a"); \
			if(fp)\
			{\
				fprintf(fp, "%s@%s,%d] " format "\n", \
				__FILE__, __func__,__LINE__, ##args ); \
			}\
			fclose(fp);\
		} \
	}while(0)

#define CHECK_CLOUD_CLIENT_LOG() \
	do{ \
		struct stat statbuf; \
		stat("/tmp/log/cloud_client_log", &statbuf); \
		int size = statbuf.st_size;	\
		if(size > 4194304) \
		{\
			system("rm -fr /tmp/log/cloud_client_log"); \
		}\
	}while(0)

#define SIMCARD_CHANGE_LOG(format, args...) \
	do{ \
		int enlog = 0; \
		huayi_uci_get_int(PKG_SYSTEM, "simcard", "enlog", &enlog); \
		if(1 == enlog)	\
		{\
			FILE *fp = fopen("/tmp/log/simcard_change_log", "a"); \
			if(fp)\
			{\
				fprintf(fp, "%s@%s,%d] " format "\n", \
				__FILE__, __func__,__LINE__, ##args ); \
			}\
			fclose(fp);\
		} \
	}while(0)

#define CHECK_SIMCARD_CHANGE_LOG() \
	do{ \
		struct stat statbuf; \
		stat("/tmp/log/simcard_change_log", &statbuf); \
		int size = statbuf.st_size; \
		if(size > 4194304) \
		{\
			system("rm -fr /tmp/log/simcard_change_log"); \
		}\
	}while(0)


#define CON_LOG_SIN 1
#define CON_LOG(format, args...) \				
						do{ \
							if(CON_LOG_SIN) \
							{\
								FILE *fp = fopen("/tmp/log/con_log", "a"); \
								if(fp)\
								{\
									fprintf(fp, "%s@%s,%d] " format "\n", \
									__FILE__, __func__,__LINE__, ##args ); \
								}\
								fclose(fp);\
							} \
							struct stat buf;\
							int size=stat("/tmp/log/con_log", &buf);\
							if(size > 20000)\
							{\
								system("rm -fr /tmp/log/con_log");\
							}\
						}while(0)


#endif /* __HUAYI_LOG_H__ */
