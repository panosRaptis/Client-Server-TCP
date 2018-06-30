#ifndef PL_H
#define PL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Lines{
    int startOffset;
    int count;
    int idLine;
    struct Lines * next;
} Lines;

typedef struct Files {    
    char * filePath;
    int count;
    Lines * headLines;
    struct Files * next;
} Files;

typedef struct PL {
    int sizeOfFiles; // #files
    bool visitFlag;
    Files *ptr;
} PL;

void pushPL(Files ** node, int idLine, int * size, char * filePath, int startOffset); //find or add a new FIles Node into PL
void pushLines(Lines ** headLines, int idDoc, int startOffset); //find (count++) or add a new Lines Node under standrd Files Node into PL
void destroyPL(PL ** headPL); //delete (free) PL
void destroyLines(Lines ** headLines); //delete (free) Lines struct

#endif /* PL_H */