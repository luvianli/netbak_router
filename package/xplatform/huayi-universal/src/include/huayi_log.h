/**
 * @file huayi_log.h
 *
 * Define API for huayi_logs.
 *
 */
#include <stdio.h>
#include <stdlib.h>	
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>


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
		if(size > 100000) \
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
									time_t currentTime = time(NULL);\
									struct tm *localTime = localtime(&currentTime);\
									fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] " format "\n", \
                                            localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, \
                                            localTime->tm_hour, localTime->tm_min, localTime->tm_sec, \
											##args ); \
								}\
								fclose(fp);\
							} \
							struct stat buf;\
							int size=stat("/tmp/log/con_log", &buf);\
							if(size > 1048576)\
							{\
								system("rm -fr /tmp/log/con_log");\
							}\
						}while(0)

#define SOCKET_LOG_SIN 1
#define CLIENT_SOCKET_LOG(format, args...) \
                            do { \
                                if (SOCKET_LOG_SIN) { \
                                    FILE *fp = fopen("/tmp/log/client_socket_log", "a"); \
                                    if (fp) { \
                                        time_t currentTime = time(NULL);\
                                        struct tm *localTime = localtime(&currentTime);\
                                        fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s@%s,%d] " format "\n", \
                                            localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, \
                                            localTime->tm_hour, localTime->tm_min, localTime->tm_sec, \
                                            __FILE__, __func__, __LINE__, ##args ); \
                                        fclose(fp); \
                                    } \
                                } \
                                struct stat buf; \
                                int size = stat("/tmp/log/client_socket_log", &buf); \
                                if (size > 9000) { \
                                    system("rm -fr /tmp/log/client_socket_log"); \
                                } \
                            } while (0)
#define SERVER_SOCKET_LOG(format, args...) \
                            do { \
                                if (SOCKET_LOG_SIN) { \
                                    FILE *fp = fopen("/tmp/log/server_socket_log", "a"); \
                                    if (fp) { \
                                        time_t currentTime = time(NULL);\
                                        struct tm *localTime = localtime(&currentTime);\
                                        fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s@%s,%d] " format "\n", \
                                            localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, \
                                            localTime->tm_hour, localTime->tm_min, localTime->tm_sec, \
                                            __FILE__, __func__, __LINE__, ##args ); \
                                        fclose(fp); \
                                    } \
                                } \
                                struct stat buf; \
                                int size = stat("/tmp/log/server_socket_log", &buf); \
                                if (size > 9000) { \
                                    system("rm -fr /tmp/log/server_socket_log"); \
                                } \
                            } while (0)
#define WISP_NETWORK_LOG_SIN 1
#define WISP_NETWORK_LOG(format, args...) \
                            do { \
                                if (WISP_NETWORK_LOG_SIN) { \
                                    FILE *fp = fopen("/tmp/log/wisp_network_log", "a"); \
                                    if (fp) { \
                                        time_t currentTime = time(NULL);\
                                        struct tm *localTime = localtime(&currentTime);\
                                        fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s@%s,%d] " format "\n", \
                                            localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, \
                                            localTime->tm_hour, localTime->tm_min, localTime->tm_sec, \
                                            __FILE__, __func__, __LINE__, ##args ); \
                                        fclose(fp); \
                                    } \
                                } \
                                struct stat buf; \
                                int size = stat("/tmp/log/wisp_network_log", &buf); \
                                if (size > 20000) { \
                                    system("rm -fr /tmp/log/wisp_network_log"); \
                                } \
                            } while (0)


#define SWITCH_SIM_LOG_SIN 1
                                                
#define SWITCH_SIM_LOG(format, args...) \
                                                    do { \
                                                        if (SWITCH_SIM_LOG_SIN) { \
                                                            time_t rawtime; \
                                                            struct tm *timeinfo; \
                                                            char timeString[20]; \
                                                            time(&rawtime); \
                                                            timeinfo = localtime(&rawtime); \
                                                            strftime(timeString, sizeof(timeString), "%Y.%m.%d-%H:%M:%S", timeinfo); \
                                                            FILE *fp = fopen("/tmp/log/switch_sim_log", "a"); \
                                                            if (fp) { \
                                                                fprintf(fp, "[%s] " format "\n", timeString, ##args); \
                                                                fclose(fp); \
                                                            } \
                                                        } \
                                                        struct stat buf; \
                                                        int size = stat("/tmp/log/switch_sim_log", &buf); \
                                                        if (size > 5000) { \
                                                            system("rm -fr /tmp/log/switch_sim_log"); \
                                                        } \
                                                    } while (0)


#define INTERNET_LOG_SIN 1
#define INTERNET_LOG(format, args...) \
                                                        do { \
                                                            if (INTERNET_LOG_SIN) { \
                                                                FILE *fp = fopen("/tmp/log/internet_log", "a"); \
                                                                if (fp) { \
                                                                    time_t currentTime = time(NULL);\
                                                                    struct tm *localTime = localtime(&currentTime);\
                                                                    fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s@%s,%d] " format "\n", \
                                                                        localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday, \
                                                                        localTime->tm_hour, localTime->tm_min, localTime->tm_sec, \
                                                                        __FILE__, __func__, __LINE__, ##args ); \
                                                                    fclose(fp); \
                                                                } \
                                                            } \
                                                            struct stat buf; \
                                                            int size = stat("/tmp/log/internet_log", &buf); \
                                                            if (size > 9000) { \
                                                                system("rm -fr /tmp/log/internet_log"); \
                                                            } \
                                                        } while (0)
                      
#define NETCHECK_LOG(format, args...) \
                        do { \
                            if (1) { \
                                FILE *fp = fopen("/etc/netcheck.log", "a"); \
                                if (fp) { \
                                    time_t currentTime = time(NULL); \
                                    struct tm *t = localtime(&currentTime); \
                                    fprintf(fp, "[%d/%02d/%02d %02d:%02d:%02d]" format "\n", \
                                            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, \
                                            t->tm_hour, t->tm_min, t->tm_sec, ##args); \
                                    fclose(fp); \
                                    struct stat buf; \
                                    if (stat("/etc/netcheck.log", &buf) == 0) { \
                                        if (buf.st_size > (10000)) { \
                                            system("rm -rf /etc/netcheck.log"); \
                                        } \
                                    } \
                                } \
                            } \
                        } while (0)

#endif /* __HUAYI_LOG_H__ */
