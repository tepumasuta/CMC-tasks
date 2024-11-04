CC=gcc-13.2
CFLAGS=-Wall -Wextra -std=c11 # Used `-fanalyzer`, but recieved false positive

all: debug

debug: CFLAGS+=-g -ggdb
debug: bin/graph

release: CFLAGS+=-DNDEBUG -O2
release: bin/graph

bin/graph: graph.o fractions.o
	$(CC) $(CFLAGS) $^ -o $@

graph.o: graph.c fractions.h
	$(CC) $(CFLAGS) $< -c -o $@

fractions.o: fractions.c fractions.h
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -rf *.o bin/graph

.PHONY: all clean debug release
