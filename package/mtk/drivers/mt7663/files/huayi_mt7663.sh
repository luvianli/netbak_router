#!/bin/sh

. /lib/wifi/mtkwifi.inc


mt7663_up() {

	disabled=$(echo $(uci -q get wireless.radio1.disabled))

	[ "$disabled" == "0" ] && {
		echo "mt7663_up($@)"
		devname=`echo $1 | sed 's/\./_/g'`
		mt76xx_up ${devname} "mt7663" "radio1"
		iwpriv rai0 set led_setting=01-00-00-00-02-00-00-02

	#wifi_serveces
	eval profile=\${${devname}_profile_path}
	eval main_ifname=\$${devname}_main_ifname
	eval ext_ifname=\$${devname}_ext_ifname
	# 1.8021xd
	lua /lib/wifi/wifi_services_sh.lua s8021x $profile $ext_ifname $main_ifname true
}
}


mt7663_down() {
	echo "mt7663_down($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	iwpriv rai0 set led_setting=01-00-00-00-02-00-00-01
	mt76xx_down ${devname} "mt7663" "radio1"

	#wifi_serveces
	eval profile=\${${devname}_profile_path}
	eval main_ifname=\$${devname}_main_ifname
	eval ext_ifname=\$${devname}_ext_ifname
	# 1.8021xd
	lua /lib/wifi/wifi_services_sh.lua s8021x $profile $ext_ifname $main_ifname false
}


mt7663_reload() {
	uci2dat -d radio1 -f /etc/wireless/mt7663/mt7663.2.dat > /tmp/uci2dat.log
	echo "mt7663_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
#	mt76xx_reload ${devname} "mt7663"
	mt7663_down ${devname} "mt7663" "radio1"
	mt7663_up ${devname} "mt7663" "radio1"
}


mt7663_restart() {
	echo "mt7663_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_restart ${devname} "mt7663" "radio1"
}

mt7663_detect() {
	if [ -e /etc/config/wireless ]; then
		uci2dat -l /etc/config/wireless -u /etc/wireless/l1profile.dat
	else
		dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless -d MT7663
	fi
}


mt7663_status() {
	iwconfig
}

mt7663_hello() {
	echo "mt7663_hello($@)"
}

#=====================================================

cmd=$1
dev=$2

if [ "${cmd}" != "" ]; then
	mt7663_${cmd} $dev;
fi

