CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -fanalyzer -O2
LFLAGS=-static

all: bin/reverse_polish.out bin/counter.out

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) -c -o $@ $<

reverse_polish.o: reverse_polish.c stack.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/reverse_polish.out: reverse_polish.o stack.o
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

counter.o: counter.c stack.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/counter.out: counter.o stack.o
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

clean:
	rm -f *.o bin/reverse_polish.out bin/counter.out

.PHONY: all clean

