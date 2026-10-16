/* MiniSearch - a mini search engine in C.
   Hash-table inverted index, trie autocomplete, min-heap top-K ranking,
   edit-distance spell correction and a circular-queue search history. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "engine.h"
#include "history.h"
#include "heap.h"
#include "tokenizer.h"

#define DEFAULT_CORPUS "data/articles.txt"

static int use_color = 0;
#define RESET  (use_color ? "\x1b[0m"    : "")
#define BOLD   (use_color ? "\x1b[1m"    : "")
#define CYAN   (use_color ? "\x1b[36m"   : "")
#define GREEN  (use_color ? "\x1b[32m"   : "")
#define RED    (use_color ? "\x1b[31m"   : "")
#define DIM    (use_color ? "\x1b[90m"   : "")
#define HL_ON  (use_color ? "\x1b[1;33m" : "[")
#define HL_OFF (use_color ? "\x1b[0m"    : "]")

static void enable_color(void) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode) &&
        SetConsoleMode(h, mode | 0x0004))      /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
        use_color = 1;
#else
    use_color = isatty(1);
#endif
}

/* Prints a prompt and reads one trimmed line. Returns 0 on EOF. */
static int read_line(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, size, stdin)) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    char *s = buf;
    while (*s == ' ') s++;
    memmove(buf, s, strlen(s) + 1);
    for (int n = (int)strlen(buf); n > 0 && buf[n - 1] == ' '; n--) buf[n - 1] = '\0';
    return 1;
}

static void banner(void) {
    printf("\n%s%s", CYAN, BOLD);
    printf("  +------------------------------------------------------+\n");
    printf("  |  M I N I S E A R C H                                 |\n");
    printf("  |  A tiny search engine built from scratch in C        |\n");
    printf("  +------------------------------------------------------+\n");
    printf("%s", RESET);
}

static int matches_term(const char *w, char terms[][WORD_LEN], int n) {
    for (int i = 0; i < n; i++)
        if (strcmp(w, terms[i]) == 0) return 1;
    return 0;
}

/* Shows ~110 characters of the document around the first matching word,
   highlighting every query word. */
static void print_snippet(const char *text, char terms[][WORD_LEN], int nterms) {
    char w[WORD_LEN];
    int pos = 0, first = 0;
    while (next_word(text, &pos, w)) {
        if (matches_term(w, terms, nterms)) { first = pos - (int)strlen(w); break; }
    }
    int from = first > 40 ? first - 40 : 0;
    if (from > 0) {                               /* start at a word boundary */
        while (text[from] && text[from] != ' ') from++;
        if (text[from]) from++;
        if (from > first) from = first;
        printf("...");
    }
    int i = from, printed = 0;
    while (text[i] && printed < 110) {
        if (isalpha((unsigned char)text[i])) {
            int j = i, n = 0;
            char lw[WORD_LEN];
            while (isalpha((unsigned char)text[j])) {
                if (n < WORD_LEN - 1) lw[n] = (char)tolower((unsigned char)text[j]);
                n++;
                j++;
            }
            int hit = 0;
            if (n < WORD_LEN) { lw[n] = '\0'; hit = matches_term(lw, terms, nterms); }
            if (hit) printf("%s%.*s%s", HL_ON, j - i, text + i, HL_OFF);
            else     printf("%.*s", j - i, text + i);
            printed += j - i;
            i = j;
        } else {
            putchar(text[i]);
            printed++;
            i++;
        }
    }
    if (text[i]) printf("...");
}

