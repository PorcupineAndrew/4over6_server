#include "User_Info_Table.h"

void init_user_info(uint32_t start_addr, int n_users) {
    for(int i = 0; i < n_users; i++) {
        user_info_table[i].fd = -1;
        user_info_table[i].v4addr.s_addr = htonl(start_addr+i); // 网络字节序
    }

    char s[24];
    inet_ntop(AF_INET, &user_info_table[0].v4addr.s_addr, s, 24);
    fprintf(stdout, 
        "User info init\n\tstart_addr: %s\n\tmax_users: %d\n",
        s, n_users
    );
}

int insert_user_table(int infd, struct in6_addr *inaddr, int n_users, pthread_mutex_t *mutex) {
	pthread_mutex_lock(mutex);
    int i = 0;
    for(; i < n_users; i++) {
        if (user_info_table[i].fd == -1) {
            user_info_table[i].fd = infd;
            user_info_table[i].count = KEEPLIVE_COUNT;
            user_info_table[i].secs = time(NULL);
            memcpy(&user_info_table[i].v6addr, inaddr, sizeof(struct in6_addr));
            break;
        }
    }
	pthread_mutex_unlock(mutex);

    if (i == n_users) return -1;

    char s4[24], s6[64];
    inet_ntop(AF_INET, &user_info_table[i].v4addr.s_addr, s4, 24);
    inet_ntop(AF_INET6, inaddr, s6, 64);
    fprintf(stdout, 
        "User info update\n\tindex: %d\n\tv4_addr: %s\n\tv6_addr: %s\n",
        i, s4, s6
    );

    return 0;
}

struct User_Info* get_user_by_IPv4(uint32_t addr, pthread_mutex_t *mutex) {
    pthread_mutex_lock(mutex);
    for (int i = 0; i < N_USERS; i++) {
        if (user_info_table[i].fd != -1 && (user_info_table[i].v4addr.s_addr == addr)) {
            pthread_mutex_unlock(mutex);
            return &user_info_table[i];
        }
    }
    pthread_mutex_unlock(mutex);
    return NULL;
}

void rm_user_by_fd(int fd, pthread_mutex_t *mutex) {
    pthread_mutex_lock(mutex);
    for (int i = 0; i < N_USERS; i++) {
        if (user_info_table[i].fd == fd) {
            user_info_table[i].fd = -1;
        }
    }
    pthread_mutex_unlock(mutex);
}
