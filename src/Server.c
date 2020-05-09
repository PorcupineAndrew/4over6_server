#include "Msg.h"
#include "User_Info.h"
#include "Buffer.h"

int init_server(int port, char* addr, int cons, char* if_name) {
    int fd;
    if ((fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof(on)) < 0) {
        perror("setsockopt\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    if (addr == NULL) server_addr.sin6_addr = in6addr_any;
    else inet_pton(AF_INET6, addr, (void*) &server_addr.sin6_addr.s6_addr);
    server_addr.sin6_port = htons(port);
    server_addr.sin6_scope_id = (if_name == NULL) ? 0 : if_nametoindex(if_name);
    if (bind(fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(fd, cons) < 0) {
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

    return fd;
} 

void process_server(int server_fd, int epoll_fd) {
    struct sockaddr_in6 clientaddr;
    memset(&clientaddr, 0, sizeof(clientaddr));
    socklen_t addrsize = sizeof(clientaddr);

    int client_fd;
    if ((client_fd = accept(server_fd, (struct sockaddr *)&clientaddr, &addrsize)) < 0) {
        perror("accept connection");
        return;
    }

    bufs[client_fd].used = 0;

    char addrpeer6[BUF_SIZE];
    getpeername(client_fd, (struct sockaddr *)&clientaddr, &addrsize);
    inet_ntop(AF_INET6, &clientaddr.sin6_addr, addrpeer6, sizeof(addrpeer6));
    infof("New connection - addr: %s\n", addrpeer6);
    infof("New connection - port: %d\n", ntohs(clientaddr.sin6_port));

    if (epoll_add_fd(epoll_fd, client_fd, EPOLL_EVENTS) < 0) {
        return;
    }
}
