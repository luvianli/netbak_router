#
# Makefile for COMLANOS HUAYI STATUS OUTPUT Module (HUAYI_STATUS)
#

all: huayi_runstate
.PHONY: all

obj-m += huayi_runstate.o 

ccflags-y += $(HUAYI_INCS) $(HUAYI_COPTS) $(CFLAGS)

huayi_runstate:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd) modules

clean:
	rm -rf *.o *.mod
