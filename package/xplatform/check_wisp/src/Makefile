
LDFLAGS += -L$(STAGING_DIR)/usr/lib -lpthread -lhuayi-universal -luci -lhuayiuci -lm -lcjson

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal

all: check_wisp

OBJS = check_wisp.o 
	
check_wisp: $(OBJS)
	$(CC) $(OBJS) -o check_wisp $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
