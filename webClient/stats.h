#ifndef STATS_H
#define STATS_H

typedef struct Stats {
    unsigned long long int numOfPages;
    unsigned long long int numOfBytes;

    pthread_mutex_t mtx;
} Stats;

Stats * createStats();
void destroyStats(Stats ** q);

#endif /* PRODUCER_CONSUMER_H */