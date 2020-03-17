#include "Macro.h"

// 出于效率考虑，使用epoll而非select
extern int listen_fd;
extern int epfd;
extern struct epoll_event event;
extern struct epoll_event *events;

int add_epoll(int fd) {
    // 注册监听事件
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

void epoll_init(int max_ev) {
    // 创建epoll句柄
    if ((epfd = epoll_create1(0)) < 0) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }
    
    // 添加监听事件
    if (add_epoll(listen_fd) < 0) {
        perror("add_epoll");
        exit(EXIT_FAILURE);
    }

    // 分配事件缓冲区
    events = calloc(max_ev, sizeof event);

    fprintf(stdout, 
        "Epoll init\n\tevent_buffer_size: %d\n",
        max_ev
    );
}

bool ev_err(uint32_t ev) {
    return (ev & EPOLLERR) || (ev & EPOLLHUP) || (!(ev & EPOLLIN));
}

int make_socket_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        perror("fcntl");
        return -1;
    }

    return 0;
}
