#!/bin/sh

. /lib/wifi/mtkwifi.inc


mt7620_up() {
	disabled=$(echo $(uci -q get wireless.radio0.disabled))
	
	[ "$disabled" != "1" ] && {
		echo "mt7620_up($@)"
		devname=`echo $1 | sed 's/\./_/g'`
		mt76xx_up ${devname} "mt7620" "radio0"
	}
}


mt7620_down() {
	echo "mt7620_down($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_down ${devname} "mt7620" "radio0"
}


mt7620_reload() {
	uci2dat -d radio0 -f /etc/wireless/mt7620/mt7620.dat > /tmp/uci2dat.log
	echo "mt7620_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	#mt76xx_reload ${devname} "mt7620"
	mt7620_down ${devname} "mt7620" "radio0"
	mt7620_up ${devname} "mt7620" "radio0"
}


mt7620_restart() {
	echo "mt7620_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_restart ${devname} "mt7620" "radio0"
}

mt7620_detect() {
	#dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless -d MT7620
	if [ -e /etc/config/wireless ]; then
		uci2dat -l /etc/config/wireless -u /etc/wireless/l1profile.dat
	else
		dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless -d MT7620
	fi
}


mt7620_status() {
	iwconfig
}

mt7620_hello() {
	echo "mt7620_hello($@)"
}

#=====================================================

cmd=$1
dev=$2

if [ "${cmd}" != "" ]; then
	mt7620_${cmd} $dev;
fi