static void run_search(Engine *e, History *h, const char *query, int allow_suggest) {
    SearchResult r;
    engine_search(e, query, &r);
    if (r.nterms == 0) {
        printf("  Please type at least one meaningful word.\n");
        return;
    }
    history_add(h, query);

    printf("\n  Searching for:");
    for (int i = 0; i < r.nterms; i++) printf(" %s", r.terms[i]);
    printf("\n");

    if (r.nhits > 0) {
        printf("  %s%d matching document%s%s (showing top %d)\n\n", GREEN, r.total_matches,
               r.total_matches == 1 ? "" : "s", RESET, r.nhits);
        for (int i = 0; i < r.nhits; i++) {
            const Document *d = &e->docs[r.hits[i].doc];
            printf("  %s%d. %s%s   %s(score %.3f)%s\n", BOLD, i + 1, d->title, RESET,
                   DIM, r.hits[i].score, RESET);
            printf("     ");
            print_snippet(d->text, r.terms, r.nterms);
            printf("\n\n");
        }
    } else {
        printf("  %sNo documents matched.%s\n", RED, RESET);
    }

    if (!allow_suggest || r.nunknown == 0) return;

    char corrected[256] = "";
    int any = 0;
    for (int i = 0; i < r.nunknown; i++) {
        if (r.suggestion[i][0]) any = 1;
        else printf("  %s'%s' is not in the index.%s\n", DIM, r.unknown[i], RESET);
    }
    if (!any) return;
    for (int t = 0; t < r.nterms; t++) {          /* rebuild the query with fixes */
        const char *word = r.terms[t];
        for (int u = 0; u < r.nunknown; u++)
            if (strcmp(r.unknown[u], word) == 0 && r.suggestion[u][0]) word = r.suggestion[u];
        if (strlen(corrected) + strlen(word) + 2 < sizeof corrected) {
            strcat(corrected, word);
            strcat(corrected, " ");
        }
    }
    size_t clen = strlen(corrected);              /* drop the trailing space */
    if (clen > 0) corrected[clen - 1] = '\0';
    printf("  %sDid you mean:%s %s ?\n", BOLD, RESET, corrected);
    char yn[16];
    if (read_line("  Search that instead? (y/n): ", yn, sizeof yn) && (yn[0] == 'y' || yn[0] == 'Y'))
        run_search(e, h, corrected, 0);
}

static void cmd_search(Engine *e, History *h) {
    char q[256];
    if (!read_line("Search: ", q, sizeof q)) return;
    if (!q[0]) return;
    run_search(e, h, q, 1);
}

static void cmd_autocomplete(const Engine *e) {
    char prefix[WORD_LEN * 2];
    if (!read_line("Type the start of a word: ", prefix, sizeof prefix) || !prefix[0]) return;
    WordFreq *arr = NULL;
    int n = trie_complete(e->trie, prefix, &arr);
    if (n == 0) {
        printf("  No indexed word starts with '%s'.\n", prefix);
    } else {
        double *sc = malloc(sizeof(double) * n);
        for (int i = 0; i < n; i++) sc[i] = arr[i].freq;
        int top[5];
        int k = topk_indices(sc, n, 5, top);       /* most frequent completions first */
        printf("  %d word%s start with '%s'. Top suggestions:\n", n, n == 1 ? "" : "s", prefix);
        for (int i = 0; i < k; i++)
            printf("   %d. %-14s (%d occurrence%s)\n", i + 1, arr[top[i]].word, arr[top[i]].freq,
                   arr[top[i]].freq == 1 ? "" : "s");
        free(sc);
    }
    free(arr);
}

static void cmd_add(Engine *e) {
    char title[TITLE_LEN], text[1024];
    if (!read_line("Title: ", title, sizeof title) || !title[0]) return;
    if (!read_line("Text (one line): ", text, sizeof text) || !text[0]) return;
    int before = e->ix->words;
    int id = engine_add_document(e, title, text);
    if (id < 0) { printf("  %sCould not add the document.%s\n", RED, RESET); return; }
    printf("  %sIndexed document #%d%s: %d words, %d new unique word%s.\n", GREEN, id + 1, RESET,
           e->docs[id].length, e->ix->words - before, e->ix->words - before == 1 ? "" : "s");
}

static void cmd_list(const Engine *e) {
    if (e->count == 0) { printf("  No documents loaded.\n"); return; }
    for (int i = 0; i < e->count; i++)
        printf("  %2d. %-40.40s %s(%d words)%s\n", i + 1, e->docs[i].title, DIM, e->docs[i].length, RESET);
}

