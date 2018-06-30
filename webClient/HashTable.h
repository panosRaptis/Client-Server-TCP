#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stdlib.h>

#include "WordQList.h"

typedef struct HashTableNode{
    WordQList * qlist;
} HashTableNode;

typedef struct HashTable{
    HashTableNode * header;
    int size;
} HashTable;

HashTable * createHashTable(int size);

int HashIndex(unsigned char *str, int size);

bool findURL(char * url, HashTable * hashTable);

void addURLInHash(char * url, HashTable * hashTable);

void destroyHashTable(HashTable ** hashTable);

#endif /* HASHTABLE_H */