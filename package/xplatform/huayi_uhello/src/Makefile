INCLUDES+=-Iinclude -I$(STAGING_DIR)/usr/include/huayi_core
TARGETS = huayi_uhello
OBJS    = huayi_uhello.o

CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L.
 
all: $(TARGETS)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -Wp,-MD,$@.d -c -o $@ $^
	
$(TARGETS): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	
clean:
	rm -f *.o $(TARGETS) *.d 