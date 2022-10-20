s_dir_y += noip-2.1.9
s_dir_y += orayddns
s_dir_y += yddns

TARGETS = dir
CFLAGS  += -Wall -fno-strict-aliasing
LDFLAGS += -L. -lpthread -lm -lhuayi-universal -luci -lhuayiuci -lcjson -ldl

all: $(TARGETS)

%.o : %.c
	$(CC) $(CFLAGS) -Wp,-MD,$@.d -c  -o $@ $^

dir:
	for i in $(s_dir_y) ; do make -C $$i all || exit $?; done

clean:
	for i in $(s_dir_y) ; do make -C $$i $@ || exit $?; done


	
