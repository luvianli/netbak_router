EXEC = huayi_mqtt_pub huayi_mqtt_sub huayi_mqtt_content huayi_mqtt_setsub
PUB_OBJS = huayi_mqtt_pub.o
SUB_OBJS = huayi_mqtt_sub.o
CON_OBJS = huayi_mqtt_content.o
SET_OBJS = huayi_mqtt_setsub.o

CFLAGS+= -Wall
CFLAGS+= -O2 -fPIC
INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_core -I$(STAGING_DIR)/usr/include

LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lcjson -lm -lmosquitto -lstrutils

all: $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

huayi_mqtt_pub:$(PUB_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

huayi_mqtt_sub:$(SUB_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

huayi_mqtt_content:$(CON_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

huayi_mqtt_setsub:$(SET_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)	
clean:
	@rm -rf *
	
