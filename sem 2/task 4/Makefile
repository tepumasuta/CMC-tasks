AS=nasm
ASFLAGS=-f elf32 -g
CC=gcc
CFLAGS=-Wall -Wextra -fanalyzer -std=c11 -O0 -m32

all: bin/main

patch.o: patch.s patch.h
	$(AS) $(ASFLAGS) -o $@ $<

main.o: main.c patch.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/main: main.o patch.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o bin/main

.PHONY: all clean
