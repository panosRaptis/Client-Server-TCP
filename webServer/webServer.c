#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "communication.h"
#include <signal.h>
#include <netdb.h>
#include <unistd.h> 
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "PollArgsList.h"
#include "inet_str_server.h"
#include "stats.h"
#include "worker.h"
#include "queue.h"
#include "helpers.h"

static int shutdownPending = 0;
struct timeval t1;

void catchInterrupt(int signo) { // signal handler for SIGINT, SIGQUIT
    printf("**** Shutting down webServer... ****\n ");
    shutdownPending = 1;
}

// http://cgi.di.uoa.gr/~ad/k22/named-pipes.pdf
int pollMonitoring(PollArgsList * pal) {
    int N = countPollArgsList(pal);
    struct pollfd * fdarray = serializePollArgsList(pal);
    int rc;
    int nextFd = -2;

    printf("> Main thread: waiting in Poll \n");
    rc = poll(fdarray, N, -1);


    if (rc == 0) { // time out
        perror("poll eof");
        free(fdarray);
        fdarray = NULL;
        return 0;
    }

    if (rc < 0) {
        perror("poll interrupted");
        free(fdarray);
        fdarray = NULL;
        return rc;
    }

    for (int i = 0; i < N; i++) {
        if ((fdarray[i].revents & POLLIN) != 0) {
            nextFd = fdarray[i].fd;
            break;
        }
    }

    free(fdarray);
    fdarray = NULL;

    printf("> poll returns: %d\n", nextFd);
    return nextFd;
}

