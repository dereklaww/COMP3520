
CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 --pedantic -std=gnu99 -g
EXC=mab.c pcb.c mlq.c

# compiles the program
test_mlq:
	$(CC) $(CFLAGS) -o mlq $(EXC) -lm
	./mlq random_mid.txt

test_mab:
	$(CC) $(CFLAGS) -o mab mab.c -lm
	./mab