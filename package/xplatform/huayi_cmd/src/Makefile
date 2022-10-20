EXEC = huayi_cmd huayi_mac huayi_sn huayi_msg
CMD_OBJS = huayi_cmd.o

MAC_OBJS = huayi_mac.o

SN_OBJS = huayi_sn.o

MSG_OBJS = huayi_msg.o

CFLAGS+= -Wall
CFLAGS+= -O2 
INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_core


LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lcjson -lm

all: $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

huayi_cmd:$(CMD_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)
	
huayi_mac:$(MAC_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)
	
huayi_sn:$(SN_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)
	
huayi_msg:$(MSG_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

clean:
	@rm -rf *
	
