#ifndef QUEUE
#define QUEUE

#include <stdlib.h>

typedef struct {

    int *array;
    int front;
    int rear;
    int occupied;
    int capacity;

} Queue;

Queue * inst_queue(int capacity);
int is_empty(Queue *queue);
int is_full(Queue *queue);
int push(Queue *queue, int item);
int pop(Queue *queue);
int front(Queue *queue);
int rear(Queue *queue);
int get_size(Queue *queue);

#endif
