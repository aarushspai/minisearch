#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "heap.h"
#include "trie.h"
#include "index.h"
#include "spell.h"
#include "history.h"
#include "engine.h"

static int passed = 0, failed = 0;
#define CHECK(name, cond) do { \
    if (cond) { printf("  [PASS] %s\n", name); passed++; } \
    else      { printf("  [FAIL] %s\n", name); failed++; } } while (0)

static void test_heap(void) {
    double s[] = {0.5, 0.0, 2.0, 2.0, 1.0};
    int out[10];
    int n = topk_indices(s, 5, 3, out);
    CHECK("top-k returns 3 results", n == 3);
    CHECK("top-k order (ties: lower id first)", out[0] == 2 && out[1] == 3 && out[2] == 4);
    CHECK("top-k ignores zero scores", topk_indices(s, 5, 10, out) == 4);

    MinHeap *h = heap_create(1);                  /* tiny: forces the heap to grow */
    for (int i = 0; i < 20; i++) heap_push(h, (double)((i * 7) % 20), i);
    int ok = 1;
    double last = -1;
    while (!heap_empty(h)) {
        double p = heap_pop(h).priority;
        if (p < last) ok = 0;
        last = p;
    }
    CHECK("heap pops in sorted order and grows", ok);
    heap_free(h);
}

static void test_trie(void) {
    TrieNode *t = trie_create();
    trie_add(t, "cat"); trie_add(t, "cat"); trie_add(t, "cat");
    trie_add(t, "car"); trie_add(t, "care"); trie_add(t, "care"); trie_add(t, "dog");
    WordFreq *arr;
    int n = trie_complete(t, "ca", &arr);
    CHECK("trie 'ca' -> 3 words", n == 3);
    CHECK("trie results alphabetical", strcmp(arr[0].word, "car") == 0 &&
          strcmp(arr[1].word, "care") == 0 && strcmp(arr[2].word, "cat") == 0);
    free(arr);
    CHECK("trie counts frequency", trie_freq(t, "cat") == 3 && trie_freq(t, "care") == 2);
    CHECK("trie unknown prefix -> 0", trie_complete(t, "x", &arr) == 0);
    trie_free(t);
}

static void test_index(void) {
    Index *ix = index_create();
    index_add(ix, "apple", 0); index_add(ix, "apple", 0); index_add(ix, "apple", 1);
    index_add(ix, "banana", 1);
    const Entry *e = index_find(ix, "apple");
    CHECK("index finds a word", e != NULL);
    CHECK("index doc_freq = 2, total = 3", e && e->doc_freq == 2 && e->total_freq == 3);
    CHECK("posting count in doc 0 is 2", e && e->postings->next && e->postings->next->doc == 0 &&
          e->postings->next->count == 2);
    CHECK("index missing word -> NULL", index_find(ix, "cherry") == NULL);
    char s[WORD_LEN];
    CHECK("suggest 'aple' -> apple", index_suggest(ix, "aple", s) && strcmp(s, "apple") == 0);
    CHECK("no suggestion for nonsense", !index_suggest(ix, "zzzzzzzz", s));
    index_free(ix);
}

static void test_spell(void) {
    CHECK("edit distance kitten/sitting = 3", edit_distance("kitten", "sitting") == 3);
    CHECK("edit distance flaw/lawn = 2", edit_distance("flaw", "lawn") == 2);
    CHECK("edit distance same = 0", edit_distance("same", "same") == 0);
    CHECK("edit distance ''/abc = 3", edit_distance("", "abc") == 3);
}

static void test_history(void) {
    History h;
    history_init(&h);
    char q[8];
    for (int i = 1; i <= 6; i++) { sprintf(q, "q%d", i); history_add(&h, q); }
    CHECK("history keeps only 5", h.size == 5);
    CHECK("history newest is q6", strcmp(history_get(&h, 0), "q6") == 0);
    CHECK("history oldest is q2 (q1 overwritten)", strcmp(history_get(&h, 4), "q2") == 0);
    history_add(&h, "q6");
    CHECK("history skips immediate repeat", h.size == 5 && strcmp(history_get(&h, 1), "q5") == 0);
}

static void test_engine(void) {
    Engine *e = engine_create();
    engine_add_document(e, "Cats", "cats purr and cats sleep");
    engine_add_document(e, "Dogs", "dogs bark and dogs run");
    engine_add_document(e, "Pets", "cats and dogs are pets");
    SearchResult r;

    engine_search(e, "cats", &r);
    CHECK("search 'cats' -> 2 matches", r.total_matches == 2 && r.nhits == 2);
    CHECK("more mentions rank higher", r.hits[0].doc == 0 && r.hits[1].doc == 2);

    engine_search(e, "cats dogs", &r);
    CHECK("matching both words ranks first", r.hits[0].doc == 2);
    CHECK("tied scores: lower id first", r.hits[1].doc == 0 && r.hits[2].doc == 1);

    engine_search(e, "the and of", &r);
    CHECK("only stop words -> no terms", r.nterms == 0 && r.nhits == 0);

    engine_search(e, "catz", &r);
    CHECK("typo detected", r.nunknown == 1 && r.nhits == 0);
    CHECK("typo suggestion is 'cats'", strcmp(r.suggestion[0], "cats") == 0);
    engine_free(e);

    e = engine_create();
    int n = engine_load_file(e, "data/articles.txt");
    CHECK("sample file loads 12 documents", n == 12);
    engine_search(e, "binary search", &r);
    CHECK("'binary search' ranks Binary Search first",
          r.nhits > 0 && strcmp(e->docs[r.hits[0].doc].title, "Binary Search") == 0);
    engine_search(e, "binery sarch", &r);
    CHECK("typos fixed to binary / search", r.nunknown == 2 &&
          strcmp(r.suggestion[0], "binary") == 0 && strcmp(r.suggestion[1], "search") == 0);
    CHECK("missing file returns -1", engine_load_file(e, "no_such_file.txt") == -1);
    engine_free(e);
}

int main(void) {
    printf("Running tests...\n");
    test_heap(); test_trie(); test_index(); test_spell(); test_history(); test_engine();
    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
