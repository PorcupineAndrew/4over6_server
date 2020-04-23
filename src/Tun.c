#include "Macro.h"

extern int tun_fd;

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
    

    fprintf(stdout, "Tun init: %s\n", devname);
}
