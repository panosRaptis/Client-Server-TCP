#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

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
#include <pthread.h>
#include <sys/socket.h>

#include "helpers.h"
#include "WordQList.h"
#include "constants.h"
#include "communication.h"
#include "HashTable.h"
#include "workersInfo.h"
#include "inetStrCrawler.h"


void network(int fd1, char * dirPath);

void * mainThread(void *);

#endif /* WORKERTHRAED_H */