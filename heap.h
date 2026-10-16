#ifndef HEAP_H
#define HEAP_H

typedef struct {
    double priority;
    int id;
} HeapItem;

typedef struct {
    HeapItem *items;
    int size;
    int capacity;
} MinHeap;

MinHeap *heap_create(int capacity);
void     heap_push(MinHeap *h, double priority, int id);
HeapItem heap_pop(MinHeap *h);                 /* removes the SMALLEST item */
int      heap_empty(const MinHeap *h);
void     heap_free(MinHeap *h);

/* Picks the k largest positive scores out of scores[0..n-1].
   Writes their indices to out[] (best first) and returns how many were found. */
int topk_indices(const double *scores, int n, int k, int out[]);

#endif
