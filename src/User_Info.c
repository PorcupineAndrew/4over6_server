#include "Msg.h"
#include "User_Info.h"
#include "Buffer.h"

void user_info_init() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sprintf(users[i].addr, ADDRESS_PREFIX ".%d", i);
        users[i].is_free = 1;
        users[i].fd = -1;
    }
    users[0].is_free = 0;
    users[1].is_free = 0;
    users[255].is_free = 0;

    for (int i = 0; i < MAX_FDS; i++) {
        bufs[i].used = 0;
    }
}

int allocate_ip_addr(int client_fd) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i ++)
        if (users[i].is_free) {
            users[i].is_free = 0;

            users[i].fd = client_fd;
            users[i].last_heartbeat_sent_secs = time(0);
            users[i].last_heartbeat_recved_secs = time(0);
            inet_pton(AF_INET, users[i].addr, (void *)&users[i].v4addr);

            struct sockaddr_in6 clientaddr;
            socklen_t addrsize = sizeof(clientaddr);
            getpeername(client_fd, (struct sockaddr *)&clientaddr, &addrsize);
            users[i].v6addr = clientaddr.sin6_addr;
            
            return i;
        }
    return -1;
}

int deallocate_ip_addr(int client_fd) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i ++)
        if (users[i].fd == client_fd && !users[i].is_free) {
            users[i].is_free = 1;
            users[i].fd = -1;
            return i;
        }
    return -1;
}

int search_user_info_by_fd(int client_fd) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i ++)
        if (users[i].fd == client_fd && !users[i].is_free) {
            return i;
        }
    return -1;
}

int search_user_info_by_addr(uint32_t addr) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i ++)
        if (users[i].v4addr.s_addr == addr && !users[i].is_free) {
            return i;
        }
    return -1;
}

void free_client_fd(int client_fd, int epoll_fd) {
    if (epoll_remove_fd(epoll_fd, client_fd) < 0) {
        return;
    }
    deallocate_ip_addr(client_fd);
    close(client_fd);
}

int write_all(int fd, void* buf, int size) {
    int offset = 0;
    uint8_t* w = (uint8_t*)buf;
    while(offset < size) {
        int ret;
        if ((ret = write(fd, w+offset, size-offset)) < 0) {
            perrorf("write data to fd = %d", fd);
            return ret;
        }
        offset += ret;
    }
    return offset;
}

void process_client(int client_fd, int tun_fd, int epoll_fd) {
    info("process_client\n");

    char* buf = bufs[client_fd].buf;
    int* nread = &bufs[client_fd].used;
    
    ssize_t size;
    size = read(client_fd, buf+*nread, BUF_SIZE-*nread);

    if (size <= 0) {
        if (errno == ECONNRESET || size == 0) {
            debug("Connection ternimated\n");
        } else {
            perror("read client");
        }
        free_client_fd(client_fd, epoll_fd);
        return;
    }
    *nread += size;

    while(1) {
        if (*nread < sizeof(Msg)) break;

        Msg msg = *(Msg *)buf;
        int msglen = msg.length;
        if (*nread < msg.length) break;

        if (msg.type == TYPE_IP_REQUEST) {
            debug("ip request\n");
            int id;
            if ((id = allocate_ip_addr(client_fd)) < 0) {
                debug("ip address pool is full\n");
                return;
            }

            char bufreply[BUF_SIZE];
            sprintf(bufreply, "%s %s %s %s %s", users[id].addr, ROUTE, DNS1, DNS2, DNS3);
            int buflen = strlen(bufreply);

            Msg reply;
            reply.type = TYPE_IP_RESPONSE;
            reply.length = buflen + sizeof(Msg);
            if (write_all(client_fd, &reply, sizeof(reply)) < (int)sizeof(reply)) {
                debug("send reply header failed\n");
                return;
            }
            if (write_all(client_fd, bufreply, buflen) < buflen) {
                debug("send reply data failed\n");
                return;
            }
        }
        else if (msg.type == TYPE_INET_REQUEST) {
            debug("net request\n");
            int datalen = msglen - sizeof(Msg);
            if (write_all(tun_fd, buf+sizeof(Msg), datalen) < datalen) {
                debug("send data to tun failed\n");
                return;
            }
        }
        else if (msg.type == TYPE_KEEPALIVE) {
            debug("keep alive\n");
            int id = search_user_info_by_fd(client_fd);
            users[id].last_heartbeat_recved_secs = time(0);
        }
        else {
            debugf("unknown type of data from client fd %d\n", client_fd);
        }

        debugf("Package client - length: %d\n", msglen);
        memmove(buf, buf + msglen, *nread - msglen);
        *nread -= msglen;
    }
}
