#!/bin/sh

restore6855Esw()
{
	echo "restore GSW to dump switch mode"
	#port matrix mode
	switch reg w 2004 ff0000 #port0
	switch reg w 2104 ff0000 #port1
	switch reg w 2204 ff0000 #port2
	switch reg w 2304 ff0000 #port3
	switch reg w 2404 ff0000 #port4
	switch reg w 2504 ff0000 #port5
	switch reg w 2604 ff0000 #port6
	switch reg w 2704 ff0000 #port7

	#LAN/WAN ports as transparent mode
	switch reg w 2010 810000c0 #port0
	switch reg w 2110 810000c0 #port1
	switch reg w 2210 810000c0 #port2
	switch reg w 2310 810000c0 #port3
	switch reg w 2410 810000c0 #port4
	switch reg w 2510 810000c0 #port5
	switch reg w 2610 810000c0 #port6
	switch reg w 2710 810000c0 #port7
	
	#clear mac table if vlan configuration changed
	switch clear
	switch vlan clear 
}

config6855Esw()
{
	#LAN/WAN ports as security mode
	switch reg w 2004 ff0003 #port0
	switch reg w 2104 ff0003 #port1
	switch reg w 2204 ff0003 #port2
	switch reg w 2304 ff0003 #port3
	switch reg w 2404 ff0003 #port4
	switch reg w 2504 ff0003 #port5
	#LAN/WAN ports as transparent port
	switch reg w 2010 810000c0 #port0
	switch reg w 2110 810000c0 #port1
	switch reg w 2210 810000c0 #port2
	switch reg w 2310 810000c0 #port3
	switch reg w 2410 810000c0 #port4
	switch reg w 2510 810000c0 #port5
	#set CPU/P7 port as user port
	switch reg w 2610 81000000 #port6
	switch reg w 2710 81000000 #port7

	switch reg w 2604 20ff0003 #port6, Egress VLAN Tag Attribution=tagged
	switch reg w 2704 20ff0003 #port7, Egress VLAN Tag Attribution=tagged

	if [ "$1" = "LLLLW" ]; then
		#VLAN member port
		#switch vlan  set 1 1 01111011
		#switch vlan  set 2 2 10000100
		#set PVID
		#switch pvid 0 2
		#switch pvid 5 2
		#LAN/WAN ports as security mode
		#switch reg w 2004 ff0003 #port0
		#switch reg w 2104 ff0003 #port1
		#switch reg w 2204 ff0003 #port2
		#switch reg w 2304 ff0003 #port3
		#switch reg w 2404 ff0003 #port4
		#switch reg w 2504 ff0003 #port5
		#switch reg w 2604 ff0003 #port6
		
		#set PVID
		switch reg w 2414 10002 #port4
		
		lan0_pvid=`uci -q get network.lan.lan0_pvid`
		switch reg w 2014 ${lan0_pvid} #port0
		
		lan1_pvid=`uci -q get network.lan.lan1_pvid`
		switch reg w 2114  ${lan1_pvid} #port1
		
		switch reg w 2214 10001 #port2
		switch reg w 2314 10001 #port3
		switch reg w 2514 10002 #port5
		#VLAN member port
		switch vlan clear
		
		switch vlan set 0 1  11110011
		switch vlan set 1 2  00001100
		
		switch reg w 0x2410 0x81000000
		switch reg w 0x2404 0xff0003
		
		vlan_id=`uci -q get network.vlan_0.vlanid`
		reg94=`uci -q get network.vlan_0.reg94`
		reg98=`uci -q get network.vlan_0.reg98`
		reg90=`uci -q get network.vlan_0.reg90`
		if [ "${vlan_id}" != "0" ]; then
			switch vlan set 2 ${vlan_id}  00000000
			switch reg w 0x94 ${reg94}
			switch reg w 0x98 ${reg98}
			switch reg w 0x90 ${reg90}
		fi
		
		vlan_id=`uci -q get network.vlan_1.vlanid`
		reg94=`uci -q get network.vlan_1.reg94`
		reg98=`uci -q get network.vlan_1.reg98`
		reg90=`uci -q get network.vlan_1.reg90`
		if [ "${vlan_id}" != "0" ]; then
			switch vlan set 3 ${vlan_id} 00000000
			switch reg w 0x94 ${reg94}
			switch reg w 0x98 ${reg98}
			switch reg w 0x90 ${reg90}
		fi
		
		vlan_id=`uci -q get network.vlan_2.vlanid`
		reg94=`uci -q get network.vlan_2.reg94`
		reg98=`uci -q get network.vlan_2.reg98`
		reg90=`uci -q get network.vlan_2.reg90`
		if [ "${vlan_id}" != "0" ]; then
			switch vlan set 4 ${vlan_id}  00000000
			switch reg w 0x94 ${reg94}
			switch reg w 0x98 ${reg98}
			switch reg w 0x90 ${reg90}
		fi
		
		vlan_id=`uci -q get network.vlan_3.vlanid`
		reg94=`uci -q get network.vlan_3.reg94`
		reg98=`uci -q get network.vlan_3.reg98`
		reg90=`uci -q get network.vlan_3.reg90`
		if [ "${vlan_id}" != "0" ]; then
			switch vlan set 5 ${vlan_id} 00000000
			switch reg w 0x94 ${reg94}
			switch reg w 0x98 ${reg98}
			switch reg w 0x90 ${reg90}
		fi
	elif [ "$1" = "WLLLL" ]; then
		#VLAN member port
		#switch vlan  set 1 1 01111011
		#switch vlan  set 2 2 10000100
		#set PVID
		#switch pvid 0 2
		#switch pvid 5 2
		#LAN/WAN ports as security mode
		#switch reg w 2004 ff0003 #port0
		#switch reg w 2104 ff0003 #port1
		#switch reg w 2204 ff0003 #port2
		#switch reg w 2304 ff0003 #port3
		#switch reg w 2404 ff0003 #port4
		#switch reg w 2504 ff0003 #port5
		#switch reg w 2604 ff0003 #port6
		
		#set PVID
		switch reg w 2014 10002 #port0
		
		lan1_pvid=`uci -q get network.lan.lan1_pvid`
		switch reg w 2114 ${lan1_pvid} #port1
		
		lan2_pvid=`uci -q get network.lan.lan2_pvid`
		switch reg w 2214  ${lan2_pvid} #port2
		
		switch reg w 2314 10001 #port3
		switch reg w 2414 10001 #port4
		switch reg w 2514 10002 #port5
		#VLAN member port
		switch vlan clear
		
		switch vlan set 0 1  01111011
		switch vlan set 1 2  10000100
		
		switch reg w 0x2010 0x81000000
		switch reg w 0x2004 0xff0003
		
		vlan_id=`uci -q get network.vlan_0.vlanid`
		reg94=`uci -q get network.vlan_0.reg94`
		reg98=`uci -q get network.vlan_0.reg98`
		reg90=`uci -q get network.vlan_0.reg90`
		if [ "${vlan_id}" != "0" ]; then
			switch vlan set 2 ${vlan_id}  00000000
			switch reg w 0x94 ${reg94}
			switch reg w 0x98 ${reg98}
			switch reg w 0x90 ${reg90}
		fi
		
		vlan_id=`uci -q get network.vlan_1.vlanid`
		reg94=`uci -q get network.vlan_1.reg94`
		reg98=`uci -q get network.vlan_1.reg98`
		reg90=`uci -q get network.vlan_1.reg90`
		if [ "${vlan_id}" != "0" ]; then
			switch vlan set 3 ${vlan_id} 00000000
			switch reg w 0x94 ${reg94}
			switch reg w 0x98 ${reg98}
			switch reg w 0x90 ${reg90}
		fi
		
		vlan_id=`uci -q get network.vlan_2.vlanid`
		reg94=`uci -q get network.vlan_2.reg94`
		reg98=`uci -q get network.vlan_2.reg98`
		reg90=`uci -q get network.vlan_2.reg90`
		if [ "${vlan_id}" != "0" ]; then
			switch vlan set 4 ${vlan_id}  00000000
			switch reg w 0x94 ${reg94}
			switch reg w 0x98 ${reg98}
			switch reg w 0x90 ${reg90}
		fi
		
		vlan_id=`uci -q get network.vlan_3.vlanid`
		reg94=`uci -q get network.vlan_3.reg94`
		reg98=`uci -q get network.vlan_3.reg98`
		reg90=`uci -q get network.vlan_3.reg90`
		if [ "${vlan_id}" != "0" ]; then
			switch vlan set 5 ${vlan_id} 00000000
			switch reg w 0x94 ${reg94}
			switch reg w 0x98 ${reg98}
			switch reg w 0x90 ${reg90}
		fi	
	fi
}

setup_switch()
{
	echo "7621 use an independent gmac as lan, LLLLW"
	restore6855Esw
	config6855Esw LLLLW
}

reset_switch()
{
	echo "7621 use an independent gmac as lan & wan"
	restore6855Esw
}
