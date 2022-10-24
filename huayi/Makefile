Topdir:=$(shell pwd)/..

HUAYI_DIR:=$(shell pwd)

SOFT_VERSION:=$(shell cat ../package/base-files/files/etc/openwrt_version)

#build_server_cores:=$(shell cat /proc/cpuinfo | grep processor | wc -l)
build_server_cores:=1

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
	cd $(Topdir);make -j$(build_server_cores)

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

4G300-H001-HY01:
	@$(MAKE) $(t) product=$@ product_name=4G300-H001-HY01 Chip=MT7620N hardware=mt7620 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M	COMPANY=HUAYI

4G300-H001-HY02:
	@$(MAKE) $(t) product=$@ product_name=4G300-H001-HY02 Chip=MT7620N hardware=mt7620 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M	COMPANY=HUAYI
	
WRO300-H001-HY02:
	@$(MAKE) $(t) product=$@ product_name=WRO300-H001-HY02 Chip=MT7620N hardware=mt7620 FLASH_SIZE=$(SIZE4M) FLASH_DRAM_SIZE=4M64M	COMPANY=HUAYI

4G300-H002-HY03:
	@$(MAKE) $(t) product=$@ product_name=4G300-H002-HY03 Chip=MT7620N hardware=mt7620 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M	COMPANY=HUAYI

4G300-H002-HY04:
	@$(MAKE) $(t) product=$@ product_name=4G300-H002-HY04 Chip=MT7620N hardware=mt7620 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M	COMPANY=HUAYI

4G300-H002-HY05:
	@$(MAKE) $(t) product=$@ product_name=4G300-H002-HY05 Chip=MT7628DA hardware=mt7628 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M  COMPANY=HUAYI

4G300-06:
	@$(MAKE) $(t) product=$@ product_name=4G300-06 Chip=MT7620N hardware=mt7620 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M  COMPANY=M

4G1200-H002-HY06:
	@$(MAKE) $(t) product=$@ product_name=4G1200-H002-HY06 Chip=MT7628DA hardware=mt7628 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M  COMPANY=HUAYI

4G300-H002-HY07:
	@$(MAKE) $(t) product=$@ product_name=4G300-H002-HY07 Chip=MT7620N hardware=mt7620 FLASH_SIZE=$(SIZE8M) FLASH_DRAM_SIZE=8M64M	COMPANY=HUAYI
	
help:
	@echo "make target   -- target maybe product model"
	@echo "eg:  make 4G300-H001-HY01 -- for huayi 300M Wireless 7620N Router 4G"
	@echo "eg:  make 4G300-H001-HY02 -- for zhongyi 300M Wireless 7620N Router 4G, 2 sim card"
	@echo "eg:  make 4G300-H002-HY03 -- for 7620N Router 4G; Europe 4G Modem, language en, 1 sim card"
	@echo "eg:  make 4G300-H002-HY04 -- for 7620N Router 4G; language cn, 1 sim card"
	@echo "eg:  make 4G300-H002-HY05 -- for huayi 300M Wireless     7628DA Router 4G"
	@echo "eg:  make WRO300-H001-HY02 -- for huayi 300M Wireless 7620N Router"
	@echo "eg:  make 4G300-06 -- for 7620N Router 4G; language cn, 1 sim card"
	@echo "eg:  make 4G1200-H002-HY06 -- for huayi 1200M Wireless     7628DA Router 1200M 4G"
	@echo "eg:  make 4G300-H002-HY07 -- for huayi 300M Wireless     7620A Router 300M 4G ,wisp+4g"
