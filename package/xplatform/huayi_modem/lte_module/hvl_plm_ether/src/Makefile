################################################################################
# 
# Copyright(c) Realtek Semiconductor Corp. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation; either version 2 of the License, or (at your option) 
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 
# Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
# The full GNU General Public License is included in this distribution in the
# file called LICENSE.
# 
################################################################################

KVER		:= $(shell uname -r)
KDIR		:= /lib/modules/$(KVER)/build
KMISC		:= /lib/modules/$(KVER)/kernel/drivers/net/usb
KEXT		:= $(shell echo $(KVER) | sed -ne 's/^2\.[567]\..*/k/p')o
KBUILD_EXTRA_SYMBOLS:=/usr/src/linux-headers-3.11.0-15-generic/Module.symvers


modules:
	#$(PWD)/src/add_header.sh  "modules" "$(KDIR)/include/linux/usb"
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD)/src modules
#	strip --strip-debug qmi_oper.$(KEXT)
	strip --strip-debug hvl_plm_ether.$(KEXT)
	

clean:
	rm -rf *.o *.ko *~ core* .dep* .*.d .*.cmd *.mod.c *.a *.s .*.flags .tmp_versions Module.symvers Modules.symvers *.order

	$(PWD)/src/add_header.sh  "clean" "$(KDIR)/include/linux/usb"
#	rmmod -f hvl_plm_ether

install:
	#install -m 744 -c hvl_plm_ether.$(KEXT) $(KMISC)
	depmod -a
	#modprobe hvl_plm_ether
	$(PWD)/src/add_header.sh  "install"
	modprobe hvl_plm_ether
#hvl_plm_ether.o


hvl_plm_ether-objs +=  qmi_oper.o \
		plm_cdc_ether.o 
                  
                  
obj-m +=  hvl_plm_ether.o

#obj-m += 
