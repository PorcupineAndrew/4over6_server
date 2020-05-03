#include "Msg.h"
#include "User_Info_Table.h"

#define IGNORE 0

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
struct User_Info* get_user_by_fd(int fd, pthread_mutex_t *mutex);
void rm_user_by_fd(int fd, pthread_mutex_t *mutex);
int make_socket_non_blocking(int fd);
void init_keep_alive_thread();
void packet_forward();

void error_handler(int sig) {
    void *arr[10];
    size_t size = backtrace(arr, 10);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(arr, size, STDERR_FILENO);
    exit(EXIT_FAILURE);
}

int main() {
    {
        struct Msg test;
        assert((void *)&test+MSG_HEADER_SIZE == (void*)test.data);
    }
    signal(SIGABRT, error_handler);

    int running = 1;

    init_server(
        SERVER_LISTEN_PORT,
        SERVER_LISTEN_ADDR,
        MAX_LISTEN_QUEUE,
        DV_NET_NAME
    );

    epoll_init(MAXEVENTS);

#if !IGNORE
    init_tun(DV_TUN_NAME);
    // make_socket_non_blocking(tun_fd);
    // 使用epoll而非thread
    if (add_epoll(tun_fd) < 0) {
        perror("add epoll");
        exit(EXIT_FAILURE);
    }
#endif

    init_user_info(POOL_START_ADDR, N_USERS);

    init_keep_alive_thread();

    while (running) {
        int n = epoll_wait(epfd, events, 1, -1);
        for (int i = 0; i < n; i++) {
            // 异常事件
            if (ev_err(events[i].events)) {
                debug("epoll event exception\n");
                rm_user_by_fd(events[i].data.fd, &MUTEX);
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
                        infof(
                            "Accepted connection\n\tdescriptor: %d\n\thost: %s\n\tport: %s\n",
                            infd, hbuf, sbuf
                        );
                    }

                    // 更新表
                    if (insert_user_table(infd, &in_addr.sin6_addr, N_USERS, &MUTEX) < 0) {
                        debug("user table full\n");
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
                //  隧道包转发
                packet_forward();
            } else if (events[i].events & EPOLLIN) {
                // 用户包处理
                debug("user msg\n");
                int fd = events[i].data.fd;
                struct User_Info *user_info = get_user_by_fd(fd, &MUTEX);
                if (user_info == NULL) {
                    debug("get_user_info_by_fd\n");
                    continue;
                }
                debug("find user info\n");

                struct Msg msg;
                int ret = recv(fd, (void*)&msg, MSG_HEADER_SIZE, 0);
                if (ret <= 0) {
                    debugf("recv from user: %d bytes\n", ret);
                    continue;
                }
                while (ret < MSG_HEADER_SIZE) {
                    ret += recv(fd, (void*)&msg+ret, MSG_HEADER_SIZE-ret, 0);
                }
                debugf("recv msg header: length=%d, type=%d\n", msg.length, msg.type);

                int len = msg.length;
                if (len > 4096 || len < MSG_HEADER_SIZE) {
                    debugf("invalid msg length %d\n", len);
                    continue;
                }
                while (ret < len) {
                    int r = recv(fd, (void*)&msg+ret, len-ret, 0);
                    if (r <= 0) {
                        continue;
                    }
                    ret += r;
                }
                debug("recv msg data\n");

                if (msg.type == IP_REQUEST) {
                    debug("ip request\n");
                    struct Msg to_send;
                    char ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &user_info->v4addr.s_addr, ip, sizeof(ip));
                    sprintf(to_send.data, "%s "ROUTE" "DNS1" "DNS2" "DNS3, ip);
                    debugf("send data [%s]\n", to_send.data);
                    to_send.type = IP_RESPONSE;
                    to_send.length = strlen(to_send.data) + MSG_HEADER_SIZE;
                    if (send(fd, &to_send, to_send.length, 0) < 0) {
                        perror("send ip response");
                    }
                } else if (msg.type == NETWORK_REQUEST) {
                    debug("network request\n");
                #if !IGNORE
                    int send_len = len-MSG_HEADER_SIZE;
                    if (write(tun_fd, msg.data, send_len) != send_len) {
                        perror("write tun");
                    }
                    debugf("send data: %d\n", send_len);
                #endif
                } else if (msg.type == KEEPALIVE) {
                    debug("keepalive\n");
                    pthread_mutex_lock(&MUTEX);
                    user_info->secs = time(NULL);
                    pthread_mutex_unlock(&MUTEX);
                } else {
                    debugf("unexpected Msg type %d\n", msg.type);
                }

            }
        }
    }
    return 0;
}
