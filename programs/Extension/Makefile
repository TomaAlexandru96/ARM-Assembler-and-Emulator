test:
	make build;

build:
	gcc -I "/usr/include/SDL2" -Wall -pedantic -Werror -g -D_POSIX_C_SOURCE199309L -std=c99 -c main.c;
	gcc main.o -Wall -pedantic -Werror -g -D_POSIX_C_SOURCE199309L -std=c99 -lSDL2 -o main;

clean:
	rm -f main.o;
	rm -f main;
