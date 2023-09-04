#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Barber {
    int id;
    pthread_cond_t thread;
    int customer_id;
} Barber;

typedef struct Customer {
    int id;
    pthread_cond_t thread;
    int current_state;
    int barber_id;
} Customer;

enum CustomerState {
    WAITING,
    CUTTING,
    LEAVING,
};

void *init_shop(int no_barbers, int seating_capacity);   
int arrive_shop(int customer_id);
void leave_shop(int customer_id, int barber_id);
void *barber_service(void *barber_id);
void *barber_done(void *barber_id);