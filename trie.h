#ifndef TRIE_H
#define TRIE_H
#include "common.h"

typedef struct TrieNode {
    struct TrieNode *children[26];   /* one pointer per letter a-z */
    int freq;                        /* >0 means a word ends here; value = occurrences */
} TrieNode;

TrieNode *trie_create(void);
void      trie_add(TrieNode *root, const char *word);   /* freq++ */
int       trie_freq(const TrieNode *root, const char *word);
/* All words starting with prefix (alphabetical). Caller must free(*out). */
int       trie_complete(const TrieNode *root, const char *prefix, WordFreq **out);
void      trie_free(TrieNode *root);

#endif
