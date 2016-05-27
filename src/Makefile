CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: assemble.o

emulate: instructionManipulation.o emulate.o 
	$(CC) instructionManipulation.o emulate.o -o emulate

emulate.o: emulate.h emulate.c
	$(CC) $(CFLAGS) emulate.c -c -o emulate.o

instructionManipulation.o: instructionManipulation.h instructionManipulation.c
	$(CC) $(CFLAGS) instructionManipulation.c -c -o instructionManipulation.o

clean:
	rm -f $(wildcard *.o)
	rm -f assemble
	rm -f emulate
