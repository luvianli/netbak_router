#!/bin/sh

. /lib/wifi/mtkwifi.inc


mt7615_up() {
	#echo "mt7615_up($@)"
	disabled=$(echo $(uci -q get wireless.radio1.disabled))

	[ "$disabled" != "1" ] && {
		echo "mt7615_up($@)"
		devname=`echo $1 | sed 's/\./_/g'`
		mt76xx_up ${devname} "mt7615" "radio1"
		iwpriv rai0 set led_setting=01-00-00-00-02-00-00-02
	}

	#wifi_serveces
	eval profile=\${${devname}_profile_path}
	eval main_ifname=\$${devname}_main_ifname
	eval ext_ifname=\$${devname}_ext_ifname
	# 1.8021xd
	lua /lib/wifi/wifi_services_sh.lua s8021x $profile $ext_ifname $main_ifname true
}


mt7615_down() {
	#echo "mt7615_down($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	iwpriv rai0 set led_setting=01-00-00-00-02-00-00-01
	mt76xx_down "${devname}" "mt7615" "radio1"

	#wifi_serveces
	eval profile=\${${devname}_profile_path}
	eval main_ifname=\$${devname}_main_ifname
	eval ext_ifname=\$${devname}_ext_ifname
	# 1.8021xd
	lua /lib/wifi/wifi_services_sh.lua s8021x $profile $ext_ifname $main_ifname false
}


mt7615_reload() {
	uci2dat -d radio1 -f /etc/wireless/mt7615/mt7615.2.dat > /tmp/uci2dat.log
	#echo "mt7615_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
#	mt76xx_reload "" "mt7615"
	mt7615_down ${devname} "mt7615" "radio1"
	mt7615_up ${devname} "mt7615" "radio1"

	#wifi_serveces
	local driver="mt7615"
	for dev in ${L1DEVNAMES}; do
		eval compat=\${${dev}_init_compatible}
		if [ "${compat}" == "${driver}" ]; then
			devnames="${dev} ${devnames}"
		fi
	done
	for devname in ${devnames}; do
		eval profile=\${${devname}_profile_path}
		eval main_ifname=\$${devname}_main_ifname
		eval ext_ifname=\$${devname}_ext_ifname
		# 1.8021xd
		lua /lib/wifi/wifi_services_sh.lua s8021x $profile $ext_ifname $main_ifname true
	done
}


mt7615_restart() {
	#echo "mt7615_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_restart ${devname} "mt7615" "radio1"
}

mt7615_detect() {
	if [ -e /etc/config/wireless ]; then
		uci2dat -l /etc/config/wireless -u /etc/wireless/l1profile.dat
	else
		dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless -d MT7615
	fi
}


mt7615_status() {
	iwconfig
}

mt7615_hello() {
	echo "mt7615_hello($@)"
}

#=====================================================

cmd=$1
dev=$2

if [ "${cmd}" != "" ]; then
	mt7615_${cmd} $dev;
fi
