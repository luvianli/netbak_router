
LDFLAGS += -L$(STAGING_DIR)/usr/lib -lpthread -lhuayi-universal -luci -lhuayiuci -lm -lcjson

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal

all: check_sim

OBJS = check_sim.o 
	
check_sim: $(OBJS)
	$(CC) $(OBJS) -o check_sim $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
