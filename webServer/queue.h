#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define THREAD_QUEUE_SIZE 200

typedef struct Queue {
    int data[THREAD_QUEUE_SIZE];
    int start;
    int end;
    int count;

    pthread_mutex_t mtx;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;
} Queue;

Queue * createQueue();
void destroyQueue(Queue ** q);

void produceQueue(Queue * q, int fd);
int consumeQueue(Queue * q);

#endif /* PRODUCER_CONSUMER_H */
