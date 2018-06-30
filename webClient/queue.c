#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#include "queue.h"

// Course Slides - Topic 6 (THreads)

Queue * createQueue(int totalWorkers) { // create Queue
    Queue * queue = (Queue *) malloc(sizeof(Queue));

    pthread_mutex_init(&queue->mtx, 0);
    pthread_cond_init(&queue->cond_nonempty, 0);
    pthread_cond_init(&queue->cond_nonfull, 0);

    queue->start = 0;
    queue->end = -1;
    queue->count = 0;
    queue->threadsWaiting = 0;
    queue->totalWorkers = totalWorkers;

    return queue;
}

void destroyQueue(Queue ** q) { // delete (free) Queue
    pthread_cond_destroy(&((*q)->cond_nonempty));
    pthread_cond_destroy(&((*q)->cond_nonfull));
    pthread_mutex_destroy(&((*q)->mtx));

    free(*q);
    *q = NULL;
}

void produceQueue(Queue * queue, char * url, HashTable * hashTable) { // add new URL in Queue (Buffer)

    pthread_mutex_lock(&queue->mtx); // lock MTX for queue & hashTable (History structure)

    if ((url == NULL) || !findURL(url, hashTable)) {

        while (queue->count >= THREAD_QUEUE_SIZE) { // full queue
            pthread_cond_wait(&queue->cond_nonfull, &queue->mtx);
        }

        queue->end = (queue->end + 1) % THREAD_QUEUE_SIZE;
        queue->data[queue->end] = url; // add url in Queue (Buffer)
        if (url != NULL) addURLInHash(url, hashTable); // add in hashTable (History Structure)
        queue->count++;
        pthread_cond_broadcast(&queue->cond_nonempty);
    }else if(url != NULL) free(url);

    pthread_mutex_unlock(&queue->mtx); // unlock MTX
}

char * consumeQueue(Queue * queue) { // remove a URL from Queue (Buffer)
    char * data = 0;

    pthread_mutex_lock(&queue->mtx); // lock MTX for Queue & hashTable (History structure)
       
    queue->threadsWaiting++;
    while (queue->count <= 0) { // empty Queue
        pthread_cond_wait(&queue->cond_nonempty, &queue->mtx);
    }
    queue->threadsWaiting--;

    data = queue->data[queue->start];
    printf("> I consume URL = %s\n", data);
    queue->start = (queue->start + 1) % THREAD_QUEUE_SIZE;
    queue->count--;

    pthread_cond_broadcast(&queue->cond_nonfull);

    pthread_mutex_unlock(&queue->mtx); // unlock MTX

    return data;
}

int getJobsRemaining(Queue * queue) { // if crawling process has fineshes => return 0
                                      // else return result > 0
    int jobs_active = 0;
    int jobs_pending = 0;
    
    pthread_mutex_lock(&queue->mtx);
    jobs_pending = queue->count;
    jobs_active = queue->totalWorkers - queue->threadsWaiting;
    pthread_mutex_unlock(&queue->mtx);
    
    if (jobs_active == 0 && jobs_pending == 0) {
        return 0;
    } else {
        return jobs_active + jobs_pending;
    }
}