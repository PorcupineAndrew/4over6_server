#include "Msg.h"
#include "User_Info_Table.h"

int listen_fd; // 监听socket

void init_server(int port, char* addr, int cons, char* if_name);

int main() {
    fprintf(stderr, "hello world\n");
    init_server(
        SERVER_LISTEN_PORT,
        // NULL,
        "fe80::d4a2:99ab:92c8:f22",
        MAX_LISTEN_QUEUE,
        "wlp2s0"
    );
}
