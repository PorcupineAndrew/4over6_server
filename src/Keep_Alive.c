#include "User_Info.h"
#include "Msg.h"

void process_heartbeat(int epoll_fd) {
    for(int i = 0; i < MAX_CLIENTS; i ++)
        if (users[i].fd >= 0 && !users[i].is_free) {
            time_t now = time(0);
            if (now - users[i].last_heartbeat_sent_secs > HEARTBEAT_INTERVAL) {
                Msg heartbeat;
                heartbeat.type = TYPE_KEEPALIVE;
                heartbeat.length = sizeof(Msg);
                if (write_all(users[i].fd, &heartbeat, sizeof(Msg)) != sizeof(Msg)) {
                    debug("fail to send heartbeat packet\n");
                }
                users[i].last_heartbeat_sent_secs = now;
            }
            if (now - users[i].last_heartbeat_recved_secs > HEARTBEAT_TIMEOUT) {
                infof("connect timeout: %d\n", i);
                free_client_fd(users[i].fd, epoll_fd);
            }
        }
}
