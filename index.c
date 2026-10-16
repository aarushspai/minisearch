#include <stdlib.h>
#include <string.h>
#include "index.h"
#include "spell.h"

/* djb2: a simple, well-known string hash. */
static unsigned long hash(const char *s) {
    unsigned long h = 5381;
    for (; *s; s++) h = ((h << 5) + h) + (unsigned char)*s;
    return h % TABLE_SIZE;
}

Index *index_create(void) { return calloc(1, sizeof(Index)); }

const Entry *index_find(const Index *ix, const char *word) {
    for (const Entry *e = ix->buckets[hash(word)]; e; e = e->next)
        if (strcmp(e->word, word) == 0) return e;
    return NULL;
}

/* Documents are indexed one after another, so if the word was already seen in
   THIS document its posting is the head of the list: no searching needed, O(1). */
void index_add(Index *ix, const char *word, int doc) {
    unsigned long b = hash(word);
    Entry *e = ix->buckets[b];
    while (e && strcmp(e->word, word) != 0) e = e->next;

    if (!e) {                                      /* brand-new word */
        e = calloc(1, sizeof(Entry));
        strcpy(e->word, word);
        e->next = ix->buckets[b];                  /* chain it at the head of the bucket */
        ix->buckets[b] = e;
        ix->words++;
    }
    e->total_freq++;
    if (e->postings && e->postings->doc == doc) {
        e->postings->count++;
    } else {
        Posting *p = malloc(sizeof(Posting));
        p->doc = doc;
        p->count = 1;
        p->next = e->postings;
        e->postings = p;
        e->doc_freq++;
    }
}

int index_suggest(const Index *ix, const char *word, char *out) {
    int len = (int)strlen(word);
    int limit = len <= 4 ? 1 : 2;                  /* short words: allow fewer typos */
    int best = limit + 1, best_freq = 0, found = 0;
    for (int b = 0; b < TABLE_SIZE; b++) {
        for (const Entry *e = ix->buckets[b]; e; e = e->next) {
            int diff = (int)strlen(e->word) - len;
            if (diff > limit || diff < -limit) continue;     /* cheap length filter */
            int d = edit_distance(word, e->word);
            if (d < best || (d == best && e->total_freq > best_freq)) {
                best = d;
                best_freq = e->total_freq;
                strcpy(out, e->word);
                found = 1;
            }
        }
    }
    return found && best <= limit;
}

void index_stats(const Index *ix, IndexStats *st) {
    st->words = ix->words;
    st->empty_buckets = 0;
    st->longest_chain = 0;
    for (int b = 0; b < TABLE_SIZE; b++) {
        int len = 0;
        for (const Entry *e = ix->buckets[b]; e; e = e->next) len++;
        if (len == 0) st->empty_buckets++;
        if (len > st->longest_chain) st->longest_chain = len;
    }
    st->load_factor = (double)ix->words / TABLE_SIZE;
}

void index_free(Index *ix) {
    for (int b = 0; b < TABLE_SIZE; b++) {
        Entry *e = ix->buckets[b];
        while (e) {
            Entry *next = e->next;
            Posting *p = e->postings;
            while (p) {
                Posting *pn = p->next;
                free(p);
                p = pn;
            }
            free(e);
            e = next;
        }
    }
    free(ix);
}
