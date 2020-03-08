#include "User_Info_Table.h"

void init_user_info(uint32_t start_addr) {
    for(int i = 0; i < N_USERS; i++) {
        user_info_table[i].fd = -1;
        user_info_table[i].v4addr.s_addr = htonl(start_addr+i); // 网络字节序
    }
}
