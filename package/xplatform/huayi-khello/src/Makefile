#
# Makefile for Statistic Module (STT)
#

all = huayi_khello.o

obj-m += $(all)

ccflags-y += $(HUAYI_INCS) $(HUAYI_COPTS) $(CFLAGS)


.PHONY: all
all:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

clean:
	rm -rf *.o *.mod
