TARGETS = yddns
OBJS = ddns_main.o provider.o md5.o base64.o blowfish.o cypher.o

INCLUDES+=-Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal
CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L. -lpthread -lm -lhuayi-universal -luci -lhuayiuci -ldl

all: $(TARGETS)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -Wp,-MD,$@.d -c -fpic -o $@ $^

$(TARGETS): $(OBJS)
	$(CC) -o  $@ $^ $(LDFLAGS)
	
clean:  
	rm -f *.o $(TARGETS) *.d 
