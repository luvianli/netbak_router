#!/bin/sh

. /lib/wifi/mtkwifi.inc


mt7602_up() {
	echo "mt7602_up($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_up ${devname} "mt7602"
}


mt7602_down() {
	echo "mt7602_down($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_down ${devname} "mt7602"
}


mt7602_reload() {
	echo "mt7602_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_reload ${devname} "mt7602"
}


mt7602_restart() {
	echo "mt7602_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_restart ${devname} "mt7602"
}

mt7602_detect() {
	dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless -d MT7602
}


mt7602_status() {
	iwconfig
}

mt7602_hello() {
	echo "mt7602_hello($@)"
}

#=====================================================

cmd=$1
dev=$2

if [ "${cmd}" != "" ]; then
	mt7602_${cmd} $dev;
fi

