#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trie.h"

static int idx(char c) {
    c = (char)tolower((unsigned char)c);
    return (c >= 'a' && c <= 'z') ? c - 'a' : -1;
}

TrieNode *trie_create(void) { return calloc(1, sizeof(TrieNode)); }

void trie_add(TrieNode *root, const char *word) {
    TrieNode *node = root;
    for (; *word; word++) {
        int i = idx(*word);
        if (i < 0) return;
        if (!node->children[i]) node->children[i] = trie_create();
        node = node->children[i];
    }
    node->freq++;
}

int trie_freq(const TrieNode *root, const char *word) {
    const TrieNode *node = root;
    for (; *word; word++) {
        int i = idx(*word);
        if (i < 0 || !node->children[i]) return 0;
        node = node->children[i];
    }
    return node->freq;
}

typedef struct {
    WordFreq *arr;
    int n, cap;
} Bag;

static void bag_add(Bag *b, const char *word, int freq) {
    if (b->n == b->cap) {
        b->cap = b->cap ? b->cap * 2 : 32;
        b->arr = realloc(b->arr, sizeof(WordFreq) * b->cap);
    }
    strcpy(b->arr[b->n].word, word);
    b->arr[b->n].freq = freq;
    b->n++;
}

/* Depth-first walk below a node. Visiting children a..z gives alphabetical order. */
static void collect(const TrieNode *node, char *buf, int depth, Bag *bag) {
    if (node->freq > 0) {
        buf[depth] = '\0';
        bag_add(bag, buf, node->freq);
    }
    if (depth >= WORD_LEN - 1) return;
    for (int i = 0; i < 26; i++) {
        if (node->children[i]) {
            buf[depth] = (char)('a' + i);
            collect(node->children[i], buf, depth + 1, bag);
        }
    }
}

int trie_complete(const TrieNode *root, const char *prefix, WordFreq **out) {
    Bag bag = {NULL, 0, 0};
    char buf[WORD_LEN];
    int depth = 0;
    const TrieNode *node = root;
    *out = NULL;
    for (; *prefix; prefix++) {                 /* walk down the prefix first */
        int i = idx(*prefix);
        if (i < 0 || depth >= WORD_LEN - 1 || !node->children[i]) return 0;
        node = node->children[i];
        buf[depth++] = (char)tolower((unsigned char)*prefix);
    }
    collect(node, buf, depth, &bag);
    *out = bag.arr;
    return bag.n;
}

void trie_free(TrieNode *root) {
    if (!root) return;
    for (int i = 0; i < 26; i++) trie_free(root->children[i]);
    free(root);
}
