#ifndef COMMON_H
#define COMMON_H

#define WORD_LEN    32     /* max chars in an indexed word (incl. '\0') */
#define TITLE_LEN   80     /* max chars in a document title */
#define MAX_TERMS   8      /* max words used from one query */
#define MAX_HITS    5      /* how many top results we show (the "K" in top-K) */
#define TABLE_SIZE  1031   /* hash table buckets (a prime number spreads keys better) */

/* A word together with how often it occurs. */
typedef struct {
    char word[WORD_LEN];
    int freq;
} WordFreq;

#endif
