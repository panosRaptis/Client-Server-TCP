#ifndef WORKERSINFO_H
#define WORKERSINFO_H

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "queue.h"
#include "stats.h"
#include "HashTable.h"

typedef struct InfoWNode {
    pthread_t tid;
    Queue * queue;
    Stats * stats;
    HashTable * hashTable;
    const char * saveDir;
    struct InfoWNode * next;
    pthread_mutex_t * fileMtx;
    int fd;
    int blockSize;
} InfoWNode;

typedef struct InfoWList {
    InfoWNode * headIWL;
} InfoWList;

InfoWNode * GetNth(InfoWList * head, int index); //get a Info Node (worker) with value = index

void destroyInfoWList(InfoWList ** head); //delete (free) InfoWList

#endif /* WORKERSINFO_H */