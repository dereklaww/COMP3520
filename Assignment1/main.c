#include "main.h"

int no_barbers;
int seat_capacity;
int no_customers;
int customer_min;
int customer_max;
int barber_min;
int barber_max;

Shop *shop;
barber_argv *barber_argv_arr;

pthread_t *barber_threads;
pthread_t *customer_threads;
pthread_t assistant_thread;
int *t_ids;


void *assistant_routine(void *arg) {
    
    while (1) {
        assistant_waiting_customer();
        assistant_waiting_barber();
        assistant_assign_customer_barber();
    }
}

// the barber thread function
void *barber_routine( void *arg ) {

    barber_argv *barber = arg;
    int barber_min = barber->barber_min;
    int barber_max = barber->barber_max;
    int barber_id = barber->barber_id;

    srand(time(0));
    int barber_pace = ((int)rand() % (barber_max - barber_min + 1)) + barber_min;

  // keep working until being terminated by the main
  while (1) {
    barber_service(barber_id);  // pick up a new customer
    sleep(barber_pace);     // spend a service time
    barber_done(barber_id);    // release the customer
  }
}

// the customer thread function
void *customer_routine( void *arg ) {

    int *id = (int *) arg;

    int barber_id = arrive_shop(*id); // enter shop and get assigned to a barber
    if (barber_id != -1) {
        leave_shop(*id, barber_id);
    }
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    
    // ask for number of barbers.
    printf("Enter total number of barbers (int): ");
    scanf("%d", &no_barbers);

    // ask for seating capacity.
    printf("Enter total number of seats (int): ");
    scanf("%d", &seat_capacity);

    // ask for the total number of customers.
    printf("Enter the total number of customers (int): ");
    scanf("%d", &no_customers);

    //ask for barber's min working pace
    printf("Enter barber's min working pace (int): ");
    scanf("%d", &barber_min);

    //ask for barber's max working pace
    printf("Enter barber's max working pace (int): ");
    scanf("%d", &barber_max);

    //ask for customer's min arrival rate
    printf("Enter customer's min arrival rate (int): ");
    scanf("%d", &customer_min);

    //ask for customer's max arrival rate
    printf("Enter customer's max arrival rate (int): ");
    scanf("%d", &customer_max);

    if ((barber_min > barber_max) 
        || (customer_min > customer_max)) {
            printf("Invalid inputs: min values > max values.\n");
    }

    barber_argv_arr = malloc(sizeof(barber_argv) * no_barbers);

    for (int i = 0; i < no_barbers; i++)
    {
        barber_argv_arr[i].barber_min = barber_min;
        barber_argv_arr[i].barber_max = barber_max;
        barber_argv_arr[i].barber_id = i;
    }

    barber_threads = malloc(sizeof(pthread_t) * no_barbers);
    customer_threads = malloc(sizeof(pthread_t) * no_customers);
    t_ids = malloc(sizeof(int) * no_customers);

    shop = init_shop(no_barbers, no_customers, seat_capacity, 
        customer_min, customer_max, barber_min, barber_max);

    
    int rc;

    rc = pthread_create(&assistant_thread, NULL, assistant_routine, NULL); // barber routine takes barber_pace as the arg
    if (rc) {
        printf("ERROR; return code from pthread_create() (assistant) is %d\n", rc);
        exit(-1);
    }
    
    for (int i = 0; i < no_barbers; i++)
    {
        rc = pthread_create(&barber_threads[i], NULL, barber_routine, (void*)&barber_argv_arr[i]); // barber routine takes barber_pace as the arg

        if (rc) {
            printf("ERROR; return code from pthread_create() (barbers) is %d\n", rc);
            exit(-1);
        }
    }
    
    srand(time(0));
    for (int i = 0; i < no_customers; i++)
    {
        sleep(((int)rand() % (customer_max - customer_min + 1))+ customer_min); //sleep a few second before creating a thread
        t_ids[i] = i;
        rc = pthread_create(&customer_threads[i], NULL, customer_routine, (void*)&t_ids[i]); //customer routine takes thread id as the arg

        if (rc) {
            printf("ERROR; return code from pthread_create() (customer) is %d\n", rc);
            exit(-1);
        }
    }

    //join barber threads.
    for (int i = 0; i < no_barbers; i++)
    {
        pthread_join(barber_threads[i], NULL);
    }

    //join customers threads.
    for (int i = 0; i < no_customers; i++)
    {
        pthread_join(customer_threads[i], NULL);
    }

}
