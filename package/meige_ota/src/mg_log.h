#ifndef MG_LOG_H
#define MG_LOG_H

extern pthread_mutex_t log_mutex;
extern FILE *log_fp;
extern char log_buf[];

#define openLog(on)     g_downInfo.debug = on

#define dprintf(fmt, args...) do { \
        int log_size = 0; \
        pthread_mutex_lock(&log_mutex); \
        log_size = snprintf(log_buf, 1024, "%s " fmt, get_time(), ##args); \
        if (log_fp) fwrite(log_buf, log_size, 1, log_fp); \
        if (log_fp != stdout) fwrite(log_buf, log_size, 1, stdout); \
        pthread_mutex_unlock(&log_mutex); \
    } while(0);

#define DBPrintf(fmt, args...) do { \
        if (g_downInfo.debug) \
        { \
            int log_size = 0; \
            pthread_mutex_lock(&log_mutex); \
            log_size = snprintf(log_buf, 1024, "%s " fmt, get_time(), ##args); \
            if (log_fp) fwrite(log_buf, log_size, 1, log_fp); \
            if (log_fp != stdout) fwrite(log_buf, log_size, 1, stdout); \
            pthread_mutex_unlock(&log_mutex); \
        }   \
    } while(0);

#define closeLog()  do { \
        if (log_fp && log_fp != stdout) fclose(log_fp); \
    } while(0);


#endif

