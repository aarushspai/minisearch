#include <string.h>
#include "history.h"

void history_init(History *h) {
    h->head = 0;
    h->size = 0;
}

void history_add(History *h, const char *query) {
    if (h->size > 0 && strcmp(history_get(h, 0), query) == 0) return;   /* skip repeats */
    int slot;
    if (h->size < HISTORY_SIZE) {
        slot = (h->head + h->size) % HISTORY_SIZE;
        h->size++;
    } else {                                   /* full: overwrite the oldest */
        slot = h->head;
        h->head = (h->head + 1) % HISTORY_SIZE;
    }
    strncpy(h->items[slot], query, TITLE_LEN - 1);
    h->items[slot][TITLE_LEN - 1] = '\0';
}

const char *history_get(const History *h, int i) {
    if (i < 0 || i >= h->size) return NULL;
    return h->items[(h->head + h->size - 1 - i) % HISTORY_SIZE];
}
