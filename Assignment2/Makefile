
CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 --pedantic -std=gnu99 -g -fsanitize=address
EXC=pcb.c mlq.c

# compiles the program
all:
	$(CC) $(CFLAGS) $(EXC) -o mlq
	./mlq random.txt
