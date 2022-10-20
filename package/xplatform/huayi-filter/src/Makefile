#
# Makefile for huayi-filter module
#

all:
.PHONY: all

ccflags-y += $(HUAYI_INCS) $(HUAYI_COPTS) $(CFLAGS)

obj-m:= huayi_qos.o huayi_filter_hook.o

huayi_filter_hook-objs:= vsf_hook.o

huayi_qos-objs:= qos_main.o
	
huayi_qos:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) $(huayi_qos-objs) 
	
huayi_filter_hook:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) $(huayi_filter_hook-objs)
	
all:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules


clean:
	rm -rf *.o *.mod 