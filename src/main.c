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
struct User_Info* get_user_by_fd(int fd, pthread_mutex_t *mutex);
void rm_user_by_fd(int fd, pthread_mutex_t *mutex);
int make_socket_non_blocking(int fd);
void init_keep_alive_thread();
void packet_forward();

int main() {
    assert(sizeof(struct Msg) == 4096+MSG_HEADER_SIZE);
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
                        fprintf(stdout, 
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
                    debugf("recv from user: %d\n", ret);
                    continue;
                }
                while (ret < MSG_HEADER_SIZE) {
                    ret += recv(fd, (void*)&msg+ret, MSG_HEADER_SIZE-ret, 0);
                }
                debug("recv header\n");

                int len = msg.length;
                if (len > 4096 || len < MSG_HEADER_SIZE) {
                    debugf("invalid msg length %d\n", len);
                    continue;
                }
                while (ret < len) {
                    ret += recv(fd, (void*)&msg+ret, len-ret, 0);
                }
                debug("recv msg\n");

                if (msg.type == IP_REQUEST) {
                    debug("ip request\n");
                } else if (msg.type == NETWORK_REQUEST) {
                    debug("network request\n");
                } else if (msg.type == KEEPALIVE) {
                    debug("keepalive\n");
                } else {
                    debugf("unexpected Msg type %d\n", msg.type);
                }

            }
        }
    }
}
