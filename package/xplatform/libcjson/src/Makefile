CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libcjson.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=cJSON.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -o $@ $^