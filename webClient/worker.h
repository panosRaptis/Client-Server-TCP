#ifndef WORKER_H
#define WORKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>

#include "Trie.h"
#include "helpers.h"
#include "FileList.h"
#include "WordQList.h"
#include "constants.h"
#include "PostingList.h"
#include "communication.h"

int mainWorker(char * transmit, char * receive);

#endif /* WORKER_H */