
LDFLAGS += -L$(STAGING_DIR)/usr/lib -lpthread -lhuayi-universal -luci -lhuayiuci -lm -lcjson

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal

all: check_network

OBJS = check_network.o 
	
check_network: $(OBJS)
	$(CC) $(OBJS) -o check_network $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
