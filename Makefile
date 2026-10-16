CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
SRC     = heap.c trie.c tokenizer.c index.c spell.c history.c engine.c

all: minisearch

minisearch: main.c $(SRC)
	$(CC) $(CFLAGS) -o minisearch main.c $(SRC) -lm

test: tests.c $(SRC)
	$(CC) $(CFLAGS) -o test_runner tests.c $(SRC) -lm
	./test_runner

clean:
	rm -f minisearch test_runner minisearch.exe test_runner.exe
