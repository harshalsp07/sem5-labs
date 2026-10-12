#include "queue.h"


void queue_init(Queue *q)
{
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}


int queue_is_empty(Queue *q)
{
    return q->size == 0;
}


int queue_is_full(Queue *q)
{
    return q->size == MAX_PROCESSES;
}


int queue_push(Queue *q, int value)
{
    if (queue_is_full(q))
        return 0;

    q->rear = (q->rear + 1) % MAX_PROCESSES;

    q->data[q->rear] = value;

    q->size++;

    return 1;
}


int queue_pop(Queue *q)
{
    if (queue_is_empty(q))
        return -1;

    int value = q->data[q->front];

    q->front = (q->front + 1) % MAX_PROCESSES;

    q->size--;

    return value;
}


int queue_peek(Queue *q)
{
    if (queue_is_empty(q))
        return -1;

    return q->data[q->front];
}
