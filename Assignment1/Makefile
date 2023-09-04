
CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 --pedantic -std=gnu99 -g -fsanitize=address


all:
	$(CC) $(CFLAGS) ass1.c -o ass1 -lpthread

test:
	gdb ./ass1

run:
	./ass1