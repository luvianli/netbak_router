CC := gcc
CFLAGS := -Wall -O2
LDFLAGS := -L$(STAGING_DIR)/usr/lib -lpthread -lm
INCLUDES += -I$(STAGING_DIR)/usr/include

SRC := meig_wireless_update.c
TARGET := meige_ota

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) 