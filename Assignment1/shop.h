#ifndef SHOP
#define SHOP

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

typedef struct Barber {
    int id;
    pthread_cond_t barber_cond;
    int customer_id;
} Barber;

typedef struct Customer {
    int id;
    pthread_cond_t customer_cond;
    int current_state;
    int barber_id;
} Customer;

typedef struct Shop {
    int no_barbers;
    int seat_capacity;
    int no_customers;
    int customer_min;
    int customer_max;
    int barber_min;
    int barber_max;
} Shop;

typedef struct Assistant {
    int ticket_id;
    pthread_cond_t customer_cond;
    pthread_cond_t barber_cond;

} Assistant;

enum CustomerState {
    WAITING,
    CUTTING,
    LEAVING,
};

Shop *init_shop(int no_barbers, int no_customers, int seating_capacity, 
    int customer_min, int customer_max, int barber_min, int barber_max);   
int arrive_shop(int customer_id);
void leave_shop(int customer_id, int barber_id);
void barber_service(int barber_id);
void barber_done(int barber_id);
void assistant_waiting_customer(void);
void assistant_waiting_barber(void);
void assistant_assign_customer_barber(void);
Barber *get_barber(int barber_id);

#endif
