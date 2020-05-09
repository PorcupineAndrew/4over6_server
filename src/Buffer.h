#ifndef BUFFER_H
#define BUFFER_H
#include "Macro.h"

typedef struct buffer_entry {
    char buf[BUF_SIZE];
    int used;
} buffer_entry;

buffer_entry bufs[MAX_FDS];
#endif
