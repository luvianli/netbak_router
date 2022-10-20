EXEC = hy_sc

CLOUD-CLIENT=$(wildcard ./*.c)


INCLUDES+=-Iinclude -I$(CURDIR)/include -I$(STAGING_DIR)/usr/include/huayi-universal
INCLUDES+=-I$(STAGING_DIR)/usr/include/
CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lpthread -lcjson -lm


all: $(EXEC)

#%.o : %.c
#	$(CC) $(CFLAGS) $(INCLUDES) -Wp,-MD,$@.d -c  -o $@ $^

$(EXEC):
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) $(CLOUD-CLIENT)	
	
clean:
	rm -f *.o $(TARGETS) *.d