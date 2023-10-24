
CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 --pedantic -std=gnu99 -g -fsanitize=address -lm
EXC=mab.c 
#pcb.c mlq.c

# compiles the program
all:
	$(CC) $(CFLAGS) $(EXC) -o mab
	./mab	
#./mlq random_mid.txt