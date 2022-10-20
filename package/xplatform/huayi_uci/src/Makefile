CFLAGS+=-O2 -Wall -fPIC
INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal
LDFLAGS += -L. -lhuayi-universal -luci -lcjson


EXEC=libhuayiuci.so

OBJS=huayi_uci.o huayi_uci_fun.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(EXEC): $(OBJS)
	$(CC) -shared -o $@ $^

all: $(EXEC)
	
clean:
	@rm -rf *.so *.o
	