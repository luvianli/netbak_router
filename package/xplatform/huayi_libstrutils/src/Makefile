	
CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libstrutils.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=libutils.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -o $@ $^
