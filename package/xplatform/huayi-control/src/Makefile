obj-m +=huayi_mod.o

ccflags-y += $(HUAYI_INCS) $(HUAYI_COPTS) $(CFLAGS)

.PHONY: all
all:
	$(MAKE) -C $(KERNELPATH) SUBDIRS=$(shell pwd)  modules

clean:
	rm -rf *.o *.cmd *.ko .tmp_versions
