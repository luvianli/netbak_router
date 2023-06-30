EXEC = huayi_detect_network

INTERNET_STRATEGY=$(wildcard ./*.c)


INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_core
CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lpthread -lcjson -lm


all: $(EXEC)

#%.o : %.c
#	$(CC) $(CFLAGS) $(INCLUDES) -Wp,-MD,$@.d -c  -o $@ $^

$(EXEC):
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) $(INTERNET_STRATEGY)	
	
clean:
	rm -f *.o $(TARGETS) *.d