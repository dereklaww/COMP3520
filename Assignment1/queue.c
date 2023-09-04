#include <stdio.h>
#include <queue.h>

Queue * inst_queue(int capacity) {
    Queue * queue = (Queue *) malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int *) malloc(queue->capacity * sizeof(int));
    return queue;
}

int is_empty(Queue *queue) {
    return (queue->size == 0);
}

int is_full(Queue *queue) {
    return (queue->size == queue->capacity);
}

void push(Queue *queue, int item) {
    if (is_full(queue)) {
        return -1;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

void pop(Queue *queue) {
    if (is_empty(queue)) {
        return -1;
    }
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
}

int front(Queue *queue) {
    if (is_empty(queue)) {
        return -1;
    }
    return queue->array[queue->front];
}