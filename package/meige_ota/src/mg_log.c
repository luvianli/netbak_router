#include "std_c.h"


FILE *log_fp = NULL;
char log_buf[1024] = {0};
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;


