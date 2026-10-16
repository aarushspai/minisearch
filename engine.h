#ifndef ENGINE_H
#define ENGINE_H
#include "common.h"
#include "index.h"
#include "trie.h"

typedef struct {
    char title[TITLE_LEN];
    char *text;
    int length;               /* number of indexed (non-stop) words */
} Document;

typedef struct {
    Document *docs;           /* dynamic array */
    int count, capacity;
    long total_words;
    Index *ix;                /* word -> which documents (hash table) */
    TrieNode *trie;           /* for autocomplete */
} Engine;

typedef struct { int doc; double score; } Hit;

typedef struct {
    Hit hits[MAX_HITS];
    int nhits;
    int total_matches;
    char terms[MAX_TERMS][WORD_LEN];        /* meaningful query words */
    int nterms;
    char unknown[MAX_TERMS][WORD_LEN];      /* query words not in the index */
    char suggestion[MAX_TERMS][WORD_LEN];   /* "did you mean" for each (may be "") */
    int nunknown;
} SearchResult;

Engine *engine_create(void);
int     engine_add_document(Engine *e, const char *title, const char *text);   /* id or -1 */
int     engine_load_file(Engine *e, const char *path);                         /* count or -1 */
void    engine_search(const Engine *e, const char *query, SearchResult *out);
void    engine_free(Engine *e);

#endif
