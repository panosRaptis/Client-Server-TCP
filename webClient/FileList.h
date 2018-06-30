#ifndef FILELIST_H
#define FILELIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PostingList.h"
#include "helpers.h"
#include "WordQList.h"
#include "communication.h"

typedef struct LineQNode {
    int idLine;
    int startOffset;
    struct LineQNode * next;
} LineQNode;

typedef struct FileQNode {
    char * filePath;
    LineQNode * ptr;
    struct FileQNode * next;
} FileQNode;

typedef struct FileQList {
    FileQNode * headFQL;
} FileQList;

void pushFileQList (FileQNode ** node, Files * files);  //find (and add Lines Node) or add new File Node into FileQList
void pushQLines(LineQNode ** headQLines, Lines * headLines);  //find or add new Line Node under a standard File Node
void destroyFileQList(FileQList ** head); //delete (free) FileQueryList (search command) of the query
void destroyLineQList(LineQNode ** headLines); //delete (free) LineQuetyList of FileQueryList (search command)
void printFileQlist(FileQList * list); //printf FileQList
void sendToPipeFileQList(FileQList * list, int fd); //sent FileQList (search command acks from workers to jobExecutor) via named-pipe

#endif /* FILELIST_H */