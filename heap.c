#include <stdlib.h>
#include "heap.h"

/* "a is smaller than b". Equal scores: the HIGHER id counts as smaller, so it is
   evicted first and lower ids rank better. This keeps results deterministic. */
static int less(HeapItem a, HeapItem b) {
    if (a.priority != b.priority) return a.priority < b.priority;
    return a.id > b.id;
}

static void swap(HeapItem *a, HeapItem *b) {
    HeapItem t = *a;
    *a = *b;
    *b = t;
}

MinHeap *heap_create(int capacity) {
    MinHeap *h = malloc(sizeof(MinHeap));
    h->items = malloc(sizeof(HeapItem) * capacity);
    h->size = 0;
    h->capacity = capacity;
    return h;
}

static void sift_up(MinHeap *h, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (!less(h->items[i], h->items[parent])) break;
        swap(&h->items[i], &h->items[parent]);
        i = parent;
    }
}

static void sift_down(MinHeap *h, int i) {
    for (;;) {
        int l = 2 * i + 1, r = 2 * i + 2, smallest = i;
        if (l < h->size && less(h->items[l], h->items[smallest])) smallest = l;
        if (r < h->size && less(h->items[r], h->items[smallest])) smallest = r;
        if (smallest == i) break;
        swap(&h->items[i], &h->items[smallest]);
        i = smallest;
    }
}

void heap_push(MinHeap *h, double priority, int id) {
    if (h->size == h->capacity) {
        h->capacity *= 2;
        h->items = realloc(h->items, sizeof(HeapItem) * h->capacity);
    }
    h->items[h->size].priority = priority;
    h->items[h->size].id = id;
    sift_up(h, h->size);
    h->size++;
}

HeapItem heap_pop(MinHeap *h) {
    HeapItem top = h->items[0];
    h->size--;
    h->items[0] = h->items[h->size];
    sift_down(h, 0);
    return top;
}

int heap_empty(const MinHeap *h) { return h->size == 0; }

void heap_free(MinHeap *h) {
    free(h->items);
    free(h);
}

/* Classic top-K trick: keep a MIN-heap of size k. Whenever it grows past k,
   throw away the smallest. What remains are the k biggest. O(n log k). */
int topk_indices(const double *scores, int n, int k, int out[]) {
    if (k <= 0) return 0;
    MinHeap *h = heap_create(k + 1);
    for (int i = 0; i < n; i++) {
        if (scores[i] <= 0) continue;
        heap_push(h, scores[i], i);
        if (h->size > k) heap_pop(h);
    }
    int count = h->size;
    for (int i = count - 1; i >= 0; i--)       /* weakest pops first -> fill from the back */
        out[i] = heap_pop(h).id;
    heap_free(h);
    return count;
}
