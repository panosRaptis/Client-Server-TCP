#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#include "WordQList.h"

int lineWords(const char sentence[ ]); //#words of a line (idDoc)
int numLines(FILE * fp); //#lines of file
void skipExtraSpaces(char * input); //skip extra Spaces or Tabs from the char * string
char * mytrimString(char * string); //trim String from 1st to last no space character
void prettyPrint(char * path, int idLine, char * strLine, int terminalWidth, WordQList * list, int count);
char * getFullPath(const char * dirPath); //get absolute path of a directory
bool isValidGETRequest(char str[ ]);

#endif /* HELPERS_H */