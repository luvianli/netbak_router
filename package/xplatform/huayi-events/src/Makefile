#
# Makefile for huayi events
#

ifneq ($(BUILD_USERSPACE),y)

obj-m += huayi-events.o

huayi-events-objs := huayi_events.o 

ccflags-y += $(HUAYI_INCS) #$(HUAYI_COPTS)
asflags-y += $(HUAYI_ASFLAGS)
ldflags-y += $(HUAYI_LDOPTS)

.PHONY: mods
mods:
	$(MAKE) -C $(KERNELPATH) M=$(shell pwd) modules

else  #($(BUILD_USERSPACE),y)

BUILD_STATIC := n

STATIC:= libhuayievents.a
DYNAMIC:= libhuayievents.so
OBJS := huayi_events_lib.o
ifeq ($(BUILD_STATIC),y)
LIBSMGQ := $(STATIC)
else
LIBSMGQ := $(DYNAMIC)
endif
#XCFLAGS=-Os -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow \
	-Wpointer-arith -Wcast-qual -Winline -I. -g


.PHONY: libhuayievents
libhuayievents:$(OBJS) $(DYNAMIC) #testlib
#	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(OBJS)
	
%: %.o
	$(CC) $(LDFLAGS) $(CFLAGS) -fPIC $(CPPFLAGS) $(HUAYI_COPTS) -o $@ $^
%.o: %.c 
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC $(HUAYI_COPTS) -o $@ -c $<


.PHONY: testevent
testevent:
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -lhuayievents testevent.c
	#$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(HUAYI_COPTS) -o $@ -lhuayievents testevent.c
#	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@  testevent.c
	
# Compilation of the dynamic library
#$(DYNAMIC): $(OBJS:.o=.so)
$(DYNAMIC): $(OBJS)
	$(CC) -shared -o $@ -Wl,-soname,$@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -fPIC $(HUAYI_COPTS) -lc $^

# Compilation of the static library
$(STATIC): $(UOBJS:.o=.so)
	$(RM) $@
	$(AR) cru $@ $^
	$(RANLIB) $@

	
#.PHONY: install
#install:all

endif #($(BUILD_USERSPACE),y)

clean:
	rm -rf *.o *.mod
