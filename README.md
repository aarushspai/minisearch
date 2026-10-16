# MiniSearch - a mini search engine in C

MiniSearch indexes a set of text documents and answers queries like a tiny Google:
ranked results, highlighted snippets, autocomplete, and "did you mean" spelling correction.
Everything is built from scratch in C - no external libraries.

## Features
- **Ranked search** using TF-IDF scoring, best 5 results shown with highlighted snippets
- **Autocomplete**: type `sor` and get `sorted, sort, sorting, sorts` ranked by frequency
- **Spell correction**: `binery sarch` -> "Did you mean: binary search ?"
- **Add documents live**: new text is indexed instantly
- **Index statistics**: hash-table load factor, collision chains, most frequent words
- **Recent searches** remembered in a circular queue

## DSA concepts used
| Concept | File | Used for |
|---|---|---|
| Hash table (separate chaining) | `index.c` | Inverted index: word -> documents, O(1) average lookup |
| Linked lists | `index.c` | Posting lists and collision chains |
| Trie | `trie.c` | Prefix autocomplete |
| Min-heap | `heap.c` | Top-K ranking in O(n log k) |
| Dynamic programming | `spell.c` | Edit distance (Levenshtein) for spell correction |
| Circular queue | `history.c` | Last 5 searches |
| Dynamic array | `engine.c` | Growing list of documents |

## Build and run
Run these inside the project folder:
```bash
gcc -Wall -Wextra -std=c11 -o minisearch main.c heap.c trie.c tokenizer.c index.c spell.c history.c engine.c -lm
./minisearch            # Windows: .\minisearch.exe
```
Tests (33 checks):
```bash
gcc -Wall -Wextra -std=c11 -o test_runner tests.c heap.c trie.c tokenizer.c index.c spell.c history.c engine.c -lm
./test_runner           # Windows: .\test_runner.exe
```
Or with make: `make` and `make test`.

## Project layout
```
main.c        user interface (menu, snippets, colours)
engine.c      documents, file loading, TF-IDF search
index.c       hash-table inverted index + spell suggestions
trie.c        autocomplete tree
heap.c        min-heap and top-K selection
spell.c       edit distance (dynamic programming)
history.c     circular-queue search history
tokenizer.c   word splitting and stop words
tests.c       automated tests
data/articles.txt   sample documents (format: "@@ Title" then text)
docs/         report and presentation guide
```
