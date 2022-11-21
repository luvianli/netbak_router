EXES=huayi_multi_tcp_client

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)


INCLUDES+= -Iinclude -I$(CURDIR)/include -I$(STAGING_DIR)/usr/include/huayi-universal

LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lpthread -lcjson -lm


all: $(EXES)

$(EXES):$(OBJS)
	$(CC) -o $@ $(CPPFLAGS) $(OBJS) $(LDFLAGS) $(INCLUDES)
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
clean:
	rm -fr *.o $(EXES)