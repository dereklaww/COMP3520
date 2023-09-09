#include "shop.h"

Barber *barbers;
Customer *customers;
Assistant assistant;

Queue *customers_queue;
Queue *barber_queue;

Shop shop;

pthread_mutex_t waiting_room_mutex, barber_room_mutex, barber_mutex;

Shop init_shop(int no_barbers, int no_customers, int seating_capacity) {
    
    shop = (Shop) {
        .no_barbers = no_barbers,
        .seat_capacity = seating_capacity,
        .no_customers = no_customers,
    };

    assistant = (Assistant) {
        .ticket_id = 0,
        .no_leaving = 0,
        .shop_close = 0
    };

    pthread_cond_init(&assistant.customer_cond, NULL);
    
    barbers = malloc(sizeof(Barber) * no_barbers);
    customers = malloc(sizeof(Customer) * no_customers);

    customers_queue = inst_queue(seating_capacity);
    barber_queue = inst_queue(no_barbers);

    pthread_mutex_init(&waiting_room_mutex, NULL);
    pthread_mutex_init(&barber_room_mutex, NULL);
    pthread_mutex_init(&barber_mutex, NULL);

    for (int i = 0; i < no_barbers; i++) {
        barbers[i].id = i;
        barbers[i].customer_id = -1;
        pthread_cond_init(&barbers[i].barber_cond, NULL);
    }

    return shop;
}

void assistant_waiting_customer(void) {

    pthread_mutex_lock(&waiting_room_mutex);
        while (is_empty(customers_queue) && assistant.no_leaving >= 0) {
            printf("Assistant: I'm waiting for customers.\n");
            pthread_cond_wait(&assistant.customer_cond, &waiting_room_mutex);
        }

    pthread_mutex_unlock(&waiting_room_mutex);

}

void assistant_waiting_barber(void) {
    
    pthread_mutex_lock(&barber_room_mutex);

        printf("Assistant: I'm waiting for barbers to become available.\n");
        printf("assistant no leaving: %d\n", assistant.no_leaving);
        while (is_empty(barber_queue)) {
            pthread_cond_wait(&assistant.barber_cond, &barber_room_mutex);
        }

        if (assistant.no_leaving == -1){
            while (!is_full(barber_queue)) {
                pthread_cond_wait(&assistant.barber_cond, &barber_room_mutex);
            }
            printf("Assistant: Hi barbers. We've finished the work for the day. See you all tomorrow!\n");
            for (int i = 0; i < shop.no_barbers; i++) {
                get_barber(i)->customer_id = -2;
                pthread_cond_signal(&get_barber(i)->barber_cond);
            }
            pthread_mutex_unlock(&barber_room_mutex);
            pthread_exit(EXIT_SUCCESS);
        }

    pthread_mutex_unlock(&barber_room_mutex);  
}

void assistant_assign_customer_barber(void) {

    pthread_mutex_lock(&waiting_room_mutex);
    int customer_id = front(customers_queue);
    pop(customers_queue);
    pthread_mutex_unlock(&waiting_room_mutex);


    pthread_mutex_lock(&barber_room_mutex);
    int barber_id = front(barber_queue);
    pop(barber_queue);
    pthread_mutex_unlock(&barber_room_mutex);
    
    customers[customer_id].barber_id = barber_id;
    get_barber(barber_id)->customer_id = customer_id;
    
    pthread_cond_signal(&get_barber(barber_id)->barber_cond);
    pthread_cond_signal(&customers[customer_id].customer_cond);

    printf("Assistant: I'm assigning Customer %d to Barber %d.\n", customer_id, barber_id);

}

