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
#include <time.h>
#include <execinfo.h>
#include <signal.h>

#ifndef __cplusplus
typedef unsigned char bool;
#endif

// 地址相关
#define N_USERS 128                                                         // 地址池大小
#define IPADDR(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))   // 地址转换
#define POOL_START_ADDR IPADDR(13, 8, 1, 2)                                 // 地址池起始地址

#define ROUTE               "0.0.0.0"
#define DNS1                "114.114.114.114"
#define DNS2                "223.5.5.5"
#define DNS3                "119.29.29.29"

// 消息相关
#define IP_REQUEST          100
#define IP_RESPONSE         101
#define NETWORK_REQUEST     102
#define NETWORK_RESPONSE    103
#define KEEPALIVE           104
#define MSG_HEADER_SIZE     5

// 服务器相关
#define SERVER_LISTEN_ADDR  "2402:f000:4:72:808::4016"
#define SERVER_LISTEN_PORT  10086
#define MAX_LISTEN_QUEUE    16
#define KEEPLIVE_COUNT      20
#define DV_NET_NAME         "eno4"
#define DV_TUN_NAME         "tun0"

// epoll相关
#define MAXEVENTS           64

// 输出相关
#define COLOR_RED           "\x1b[31m"
#define COLOR_GREEN         "\x1b[32m"
#define COLOR_YELLOW        "\x1b[33m"
#define COLOR_BLUE          "\x1b[34m"
#define COLOR_MAGENTA       "\x1b[35m"
#define COLOR_CYAN          "\x1b[36m"
#define COLOR_WHITE         "\x1b[37m"
#define COLOR_RESET         "\x1b[0m"

#define INFO_LOGGER         COLOR_GREEN"INFO"COLOR_RESET
#define DEBUG_LOGGER        COLOR_CYAN"DEBUG"COLOR_RESET
#define ERROR_LOGGER        COLOR_RED"ERROR"COLOR_RESET
#define TRACE_FMT           COLOR_YELLOW" %s:%d:%s() "COLOR_RESET
#define TIME_FMT            "%Y-%m-%d %H:%M:%S"

#define INFO                1
#define DEBUG               1
#define ERROR               1

#define infof(fmt, ...) \
            do { if (INFO) { char _buffer_[26]; time_t _timer_ = time(NULL); \
            struct tm *_tm_info_ = localtime(&_timer_); \
            strftime(_buffer_, 26, TIME_FMT, _tm_info_); \
            fprintf(stdout, INFO_LOGGER COLOR_YELLOW" %s "COLOR_RESET fmt, \
            _buffer_, __VA_ARGS__); }} while (0)
#define info(x) infof("%s", (x))

#define debugf(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, DEBUG_LOGGER TRACE_FMT fmt, __FILE__, \
                    __LINE__, __func__, __VA_ARGS__); } while (0)
#define debug(x) debugf("%s", (x))

#define perrorf(fmt, ...)  \
            do { if (ERROR) fprintf(stderr, ERROR_LOGGER TRACE_FMT fmt ": %s\n", __FILE__, \
                    __LINE__, __func__, __VA_ARGS__, strerror(errno)); } while (0)
#define perror(x) perrorf("%s", (x))

#endif
