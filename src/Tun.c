#include "Msg.h"
#include "User_Info.h"
#include "Buffer.h"

int init_tun(const char* devname) {
    int fd;
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("open /dev/net/tun");
        exit(EXIT_FAILURE);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, devname, IFNAMSIZ);

    if (ioctl(fd, TUNSETIFF, (void*) &ifr) < 0) {
        perror("ioctl TUNSETIFF");
        exit(EXIT_FAILURE);
    }

    if (system("ifconfig " DV_TUN_NAME " " ADDRESS_PREFIX ".1 netmask " NETMASK " up") != 0) {
        perror("ifconfig");
        exit(EXIT_FAILURE);
    }
    if (system("iptables -t nat -A POSTROUTING -s " ADDRESS_PREFIX ".0/24 -j MASQUERADE") != 0) {
        perror("iptables");
        exit(EXIT_FAILURE);
    }

    infof("Tun init: %s\n", devname);

    return fd;
}

void process_tun(int tun_fd) {
    char* buf = bufs[tun_fd].buf;
    int* nread = &bufs[tun_fd].used;

    int size;
    if ((size = read(tun_fd, buf+*nread, BUF_SIZE-*nread)) < 0) {
        perror("read from tun");
        return;
    }
    *nread += size;

    while(1) {
        if (*nread < (int)sizeof(struct iphdr))
            break;
        
        struct iphdr hdr = *(struct iphdr *)buf;
        int iplen = *nread;
        if (*nread < iplen) break;

        do {
            int id = search_user_info_by_addr(hdr.daddr);
            if (id == -1) {
                debug("can not locate ip packet dest\n");
                continue;
            }
            int client_fd = users[id].fd;

            Msg reply;
            reply.type = TYPE_INET_RESPONSE;
            reply.length = iplen + sizeof(reply);

            int ret;
            if ((ret = write_all(client_fd, &reply, sizeof(reply))) < (int)sizeof(reply)) {
                debug("send reply header failed\n");
                return;
            }
            if ((ret = write_all(client_fd, buf, iplen)) < 0) {
                debug("send reply data failed\n");
                return;
            }

            infof("Package tunnel - length: %lu\n", iplen + sizeof(Msg));
        } while(0);

        memmove(buf, buf + iplen, *nread - iplen);
        *nread -= iplen;
    }
}
