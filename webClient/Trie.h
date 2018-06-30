#ifndef TRIE_H
#define TRIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "WordQList.h"
#include "PostingList.h"
#include "FileList.h"
#include "helpers.h"

typedef struct Node {
    char c;
    struct Node * right;
    struct Node * down;
    PL * headPL;
} Node;

typedef struct Trie {
    Node * headTrie;
    int numWords;
    int numChars;
    int numLines;
} Trie;


void destroyTrie(Trie ** trie); // delete (free) Trie

Node * searchWord(Trie * trie, char * w); // search word into Trie

void addTrie(Trie ** trie, int idDoc, char * w, char * filePath, int startOffset); // add word (token) into Trie

// --------------------------------------------------------------------------------------------------

FileQList * searchCommand(Trie * trie, WordQList * qlist);

char * currentTime();

#endif /* TRIE_H */