#ifndef HISTORY_H
#define HISTORY_H
#include "common.h"

#define HISTORY_SIZE 5

/* Circular buffer remembering the last few searches; oldest is overwritten. */
typedef struct {
    char items[HISTORY_SIZE][TITLE_LEN];
    int head;     /* index of the oldest entry */
    int size;
} History;

void        history_init(History *h);
void        history_add(History *h, const char *query);
/* i = 0 is the newest search. Returns NULL if i is out of range. */
const char *history_get(const History *h, int i);

#endif
