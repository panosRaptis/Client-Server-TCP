
#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "HashTable.h"

#define THREAD_QUEUE_SIZE 200

typedef struct Queue {
    char * data[THREAD_QUEUE_SIZE];
    int start;
    int end;
    int count;
    int totalWorkers;
    int threadsWaiting;

    pthread_mutex_t mtx;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;
} Queue;

Queue * createQueue(int totalWorkers);
void destroyQueue(Queue ** q);

void produceQueue(Queue * q, char * url, HashTable * hashTable);
char * consumeQueue(Queue * q);

int getJobsRemaining(Queue * q);
#endif /* PRODUCER_CONSUMER_H */

