#include "HashTable.h"

HashTable * createHashTable(int size){ // create HashTable
    HashTable * hashTable = malloc(sizeof(HashTable));
    hashTable->header = malloc(size * sizeof(HashTableNode));
    hashTable->size = size;
    for(int i = 0; i < size; i++){
        hashTable->header[i].qlist = createWordQList();
    }
    return hashTable;
}

// https://stackoverflow.com/questions/7666509/hash-function-for-string
int HashIndex(unsigned char * str, int size){
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != 0) {hash = ((hash << 5) + hash) + c;}

    int result = (int) (hash % size);
    return result;
}

bool findURL(char * url, HashTable * hashTable){ // find if URL already exists in HashTable structure
    int pos = HashIndex((unsigned char *) url, hashTable->size);
    int res = findWordQList(hashTable->header[pos].qlist, url);
    if (res == 0) return true;
    else return false;
}

void addURLInHash(char * url, HashTable * hashTable){ // add new URL in HashTable
    int pos = HashIndex((unsigned char *) url, hashTable->size);
    addQList(hashTable->header[pos].qlist, url);
}

void destroyHashTable(HashTable ** hashTable){ // delete (free) HashTable
    for(int i = 0; i < (*hashTable)->size; i++){
        destroyWordQList(&((*hashTable)->header[i].qlist));
    }
    free((*hashTable)->header);
    (*hashTable)->header = NULL;

    free(*hashTable);
    *hashTable = NULL;
}