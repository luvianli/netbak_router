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
del_gateway=`huayi_cmd get blend wisp_gateway`
wisp_device=`huayi_cmd get blend wisp_device`
rep_24g_status=`huayi_cmd get system rep_24g_status`
rep_58g_status=`huayi_cmd get system rep_58g_status`
wisp_type="radio0_cli0"

if [ $1 = "del" ];then
#	echo "#############del############" >>/tmp/log/con_log
	if [ $rep_24g_status -eq 1 ];then
		wisp_type="radio0_cli0"
		huayi_cmd set system rep_24g_status 0
	elif [ $rep_58g_status -eq 1 ];then
		wisp_type="radio1_cli0"
		huayi_cmd set system rep_58g_status 0
		
	fi
		uci set wireless.${wisp_type}.apcli_ssid=huayi_p1stdjkdsFG
		uci set wireless.${wisp_type}.apcli_enable=1
		uci set wireless.${wisp_type}.disabled=1
		uci commit wireless
		
	if [ "$link_check" != 0 ];then
		huayi_cmd set blend wisp_link 0
	fi
	if [ "$route_check" != "$ifname" ];then
		route del default gw $del_gateway
#	else
#		route del default dev apcli0
#		route del default dev apclii0
	fi
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
	route add default gw $del_gateway dev $wisp_device
else
	echo ". /lib/netifd/hy_del_wisp.sh del/rst"
fi

