#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#include "queue.h"

// Course Slides - Topic 6
Queue * createQueue() {
    Queue * queue = (Queue *) malloc(sizeof(Queue));

    pthread_mutex_init(&queue->mtx, 0);
    pthread_cond_init(&queue->cond_nonempty, 0);
    pthread_cond_init(&queue->cond_nonfull, 0);

    queue->start = 0;
    queue->end = -1;
    queue->count = 0;

    return queue;
}

void destroyQueue(Queue ** q) {
    pthread_cond_destroy(&((*q)->cond_nonempty));
    pthread_cond_destroy(&((*q)->cond_nonfull));
    pthread_mutex_destroy(&((*q)->mtx));

    free(*q);
    *q = NULL;
}

void produceQueue(Queue * queue, int fd) {

    pthread_mutex_lock(&queue->mtx);
    
    while (queue->count >= THREAD_QUEUE_SIZE) {
        pthread_cond_wait(&queue->cond_nonfull, &queue->mtx);
    }
    
    queue->end = (queue->end + 1) % THREAD_QUEUE_SIZE;
    queue->data[queue->end] = fd;
    queue->count++;
    
    pthread_cond_broadcast(&queue->cond_nonempty);
    
    pthread_mutex_unlock(&queue->mtx);
}

int consumeQueue(Queue * queue) {
    int data = 0;
    
    pthread_mutex_lock(&queue->mtx);

    while (queue->count <= 0) {
        pthread_cond_wait(&queue->cond_nonempty, &queue->mtx);
    }

    data = queue->data[queue->start];
    queue->start = (queue->start + 1) % THREAD_QUEUE_SIZE;
    queue->count--;
    
    pthread_cond_broadcast(&queue->cond_nonfull);
    
    pthread_mutex_unlock(&queue->mtx);
    
    return data;
}