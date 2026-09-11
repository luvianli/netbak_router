
LDFLAGS += -L$(STAGING_DIR)/usr/lib -lpthread -lhuayi-universal -luci -lhuayiuci -lm -lcjson

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal

all: wisp_rep

OBJS = wisp_rep.o

wisp_rep: $(OBJS)
	$(CC) $(OBJS) -o wisp_rep $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
