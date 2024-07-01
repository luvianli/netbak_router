#!/bin/sh
# 2018 Hua Shao <hua.shao@mediatek.com>


profile_backup() {
	echo "profile_backup"
}


profile_diff() {
	echo "profile_diff"
}



# vifs_up and vifs_down accept ifname patterns, for example:
#
#	 vifs_up ra0			 -- ra0
#	 vifs_up rai0			-- rai0
#	 vifs_up ra?			 -- ra0, ra1, ..., ra9, raa, rab, ....
#	 vifs_up ra*			 -- ra0, rai0, ra1, rai1asdfasdfasdf, ....
#	 vifs_up ra[0-9]		 -- ra0, ra1, ..., ra9
#	 vifs_up ra1[1-3]		-- ra11, ra12, ra13
#	 vifs_up ra[0-9]*		-- ra0, ra1, ..., ra10, ra11, ....

vifs_up() {
	[ "$1" == "" ] && exit
	local vif=$1
	local bridge=$2
	
	ifconfig ${vif} up;
	[ "${bridge}" == "" ] || {
		brctl addif br-${bridge} ${vif} 2>/dev/null || true;
	}
}

vifs_down() {
	[ "$1" == "" ] && exit
	local vif=$1
	local bridge=$2
	
	ifconfig $vif down;

	[ "${bridge}" == "" ] || {
		brctl delif br-${bridge} ${vif} 2>/dev/null || true;
	}
}


# common API for mt76xx drivers
# $1 = devname
# $2 = driver
mt76xx_up() {
	local devname=`echo $1 | sed 's/\./_/g'`
	local driver=$2
	local radioid=$3
	local bssidnum
	
	[ "$1" == "" -o "$2" == "" -o "$3" == "" ]  && {
		echo "mtkwifi.inc===============>invalid arguments!" > /dev/console
		exit
	}
	
	[ -f /lib/modules/`uname -r`/${driver}.ko ] && modprobe ${driver}.ko
	
	bssidnum=`uci -q get wireless.$radioid.bssidnum`

	vap_id=0
	while [ $vap_id -lt $bssidnum ]
	do			
		vap_enable=`uci get -q wireless.$radioid"_"$vap_id.disabled`
		
		[ "${vap_enable}" == "0" ] && {
			vap_ifname=`uci get -q wireless.$radioid"_"$vap_id.ifname`
			vap_bridge=`uci get -q wireless.$radioid"_"$vap_id.network`
			[ -n "${vap_ifname}" ] && {
				echo vifs_up ${vap_ifname} ${vap_bridge} > /dev/console
				vifs_up ${vap_ifname} ${vap_bridge}
			}
		}
		vap_id=$(( $vap_id + 1 ))
	done
	
	sta_enable=`uci get -q wireless.$radioid"_"cli0.disabled`
	[ "${sta_enable}" == "0" ] && {
		sta_ifname=`uci get -q wireless.$radioid"_"cli0.ifname`
		sta_bridge=`uci get -q wireless.$radioid"_"cli0.network`
		[ -n "${sta_ifname}" ] && {
			echo vifs_up ${sta_ifname} ${sta_bridge} > /dev/console
			vifs_up ${sta_ifname} ${sta_bridge}
		}
	}
}


# $1 = devname
# $2 = driver
# $3 = radioid
# radioid add by huayi platform,it will be radio0 radio1 radio2...
mt76xx_down() {
	local devname=`echo $1 | sed 's/\./_/g'`
	local driver=$2
	local radioid=$3
	local bssidnum
	
	[ "$1" == "" -o "$2" == "" -o "$3" == "" ]  && {
		echo "mtkwifi.inc===============>invalid arguments!" > /dev/console
		exit
	}
	
	[ -f /lib/modules/`uname -r`/${driver}.ko ] && modprobe ${driver}.ko
	
	sta_ifname=`uci get -q wireless.$radioid"_"cli0.ifname`
	sta_bridge=`uci get -q wireless.$radioid"_"cli0.network`
	
	[ -n "${sta_ifname}" ] && {
		echo vifs_down ${sta_ifname} ${sta_bridge} > /dev/console
		vifs_down ${sta_ifname} ${sta_bridge}
	}
	
	bssidnum=`uci -q get wireless.$radioid.bssidnum`
	
	vap_id=0
	while [ $vap_id -lt $bssidnum ]
	do			
		vap_ifname=`uci get -q wireless.$radioid"_"$vap_id.ifname`
		vap_bridge=`uci get -q wireless.$radioid"_"$vap_id.network`
		[ -n "${vap_ifname}" ] && {
			echo vifs_down ${vap_ifname} ${vap_bridge} > /dev/console
			vifs_down ${vap_ifname} ${vap_bridge}
		}
		
		vap_id=$(( $vap_id + 1 ))
	done
}


