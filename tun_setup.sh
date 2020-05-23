#!/usr/bin/env bash
#-*- coding: utf-8 -*-

TUN=tun0
NIC=eno4

set_up_tun() {
    sudo sysctl -w net.ipv4.ip_forward=1
    sudo ip tuntap add mode tun dev "$TUN"
    sudo ip addr add 13.8.0.1/24 dev "$TUN"
    sudo ip link set dev "$TUN" up
    sudo iptables -t nat -A POSTROUTING -s 13.8.0.0/24 -o "$NIC" -j MASQUERADE
    sudo iptables -A FORWARD -i "$TUN" -j ACCEPT
    sudo iptables -A FORWARD -o "$TUN" -j ACCEPT
}

rm_tun() {
    sudo ip tuntap del mode tun dev "$TUN"
    sudo iptables -t nat --flush
}

rm_tun
set_up_tun
