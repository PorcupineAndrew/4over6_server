#!/usr/bin/env bash
#-*- coding: utf-8 -*-

TUN=tun0
NIC=eno4

sudo ip tuntap add mode tun dev "$TUN"
sudo addr add 13.8.1.1/24 dev "$TUN"
sudo ip link set dev "$TUN" up
sudo iptables -t nat -A POSTROUTING -s 13.8.1.0/24 -o "$NIC" -j MASQUERADE
