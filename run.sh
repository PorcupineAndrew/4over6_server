#!/usr/bin/env bash
#-*- coding: utf-8 -*-

cd $(dirname "$0") && make clean && make
sudo iptables -t nat --flush
sudo ./bin/4over6_server
