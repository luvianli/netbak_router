TARGETS = huayi_capwap_client
OBJS    += WTPepoll.o WTPtimer.o WTPsocket.o WTPmain.o WTPcommon.o WTPhandlereload.o
OBJS    += WTPstate.o WTPelement.o WTPelementuser.o WTPmessagehandler.o

CFLAGS+=-O2 -Wall
INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include
LDFLAGS += -L. -lhuayi_udp -lpthread -luci -lhuayi-universal -lhuayievents -lcjson -lhuayiuci -lm -lz


all: $(TARGETS)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
	
$(TARGETS): $(OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)
	
clean:
	rm -f *.o $(TARGETS) *.d 