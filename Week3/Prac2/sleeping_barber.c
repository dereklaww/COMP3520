#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int no_of_customers;
int seat_capacity;
int no_of_seats; //initialized to seat capacity
int barber_chair = 1; //initially available

void * barber_routine(void *);
void * customer_routine(void *);

//declare global mutex and condition variables
// mutex - to lock critical section of code
// within critical section - wait  for signal to continue running

//declare global mutex and condition variables
pthread_mutex_t access_chair, access_barber;
pthread_cond_t customer_arrived, barber_ready, customer_ready, barber_done;

int main(int argc, char ** argv)
{

    pthread_t *threads; //system thread id
    int *t_ids; //user-defined thread id
    int barber_pace, customer_rate;

    int rc, k;

    // ask for seeating capacity.
    printf("Enter total number of seats (int): ");
    scanf("%d", &seat_capacity);
    no_of_seats = seat_capacity;

    // ask for the total number of customers.
    printf("Enter the total number of customers (int): ");
    scanf("%d", &no_of_customers);

    //ask for barber's working pace
    printf("Enter barber's working pace (int): ");
    scanf("%d", &barber_pace);

    //ask for consumers' arrival rate
    printf("Enter customer's arriving pace (int): ");
    scanf("%d", &customer_rate);

    //Initialize mutex and condition vairable objects

    rc = pthread_mutex_init(&access_chair, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_mutex_init() (farmer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_mutex_init(&access_barber, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_mutex_init() (farmer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&customer_arrived, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (consumer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&barber_ready, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (consumer) is %d\n", rc);
        exit(-1);
    }


    rc = pthread_cond_init(&customer_ready, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (consumer) is %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_init(&barber_done, NULL);

    if (rc) {
        printf("ERROR; return code from pthread_cond_init() (consumer) is %d\n", rc);
        exit(-1);
    }

    threads = malloc((no_of_customers+1) * sizeof(pthread_t)); //total is No_Of_Consuers + 1 to include farmer

    if (threads == NULL){
        fprintf(stderr, "threads out of memory\n");
        exit(1);
    }

    t_ids = malloc((no_of_customers+1) * sizeof(int)); //total is No_Of_Consuers+ 1 to include farmer

    if(t_ids == NULL){
        fprintf(stderr, "t out of memory\n");
        exit(1);
    }

    //create the barber thread.
    rc = pthread_create(&threads[0], NULL, barber_routine, (void *)
    &barber_pace); //barber routine takes barber_pace as the arg

    if (rc) {
        printf("ERROR; return code from pthread_create() (barber) is %d\n",
        rc);
        exit(-1);
    }

    //create consumers according to the arrival rate
    srand(time(0));
    for (k = 1; k<no_of_customers + 1; k++)
    {
        sleep((int)rand() % customer_rate); //sleep a few second before creating a thread
        t_ids[k] = k;
        rc = pthread_create(&threads[k], NULL, customer_routine, (void*)&t_ids[k]); //customer routine takes thread id as the arg

        if (rc) {
            printf("ERROR; return code from pthread_create() (customer) is %d\n", rc);
            exit(-1);
        }
    }

    //join customer threads.
    for (k = 1; k<no_of_customers + 1; k++)
    {
        pthread_join(threads[k], NULL);
    }

    //terminate the farmer thread using pthread_cancel().
    pthread_cancel(threads[0]);

    //deallocate allocated memory
    free(threads);
    free(t_ids);

    //destroy mutex and condition variable objects
    pthread_mutex_destroy(&access_chair);
    pthread_mutex_destroy(&access_barber);
    pthread_cond_destroy(&customer_arrived);
    pthread_cond_destroy(&barber_ready);
    pthread_cond_destroy(&customer_ready);
    pthread_cond_destroy(&barber_done);
    pthread_exit(EXIT_SUCCESS);

}

void *barber_routine(void *arg) {

    int *barber_pace;
    barber_pace = (int *) arg;

    while (1) {
    // access chair
    pthread_mutex_lock(&access_chair);

    while (no_of_seats == seat_capacity) {
        printf("Barber: The number of free seats is %d. No customers waiting and I'll go to sleep.\n", no_of_seats);
        pthread_cond_wait(&customer_arrived, &access_chair);
    }

    printf("Barber: The number of free seats now is %d, Call next customer. \n", no_of_seats);
    pthread_cond_signal(&barber_ready);
    no_of_seats++;

    pthread_mutex_unlock(&access_chair);

    // access barber
    pthread_mutex_lock(&access_barber);

    printf("Barber: wait for the customer to sit on the barber chair. \n");

    while (barber_chair) {
        pthread_cond_wait(&customer_ready, &access_barber);
    }

    printf("Barber: Start serving the customer.. \n");
    sleep(*barber_pace);
    printf("Barber: finished cutting. Bye! \n");
    pthread_cond_signal(&barber_done);
    barber_chair++;
    pthread_mutex_unlock(&access_barber);
    }

}

void *customer_routine(void * arg){

    int *customer_id;
    customer_id = (int *) arg;

    printf("Customer [%d]: I have arrived at the barber shop\n", *customer_id);

    // access waiting room

    pthread_mutex_lock(&access_chair);

    if (no_of_seats == 0) {
        printf("Customer %d: oh no! all seats have been taken and I'll leave now!\n", *customer_id);
        pthread_mutex_unlock(&access_chair);
        pthread_exit(EXIT_SUCCESS);
    } else {
        printf("Customer %d: I'm lucky to get a free seat from %d\n", *customer_id, no_of_seats);
        no_of_seats--;
        pthread_cond_signal(&customer_arrived);
        printf("Customer %d: wait to be called\n", *customer_id);
        pthread_cond_wait(&barber_ready, &access_chair);
        printf("Customer %d: I'm to be served.\n", *customer_id);
    }

    pthread_mutex_unlock(&access_chair);

    //access barber

    pthread_mutex_lock(&access_barber);
    printf("Customer %d: sit on the barber chair\n", *customer_id);
    barber_chair--;
    pthread_cond_signal(&customer_ready);
    

    printf("Customer %d: I am being served\n", *customer_id);

    pthread_cond_wait(&barber_done, &access_barber);

    printf("Customer %d: Well done. Thank barber, bye!\n", *customer_id);

    pthread_mutex_unlock(&access_barber);

    pthread_exit(EXIT_SUCCESS);

}

