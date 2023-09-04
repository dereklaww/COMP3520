#ifndef QUEUE
#define QUEUE

typedef struct {

    int *array;
    int front;
    int rear;
    int size;
    int capacity;

} Queue;

Queue * inst_queue(int capacity);
int is_empty(Queue *queue);
int is_full(Queue *queue);
void push(Queue *queue, int item);
void pop(Queue *queue);
int front(Queue *queue);

#endif