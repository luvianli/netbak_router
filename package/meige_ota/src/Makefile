EXEC = meige_ota

MEIGE_OTA_SRCS = $(wildcard ./*.c)

INCLUDES += -I$(STAGING_DIR)/usr/include/
CFLAGS += -Wall -fno-strict-aliasing
#LDFLAGS += -lpthread -lcjson

all: $(EXEC)

$(EXEC):
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(INCLUDES) $(MEIGE_OTA_SRCS)

clean:
	rm -f *.o $(EXEC) *.d