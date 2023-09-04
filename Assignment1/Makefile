
CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 --pedantic -std=gnu99 -g -fsanitize=address
EXC=main.c queue.c shop.c


all:
	$(CC) $(CFLAGS) $(EXC) -o main -lpthread

test:
	gdb ./main

run:
	./main