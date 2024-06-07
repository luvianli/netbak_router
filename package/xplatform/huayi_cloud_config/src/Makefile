CFLAGS+=-O2 -Wall -fPIC
INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_core -I$(STAGING_DIR)/usr/include
LDFLAGS += -L. -lhuayi-universal -luci -lcjson -lstrutils

EXEC=libhyclconf.so

OBJS=sysinfo_config.o 4Ginfo_config.o wireless_config.o ota_upgrade.o network_config.o base64.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(EXEC): $(OBJS)
	$(CC) -shared -o $@ $^

all: $(EXEC)
	
clean:
	@rm -rf *.so *.o
	
