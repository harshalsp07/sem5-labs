#ifndef QUEUE_H
#define QUEUE_H

#define MAX_PROCESSES 100

typedef struct
{
    int data[MAX_PROCESSES];

    int front;
    int rear;
    int size;

} Queue;


void queue_init(Queue *q);

int queue_is_empty(Queue *q);

int queue_is_full(Queue *q);

int queue_push(Queue *q, int value);

int queue_pop(Queue *q);

int queue_peek(Queue *q);

#endif
