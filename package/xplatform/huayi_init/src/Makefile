INCLUDES+=-Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal
TARGETS = huayi_init 
INIT_OBJS = huayi_init.o

CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lcjson -lm

all: $(TARGETS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

huayi_init:$(INIT_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS) 

clean:
	rm -f *.o $(TARGETS) *.d

