#ifndef WORDPollArgsLIST_H
#define WORDPollArgsLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#include "constants.h"

typedef struct PollArgsNode {
    int fd;
    struct PollArgsNode * next;
} PollArgsNode;

typedef struct PollArgsList {
    PollArgsNode * headWPAL;
} PollArgsList;

void addPollArgsList(PollArgsList *, int val); //add fd (val) in list

void removePollArgsList(PollArgsList *, int val); //remove val (fd) from PollArgsList

void printPollArgsList(PollArgsList * list); //print words list of the query

int countPollArgsList(PollArgsList * list); //#node of PollArgsList

PollArgsList * createPollArgsList(); //create new PollArgsList

void destroyPollArgsList(PollArgsList ** list); //destroy (free) PollArgsList

int findPollArgsList(PollArgsList * List, int fd); //find fd in PollArgsList

void deleteLastPollArgsNode(PollArgsList ** list);

struct pollfd * serializePollArgsList(PollArgsList * list); //create fdarray

int pollWMonitoring(PollArgsList * pal);

#endif /* WORDLIST_H */