// https://stackoverflow.com/questions/2150291/how-do-i-measure-a-time-interval-in-c
char * measureTime() { // measure time stats format is xx Hours : yyMins : zz Secs : kkk mSecs 
    struct timeval t2;
    gettimeofday(&t2, NULL);
    long int totalTime = (long int) (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0; // ms
    char * buf = calloc(TIME_BUF_SIZE, sizeof(char));
    int hours = totalTime / 3600000;
    totalTime = totalTime % 3600000;
    int mins = totalTime / 60000;
    totalTime = totalTime % 60000;
    int secs = totalTime / 1000;
    totalTime = totalTime % 1000;

    sprintf(buf, "%02d:%02d:%02d.%03d", hours, mins, secs, (int)totalTime);
    return buf;
}

int command(int active_fd, Stats * stats) { // service command port
    char communicationBuf[BUF_SIZE];
    memset(communicationBuf, '\0', BUF_SIZE);
    printf("> Starting Command Handler...\n");
    int code = 1;

    int nread = myReadTerminalCommand(active_fd, communicationBuf, BUF_SIZE);

    if (nread == BUF_SIZE) { // buffer overflow
        printf("note: overflow, invalid command from client\n");
        close(active_fd);
        return -1;
    }

    for (int i = 0; i < strlen(communicationBuf); i++) {
        if (communicationBuf[i] == '\r' || communicationBuf[i] == '\n') {
            communicationBuf[i] = '\0';
            break;
        } else {
            communicationBuf[i] = toupper(communicationBuf[i]);
        }
    }

    if (strcmp(communicationBuf, "STATS") == 0) { // if "STATS" => read stats from global vars
        memset(communicationBuf, '\0', BUF_SIZE);
        char * time = measureTime();

        pthread_mutex_lock(&stats->mtx); // lock mutex of stats
        unsigned long long int servedPages = stats->numOfPages;
        unsigned long long int servedBytes = stats->numOfBytes;
        pthread_mutex_unlock(&stats->mtx); // unlock mutex of stats

        sprintf(communicationBuf, "Server up for %s, served %llu pages, %llu bytes.\n", time, servedPages, servedBytes);
        myWrite(active_fd, communicationBuf, strlen(communicationBuf)); // + send them to socket
        free(time);
    } else if (strcmp(communicationBuf, "SHUTDOWN") == 0) { // if "SHUTDOWN" => break loop in main => exit
        code = 0;
    } else {
        memset(communicationBuf, '\0', BUF_SIZE);
        strcpy(communicationBuf, "Invallid Command!!\n");
        myWrite(active_fd, communicationBuf, strlen(communicationBuf)); // send to socket
    }
    printf("> Closing command connection.\n");
    close(active_fd); // close connection
    return code;
}

int main(int argc, char** argv) {
    int err;
    int serverPort = -1;
    int commandPort = -1;
    int numOfThreads;
    char * rootDir = NULL;
    char * endPtrStrtolCheck;

    gettimeofday(&t1, NULL);

    struct sockaddr_in client = {0};
    struct sockaddr *clientptr = (struct sockaddr *) &client;
    socklen_t clientlen = sizeof(client);

    static struct sigaction actSP;
    actSP.sa_handler = SIG_IGN;
    sigfillset(&(actSP.sa_mask));
    sigaction(SIGPIPE, &actSP, NULL);

    if (argc != 9) { // typical controls
        printf("> Use correct arguments!!\n");
        exit(EXIT_FAILURE);
    } else {
        for (int i = 0; i < argc; i++) {
            if ((i + 1 < argc) && (strcmp(argv[i], "-p") == 0)) {
                endPtrStrtolCheck = NULL;
                serverPort = strtol(argv[i + 1], &endPtrStrtolCheck, 10); // check if argument is incorect
                if (serverPort < 0 || argv[i + 1] == endPtrStrtolCheck) {
                    fprintf(stderr, "Incorrect port!!\n");
                    exit(EXIT_FAILURE);
                }
                printf("> serving_port = %d\n", serverPort);
            } else if ((i + 1 < argc) && (strcmp(argv[i], "-c") == 0)) {
                endPtrStrtolCheck = NULL;
                commandPort = strtol(argv[i + 1], &endPtrStrtolCheck, 10); // check if argument is incorect
                if (commandPort < 0 || argv[i + 1] == endPtrStrtolCheck) {
                    fprintf(stderr, "Incorrect command_port!!\n");
                    exit(EXIT_FAILURE);
                }
                printf("> command_port = %d\n", commandPort);
            } else if ((i + 1 < argc) && (strcmp(argv[i], "-t") == 0)) {
                endPtrStrtolCheck = NULL;
                numOfThreads = strtol(argv[i + 1], &endPtrStrtolCheck, 10); // check if argument is incorect
                if (numOfThreads < 0 || argv[i + 1] == endPtrStrtolCheck) {
                    fprintf(stderr, "Incorrect mum_of_threads!!\n");
                    exit(EXIT_FAILURE);
                }
                printf("> num_of_threads = %d\n", numOfThreads);
            } else if ((i + 1 < argc) && (strcmp(argv[i], "-d") == 0)) {
                rootDir = argv[i + 1];
                printf("> root_dir = %s\n", rootDir);
            }
        }
    }

    Queue * queue = createQueue(); // create buffer
    Stats * stats = createStats(); // create Statistics
    InfoWList * ilist = malloc(sizeof(InfoWList)); // create Info Workers structure
    ilist->headIWL = NULL;
    InfoWNode * current = NULL;
    pthread_mutex_t * dateMtx = malloc(sizeof(pthread_mutex_t)); // mutex for 

    pthread_mutex_init(dateMtx, 0);

    for (int i = 0; i < numOfThreads; i++) {
        if (ilist->headIWL == NULL) {
            ilist->headIWL = malloc(sizeof(InfoWNode));
            current = ilist->headIWL;
            current->dateMtx = dateMtx;
            current->tid = -1;
            current->next = NULL;
        } else {
            current->next = malloc(sizeof(InfoWNode));
            current = current->next;
            current->dateMtx = dateMtx;
            current->tid = -1;
            current->next = NULL;
        }
    }

    InfoWNode * curWork = ilist->headIWL;
    while (curWork != NULL) {
        curWork->queue = queue;
        curWork->rootdir = rootDir;
        curWork->stats = stats;

        if ((err = pthread_create(&curWork->tid, NULL, mainWorker, (void *) curWork)) > 0) { // Create a thread for thread pool
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        curWork = curWork->next;
    }

    static struct sigaction act; // handler for SIGINT
    act.sa_handler = catchInterrupt;
    sigfillset(&(act.sa_mask));
    sigaction(SIGINT, &act, NULL);


    int serverFd = createSocket(serverPort);
    int commandFd = createSocket(commandPort);
    int activeFd;
    int clientFd;

    PollArgsList * pal = createPollArgsList();

    addPollArgsList(pal, serverFd);
    addPollArgsList(pal, commandFd);

    while (shutdownPending == 0 && (activeFd = pollMonitoring(pal)) > 0 && shutdownPending == 0) {
        int code;
        if ((clientFd = accept(activeFd, clientptr, &clientlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if (activeFd == serverFd) {
            printf("> connection from serverFd\n");

            curWork = ilist->headIWL;
            while (curWork != NULL) {
                curWork->queue = queue;
                curWork->rootdir = rootDir;
                curWork->stats = stats;
                
                err = pthread_tryjoin_np(curWork->tid, NULL);
             
                if (err == 0) {     // join successful, thus thread has died           
                    if ((err = pthread_create(&curWork->tid, NULL, mainWorker, (void *) curWork)) > 0) { // regenerate thread
                        perror("pthread_create");
                        exit(EXIT_FAILURE);
                    }
                }
                curWork = curWork->next;
            }

            produceQueue(queue, clientFd);
        }

        if (activeFd == commandFd) {
            printf("> connection from commandFd\n");
            code = command(clientFd, stats); // command function

            if (code == 0) break;
        }
    }

    curWork = ilist->headIWL;
    while (curWork != NULL) { // sent "signal" to the workers for smooth termation
        produceQueue(curWork->queue, 0);
        curWork = curWork->next;
    }

    curWork = ilist->headIWL;
    while (curWork != NULL) { // wait threads workers
        void * status = NULL;
        pthread_join(curWork->tid, &status);

        printf("> Thread %lu finished with status : %d\n", (unsigned long int) curWork->tid, *((int*) status));
        free(status);
        curWork = curWork->next;
    }

    close(serverFd);
    close(commandFd);

    pthread_mutex_destroy(dateMtx);
    free(dateMtx);

    destroyInfoWList(&ilist);
    destroyPollArgsList(&pal);
    destroyQueue(&queue);
    destroyStats(&stats);
    printf("> GoodBye !!\n");

    return (EXIT_SUCCESS);
}