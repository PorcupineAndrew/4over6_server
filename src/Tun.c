#include "Msg.h"
#include "User_Info_Table.h"

extern int tun_fd;
extern pthread_mutex_t MUTEX;
struct User_Info* get_user_by_IPv4(uint32_t addr, pthread_mutex_t *mutex);

void init_tun(const char* devname) {
    if ((tun_fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("open /dev/net/tun");
        exit(EXIT_FAILURE);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags |= IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, devname, IFNAMSIZ);

    // ioctl会使用ifr.ifr_name作为隧道名
    if (ioctl(tun_fd, TUNSETIFF, (void*) &ifr) < 0) {
        perror("ioctl TUNSETIFF");
        exit(EXIT_FAILURE);
    }

    infof("Tun init: %s\n", devname);
}

char packet_buf[4096];
struct Msg msg_buf;

void packet_forward() {
    info("packet_forward\n");
    memset(packet_buf, 0, sizeof(packet_buf));
    int ret;
    if ((ret = read(tun_fd, (void *)packet_buf, sizeof(packet_buf))) < 0) {
        perror("read ip packet");
        return;
    }
    debugf("packet readed: %d\n", ret);

    struct iphdr *hdr = (struct iphdr*)packet_buf;
    int length = ntohs(hdr->tot_len);
    unsigned int dst_addr = hdr->daddr;
    debugf("length: %d\n", length);
    if (length != ret) {
        debug("invalid length\n");
        length = ret;
    }

    // if (read(tun_fd, (void *)&packet_buf[20], length-20) <= 0) {
    //     perror("read ip packet");
    //     return;
    // }
    // debug("data readed\n");

    char sbuf[INET_ADDRSTRLEN], dbuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &hdr->saddr, sbuf, sizeof(sbuf));
    inet_ntop(AF_INET, &hdr->daddr, dbuf, sizeof(dbuf));
    infof("packet from %s to %s with size %d\n", sbuf, dbuf, length);

    if (htonl(POOL_START_ADDR) <= dst_addr && dst_addr < htonl(POOL_START_ADDR + N_USERS)) {
        struct User_Info *user_info = get_user_by_IPv4(dst_addr, &MUTEX);
        if (user_info == NULL) {
            debugf("no valid user for %s\n", dbuf);
            return;
        } else {
            debug("find user\n");
        }

        msg_buf.type = NETWORK_RESPONSE;
        msg_buf.length = length + MSG_HEADER_SIZE;
        memcpy(msg_buf.data, packet_buf, length);

        if (send(user_info->fd, (void *)&msg_buf, msg_buf.length, 0) < 0) {
            perror("send response");
        }
    } else {
        debug("invalid src address\n");
    }
}
