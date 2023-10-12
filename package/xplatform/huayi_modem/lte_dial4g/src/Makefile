
LDFLAGS += -L$(STAGING_DIR)/usr/lib -lsqlite3 -lhuayi_sqlite3 -lpthread -lhuayi-universal -luci -lhuayiuci -lm -lcjson

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_core

all: huayi_dial4g quectel_cm huayi_dial4g_init

OBJS = main.o huayi_sms.o quectel_dial4g.o neoway_dial4g.o slm_dial4g.o
OBJS_INIT = hy_dial4g_init.o

#OBJS_WAN = wan_policy.o

huayi_dial4g: $(OBJS)
	$(CC) $(OBJS) -o huayi_dial4g $(LDFLAGS)
	
huayi_dial4g_init: $(OBJS_INIT)
	$(CC) $(OBJS_INIT) -o huayi_dial4g_init $(LDFLAGS)
	
#wan_policy: $(OBJS_WAN)
#	$(CC) $(OBJS_WAN) -o wan_policy $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

quectel_cm:
	$(MAKE) -C quectel-CM
