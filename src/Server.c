#include "Msg.h"
#include "User_Info_Table.h"

extern int listen_fd;

void init_server(int port, char* addr, int cons, char* if_name) {
    // 创建套接字
    if ((listen_fd = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 绑定地址
    struct sockaddr_in6 server_addr;
    server_addr.sin6_family = AF_INET6;
    if (addr == NULL) server_addr.sin6_addr = in6addr_any;
    else inet_pton(AF_INET6, addr, (void*) &server_addr.sin6_addr.s6_addr);
    server_addr.sin6_port = htons(port);
    server_addr.sin6_scope_id = (if_name == NULL) ? 0 : if_nametoindex(if_name);
    if (bind(listen_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(listen_fd, cons) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // 打印
    char s[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &server_addr.sin6_addr, s, sizeof(s));
    infof( 
        "Server init\n\taddr: %s\n\tport: %d\n\tscope_id: %d\n",
        s, ntohs(server_addr.sin6_port), server_addr.sin6_scope_id
    );
} 
