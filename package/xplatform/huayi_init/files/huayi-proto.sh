#!/bin/sh


_huayi_set_ipv4_addr() {
	local str="$PROTO_IPADDR"
	local address mask broadcast ptp
	address="${str%%/*}"
	str="${str#*/}"
	mask="${str%%/*}"
	str="${str#*/}"
	broadcast="${str%%/*}"
	str="${str#*/}"
	ptp="$str"
	
	local netmask=`ipcalc.sh $address $mask | grep NETMASK`
	netmask="${netmask#*=}"
	
	[ -n "$address" ] && {
		huayi_cmd set network "$section"_ipv4 "$address"
	}
	
	[ -n "$netmask" ] && {
		huayi_cmd set network "$section"_netmask "$netmask"
	}
	
}


_huayi_set_route() {
	for item in $PROTO_ROUTE; do
		echo 
	done
	local str="$item";
	local target="${str%%/*}"
	str="${str#*/}"
	local mask="${str%%/*}"
	str="${str#*/}"
	local gw="${str%%/*}"
	str="${str#*/}"
	local metric="${str%%/*}"
	str="${str#*/}"
	local valid="${str%%/*}"
	str="${str#*/}"
	local table="${str%%/*}"
	str="${str#*/}"
	local source="${str}"
	
	[ -n "$gw" ] && {
		huayi_cmd set network "$section"_gateway "$gw"
	}
	
}

_huayi_set_dns() {
	[ -n "$PROTO_DNS" ] && {
		huayi_cmd set network "$section"_dns_server "0.0.0.0"
		for item in $PROTO_DNS; do
			huayi_cmd set network "$section"_dns_server "$item"
		done
	}
	
}

huayi_send_update() {
	local interface="$1"

	case $interface in
		wan4g | \
		wan)
			section="wan"
			;;
		lan)
			section="lan"
			;;
		*)
			exit 0
			;;
	esac
	
    _huayi_set_ipv4_addr $section
    
	_huayi_set_route $section
    
	_huayi_set_dns $section
}
