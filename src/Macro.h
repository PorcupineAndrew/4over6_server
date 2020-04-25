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
#include <linux/if_tun.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <sys/ioctl.h>
#include <assert.h>

#ifndef __cplusplus
typedef unsigned char bool;
#endif

// 地址相关
#define N_USERS 128                                                         // 地址池大小
#define IPADDR(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))   // 地址转换
#define POOL_START_ADDR IPADDR(13, 8, 0, 2)                                 // 地址池起始地址

// 消息相关
#define IP_REQUEST          100
#define IP_RESPONSE         101
#define NETWORK_REQUEST     102
#define NETWORK_RESPONSE    103
#define KEEPALIVE           104
#define MSG_HEADER_SIZE     8

// 服务器相关
#define SERVER_LISTEN_PORT  10086
#define MAX_LISTEN_QUEUE    16
#define KEEPLIVE_COUNT      20
#define MY_TUN_NAME         "tun0"

// epoll相关
#define MAXEVENTS           64

// debug相关
#define DEBUG               1
#define debugf(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, "DEBUG-%s:%d:%s(): " fmt, __FILE__, \
                    __LINE__, __func__, __VA_ARGS__); } while (0)
#define debug(x) debugf("%s", (x))

#endif
