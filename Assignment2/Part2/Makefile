
CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 --pedantic -std=gnu99 -g -lm
EXC=mab.c pcb.c mlq.c

# compiles the program
test_mlq:
	$(CC) $(CFLAGS) $(EXC) -o mlq
	./mlq random_mid.txt

test_mab:
	$(CC) $(CFLAGS) mab.c -o mab
	./mab