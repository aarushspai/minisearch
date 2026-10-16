#ifndef INDEX_H
#define INDEX_H
#include "common.h"

/* "word appears `count` times in document `doc`" - a node of a linked list. */
typedef struct Posting {
    int doc;
    int count;
    struct Posting *next;
} Posting;

/* One word in the hash table (a node in a bucket's chain). */
typedef struct Entry {
    char word[WORD_LEN];
    int doc_freq;          /* in how many documents the word appears */
    int total_freq;        /* total occurrences in all documents */
    Posting *postings;     /* which documents contain it */
    struct Entry *next;    /* next word in the same bucket (collision chain) */
} Entry;

typedef struct {
    Entry *buckets[TABLE_SIZE];
    int words;             /* number of distinct words */
} Index;

typedef struct {
    int words, empty_buckets, longest_chain;
    double load_factor;
} IndexStats;

Index       *index_create(void);
void         index_add(Index *ix, const char *word, int doc);
const Entry *index_find(const Index *ix, const char *word);
/* Closest indexed word within a small edit distance. Returns 1 if found. */
int          index_suggest(const Index *ix, const char *word, char *out);
void         index_stats(const Index *ix, IndexStats *st);
void         index_free(Index *ix);

#endif
