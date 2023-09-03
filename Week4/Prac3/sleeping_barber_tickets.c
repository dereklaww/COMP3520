#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int no_of_customers;
int seat_capacity;
int no_waiting = 0; //initialized to seat capacity
int ticket_index = 0;
int calling_index = 0;
int barber_chair = 1;
int completed_haircuts = 0;
int customer_signaled = 0;
int work_done = 0;
int barber_working = 0;
int barber_busy = 0;
int no_leaving = 0;
int shop_closed = 0;

void * barber_routine(void *);
void * customer_routine(void *);
void * assistant_routine(void *);

struct barber_struct {
    int barber_min;
    int barber_max;
};

//declare global mutex and condition variables
// mutex - to lock critical section of code
// within critical section - wait  for signal to continue running

//declare global mutex and condition variables
pthread_mutex_t access_waitingroom, access_barberroom, access_barber;

pthread_cond_t customer_arrived, barber_wakeup, barber_ready_cond, customer_ready, barber_done, customer_signalcond, shop_closed_cond;
pthread_cond_t *waiting_tickets;


int main(int argc, char ** argv)
{
    pthread_t *threads; //system thread id

    int *t_ids; //user-defined thread id
    int barber_pace_min, barber_pace_max, customer_rate_min, customer_rate_max;

    int rc, k;

    struct barber_struct barber_args;

    // ask for seeating capacity.
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

    //Initialize mutex and condition vairable objects

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

    rc = pthread_cond_init(&barber_wakeup, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (customer) is %d\n", rc);
        exit(-1);
    }


    rc = pthread_cond_init(&barber_ready_cond, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (customer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&customer_signalcond, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (customer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&customer_ready, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (customer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&barber_done, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (customer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&shop_closed_cond, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (customer) is %d\n", rc);
        exit(-1);
    }

    waiting_tickets = malloc(seat_capacity * sizeof(pthread_cond_t));

    for (int i = 0; i < seat_capacity; i++) {
        rc = pthread_cond_init(&waiting_tickets[i], NULL);

        if (rc) {
            printf("ERROR; return code from pthread_cond_init() (tickets) is %d\n",
            rc);
            exit(-1);
        }
    }

    threads = malloc((no_of_customers+2) * sizeof(pthread_t)); //total is No_Of_Consuers + 2 to include barber and assistant

    if (threads == NULL){
        fprintf(stderr, "threads out of memory\n");
        exit(1);
    }

    t_ids = malloc((no_of_customers+2) * sizeof(int)); //total is No_Of_Consuers+ 1 to include barber and assistant

    if(t_ids == NULL){
        fprintf(stderr, "t out of memory\n");
        exit(1);
    }

    //create the barber thread.
    rc = pthread_create(&threads[0], NULL, barber_routine, (void *)
    &barber_args); //barber routine takes barber_pace as the arg

    if (rc) {
        printf("ERROR; return code from pthread_create() (barber) is %d\n",
        rc);
        exit(-1);
    }

    //assistant thread
    rc = pthread_create(&threads[1], NULL, assistant_routine, NULL); 

    if (rc) {
        printf("ERROR; return code from pthread_create() (assistant) is %d\n",
        rc);
        exit(-1);
    }

    //create consumers according to the arrival rate
    srand(time(0));
    for (k = 2; k<no_of_customers + 2; k++)
    {
        sleep(((int)rand() % (customer_rate_max - customer_rate_min + 1))+ customer_rate_min); //sleep a few second before creating a thread
        t_ids[k] = k;
        rc = pthread_create(&threads[k], NULL, customer_routine, (void*)&t_ids[k]); //customer routine takes thread id as the arg

        if (rc) {
            printf("ERROR; return code from pthread_create() (customer) is %d\n", rc);
            exit(-1);
        }
    }

    //join customer threads.
    for (k = 2; k<no_of_customers + 2; k++)
    {
        pthread_join(threads[k], NULL);
    }

    //terminate the barber thread using pthread_cancel().
//pthread_cancel(threads[0]);

    //deallocate allocated memory
    free(threads);
    free(t_ids);

    //destroy mutex and condition variable objects
    pthread_mutex_destroy(&access_waitingroom);
    pthread_mutex_destroy(&access_barberroom);
    pthread_mutex_destroy(&access_barber);
    pthread_cond_destroy(&customer_arrived);
    pthread_cond_destroy(&customer_signalcond);
    pthread_cond_destroy(&barber_wakeup);
    pthread_cond_destroy(&barber_ready_cond);
    pthread_cond_destroy(&customer_ready);
    pthread_cond_destroy(&barber_done);
    pthread_exit(EXIT_SUCCESS);

}

void *assistant_routine(void *arg) {

    while (1) {

        pthread_mutex_lock(&access_waitingroom);

        while ((no_waiting == 0) && (calling_index < no_of_customers - 1)) {
            printf("Assistant: I'm waiting for customers.\n");
            pthread_cond_wait(&customer_arrived, &access_waitingroom);
        }
        pthread_mutex_unlock(&access_waitingroom);
        

        pthread_mutex_lock(&access_barberroom);
        while (barber_busy) {
            if ((calling_index < no_of_customers - 1)){
                printf("Assistant: I'm waiting for barber to become available.\n");
            }
            pthread_cond_wait(&barber_ready_cond, &access_barberroom);
        }

        if (!barber_busy && (calling_index == no_of_customers)) {
            printf("Assistant: Hi Barber, we've finished the work for the day.\n");
            shop_closed = 1;
            pthread_exit(EXIT_SUCCESS);
        }


        customer_signaled = 1;
        pthread_cond_signal(&customer_signalcond);
        pthread_mutex_unlock(&access_barberroom);
        pthread_mutex_lock(&access_waitingroom);

        printf("Assistant: Call one customer with a ticket numbered %d\n", calling_index + 1);
        no_waiting--;
        pthread_cond_signal(&waiting_tickets[calling_index % seat_capacity]);
        pthread_mutex_unlock(&access_waitingroom);
    
    }
    pthread_exit(EXIT_SUCCESS);
}

void *barber_routine(void *arg) {

    struct barber_struct *barber = arg;
    int barber_min = barber->barber_min;
    int barber_max = barber->barber_max;
    
    while (1) {

        srand(time(0));
        int barber_pace = ((int)rand() % (barber_max - barber_min + 1)) + barber_min;

        
        // access barber
        pthread_mutex_lock(&access_barberroom);
        barber_busy = 0;
        pthread_cond_signal(&barber_ready_cond);
        pthread_mutex_unlock(&access_barberroom);

    
        if (calling_index == no_of_customers){
            while (!shop_closed);
            printf("Barber: Thank Assistant and see you tomorrow!\n");
            pthread_exit(EXIT_SUCCESS);
        }

        pthread_mutex_lock(&access_barberroom);
        while (!customer_signaled) {
            printf("Barber: I'm now ready to accept a new customer. \n");
            pthread_cond_wait(&customer_signalcond, &access_barberroom);
        }
        barber_busy = 1;
        pthread_mutex_unlock(&access_barberroom);
       
        pthread_mutex_lock(&access_barber);
        barber_working = 1;

        while (barber_chair) {
            printf("Barber: wait for the customer to sit on the barber chair. \n");
            pthread_cond_wait(&customer_ready, &access_barber);
        }

        printf("Barber: Hello customer %d!\n", calling_index + 1);
        sleep(barber_pace);
        printf("Barber: finished cutting. Good bye, customer %d! \n", calling_index + 1);
        calling_index++;
        barber_working = 0;
        pthread_cond_signal(&barber_done);
        barber_chair = 1;
        pthread_mutex_unlock(&access_barber);
    }
}

void *customer_routine(void * arg){

    int *customer_arg;
    customer_arg = (int *) arg;

    printf("Customer [%d]: I have arrived at the barber shop\n", (*customer_arg) - 1);

    // access waiting room


    int ticket_number = ticket_index % seat_capacity;

    pthread_mutex_lock(&access_waitingroom);
    if (no_waiting == seat_capacity) {
        printf("Customer [%d]: oh no! all seats have been taken and I'll leave now!\n", (*customer_arg) - 1);
        no_leaving++;
        pthread_mutex_unlock(&access_waitingroom);
        pthread_exit(EXIT_SUCCESS);
        
    } else {
        printf("Customer [%d]: I'm lucky to get a free seat and a ticket numbered %d\n", (*customer_arg) - 1, ticket_number + 1);
        no_waiting++;
        ticket_index++;
        pthread_cond_signal(&customer_arrived);

        while (!customer_signaled){
            printf("Customer [%d]: wait to be called\n", (*customer_arg) - 1);
            pthread_cond_wait(&waiting_tickets[ticket_number], &access_waitingroom);
        }
        customer_signaled = 0;
        printf("Customer [%d]: My ticket numbered %d has been called. Hello, Barber\n", (*customer_arg) - 1, ticket_number + 1);
    }
    pthread_mutex_unlock(&access_waitingroom);


    //access barber

    pthread_mutex_lock(&access_barber);
    printf("Customer [%d]: sit on the barber chair\n", (*customer_arg) - 1);
    barber_chair = 0;
    pthread_cond_signal(&customer_ready);

    while (barber_working) {
        printf("Customer [%d]: I am being served\n", (*customer_arg) - 1);
        pthread_cond_wait(&barber_done, &access_barber);
    }

    no_leaving++;
    printf("Customer [%d]: Well done. Thank barber, bye!\n", (*customer_arg) - 1);

    if (no_leaving == no_of_customers) {
        no_leaving = -1;
    }
    pthread_mutex_unlock(&access_barber);

    pthread_exit(EXIT_SUCCESS);

}

//gcc -Wall -pedantic -std=gnu99 sleeping_barber_tickets.c -o sleeping_barber_tickets -lpthread          