# $1 = devname
# $2 = driver
# $3 = radioid
# radioid add by huayi platform,it will be radio0 radio1 radio2...
# we didn't use reload api 
mt76xx_reload() {
	local devname=`echo $1 | sed 's/\./_/g'`
	local devnames=${devname}
	local driver=$2
	local radioid=$3
	local need_restart=0
	local need_downup=0
	if [ "${devnames}" == "" ]; then
		if [ "${driver}" == "" ]; then
			echo "Invalid arguments! Both devname and driver are empty."
		fi
		for dev in ${L1DEVNAMES}; do
			eval compat=\${${dev}_init_compatible}
			if [ "${compat}" == "${driver}" ]; then
				devnames="${dev} ${devnames}"
			fi
		done
	fi

	for devname in ${devnames}; do
		eval profile=\${${devname}_profile_path}

		# 0. backup profiles first
		mkdir -p /tmp/mtk/wifi
		local profile_bak=`echo ${profile} | awk -F "/" '{print $(NF-0)}'`
		profile_bak=/tmp/mtk/wifi/${profile_bak}.bak
		cp ${profile} ${profile_bak} || true

		# 1. call uci2dat to save uci changes into profile
		#uci2dat -d ${devname} -f ${profile}

		# 2. check if we can do quick reload, "uci changes wireless" is a possible alternative
		datdiff ${profile_bak} ${profile} > /tmp/mtk/wifi/datdiff-${devname}.tmp
		if cat /tmp/mtk/wifi/datdiff-${devname}.tmp | grep -q BssidNum=; then
			# need call mt76xx_restart
			need_restart=1
			mt76xx_restart "" "${driver}" "${radioid}"
		fi

		if cat /tmp/mtk/wifi/datdiff-${devname}.tmp\
			| grep -Eq '(HT_LDPC|VHT_SGI|VHT_BW_SIGNAL|VHT_LDPC|E2pAccessMode|HT_BSSCoexistence)='; then
			need_downup=1
		fi

		# 3. quick reload
		if [ "0" == "${need_downup}"  -a "0" == "${need_restart}" ]; then
			datdiff -x ${profile_bak} ${profile} > /tmp/mtk/wifi/datdiff-${devname}.sh
			cat /tmp/mtk/wifi/datdiff-${devname}.sh
			mt76xx_up "${devname}" "${driver}" "${radioid}"
			sh /tmp/mtk/wifi/datdiff-${devname}.sh 2>/dev/null
		fi
	done

	#4 . normal reload (Both single band mode and DBDC mode)
	if [ "1" == "${need_downup}" -a "0" == "${need_restart}" ]; then
		# normal reload
		mt76xx_down "" "${driver}" "${radioid}"
		mt76xx_up "" "${driver}" "${radioid}"
	fi

	#5 . normal restart
	if [ "1" == "${need_restart}" ]; then
		# In order to display vifs in UI correctly
		#uci2dat
		#dat2uci
		[ -f "/tmp/mtk/wifi/reboot_required" ] && /sbin/wifi down && reboot -f >/dev/null 2>&1
	fi

	# remove tmp files
	# rm -f /tmp/mtk/wifi/*.dat.bak   || true
	# rm -f /tmp/mtk/wifi/*.sh  || true
	# rm -f /tmp/mtk/wifi/*.tmp || true
}




# $1 = devname
# $2 = driver
# $3 = radioid
# radioid add by huayi platform,it will be radio0 radio1 radio2...
mt76xx_restart() {
	local devname=`echo $1 | sed 's/\./_/g'`
	local driver=$2
	local radioid=$3

	[ -f /lib/modules/`uname -r`/${driver}.ko ] || echo "failed to detect ${driver}.ko"
	if [ -d /sys/module/${driver} ]; then
		mt76xx_down "${devname}" "${driver}" "${radioid}"
		rmmod ${driver};
		modprobe ${driver}.ko;
		mt76xx_up "${devname}" "${driver}" "${radioid}"
	else
		# if wifi driver is built-in, it's necessary action to reboot the device
		mkdir -p /tmp/mtk/wifi
		echo reboot_required > /tmp/mtk/wifi/reboot_required
	fi

}


# $1 = devname
# $2 = driver
mt76xx_detect() {
	dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless
}


# $1 = devname
# $2 = driver
mt76xx_status() {
	iwconfig
}

# $1 = devname
# $2 = driver
mt76xx_hello() {
	echo "mt76xx_hello($@)"
}

