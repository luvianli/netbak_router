########################################################
#  Copyright (C) 2018-2099 Quectel Wireless Co.,Ltd. 
#  Quectel Wireless Proprietary and Confidential.
#
#  Compile test_api.
#  Created by Nebula.Li 2019-07-24
#
########################################################

#_COMMON_BEGIN
#_COMMON_END
#LDFLAGS += -L$(STAGING_DIR)/usr/lib  -lhuayi-universal -luci -lhuayiuci -lm -lcjson
#LDFLAGS += -L$(STAGING_DIR)/usr/lib
#LDFLAGS += -L. -lhuayi-universal -lpthread -lcjson -lm
#LDFLAGS += -L$(STAGING_DIR)/usr/lib  -lhuayi-universal
INCLUDES+= -Iinclude -I$(CURDIR)/include -I$(STAGING_DIR)/usr/include/huayi-universal
#LDFLAGS += -L. -lhuayi-universal -lhuayiuci
LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lpthread -lcjson -lm

SRCS=$(wildcard *.c)
#通配符 获取工作目录下的所有的.c文件列表
OBJS=$(SRCS:.c=.o)
#把所有的*.c文件替换为*.o文件.等于$(patsubst %.c, %.o, $(mixed))。
#OBJS = switch_sim.o 
EXES=switch_sim

all: $(EXES)
	
$(EXES):$(OBJS)
	#$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS) $(INCLUDES)
	$(CC) -o $@ $(CPPFLAGS) $(OBJS) $(LDFLAGS) $(INCLUDES) 

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
