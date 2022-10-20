TARGETS = huayi_dhcp_detect
OBJS    = send_probe.o jobqueen.o 

CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L. -lpthread

all: $(TARGETS)

%.o : %.c
	$(CC) $(CFLAGS) -Wp,-MD,$@.d -c -o $@ $^
	
$(TARGETS): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	
clean:
	rm -f *.o $(TARGETS) *.d


