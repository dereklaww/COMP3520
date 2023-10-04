/* SID: 510226244 */ 
/* COMP3520: Assignment 1 */ 

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "shop.h"

void * barber_routine(void *);
void * customer_routine(void *);
void * assistant_routine(void *);

typedef struct barber_argv {
    int barber_min;
    int barber_max;
    int barber_id;
} barber_argv;

int main(int argc, char **argv);

