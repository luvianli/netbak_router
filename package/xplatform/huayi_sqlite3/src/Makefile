INCLUDES=-Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal
LDFLAGS+= -L$(STAGING_DIR)/usr/lib -lsqlite3

EXEC=libhuayi_sqlite3.a
OBJS=huayi_sqlite3.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $(INCLUDES) $<

$(EXEC): $(OBJS)
	$(AR) curv $(EXEC) $(OBJS)
	#$(CC) -shared -o $@ $^

clean:
	@rm -rf *.a *.o
	