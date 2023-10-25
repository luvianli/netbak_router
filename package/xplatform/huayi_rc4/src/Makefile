
LDFLAGS += -L$(STAGING_DIR)/usr/lib -lpthread -lhuayi-universal -luci -lhuayiuci -lm -lcjson

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal

all: huayi_rc4

OBJS = huayi_rc4.o 
	
huayi_rc4: $(OBJS)
	$(CC) $(OBJS) -o huayi_rc4 $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
