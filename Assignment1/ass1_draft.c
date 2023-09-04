#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int no_of_customers;
int seat_capacity;
int no_of_barbers;
int no_waiting = 0; //initialized to seat capacity
int ticket_index = 0;
int calling_index = 0;

int no_leaving = 0;
int shop_closed = 0;
int *customer_signal_arr;
int *barber_signal_arr;
int *next_customer_id;
int *barber_ready_queue;
int *barber_busy;
int *barber_chair;
int *barber_working;
int barber_ready_index = 0;
int barber_calling_index = 0;

void * barber_routine(void *);
void * customer_routine(void *);
void * assistant_routine(void *);

struct barber_arg_struct {
    int barber_min;
    int barber_max;
    int barber_id;
};

struct barber_struct {
    int barber_id;
    int barber_queue_index;
    int customer_ticket_number;
};

struct customer_struct {
    int customer_id;
    int customer_ticket_number;
    int barber_queue_index;
};

//declare global mutex and condition variables
// mutex - to lock critical section of code
// within critical section - wait  for signal to continue running

//declare global mutex and condition variables
pthread_mutex_t access_waitingroom, access_barberroom, access_barber;

pthread_cond_t customer_arrived, shop_closed_cond;
pthread_cond_t *waiting_tickets, *barber_ready_cond, *barber_done, *customer_signalcond, *customer_ready;

struct barber_struct *barber_struct_arr;
struct customer_struct *customer_struct_arr;

