#!/bin/sh
resolv_addr="/tmp/resolv.conf.auto"
cat_dns_wan4g=`cat $resolv_addr | grep -wn wan4g`
cat_dns_wan=`cat $resolv_addr | grep -wn wan`
sed_wan_place=$((${cat_dns_wan4g:0:1}-1))
a="1,$sed_wan_place d"
#sed_wan=`sed -i "$a" $resolv_addr`
route_check=`route | grep "default" | awk {'print $8'}`
ifname="eth1"
link_check=`huayi_cmd get blend wisp_link`

if [ $1 = "del" ];then
	if [ -n "$cat_dns_wan4g" ]&&[ -n "$cat_dns_wan" ];then
			 sed -i "$a" $resolv_addr
	else
			echo "-1"
	fi
elif [ $1 = "rst" ];then
#	echo "#############rst############" >>/tmp/log/con_log
	huayi_cmd set blend wisp_link 1
	ifup wan
	route del default dev eth1
	route add default gw $2 dev $3
else
	echo ". /lib/netifd/hy_del_wisp.sh del/rst [gateway] [ifname]"
fi