//require assistant to assign a barber to a customer and ticket id
int arrive_shop(int customer_id) {

    pthread_mutex_lock(&waiting_room_mutex);
    printf("Customer [%d]: I have arrived at the barber shop. \n", customer_id);

    if (is_full(customers_queue)) {
        printf("Customer [%d]: Oh no! All seats have been taken and I'll leave now!\n", customer_id);
        assistant.no_leaving++;

        if (assistant.no_leaving == shop.no_customers) {
            printf("here\n");
            assistant.no_leaving = -1;
            pthread_cond_signal(&assistant.customer_cond);
        }
        pthread_mutex_unlock(&waiting_room_mutex);
        return -1;
    }

    customers[customer_id] = (Customer) {
        .id = customer_id,
        .barber_id = -1
    };

    pthread_cond_init(&customers[customer_id].customer_cond, NULL);
    int barber_id;
    int ticket_id = assistant.ticket_id % shop.seat_capacity;
    printf("Customer [%d]: I'm lucky to get a free seat and a ticket numbered %d\n", customer_id, ticket_id);
    push(customers_queue, customer_id);
    assistant.ticket_id++;
    pthread_cond_signal(&assistant.customer_cond);

    while (customers[customer_id].barber_id == -1) {
        printf("Customer [%d]: I'm waiting to be assigned to a barber.\n", customer_id);
        pthread_cond_wait(&customers[customer_id].customer_cond, &waiting_room_mutex);
    }

    barber_id = customers[customer_id].barber_id;
    printf("Customer [%d]: My ticket numbered %d has been called. Hello, Barber %d\n",
      customer_id, ticket_id, barber_id);
    assistant.ticket_id--;
    pthread_mutex_unlock(&waiting_room_mutex);
    
    pthread_mutex_lock(&barber_mutex);
    customers[customer_id].current_state = CUTTING;
    pthread_cond_signal(&get_barber(barber_id)->barber_cond);
    pthread_mutex_unlock(&barber_mutex);
    return barber_id;
}

void leave_shop(int customer_id, int barber_id) {

    pthread_mutex_lock(&barber_mutex);
    
    printf("Customer [%d]: I am being served\n", customer_id);

    while (customers[customer_id].barber_id != -1) {
        pthread_cond_wait(&customers[customer_id].customer_cond, &barber_mutex);
    }

    customers[customer_id].current_state = LEAVING;
    printf("Customer [%d]: Well done. Thank Barber %d, bye!\n", customer_id, barber_id);
    assistant.no_leaving++;
    printf("customer: no leaving: %d\n", assistant.no_leaving);
    if (assistant.no_leaving == shop.no_customers) {
        assistant.no_leaving = -1;
        pthread_cond_signal(&assistant.customer_cond);
    }
    // pthread_cond_signal(&get_barber(barber_id)->barber_cond);
    pthread_mutex_unlock(&barber_mutex);
}

void barber_service(int barber_id) {
    pthread_mutex_lock(&barber_room_mutex);

    if (get_barber(barber_id)->customer_id == -1) {
        push(barber_queue, barber_id);
        pthread_cond_signal(&assistant.barber_cond);
        printf("Barber[%d]: I'm now ready to accept a new customer. \n", barber_id);
        while (get_barber(barber_id)->customer_id == -1) {
            pthread_cond_wait(&get_barber(barber_id)->barber_cond, &barber_room_mutex);
        }

        if (get_barber(barber_id)->customer_id == -2) {
            printf("Barber [%d]: Thanks Assistant. See you tomorrow!\n", barber_id);
            pthread_mutex_unlock(&barber_room_mutex);
            pthread_exit(EXIT_SUCCESS);
        }
    }
    pthread_mutex_unlock(&barber_room_mutex);

    pthread_mutex_lock(&barber_mutex);

    while (customers[get_barber(barber_id)->customer_id].current_state != CUTTING) {
        printf("Barber[%d]: I'm now waiting for customer to sit down. \n", barber_id);
        pthread_cond_wait(&get_barber(barber_id)->barber_cond, &barber_mutex);
    }

    printf("Barber [%d]: Hello, Customer %d. \n", barber_id, get_barber(barber_id)->customer_id);
    pthread_mutex_unlock(&barber_mutex);
}

void barber_done(int barber_id) {
    pthread_mutex_lock(&barber_mutex);

    printf("Barber [%d]: Finished cutting. Good bye Customer %d.\n", barber_id, get_barber(barber_id)->customer_id);

    customers[get_barber(barber_id)->customer_id].barber_id = -1;
    pthread_cond_signal(&customers[get_barber(barber_id)->customer_id].customer_cond);

    // while (customers[get_barber(barber_id)->customer_id].current_state != LEAVING) {
    //     pthread_cond_wait(&get_barber(barber_id)->barber_cond, &barber_mutex);
    // }

    get_barber(barber_id)->customer_id = -1;
    pthread_mutex_unlock(&barber_mutex);
}

Barber *get_barber(int barber_id) {
    for (int i = 0; i < shop.no_barbers; i++) {
        if (barbers[i].id == barber_id) {
            return &barbers[i];
        }
    }

    return NULL;
}

