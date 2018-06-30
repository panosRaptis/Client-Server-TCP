#ifndef WORDQLIST_H
#define WORDQLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "constants.h"

typedef struct WordQNode {
    char * word;
    struct WordQNode * next;
} WordQNode;

typedef struct WordQList {
    WordQNode * headWQL;
} WordQList;

void addQList(WordQList *, char * val); //add word in list

void printQList(WordQList * list); //print words list

int countQList(WordQList * list); //print count of words list

WordQList * createWordQList(); //create word list

void destroyWordQList(WordQList ** list); //delete word list

int findWordQList(WordQList * List, char* word); //find word in list

void deleteLastQNode(WordQList ** list);

int sendQList(WordQList * list, int senderFd); //send words list use named-pipe

WordQList * receiveQList(int receiverFd) ; //receive words list from a named-pipe

char * serializeQList(WordQList * list); //serialize received worker command from a named-pipe

#endif /* WORDLIST_H */