#include "User_Info_Table.h"
#include "Msg.h"

extern pthread_mutex_t MUTEX;
extern pthread_t keep_alive_thread;
int rm_epoll(int fd);

char keepalive_buf[MSG_HEADER_SIZE];

void* keep_alive_poll() {
    pthread_mutex_lock(&MUTEX);
    while(1) {
        pthread_mutex_unlock(&MUTEX);
        sleep(1);
        pthread_mutex_lock(&MUTEX);
        for (int i = 0; i < N_USERS; i++) {
            int fd = user_info_table[i].fd;
            if (fd == -1) continue;
            if (time(NULL) - user_info_table[i].secs > 60) {
                user_info_table[i].fd = -1;
                rm_epoll(fd);
                close(fd);
            } else if (--user_info_table[i].count == 0) {
                user_info_table[i].count = KEEPLIVE_COUNT;
                struct Msg *kp_msg = (struct Msg *) keepalive_buf;
                kp_msg->length = MSG_HEADER_SIZE;
                kp_msg->type = KEEPALIVE;
                if (send(fd, (void *)kp_msg, kp_msg->length, 0) < 0) {
                    perror("send kp_msg");
                }
            }
        }
    }
}

void init_keep_alive_thread() {
    if (pthread_create(&keep_alive_thread, NULL, keep_alive_poll, NULL) < 0) {
        perror("init keep alive thread");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "init keep_alive_thread\n");
}
    
