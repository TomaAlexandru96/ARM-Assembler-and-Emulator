test:
	make build;
	./demo;

build:
	gcc -I "/usr/include/SDL" -Wall -pedantic -Werror -g -D_POSIX_C_SOURCE199309L -std=c99 -c demo.c;
	gcc demo.o -Wall -pedantic -Werror -g -D_POSIX_C_SOURCE199309L -std=c99 -lSDL -o demo;

clean:
	rm -f demo.o;
	rm -f demo;
