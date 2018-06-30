#ifndef JOBEXECUTOR_H
#define JOBEXECUTOR_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <poll.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "constants.h"
#include "helpers.h"
#include "worker.h"
#include "communication.h"
#include "workersPInfo.h"
#include "WordQList.h"
#include "PollArgsList.h"

void initialize(int w, char * fileName, int * flagForInitialize);

void searchWCommand(WordQList * qlist, int activeFd, int terminalSize);

#endif /* JOBEXECUTOR_H */