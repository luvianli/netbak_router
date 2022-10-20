
WFLAGS=-Wall
CFLAGS=-O2 
INCLUDES=-Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal
LDFLAGS=-lhuayi-universal -luci -lhuayiuci -lm -lcjson
EXEC=huayi_proto_detect

all: $(EXEC)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

OBJ=check_wanlinktype.o dhcpcheck.o pppoecheck.o

$(EXEC): $(OBJ)
	$(CC) -o $@  $^ $(LIBS) $(LDFLAGS)
