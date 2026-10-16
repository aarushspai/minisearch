#ifndef SPELL_H
#define SPELL_H

/* Levenshtein distance: the fewest single-letter inserts, deletes or
   replacements needed to turn a into b. Returns 99 for over-long words. */
int edit_distance(const char *a, const char *b);

#endif
