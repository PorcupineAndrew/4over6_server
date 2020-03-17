#ifndef MACRO_H
#define MACRO_H

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <net/if.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#ifndef __cplusplus
typedef unsigned char bool;
#endif

// 地址相关
#define N_USERS 128                                                         // 地址池大小
#define IPADDR(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))   // 地址转换
#define POOL_START_ADDR IPADDR(13, 8, 0, 2)                                 // 地址池起始地址

// 客户信息
#define ASSIGNED            1
#define FREE                0

// 消息类型
#define IP_REQUEST          100
#define IP_RESPONSE         101
#define NETWORK_REQUEST     102
#define NETWORK_RESPONSE    103
#define KEEPALIVE           104

// 服务器相关
#define SERVER_LISTEN_PORT  10086
#define MAX_LISTEN_QUEUE    16

// epoll相关
#define MAXEVENTS           64

#endif
