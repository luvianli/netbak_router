#!/bin/sh

DEVICE=usb0
VDEVICE=lte0
IPADDR=$1
GATEWAY=$2
NETMASK=$3
DNS1=$4
DNS2=$5
VLAN_TAG=100

echo "DEVICE: " $DEVICE
echo "VDEVICE: " $VDEVICE
echo "IPADDR: " $IPADDR
echo "GATEWAY: " $GATEWAY
echo "NETMASK: " $NETMASK
echo "DNS1: " $DNS1
echo "DNS2: " $DNS2

ifconfig $DEVICE 192.168.30.10 netmask 255.255.255.0 up
ip link set dev $VDEVICE down 
ip link delete $VDEVICE 

vconfig add $DEVICE $VLAN_TAG
ip link set $DEVICE.$VLAN_TAG name $VDEVICE
ifconfig $VDEVICE -arp
ip link set dev $DEVICE up
ip link set dev $VDEVICE up
ifconfig $VDEVICE $IPADDR netmask $NETMASK 
ifconfig $VDEVICE mtu 1496
route add default gw $GATEWAY 

## Please remove default gateway which get from usb interface
## you can get this gateway ip address by "ucfg get config lan br gateway".
route del default gw 192.168.30.1

echo "nameserver $DNS1" > /etc/resolv.conf
echo "nameserver $DNS2" >> /etc/resolv.conf


