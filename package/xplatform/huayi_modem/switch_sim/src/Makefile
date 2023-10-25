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


#QL_LIB+=$(QL_COMMON_LIB)/libql_peripheral.a  $(QL_COMMON_LIB)/libql_common_api.a

SRCS += switch_sim.c
OBJS= $(SRCS:.c=.o)

LDFLAGS += -L$(STAGING_DIR)/usr/lib -lpthread -lhuayi-universal -luci -lhuayiuci -lm -lcjson

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal

TARGET=switch_sim

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(EXT_CFLAGS) -c -o $@ $<

#$(TARGET):$(OBJS) $(QL_LIB)
$(TARGET):$(OBJS)
	$(CC)  $(LDFLAGS) $(EXT_LDFLAGS) -o $@ $^ 

clean:
	rm -rf *.o $(TARGET)
