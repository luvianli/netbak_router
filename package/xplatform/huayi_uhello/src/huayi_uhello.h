#ifndef __HUAYI_UHELLO_HEADER__
#define __HUAYI_UHELLO_HEADER__

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/un.h>
#include <errno.h>
#include <unistd.h> 

#include "huayi_config_common.h"


typedef enum {
	X_FALSE = 0,
	X_TRUE = 1
} Uhello_Bool;



#define HELLO_LOG(format, args...) \
	do{ \
		if(1)	\
		{\
			FILE *fp = fopen("/tmp/log/hello_log", "a"); \
			if(fp)\
			{\
				fprintf(fp, "%s@%s,%d] " format "\n", \
				__FILE__, __func__,__LINE__, ##args ); \
			}\
			fclose(fp);\
		} \
	}while(0)

#endif /* HELLO_LOG */

