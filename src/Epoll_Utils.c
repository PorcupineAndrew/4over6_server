#include "Macro.h"

int epoll_add_fd(int epoll_fd, int fd, int events) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perrorf("add fd %d to epoll", fd);
        return -1;
    }
    return 0;
}

int epoll_remove_fd(int epoll_fd, int fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0) {
        perrorf("remove fd %d from epoll", fd);
        return -1;
    }
    return 0;
}

int epoll_init(int max_ev) {
    int fd;
    if ((fd = epoll_create(max_ev)) < 0) {
        perror("create epoll fd");
        exit(EXIT_FAILURE);
    }
    return fd;
}

