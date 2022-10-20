#!/bin/sh

. /lib/wifi/mtkwifi.inc


mt7628_up() {
	echo "mt7628_up($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_up ${devname} "mt7628"
}


mt7628_down() {
	echo "mt7628_down($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_down ${devname} "mt7628"
}


mt7628_reload() {
	echo "mt7628_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_reload ${devname} "mt7628"
}


mt7628_restart() {
	echo "mt7628_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_restart ${devname} "mt7628"
}

mt7628_detect() {
	if [ -e /etc/config/wireless ]; then
		uci2dat -l /etc/config/wireless -u /etc/wireless/l1profile.dat
	else
		dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless -d MT7628
	fi
}


mt7628_status() {
	iwconfig
}

mt7628_hello() {
	echo "mt7628_hello($@)"
}

#=====================================================

cmd=$1
dev=$2

if [ "${cmd}" != "" ]; then
	mt7628_${cmd} $dev;
fi

