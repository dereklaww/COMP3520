#  SID: 510226244
#  COMP3520 Assignment 1 


CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 --pedantic -std=gnu99 -g -fsanitize=address
EXC=main.c queue.c shop.c

# compiles the program
all:
	$(CC) $(CFLAGS) $(EXC) -o main -lpthread

# for debugging
test:
	gdb ./main

# compiles and run the program
run:
	$(CC) $(CFLAGS) $(EXC) -o main -lpthread
	./main