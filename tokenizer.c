#include <ctype.h>
#include <string.h>
#include "common.h"
#include "tokenizer.h"

static const char *STOP[] = {
    "the", "and", "is", "are", "was", "were", "of", "to", "in", "on", "for",
    "with", "as", "by", "at", "an", "it", "its", "this", "that", "from", "or",
    "be", "can", "has", "have", "had", "not", "but", "which", "their", "they",
    "into", "than", "then", "also", "such", "when", "each", "so", "if", "you",
    "your", "does", "do", "who", "what", "how", "there", "these", "those"
};

int is_stopword(const char *word) {
    for (size_t i = 0; i < sizeof(STOP) / sizeof(STOP[0]); i++)
        if (strcmp(word, STOP[i]) == 0) return 1;
    return 0;
}

int next_word(const char *t, int *pos, char *out) {
    int i = *pos;
    for (;;) {
        while (t[i] && !isalpha((unsigned char)t[i])) i++;      /* skip punctuation, digits, spaces */
        if (!t[i]) { *pos = i; return 0; }
        int len = 0;
        while (isalpha((unsigned char)t[i])) {
            if (len < WORD_LEN - 1) out[len] = (char)tolower((unsigned char)t[i]);
            len++;
            i++;
        }
        if (len >= 2 && len < WORD_LEN) {                        /* ignore 1-letter and huge words */
            out[len] = '\0';
            *pos = i;
            return 1;
        }
    }
}
