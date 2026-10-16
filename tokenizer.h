#ifndef TOKENIZER_H
#define TOKENIZER_H

/* Reads the next lowercase word (letters only, 2..31 chars) starting at *pos.
   Returns 1 and fills out, or 0 when the text is finished. */
int next_word(const char *text, int *pos, char *out);

/* Common words like "the" or "and" that carry no search value. */
int is_stopword(const char *word);

#endif
