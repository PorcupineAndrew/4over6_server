#include "Msg.h"
#include "User_Info.h"
#include "Buffer.h"

void error_handler(int sig) {
    void *arr[10];
    size_t size = backtrace(arr, 10);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(arr, size, STDERR_FILENO);
    exit(EXIT_FAILURE);
}

int main() {
    signal(SIGABRT, error_handler);

    int server_fd = init_server(
        SERVER_LISTEN_PORT,
        SERVER_LISTEN_ADDR,
        MAX_FDS,
        DV_NET_NAME
    );
    int tun_fd = init_tun(DV_TUN_NAME);
    int epoll_fd = epoll_init(EPOLL_MAX_EVENTS);

    epoll_add_fd(epoll_fd, server_fd, EPOLL_EVENTS);
    epoll_add_fd(epoll_fd, tun_fd, EPOLL_EVENTS);

    user_info_init();

    int running = 1;
    int num_events;
    struct epoll_event events[EPOLL_MAX_EVENTS];
    while (running) {
        if ((num_events = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, 1)) < 0) {
            debug("failed to wait for epoll events\n");
        }
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_fd) {
                process_server(server_fd,epoll_fd);
            } else if (events[i].data.fd == tun_fd) {
                process_tun(tun_fd);
            } else {
                process_client(events[i].data.fd, tun_fd, epoll_fd);
            }
        }
        process_heartbeat(epoll_fd);
        fflush(stdout);
    }
}
