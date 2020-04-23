#include "Msg.h"
#include "User_Info_Table.h"

#define IGNORE 1

int listen_fd;
int epfd;
int tun_fd;
struct epoll_event event;
struct epoll_event *events;
pthread_mutex_t MUTEX;
pthread_t keep_alive_thread;

void init_user_info(uint32_t start_addr, int n_user);
void init_server(int port, char* addr, int cons, char* if_name);
void init_tun(const char* devname);
void epoll_init(int max_ev);
int add_epoll(int fd);
bool ev_err(uint32_t ev);
int insert_user_table(int infd, struct in6_addr *inaddr, int n_users, pthread_mutex_t *mutex);
int make_socket_non_blocking(int fd);
void init_keep_alive_thread();

int main() {
    int running = 1;

    init_server(
        SERVER_LISTEN_PORT,
        // NULL,
        "fe80::d4a2:99ab:92c8:f22",
        MAX_LISTEN_QUEUE,
        "wlp2s0"
    );

    epoll_init(MAXEVENTS);

#if !IGNORE
    init_tun(MY_TUN_NAME);
    // 使用epoll而非thread
    if (add_epoll(tun_fd) < 0) {
        perror("add epoll");
        exit(EXIT_FAILURE);
    }
#endif

    init_user_info(POOL_START_ADDR, N_USERS);

    init_keep_alive_thread();

    while (running) {
        int n = epoll_wait(epfd, events, MAXEVENTS, -1);
        for (int i = 0; i < n; i++) {
            // 异常事件
            if (ev_err(events[i].events)) {
                // TODO
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            
            if (listen_fd == events[i].data.fd) {
                // 处理多连接请求
                while (1) {
                    struct sockaddr_in6 in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;

                    // 结束
                    if ((infd = accept(listen_fd, (struct sockaddr *)&in_addr, &in_len)) < 0) {
                        if (!(errno == EAGAIN) && !(errno == EWOULDBLOCK))
                            perror("accept");
                        break;
                    }

                    // 打印
                    if (getnameinfo((struct sockaddr *)&in_addr, in_len,
                        hbuf, sizeof hbuf,
                        sbuf, sizeof sbuf,
                        NI_NUMERICSERV | NI_NUMERICHOST) == 0) {
                        fprintf(stdout, 
                            "Accepted connection\n\tdescriptor: %d\n\thost: %s\n\tport: %s\n",
                            infd, hbuf, sbuf
                        );
                    }

                    // 更新表
                    if (insert_user_table(infd, &in_addr.sin6_addr, N_USERS, &MUTEX) < 0) {
                        fprintf(stderr, "user table full\n");
                        close(infd);
                        continue;
                    }

                    // 注册事件
                    make_socket_non_blocking(infd);
                    if (add_epoll(infd) < 0) {
                        perror("add_epoll");
                        close(infd);
                        continue;
                    }
                } continue;
            } else if (events[i].data.fd == tun_fd) {
                // TODO
                fprintf(stdout, "something happened");
            } else
                fprintf(stdout, "something happened");
        }
    }
}
