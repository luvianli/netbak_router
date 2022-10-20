INCLUDES=-Iinclude
LDFLAGS+= -L

EXEC=libhuayi_udp.a
OBJS=CWmessage.o CWprotocol.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $(INCLUDES) $<

$(EXEC): $(OBJS)
	$(AR) curv $(EXEC) $(OBJS)

clean:
	@rm -rf *.a *.o
	
