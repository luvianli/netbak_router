#
# Makefile for Driver Abstraction Layer (DAL)
#

all: huayi-kalarm #test-huayi-alarm
.PHONY: all

#huayi-kalarm-objs := huayi-kalarm.o
obj-m += huayi-kalarm.o

ccflags-y += $(HUAYI_INCS) $(HUAYI_COPTS) $(CFLAGS)

huayi-kalarm:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

#test-huayi-alarm:
#	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -lhuayievents test-huayi-alarm.c

clean:
	rm -rf *.o *.mod