static void cmd_stats(const Engine *e) {
    IndexStats st;
    index_stats(e->ix, &st);
    printf("\n  %sIndex statistics%s\n", BOLD, RESET);
    printf("  Documents ............ %d\n", e->count);
    printf("  Indexed words ........ %ld\n", e->total_words);
    printf("  Unique words ......... %d\n", st.words);
    printf("  Hash buckets ......... %d  (empty: %d)\n", TABLE_SIZE, st.empty_buckets);
    printf("  Load factor .......... %.2f\n", st.load_factor);
    printf("  Longest collision chain: %d\n\n", st.longest_chain);

    WordFreq *arr = NULL;
    int n = trie_complete(e->trie, "", &arr);
    if (n > 0) {
        double *sc = malloc(sizeof(double) * n);
        for (int i = 0; i < n; i++) sc[i] = arr[i].freq;
        int top[10];
        int k = topk_indices(sc, n, 10, top);
        printf("  %sTop %d most frequent words%s\n", BOLD, k, RESET);
        for (int i = 0; i < k; i++) {
            int bar = arr[top[i]].freq * 24 / arr[top[0]].freq;
            printf("   %-14s %3d  ", arr[top[i]].word, arr[top[i]].freq);
            for (int b = 0; b < bar; b++) putchar('#');
            putchar('\n');
        }
        free(sc);
    }
    free(arr);
}

static void cmd_load(Engine *e) {
    char path[256];
    if (!read_line("File path: ", path, sizeof path) || !path[0]) return;
    int n = engine_load_file(e, path);
    if (n < 0) printf("  %sCould not open '%s'.%s\n", RED, path, RESET);
    else       printf("  %sLoaded %d document%s.%s\n", GREEN, n, n == 1 ? "" : "s", RESET);
}

static void cmd_recent(Engine *e, History *h) {
    if (h->size == 0) { printf("  No searches yet.\n"); return; }
    printf("  Recent searches (newest first):\n");
    for (int i = 0; i < h->size; i++) printf("   %d. %s\n", i + 1, history_get(h, i));
    char in[16];
    if (!read_line("  Number to search again (blank to skip): ", in, sizeof in) || !in[0]) return;
    int pick = atoi(in);
    const char *q = history_get(h, pick - 1);
    if (!q) { printf("  Invalid number.\n"); return; }
    char copy[TITLE_LEN];
    strcpy(copy, q);                              /* history may change while searching */
    run_search(e, h, copy, 1);
}

int main(void) {
    enable_color();
    Engine *e = engine_create();
    History hist;
    history_init(&hist);

    banner();
    int n = engine_load_file(e, DEFAULT_CORPUS);
    if (n < 0)
        printf("\n  %sCould not find %s.%s Run the program from the project folder,\n"
               "  or use option 6 to load a file.\n", RED, DEFAULT_CORPUS, RESET);
    else
        printf("\n  Indexed %d documents, %d unique words.\n", n, e->ix->words);

    char choice[16];
    for (;;) {
        printf("\n  1) Search\n  2) Autocomplete a word\n  3) Add a document\n"
               "  4) List documents\n  5) Index statistics\n  6) Load documents from a file\n"
               "  7) Recent searches\n  0) Exit\n");
        if (!read_line("Choose: ", choice, sizeof choice)) break;
        if      (strcmp(choice, "1") == 0) cmd_search(e, &hist);
        else if (strcmp(choice, "2") == 0) cmd_autocomplete(e);
        else if (strcmp(choice, "3") == 0) cmd_add(e);
        else if (strcmp(choice, "4") == 0) cmd_list(e);
        else if (strcmp(choice, "5") == 0) cmd_stats(e);
        else if (strcmp(choice, "6") == 0) cmd_load(e);
        else if (strcmp(choice, "7") == 0) cmd_recent(e, &hist);
        else if (strcmp(choice, "0") == 0) { printf("  Bye!\n"); break; }
        else printf("  Invalid choice.\n");
    }
    engine_free(e);
    return 0;
}
