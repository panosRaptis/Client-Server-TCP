#ifndef WORKERSPINFO_H
#define WORKERPINFO_H

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

typedef struct Paths {
    char * str;
    struct Paths * next;
} Paths;

typedef struct InfoPNode {
    Paths * paths;
    pid_t pid;
    char * strTransmit;
    char * strReceive;
    int fd1;
    int fd2;
    int status;
    int numOfPaths;
    struct InfoPNode * next;
} InfoPNode;

typedef struct InfoPList {
    InfoPNode * headP;
} InfoPList;

InfoPList * plist;

void addPath(Paths * path, char * val); //add Path Node under standard File Node

InfoPNode * GetPNth(InfoPList * head, int index); //get a Info Node (worker) with value = index

void destroyInfoPList(InfoPList ** head); //delete (free) InfoPList

void destroyPaths(Paths ** path);  //delete (free) Paths

#endif /* WORKERSPINFO_H */