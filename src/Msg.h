#ifndef MSG_H
#define MSG_H
#include "Macro.h"

/* 消息类型 
 * 类型 | 长度 | 数据 | 备注
 * 100  |      | null | 客户端IP地址请求
 * 101  |      |      | IP地址回应
 * 102  |      |      | 上网请求
 * 103  |      |      | 上网回应
 * 104  |      | null | 心跳包
**/
struct Msg {
    int length;
    char type;          // NOTE: padding here
    char data[4096];
};

#endif
