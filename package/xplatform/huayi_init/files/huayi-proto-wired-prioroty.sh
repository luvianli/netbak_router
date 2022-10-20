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
		if [ "$wan_type" == "prior" ]; then
			case $interface in
				wan)
					huayi_cmd set blend wired_ipv4 "$address"
					;;
				wan4g)
					huayi_cmd set blend cpe_ipv4 "$address"
					;;
				*)
					huayi_cmd set network "$section"_ipv4 "$address"
					;;
			esac
		else
			huayi_cmd set network "$section"_ipv4 "$address"
		fi
	}
	
	[ -n "$netmask" ] && {
		if [ "$wan_type" == "prior" ]; then
			case $interface in
				wan)
					huayi_cmd set blend wired_netmask "$netmask"
					;;
				wan4g)
					huayi_cmd set blend cpe_netmask "$netmask"
					;;
				*)
					huayi_cmd set network "$section"_netmask "$netmask"
					;;
			esac
		else
			huayi_cmd set network "$section"_netmask "$netmask"
		fi
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
		if [ "$wan_type" == "prior" ]; then
			case $interface in
				wan)
					huayi_cmd set blend wired_gateway "$gw"
					;;
				wan4g)
					huayi_cmd set blend cpe_gateway "$gw"
					;;
				*)
					huayi_cmd set network "$section"_gateway "$gw"
					;;
			esac
		else
			huayi_cmd set network "$section"_gateway "$gw"

		fi
	}
	
}

_huayi_set_dns() {
	[ -n "$PROTO_DNS" ] && {
		huayi_cmd set network "$section"_dns_server "0.0.0.0"
		for item in $PROTO_DNS; do
			if [ "$wan_type" == "prior" ]; then
				case $interface in
					wan)
						huayi_cmd set blend wired_dns_server "$item"
						;;
					wan4g)
						huayi_cmd set blend cpe_dns_server "$item"
						;;
					*)
						huayi_cmd set network "$section"_dns_server "$item"
						;;
				esac
			else
				huayi_cmd set network "$section"_dns_server "$item"
			fi
		done
	}
	
}

huayi_send_update() {
	interface="$1"
	wan_type=`uci get system.mode.wan_type`

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
	
    _huayi_set_ipv4_addr $section $interface $wan_type
    
	_huayi_set_route $section $interface $wan_type
    
	_huayi_set_dns $section $interface $wan_type
}