int main(int argc, char ** argv)
{
    pthread_t *threads; //system thread id

    int *t_ids; //user-defined thread id
    int barber_pace_min, barber_pace_max, customer_rate_min, customer_rate_max;

    int rc, k;

    struct barber_arg_struct barber_args;

    // ask for number of barbers.
    printf("Enter total number of barbers (int): ");
    scanf("%d", &no_of_barbers);

    // ask for seating capacity.
    printf("Enter total number of seats (int): ");
    scanf("%d", &seat_capacity);

    // ask for the total number of customers.
    printf("Enter the total number of customers (int): ");
    scanf("%d", &no_of_customers);

    //ask for barber's min working pace
    printf("Enter barber's min working pace (int): ");
    scanf("%d", &barber_pace_min);

    //ask for barber's max working pace
    printf("Enter barber's max working pace (int): ");
    scanf("%d", &barber_pace_max);

    //ask for customer's min arrival rate
    printf("Enter customer's min arrival rate (int): ");
    scanf("%d", &customer_rate_min);

    //ask for customer's max arrival rate
    printf("Enter customer's max arrival rate (int): ");
    scanf("%d", &customer_rate_max);

    if ((barber_pace_min > barber_pace_max) 
        || (customer_rate_min > customer_rate_max)) {
            printf("Invalid inputs: min values > max values.\n");
    }

    barber_args.barber_min = barber_pace_min;
    barber_args.barber_max = barber_pace_max;

    //Initialize mutex and condition variable objects

    rc = pthread_mutex_init(&access_waitingroom, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_mutex_init() (barber) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_mutex_init(&access_barber, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_mutex_init() (barber) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_mutex_init(&access_barberroom, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_mutex_init() (barber) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&customer_arrived, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (customer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&shop_closed_cond, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (customer) is %d\n", rc);
        exit(-1);
    }

    customer_signal_arr = calloc(seat_capacity, sizeof(int));
    next_customer_id = calloc(seat_capacity, sizeof(int));
    barber_ready_queue = calloc(no_of_barbers, sizeof(int));
    barber_busy = calloc(no_of_barbers, sizeof(int));
    barber_signal_arr = calloc(no_of_barbers, sizeof(int));
    barber_working = calloc(no_of_barbers, sizeof(int));
    barber_chair = calloc(no_of_barbers, sizeof(int));

    for (int i = 0; i < no_of_barbers; i++) {
        barber_chair[i] = 1;
    }

    waiting_tickets = malloc(seat_capacity * sizeof(pthread_cond_t));

    barber_struct_arr = malloc(no_of_barbers * sizeof(struct barber_struct));
    customer_struct_arr = malloc(no_of_customers * sizeof(struct customer_struct));

    for (int i = 0; i < seat_capacity; i++) {
        rc = pthread_cond_init(&waiting_tickets[i], NULL);

        if (rc) {
            printf("ERROR; return code from pthread_cond_init() (tickets) is %d\n",
            rc);
            exit(-1);
        }
    }

    barber_ready_cond = malloc(no_of_barbers * sizeof(pthread_cond_t));

    for (int i = 0; i < no_of_barbers; i++) {
        rc = pthread_cond_init(&barber_ready_cond[i], NULL);

        if (rc) {
            printf("ERROR; return code from pthread_cond_init() (tickets) is %d\n",
            rc);
            exit(-1);
        }
    }

    barber_done = malloc(no_of_barbers * sizeof(pthread_cond_t));

    for (int i = 0; i < no_of_barbers; i++) {
        rc = pthread_cond_init(&barber_done[i], NULL);

        if (rc) {
            printf("ERROR; return code from pthread_cond_init() (tickets) is %d\n",
            rc);
            exit(-1);
        }
    }

    customer_signalcond = malloc(no_of_barbers * sizeof(pthread_cond_t));

    for (int i = 0; i < no_of_barbers; i++) {
        rc = pthread_cond_init(&customer_signalcond[i], NULL);

        if (rc) {
            printf("ERROR; return code from pthread_cond_init() (tickets) is %d\n",
            rc);
            exit(-1);
        }
    }

    customer_ready = malloc(no_of_customers * sizeof(pthread_cond_t));

    for (int i = 0; i < no_of_customers; i++) {
        rc = pthread_cond_init(&customer_ready[i], NULL);

        if (rc) {
            printf("ERROR; return code from pthread_cond_init() (tickets) is %d\n",
            rc);
            exit(-1);
        }
    }

    threads = malloc((no_of_customers + no_of_barbers + 1) * sizeof(pthread_t)); 

    if (threads == NULL){
        fprintf(stderr, "threads out of memory\n");
        exit(1);
    }

    t_ids = malloc((no_of_customers) * sizeof(int)); 

    if(t_ids == NULL){
        fprintf(stderr, "t out of memory\n");
        exit(1);
    }

    //create barbers
    int barber_id = 0;
    for (k = 0; k<no_of_barbers; k++)
    {
        barber_id = k;
        barber_args.barber_id = barber_id;
        rc = pthread_create(&threads[k], NULL, barber_routine, (void*)&barber_args); // barber routine takes barber_pace as the arg

        if (rc) {
            printf("ERROR; return code from pthread_create() (barbers) is %d\n", rc);
            exit(-1);
        }
    }

    rc = pthread_create(&threads[no_of_barbers], NULL, assistant_routine, NULL); 

    if (rc) {
        printf("ERROR; return code from pthread_create() (assistant) is %d\n",
        rc);
        exit(-1);
    }

    //create consumers according to the arrival rate
    srand(time(0));
    for (k = 0; k<no_of_customers; k++)
    {
        sleep(((int)rand() % (customer_rate_max - customer_rate_min + 1))+ customer_rate_min); //sleep a few second before creating a thread
        t_ids[k] = k;
        rc = pthread_create(&threads[k + no_of_barbers + 1], NULL, customer_routine, (void*)&t_ids[k]); //customer routine takes thread id as the arg

        if (rc) {
            printf("ERROR; return code from pthread_create() (customer) is %d\n", rc);
            exit(-1);
        }
    }

    //join customer threads.
    for (k = 0; k<no_of_barbers; k++)
    {
        pthread_join(threads[k], NULL);
    }

        //join customer threads.
    for (k = no_of_barbers + 1; k<no_of_customers; k++)
    {
        pthread_join(threads[k], NULL);
    }

    //terminate the barber thread using pthread_cancel().
//pthread_cancel(threads[0]);

    printf("Main thread: All customers have now been served. Salon is closed now.\n");

    //deallocate allocated memory
    free(threads);
    free(t_ids);
    free(waiting_tickets);
    free(customer_signal_arr);
    free(next_customer_id);

    free(barber_signal_arr);
    free(barber_ready_queue);
    free(barber_busy);
    free(barber_chair);
    free(barber_working);
    free(barber_struct_arr);
    free(customer_struct_arr);

    //destroy mutex and condition variable objects
    pthread_mutex_destroy(&access_waitingroom);
    pthread_mutex_destroy(&access_barberroom);
    pthread_mutex_destroy(&access_barber);
    pthread_cond_destroy(&customer_arrived);
    pthread_cond_destroy(&shop_closed_cond);

    for (int i = 0; i < seat_capacity; i++) {
        pthread_cond_destroy(&waiting_tickets[i]);
    }

    for (int i = 0; i < no_of_barbers; i++) {
        pthread_cond_destroy(&barber_ready_cond[i]);
        pthread_cond_destroy(&barber_done[i]);
        pthread_cond_destroy(&customer_signalcond[i]);
    }

    for (int i = 0; i < no_of_customers; i++) {
        pthread_cond_destroy(&customer_ready[i]);
    }

    free(waiting_tickets);
    free(barber_ready_cond);
    free(barber_done);
    free(customer_signalcond);
    free(customer_ready);

    pthread_exit(EXIT_SUCCESS);

}

void *assistant_routine(void *arg) {

    while (1) {

        pthread_mutex_lock(&access_waitingroom);

        while ((no_waiting == 0) && (no_leaving < no_of_customers - 1)) {
            printf("Assistant: I'm waiting for customers.\n");
            pthread_cond_wait(&customer_arrived, &access_waitingroom);
        }
        pthread_mutex_unlock(&access_waitingroom);
        

        pthread_mutex_lock(&access_barberroom);
        while (barber_busy[barber_calling_index % no_of_barbers]) {
            if ((calling_index < no_of_customers - 1)){
                printf("Assistant: I'm waiting for barber to become available.\n");
            }
            pthread_cond_wait(&barber_ready_cond[barber_calling_index % no_of_barbers], &access_barberroom);
        }

        if (!(barber_busy[barber_calling_index % no_of_barbers]) && (no_leaving == no_of_customers)) {
            printf("Assistant: Hi Barber, we've finished the work for the day.\n");
            shop_closed = 1;
            pthread_exit(EXIT_SUCCESS);
        }

        printf("calling index: %d\n", calling_index); 
        struct barber_struct *current_barber_struct = &barber_struct_arr[barber_calling_index % no_of_barbers];
        struct customer_struct *current_customer_struct = &customer_struct_arr[calling_index % seat_capacity];

        current_barber_struct->customer_ticket_number = current_customer_struct->customer_ticket_number;
        current_customer_struct->barber_queue_index = current_barber_struct->barber_queue_index;

        barber_signal_arr[barber_calling_index % no_of_barbers] = 1;
        pthread_cond_signal(&customer_signalcond[barber_calling_index % no_of_barbers]);
        barber_calling_index++;
        pthread_mutex_unlock(&access_barberroom);
        pthread_mutex_lock(&access_waitingroom);

        customer_signal_arr[calling_index % seat_capacity] = 1;
        printf("Assistant: Call one customer with a ticket numbered %d\n", calling_index + 1);
        no_waiting--;
        pthread_cond_signal(&waiting_tickets[calling_index % seat_capacity]);
        pthread_mutex_unlock(&access_waitingroom);
    
    }
    pthread_exit(EXIT_SUCCESS);
}

void *barber_routine(void *arg) {

    struct barber_arg_struct *barber = arg;
    int barber_min = barber->barber_min;
    int barber_max = barber->barber_max;
    int barber_id = barber->barber_id;

    struct barber_struct *barber_struct;
    
    while (1) {

        srand(time(0));
        int barber_pace = ((int)rand() % (barber_max - barber_min + 1)) + barber_min;
        int barber_queue_id = 0;
        // access barber
        pthread_mutex_lock(&access_barberroom);

        barber_queue_id = barber_ready_index % no_of_barbers;
        barber_ready_index++;

        barber_struct = &barber_struct_arr[barber_queue_id];
        barber_struct->barber_id = barber->barber_id;
        barber_struct->barber_queue_index = barber_queue_id;

        barber_busy[barber_queue_id] = 0;
        pthread_cond_signal(&barber_ready_cond[barber_queue_id]);
        pthread_mutex_unlock(&access_barberroom);
    
        if (no_leaving == no_of_customers){
            while (!shop_closed);
            printf("Barber: Thank Assistant and see you tomorrow!\n");
            pthread_exit(EXIT_SUCCESS);
        }

        pthread_mutex_lock(&access_barberroom);
        while (!barber_signal_arr[barber_queue_id]) {
            printf("Barber[%d]: I'm now ready to accept a new customer. \n", barber_id);
            pthread_cond_wait(&customer_signalcond[barber_queue_id], &access_barberroom);
        }
        
        barber_signal_arr[barber_queue_id] = 0;
        barber_busy[barber_queue_id] = 1;
        pthread_mutex_unlock(&access_barberroom);
       
        pthread_mutex_lock(&access_barber);
        printf("barber queue id: %d\n", barber_queue_id);
        barber_working[barber_queue_id] = 1;

        while (barber_chair[barber_queue_id]) {
            printf("Barber: wait for the customer to sit on the barber chair. \n");
            pthread_cond_wait(&customer_ready[barber_struct->customer_ticket_number], &access_barber);
        }

        printf("Barber: Hello Customer %d!\n", next_customer_id[calling_index % seat_capacity]);
        sleep(barber_pace);
        printf("Barber: Finished cutting. Good bye, Customer %d! \n", next_customer_id[calling_index % seat_capacity]);
        calling_index++;
        no_leaving++;
        barber_working = 0;
        pthread_cond_signal(&barber_done[barber_queue_id]);
        barber_chair[barber_queue_id] = 1;
        pthread_mutex_unlock(&access_barber);
    }
}

void *customer_routine(void * arg){

    int *customer_arg;
    customer_arg = (int *) arg;

    pthread_mutex_lock(&access_waitingroom);
    printf("Customer [%d]: I have arrived at the barber shop\n", *customer_arg + 1);

    // access waiting room
    int ticket_number = ticket_index % seat_capacity;

    struct customer_struct *customer_struct = &customer_struct_arr[ticket_index];
    customer_struct->customer_id = *customer_arg + 1;
    customer_struct->customer_ticket_number = ticket_number;

    if (no_waiting == seat_capacity) {
        printf("Customer [%d]: Oh no! All seats have been taken and I'll leave now!\n", *customer_arg + 1);
        no_leaving++;
        pthread_mutex_unlock(&access_waitingroom);
        pthread_exit(EXIT_SUCCESS);
        
    } else {
        printf("Customer [%d]: I'm lucky to get a free seat and a ticket numbered %d\n", *customer_arg + 1, ticket_number + 1);
        next_customer_id[ticket_number] = *customer_arg + 1;
        no_waiting++;
        ticket_index++;
        pthread_cond_signal(&customer_arrived);

        while (!(customer_signal_arr[ticket_number])){
            printf("Customer [%d]: wait to be called\n", *customer_arg + 1);
            pthread_cond_wait(&waiting_tickets[ticket_number], &access_waitingroom);
        }
        customer_signal_arr[ticket_number] = 0;
        printf("Customer [%d]: My ticket numbered %d has been called. Hello, Barber n\n", *customer_arg + 1, ticket_number + 1);
    }
    pthread_mutex_unlock(&access_waitingroom);


    //access barber

    pthread_mutex_lock(&access_barber);
    printf("Customer [%d]: sit on the barber chair \n", *customer_arg + 1);
    barber_chair[customer_struct->barber_queue_index] = 0;
    pthread_cond_signal(&customer_ready[customer_struct->barber_queue_index]);
    printf("barber_working[0] = %d, barber_index = %d\n", barber_working[0], customer_struct->barber_queue_index);
    while (barber_working[customer_struct->barber_queue_index]) {
        printf("Customer [%d]: I am being served\n", *customer_arg + 1);
        pthread_cond_wait(&barber_done[customer_struct->barber_queue_index], &access_barber);
    }
    printf("Customer [%d]: Well done. Thank barber n, bye!\n", *customer_arg + 1);

    pthread_mutex_unlock(&access_barber);

    pthread_exit(EXIT_SUCCESS);

}

//gcc -Wall -pedantic -std=gnu99 sleeping_barber_tickets.c -o sleeping_barber_tickets -lpthread          
