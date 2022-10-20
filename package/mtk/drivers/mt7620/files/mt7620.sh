#!/bin/sh

. /lib/wifi/mtkwifi.inc


mt7620_up() {
	echo "mt7620_up($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_up ${devname} "mt7620"
}


mt7620_down() {
	echo "mt7620_down($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_down ${devname} "mt7620"
}


mt7620_reload() {
	echo "mt7620_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_reload ${devname} "mt7620"
}


mt7620_restart() {
	echo "mt7620_reload($@)"
	devname=`echo $1 | sed 's/\./_/g'`
	mt76xx_restart ${devname} "mt7620"
}

mt7620_detect() {
	dat2uci -l /etc/wireless/l1profile.dat -u /etc/config/wireless -d MT7620
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

