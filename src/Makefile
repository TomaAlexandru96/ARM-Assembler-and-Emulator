CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_BSD_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean

all: assemble emulate

assemble: adts.o mappings.o assemble.o
	$(CC) adts.o mappings.o assemble.o -o assemble

emulate: instructionManipulation.o emulate.o 
	$(CC) instructionManipulation.o emulate.o -o emulate

emulate.o: emulate.h emulate.c
	$(CC) $(CFLAGS) emulate.c -c -o emulate.o

instructionManipulation.o: instructionManipulation.h instructionManipulation.c
	$(CC) $(CFLAGS) instructionManipulation.c -c -o instructionManipulation.o

assemble.o: assemble.h assemble.c
	$(CC) $(CFLAGS) assemble.c -c -o assemble.o

mappings.o: mappings.h mappings.c
	$(CC) $(CFLAGS) mappings.c -c -o mappings.o

adts.o: adts.h adts.c
	$(CC) $(CFLAGS) adts.c -c -o adts.o

clean:
	rm -f $(wildcard *.o)
	rm -f assemble
	rm -f emulate
