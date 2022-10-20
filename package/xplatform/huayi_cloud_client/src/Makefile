EXEC = hy_cloud_client

CLOUD-CLIENT=$(wildcard ./*.c)


INCLUDES+=-Iinclude -I$(CURDIR)/include -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_cloud_config
INCLUDES+=-I$(STAGING_DIR)/usr/include/curl -I$(STAGING_DIR)/usr/include/
CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lpthread -lcjson -lm -lhyclconf -lcurl -lstrutils


all: $(EXEC)

#%.o : %.c
#	$(CC) $(CFLAGS) $(INCLUDES) -Wp,-MD,$@.d -c  -o $@ $^

$(EXEC):
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) $(CLOUD-CLIENT)	
	
clean:
	rm -f *.o $(TARGETS) *.d