EXEC = huayi_mqtt_pub huayi_mqtt_sub
PUB_OBJS = huayi_mqtt_pub.o
SUB_OBJS = huayi_mqtt_sub.o

CFLAGS+= -Wall
CFLAGS+= -O2 
INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_core


LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lcjson -lm -lmosquitto

all: $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

huayi_mqtt_pub:$(PUB_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

huayi_mqtt_sub:$(SUB_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

clean:
	@rm -rf *
	
