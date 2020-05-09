#ifndef USER_INFO_H
#define USER_INFO_H
#include "Macro.h"

/* 客户信息表
 * 同时作为地址池
**/
typedef struct User_Info {
    char addr[INET_ADDRSTRLEN];
    int is_free;

    int fd;
    time_t last_heartbeat_sent_secs;
    time_t last_heartbeat_recved_secs;
    struct in_addr v4addr;
    struct in6_addr v6addr;
} User_Info;

User_Info users[MAX_CLIENTS];
#endif
