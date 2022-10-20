include $(TOPDIR)/.config

EXEC = cgi_huayi.cgi cgi_stream.cgi
HUAYI_OBJS = fastcgi_common.o fastcgi_main.o fastcgi_system.o fastcgi_network.o fastcgi_firewall.o fastcgi_wireless.o fastcgi_security.o fastcgi_basic.o
STREAM_OBJS = fastcgi_common.o fastcgi_stream.o fastcgi_updownload.o fastcgi_security.o
MODEM_OBJS = 

CFLAGS+=-O2 -Wall
INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_core

ifeq ($(CONFIG_PACKAGE_HUAYI_nginx),y)
LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lfcgi -lcjson -lz -lcrypto -lssl
else
LDFLAGS += -L. -lhuayi-universal -luci -lhuayiuci -lcjson  -lm
endif

ifeq ($(CONFIG_PACKAGE_huayi_dial4g),y)
EXEC+= cgi_modem.cgi
MODEM_OBJS += fastcgi_modem.o fastcgi_common.o fastcgi_security.o
endif

ifeq ($(CONFIG_HUAYI_SUPPORT_NO_LOGIN_OPTION),y)
EXEC+= cgi_no_login.cgi
NO_LOGIN_OBJS += fastcgi_no_login.o fastcgi_common.o fastcgi_security.o
endif

ifeq ($(CONFIG_PACKAGE_huayi_mysql),y)
LDFLAGS += -L$(STAGING_DIR)/usr/lib/mysql -lmysqlclient -lhuayi_mysql
endif

ifeq ($(CONFIG_PACKAGE_huayi_sqlite3),y)
LDFLAGS += -L$(STAGING_DIR)/usr/lib/ -lsqlite3 -lhuayi_sqlite3
endif

ifneq ($(findstring $(CONFIG_PACKAGE_pptpd)$(CONFIG_PACKAGE_xl2tpd),yy),)
HUAYI_OBJS += fastcgi_vpn.o
endif

all: $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

cgi_huayi.cgi:$(HUAYI_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)
	
cgi_stream.cgi:$(STREAM_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

cgi_modem.cgi:$(MODEM_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)
	
cgi_no_login.cgi:$(NO_LOGIN_OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

clean:
	@rm -rf *.cgi *.o 
	


 

