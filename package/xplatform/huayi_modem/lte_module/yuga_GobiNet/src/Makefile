ifneq ($(KERNELRELEASE),)
obj-m := yuga_GobiNet.o
yuga_GobiNet-objs := GobiUSBNet.o QMIDevice.o QMI.o
else

PWD := $(shell pwd)
KVR ?= $(shell uname -r)
#KDIR :=/home/xiaozhenhe/RK/rk3288-box/kernel
KDIR :=/lib/modules/$(KVR)/build
#KDIR := ~/linux-2.6.21
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.ko *.o *.cmd
endif


