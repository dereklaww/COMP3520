/* SID: 510226244 */ 
/* COMP3520: Assignment 1 */ 

#include <stdio.h>
#include "queue.h"

Queue * inst_queue(int capacity) {
    Queue * queue = malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = queue->occupied = 0;
    queue->rear = capacity - 1;
    queue->array = (int *) calloc(sizeof(int), queue->capacity);
    return queue;
}

int is_empty(Queue *queue) {
    return (queue->occupied == 0);
}

int is_full(Queue *queue) {
    return (queue->occupied == queue->capacity);
}

int push(Queue *queue, int item) {
    if (is_full(queue)) {
        return -1;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->occupied = queue->occupied + 1;

    return 0;
}

int pop(Queue *queue) {
    if (is_empty(queue)) {
        return -1;
    }
    queue->front = (queue->front + 1) % queue->capacity;
    queue->occupied = queue->occupied - 1;

    return 0;
}

int front(Queue *queue) {
    if (is_empty(queue)) {
        return -1;
    }
    return queue->array[queue->front];
}

int rear(Queue *queue) {
    if (is_empty(queue)) {
        return -1;
    }
    return queue->array[queue->rear];
}

int get_size(Queue *queue) {
    return queue->occupied;
}

void destroy(Queue *queue) {
    free(queue->array);
    free(queue);
}

