#
# Makefile for huayi-universal module
#

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal

all: huayi-k-universal huayi-u-universal
.PHONY: all

obj-m += huayi_proc.o huayi_dal.o 

ccflags-y += $(HUAYI_INCS) $(HUAYI_COPTS) $(CFLAGS)

huayi-k-universal:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules


huayi-u-universal: libhuayi-universal
.PHONY: huayi-u-universal

libhuayi-universal:
	$(CC) -shared -s -o libhuayi-universal.so -Wl,-soname,libhuayi-universal.so $(CFLAGS) $(INCLUDES) -fPIC $(LDFLAGS) -lc huayi_lock.c huayi_atcmd.c

clean:
	rm -rf *.o *.mod *.so
