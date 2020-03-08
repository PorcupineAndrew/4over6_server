#ifndef USER_INFO_H
#define USER_INFO_H
#include "Macro.h"

/* 客户信息表
 * 同时作为地址池
**/
struct User_Info {
    int fd;                             // 套接字描述符
    int count;                          // 标志位
    unsigned long int secs;             // 上次收到keeplive时间
    struct in_addr v4addr;              // 服务器给客户端分配的IPv4地址
    struct in6_addr v6addr;             // 客户端的IPv6地址
} user_info_table[N_USERS];

#endif
