##########################################################################
## 	Please select one of HOST_TYPE(EMBEDDED or LINUX_PC) in Makefile.
##	You need to modify COMPILER_PATH and COMPILER_PREFIX appropriately

HOST_TYPE=LINUX_PC
#HOST_TYPE=EMBEDDED

##########################################################################

ifeq ($(HOST_TYPE),EMBEDDED)
COMPILER_PATH = /opt/mips-openwrt-linux-uclibc/usr/bin/
COMPILER_PREFIX =  mips-openwrt-linux-uclibc-
CFLAGS  += -D__UCLINUX__
else
COMPILER_PATH = 
COMPILER_PREFIX =
endif

CFLAGS	+= -Wall -Wextra -Wno-unused -Wconversion
CFLAGS	+= -g -O2

INCLUDES+= -Iinclude -I$(STAGING_DIR)/usr/include/huayi-universal -I$(STAGING_DIR)/usr/include/huayi_core
###########    F O R   D E V E L O P M E N T    #########
#CFLAGS  += -DNOT_USE_CONNECTION_MANAGER
#CFLAGS  += -DNOT_USE_SETUP_IP
#########################################################

#CC	= $(COMPILER_PATH)$(COMPILER_PREFIX)gcc
#STRIP	= $(COMPILER_PATH)$(COMPILER_PREFIX)strip

TARGET = huayi_atcmd_server 
OBJS	= \
	main.o \
	cm_dbg.o \
	cmd.o \
	sm.o \
	tty.o \
	atc_parser.o \
	q_thread_mq.o \
	thread_wrapper.o \
	at_tok.o \

LIBS	= -lpthread -luci -lhuayiuci -lm -lcjson -lhuayi-universal

all: $(TARGET) huayi_AT_cli

huayi_AT_cli: cli_atc.o
	$(CC) cli_atc.o -o huayi_AT_cli $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	@echo "Building $@"
	@$(CC) $(OBJS) $(LIBS) -o $@
	@echo "Striping $@"
	@$(STRIP) $@

clean:
	-rm -f $(TARGET) $(OBJS)
