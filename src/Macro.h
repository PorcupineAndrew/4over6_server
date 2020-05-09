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
#include <linux/if_tun.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <time.h>
#include <execinfo.h>
#include <signal.h>

// 地址相关
#define ADDRESS_PREFIX      "13.8.0"
#define NETMASK             "255.255.255.0"
#define ROUTE               "0.0.0.0"
#define DNS1                "166.111.8.28"
#define DNS2                "166.111.8.29"
#define DNS3                "8.8.8.8"

// 消息相关
#define TYPE_IP_REQUEST     100
#define TYPE_IP_RESPONSE    101
#define TYPE_INET_REQUEST   102
#define TYPE_INET_RESPONSE  103
#define TYPE_KEEPALIVE      104

// 服务器相关
#define SERVER_LISTEN_ADDR  "2402:f000:4:72:808::4016"
#define SERVER_LISTEN_PORT  10086
#define HEARTBEAT_INTERVAL  20
#define HEARTBEAT_TIMEOUT   60
#define DV_NET_NAME         "eno4"
#define DV_TUN_NAME         "tun0"
#define BUF_SIZE            65536
#define MAX_CLIENTS         256
#define MAX_FDS             2048

// epoll相关
#define EPOLL_MAX_EVENTS    MAX_FDS
#define EPOLL_EVENTS        (EPOLLIN|EPOLLERR|EPOLLHUP)

// 输出相关
#define COLOR_RED           "\x1b[31m"
#define COLOR_GREEN         "\x1b[32m"
#define COLOR_YELLOW        "\x1b[33m"
#define COLOR_BLUE          "\x1b[34m"
#define COLOR_MAGENTA       "\x1b[35m"
#define COLOR_CYAN          "\x1b[36m"
#define COLOR_WHITE         "\x1b[37m"
#define COLOR_RESET         "\x1b[0m"

#define INFO_LOGGER         COLOR_GREEN "INFO" COLOR_RESET
#define DEBUG_LOGGER        COLOR_CYAN "DEBUG" COLOR_RESET
#define ERROR_LOGGER        COLOR_RED "ERROR" COLOR_RESET
#define TRACE_FMT           COLOR_YELLOW " %s:%d:%s() " COLOR_RESET
#define TIME_FMT            "%Y-%m-%d %H:%M:%S"

#define INFO                1
#define DEBUG               1
#define ERROR               1

#define infof(fmt, ...) \
            do { if (INFO) { char _buffer_[26]; time_t _timer_ = time(NULL); \
            struct tm *_tm_info_ = localtime(&_timer_); \
            strftime(_buffer_, 26, TIME_FMT, _tm_info_); \
            fprintf(stdout, INFO_LOGGER COLOR_YELLOW " %s " COLOR_RESET fmt, \
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


// Server.c
void process_server(int server_fd, int epoll_fd);
int init_server(int port, char* addr, int cons, char* if_name);

// User_info.c
void user_info_init();
int allocate_ip_addr(int client_fd);
int deallocate_ip_addr(int client_fd);
int search_user_info_by_fd(int client_fd);
int search_user_info_by_addr(uint32_t addr);
void free_client_fd(int client_fd, int epoll_fd);
int write_all(int fd, void* buf, int size);
void process_client(int client_fd, int tun_fd, int epoll_fd);

// Tun.c
int init_tun(const char* devname);
void process_tun(int tun_fd);

// Keep_Alive.c
void process_heartbeat(int epoll_fd);

// Epoll_Utils.c
int epoll_init(int max_ev);
int epoll_remove_fd(int epoll_fd, int fd);
int epoll_add_fd(int epoll_fd, int fd, int events);
#endif
