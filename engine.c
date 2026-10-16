#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "engine.h"
#include "heap.h"
#include "tokenizer.h"

Engine *engine_create(void) {
    Engine *e = calloc(1, sizeof(Engine));
    e->ix = index_create();
    e->trie = trie_create();
    return e;
}

int engine_add_document(Engine *e, const char *title, const char *text) {
    if (!title || !text) return -1;
    if (e->count == e->capacity) {                       /* dynamic array: double when full */
        int cap = e->capacity ? e->capacity * 2 : 16;
        Document *nd = realloc(e->docs, sizeof(Document) * cap);
        if (!nd) return -1;
        e->docs = nd;
        e->capacity = cap;
    }
    int id = e->count;
    Document *d = &e->docs[id];
    strncpy(d->title, title, TITLE_LEN - 1);
    d->title[TITLE_LEN - 1] = '\0';
    d->text = malloc(strlen(text) + 1);
    strcpy(d->text, text);
    d->length = 0;

    char w[WORD_LEN];
    int pos = 0;
    while (next_word(text, &pos, w)) {
        if (is_stopword(w)) continue;
        index_add(e->ix, w, id);
        trie_add(e->trie, w);
        d->length++;
        e->total_words++;
    }
    e->count++;
    return id;
}

/* Small growable string used while reading a document body. */
static void append(char **buf, size_t *len, size_t *cap, const char *s) {
    size_t n = strlen(s);
    if (*len + n + 2 > *cap) {
        *cap = (*len + n + 2) * 2;
        *buf = realloc(*buf, *cap);
    }
    memcpy(*buf + *len, s, n);
    *len += n;
    (*buf)[*len] = '\0';
}

/* File format:  a line "@@ Title" starts a document; the lines below are its text. */
int engine_load_file(Engine *e, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[1024], title[TITLE_LEN] = "";
    char *body = NULL;
    size_t blen = 0, bcap = 0;
    int have = 0, loaded = 0;

    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "@@", 2) == 0) {
            if (have && blen > 0 && engine_add_document(e, title, body) >= 0) loaded++;
            blen = 0;
            if (body) body[0] = '\0';
            const char *t = line + 2;
            while (*t == ' ') t++;
            strncpy(title, t, TITLE_LEN - 1);
            title[TITLE_LEN - 1] = '\0';
            have = 1;
        } else if (have && line[0]) {
            append(&body, &blen, &bcap, line);
            append(&body, &blen, &bcap, " ");
        }
    }
    if (have && blen > 0 && engine_add_document(e, title, body) >= 0) loaded++;
    free(body);
    fclose(f);
    return loaded;
}

/* TF-IDF ranking:
     tf  = how often the word appears in the document / document length
     idf = log(1 + N / df): rare words are worth more than common ones
   A document's score is the sum over all query words. */
void engine_search(const Engine *e, const char *query, SearchResult *out) {
    memset(out, 0, sizeof(SearchResult));
    double *scores = calloc(e->count > 0 ? e->count : 1, sizeof(double));
    char w[WORD_LEN];
    int pos = 0;

    while (next_word(query, &pos, w)) {
        if (is_stopword(w)) continue;
        int dup = 0;
        for (int t = 0; t < out->nterms; t++)
            if (strcmp(out->terms[t], w) == 0) dup = 1;
        if (dup) continue;
        if (out->nterms >= MAX_TERMS) break;
        strcpy(out->terms[out->nterms++], w);

        const Entry *en = index_find(e->ix, w);
        if (!en) {
            int u = out->nunknown++;
            strcpy(out->unknown[u], w);
            if (!index_suggest(e->ix, w, out->suggestion[u])) out->suggestion[u][0] = '\0';
            continue;
        }
        double idf = log(1.0 + (double)e->count / en->doc_freq);
        for (const Posting *p = en->postings; p; p = p->next)
            scores[p->doc] += ((double)p->count / e->docs[p->doc].length) * idf;
    }

    for (int i = 0; i < e->count; i++)
        if (scores[i] > 0) out->total_matches++;

    int idx[MAX_HITS];
    int n = topk_indices(scores, e->count, MAX_HITS, idx);   /* min-heap picks the best K */
    for (int i = 0; i < n; i++) {
        out->hits[i].doc = idx[i];
        out->hits[i].score = scores[idx[i]];
    }
    out->nhits = n;
    free(scores);
}

void engine_free(Engine *e) {
    for (int i = 0; i < e->count; i++) free(e->docs[i].text);
    free(e->docs);
    index_free(e->ix);
    trie_free(e->trie);
    free(e);
}
