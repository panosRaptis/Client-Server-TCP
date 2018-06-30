#ifndef WORKERSINFO_H
#define WORKERINFO_H

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "queue.h"
#include "stats.h"

typedef struct InfoWNode {
    pthread_t tid;
    Queue * queue;
    Stats * stats;
    const char * rootdir;
    struct InfoWNode * next;
    pthread_mutex_t * dateMtx;
} InfoWNode;

typedef struct InfoWList {
    InfoWNode * headIWL;
} InfoWList;

InfoWNode * GetNth(InfoWList * head, int index); //get a Info Node (worker) with value = index

void destroyInfoWList(InfoWList ** head); //delete (free) InfoWList

#endif /* WORKERSINFO_H */