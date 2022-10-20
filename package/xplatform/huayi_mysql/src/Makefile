INCLUDES=-Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal
LDFLAGS+= -L$(STAGING_DIR)/usr/lib/mysql -lmysqlclient

EXEC=libhuayi_mysql.a
OBJS=huayi_mysql.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $(INCLUDES) $<

$(EXEC): $(OBJS)
	$(AR) curv $(EXEC) $(OBJS)
	#$(CC) -shared -o $@ $^

clean:
	@rm -rf *.a *.o
	