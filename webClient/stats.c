#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

#include "stats.h"

Stats * createStats() { // create Stat structure
    Stats * stats = (Stats *) malloc(sizeof(Stats));

    pthread_mutex_init(&stats->mtx, 0);

    stats->numOfPages = 0;
    stats->numOfBytes = 0;

    return stats;
}

void destroyStats(Stats ** q) { // delete (free) Stats
    pthread_mutex_destroy(&((*q)->mtx));
    free(*q);
}