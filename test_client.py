#!/usr/bin/env python3
#-*- coding: utf-8 -*-
# **********************************************************************
# * Description   : test script for 4over6
# * Last change   : 10:29:29 2020-04-27
# * Author        : Yihao Chen
# * Email         : chenyiha17@mails.tsinghua.edu.cn
# * License       : www.opensource.org/licenses/bsd-license.php
# **********************************************************************
import socket
import sys
import threading

BYTEORDER = 'little'
SERVER = {
    "v6addr": "2402:f000:4:72:808::4016", 
    "port": 10086,
    "flow_info": 0,
    "scope_id": 3,
}

class Msg:

    IP_REQUEST = 100
    IP_RESPONSE = 101
    NETWORK_REQUEST = 102
    NETWORK_RESPONSE = 103
    KEEPALIVE = 104

    @staticmethod
    def to_Msg(buf: bytes):
        kwargs = dict(byteorder=BYTEORDER, signed=True)
        _length = int.from_bytes(buf[:4], **kwargs)
        assert _length == len(buf)
        _type = int.from_bytes(buf[4:5], **kwargs)
        _data = buf[5:]
        return Msg(_type, _data)

    def __init__(self, _type: int, _data: bytes):
        self.type = _type
        self.data = _data
        self.length = len(_data) + 5

    def to_bytes(self):
        self.length = len(self.data) + 5
        kwargs = dict(byteorder=BYTEORDER, signed=True)
        buf = self.length.to_bytes(4, **kwargs)
        buf += self.type.to_bytes(1, **kwargs)
        buf += self.data
        return buf

s = socket.socket(socket.AF_INET6, socket.SOCK_STREAM, 0)
s.connect(tuple(SERVER.values()))

def recv():
    kwargs = dict(byteorder=BYTEORDER, signed=True)
    chunks = [s.recv(4)]
    totalrecv = 4
    Len = int.from_bytes(chunks[0], **kwargs)
    if Len <= 0 or Len > 4104:
        return None
    while totalrecv < Len:
        chunk = s.recv(Len - totalrecv)
        if chunk == b'':
            raise RuntimeError("socket connection down")
        totalrecv += len(chunk)
        chunks.append(chunk)
    return Msg.to_Msg(b''.join(chunks))

def handle_recv():
    while True:
        msg = recv()
        if msg is None:
            print("socket broken")
            break
        print("receive: ")
        print(f"\tlength:\t{msg.length}")
        print(f"\ttype:\t{msg.type}")
        print(f"\tdata:\t{msg.data.decode()}")

t = threading.Thread(target=handle_recv)
t.start()

def send(_type, _data):
    tosend = Msg(_type, _data).to_bytes()
    totalsent = 0
    while totalsent < len(tosend):
        sent = s.send(tosend[totalsent:])
        if sent == 0:
            raise RuntimeError("socket connection down")
        totalsent += sent

while True:
    try:
        tp = int(input("type>"))
        dt = input("data>").encode()
        send(tp, dt)
    except Exception as e:
        print(e)
        continue
