Topdir:=$(shell pwd)/..

HUAYI_DIR:=$(shell pwd)

SOFT_VERSION:=$(shell cat ../package/base-files/files/etc/openwrt_version)

#build_server_cores:=$(shell cat /proc/cpuinfo | grep processor | wc -l)
build_server_cores:=4

DATE := $(shell date +%F)

WEB_FILE := $(Topdir)/bin/ramips/huayi_firmware.bin

RELEASE_IMG_DIR := $(HUAYI_DIR)/images

COMPILE_CONFIG_DIR := $(HUAYI_DIR)/config

COMPILE_PRODUCT_DIR := $(HUAYI_DIR)/product

SIZE4M=4194304

SIZE8M=8388608

SIZE16M=16777216

SIZE32M=33554432

FlashType=spi

FlashId=None

dest=${COMPANY}_${product_name}_${Chip}_${FlashType}_${FLASH_DRAM_SIZE}_${FlashId}_${SOFT_VERSION}_${DATE}

t=all

FlashKey=0

check_config:
	if [ -e $(COMPILE_CONFIG_DIR)/$(product)/$(product)-encryption.config ]; then \
		cp -rf $(COMPILE_CONFIG_DIR)/$(product)/$(product)-encryption.config .config; \
	else \
		echo "$(COMPILE_CONFIG_DIR)/$(product)/$(product)-encryption.config not exist !!"; \
		exit -1; \
	fi;	\


clean:
	cd $(Topdir); \
	cp -rf $(COMPILE_CONFIG_DIR)/$(product)/$(product).config .config; \
	make clean;rm -rf tmp/


def_config:
	cd $(Topdir); \
	cp -rf $(COMPILE_PRODUCT_DIR)/$(product)/target_patches/* $(Topdir)/target/linux/ramips/patches; \
	cp -rf $(COMPILE_CONFIG_DIR)/$(product)/$(product).kernel_config $(Topdir)/target/linux/ramips/$(hardware)/config-3.10; \
	if [ "$(FlashKey)" = "0" ];then \
		cp -rf $(COMPILE_CONFIG_DIR)/$(product)/$(product).config .config; \
		make defconfig; \
	else \
		make defconfig; \
	fi

real_make:
	cd $(Topdir);make 
	#-j$(build_server_cores)

release:
	cd $(hardware); \
	cp -f $(WEB_FILE) huayi_firmware.bin; \
	../fmerge ${product}.map tmp.flash; \
	( dd if=tmp.flash bs=${FLASH_SIZE} conv=sync ) > ${dest}.flash; \
	rm -f tmp.flash; \
	mv huayi_firmware.bin ${dest}.bin; \
	mkdir -p $(RELEASE_IMG_DIR); \
	mv ${dest}.* $(RELEASE_IMG_DIR);

all: check_config clean def_config 
	@echo "Building Release $(product) firmware"
	make def_config
	make real_make
	make release

ZC-GW06:
	@$(MAKE) $(t) product=$@ product_name=ZC-GW06 Chip=MT7620N hardware=mt7620 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M	COMPANY=HUAYI

ZC-GW05:
	@$(MAKE) $(t) product=$@ product_name=ZC-GW05 Chip=MT7628MT7613 hardware=mt7628 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M COMPANY=HUAYI
	
ZC-GW03-2:
	@$(MAKE) $(t) product=$@ product_name=ZC-GW03-2 Chip=MT7628DA hardware=mt7628 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M	COMPANY=HUAYI

ZC-GW01-2:
	@$(MAKE) $(t) product=$@ product_name=ZC-GW01-2 Chip=MT7628DA hardware=mt7628 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M	COMPANY=HUAYI
	
help:
	@echo "make target   -- target maybe product model"
	@echo "eg:  make ZC-GW06         -- for 7620N Router 300M 4G ,wisp+4g"
	@echo "eg:  make ZC-GW03-2 -- for 7628DA Router 300M 4G, wired+4g"
	@echo "eg:  make ZC-GW05 -- for 1200M Wireless 7628DA+7613B Modem, wisp+4g"
	@echo "eg:  make ZC-GW01-2 -- for 7628DA Router 300M 4G, wired+4g"
