#include "PostingList.h"

void pushPL(Files ** node, int idLine, int * size, char * filePath, int startOffset) { // find or add a new FIles Node into PL
    Files * current = *node;
    
    if (*node == NULL) { // create 1st node
        *node = malloc(sizeof(Files));
        (*node)->filePath = calloc(strlen(filePath) + 1, sizeof(char));
        strcpy((*node)->filePath, filePath);
        (*node)->count = 1;
        (*node)->headLines = malloc(sizeof(Lines));
        (*node)->headLines->startOffset = startOffset;
        (*node)->headLines->count = 1;
        (*node)->headLines->idLine = idLine;
        (*node)->headLines->next = NULL;
        (*node)->next = NULL;
        (*size)++;
    } else {
        if (strcmp(current->filePath,filePath) == 0) { // check 1st Node
            current->count++;
            pushLines(&(current->headLines), idLine, startOffset); // find (count++) or add a new Lines Node under standrd Files Node into PL
            return;
        }

        while (current->next != NULL) { // check >=2 Nodes
            current = current->next;
            
            if (strcmp(current->filePath,filePath) == 0) {
                current->count++;
                pushLines(&(current->headLines), idLine, startOffset); // find (count++) or add a new Lines Node under standrd Files Node into PL
                return;
            }
        }

        current->next = malloc(sizeof(Files)); // add new Files Node
        current = current->next;
        current->filePath = calloc(strlen(filePath) + 1, sizeof(char));
        strcpy(current->filePath, filePath);
        current->count = 1;
        current->headLines = malloc(sizeof(Lines)); // add one Line of this Files Node
        current->headLines->startOffset = startOffset;
        current->headLines->count = 1;
        current->headLines->idLine = idLine;
        current->headLines->next = NULL;
        current->next = NULL;
        (*size)++;
    }
}

void pushLines(Lines ** headLines, int idLine, int startOffset){
    Lines * current = *headLines;

    if (current->idLine == idLine){ // check 1st Lines Node
        current->count++;
        return;
    }

    while (current->next != NULL) { // check >= 2nd Lines NOde
        current = current->next;
            
        if (current->idLine == idLine) {
            current->count++; // countLines++;
            return;
        }
    }

    current->next = malloc(sizeof(Lines)); // add new Lines Node under a standard FIles Node
    current = current->next;
    current->startOffset = startOffset;
    current->idLine = idLine;
    current->count = 1;
    current->next = NULL;
}

void destroyPL(PL ** headPL) { // delete (free) PL
    Files * current = (*headPL)->ptr;
    Files * next;

    while(current != NULL){
        next = current->next;
        destroyLines(&(current->headLines));
        free(current->filePath);
        free(current);
        current = next;
    }
    
    free(*headPL);
    *headPL = NULL;
}

void destroyLines(Lines ** headLines){ // delete (free) Lines struct
    Lines * current = *headLines;
    Lines * next;

    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }

    *headLines = NULL;
}