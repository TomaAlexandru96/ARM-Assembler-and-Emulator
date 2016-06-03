CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -pedantic -Werror

.SUFFIXES: .c .o

.PHONY: all clean

all: main

main: main.o

clean:
	rm -f $(wildcard *.o)
	rm -f main
