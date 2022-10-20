EXEC = huayi_core huayi-reset

#CORE_OBJS = huayi_core_main.o huayi_socket.o huayi_eloop.o huayi_utimer_functions.o huayi_core_queue.o
#CORE_OBJS += huayi_handle_reload.o huayi_reload_system.o huayi_reload_network.o huayi_reload_firewall.o huayi_reload_qos.o huayi_reload_vpn.o
#RELOAD_OBJS = huayi_reload.o

CORE=$(wildcard core/*.c)
EVENT_HANDLE=$(wildcard events_handle/*.c)
RELOAD_HANDLE=$(wildcard reload_handle/*.c)
RELOAD_POST=$(wildcard reload_post/*.c)
TIMER_TASK=$(wildcard timer_task/*.c)


INCLUDES+=-Iinclude -I$(CURDIR)/include -I$(STAGING_DIR)/usr/include/huayi-universal
CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lpthread -lcjson -lm


all: $(EXEC)

#%.o : %.c
#	$(CC) $(CFLAGS) $(INCLUDES) -Wp,-MD,$@.d -c  -o $@ $^

huayi_core:$(CORE_OBJS)
	#$(CC) -s -o $@ $^ $(LDFLAGS)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) $(CORE) $(EVENT_HANDLE) $(RELOAD_HANDLE) $(TIMER_TASK)	
	

huayi-reset:$(RELOAD_OBJS)
	#$(CC) -s -o $@ $^ $(LDFLAGS)
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) $(RELOAD_POST)

clean:
	rm -f *.o $(TARGETS) *.d