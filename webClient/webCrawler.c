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
#include <dirent.h>
#include <sys/ioctl.h>

#include "stats.h"
#include "workerThread.h"
#include "queue.h"
#include "HashTable.h"
#include "jobExecutor.h"

static int shutdownPending = 0;
struct timeval t1;
int flagForInitialize = 0;

void catchInterrupt(int signo) { // signal handler for SIGINT, SIGQUIT
    printf("**** Shutting down webCrawler... ****\n ");
    shutdownPending = 1;
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

    sprintf(buf, "%02d:%02d:%02d.%03d", hours, mins, secs, (int) totalTime);
    return buf;
}

int command(int active_fd, Stats * stats, Queue * queue, char * filePath, int terminalSize) { // service command port
    bool flag = false;
    char communicationBuf[BUF_SIZE];
    char commandBuf[BUF_SIZE];
    memset(communicationBuf, '\0', BUF_SIZE);
    memset(commandBuf, '\0', BUF_SIZE);
    printf("> Starting Command Handler...\n");
    int code = 1;

    int nread = myReadTerminalCommand(active_fd, communicationBuf, BUF_SIZE); // read commands from socket

    if (nread == BUF_SIZE) { // buffer overflow
        printf("> note: overflow, invalid command from client\n");
        close(active_fd);
        return -1;
    }

    for (int i = 0; i < strlen(communicationBuf); i++) {
        if (communicationBuf[i] == '\r' || communicationBuf[i] == '\n') {
            communicationBuf[i] = '\0';
            commandBuf[i] = '\0';
            break;
        } else {
            commandBuf[i] = communicationBuf[i];
            if(strcmp(commandBuf, "SEARCH ") == 0) flag = true;
            if(!flag) {
                communicationBuf[i] = toupper(communicationBuf[i]);
                commandBuf[i] = toupper(communicationBuf[i]); // capitalize string of command
            }
            else commandBuf[i] = communicationBuf[i];
        }
    }

    if (strcmp(communicationBuf, "STATS") == 0) { // if "STATS" => read stats from global vars
        memset(communicationBuf, '\0', BUF_SIZE);
        char * time = measureTime();

        pthread_mutex_lock(&stats->mtx); // lock mutex of stats
        unsigned long long int servedPages = stats->numOfPages;
        unsigned long long int servedBytes = stats->numOfBytes;
        pthread_mutex_unlock(&stats->mtx); // unlock mutex of stats

        sprintf(communicationBuf, "Crawler up for %s, downloaded %llu pages, %llu bytes.\n", time, servedPages, servedBytes);
        myWrite(active_fd, communicationBuf, strlen(communicationBuf)); // send statistics them to socket
        free(time);
    } else if (strcmp(communicationBuf, "SHUTDOWN") == 0) { // if "SHUTDOWN" => break loop in main => exit
        code = 0;
    } else{
        char * memory;
        char * tempBuf = calloc(strlen(commandBuf) + 1, sizeof(char));
        strcpy(tempBuf, commandBuf);
        char * firstWord = strtok_r(tempBuf, " ", &memory);
        if(strcmp(firstWord, "SEARCH") == 0){
            int remaining = getJobsRemaining(queue);        
            if (remaining > 0) { // if no crawling processes is finished => I'm so sorry, but ...
                printf("> Sorry, please try again later...\n");
            } else {
                printf("> We should be ready to go!!\n");
                char * token = NULL;
                int words = lineWords(commandBuf);
                WordQList * qlist = createWordQList(); // create Word List of Query
                addQList(qlist, "/search");

                unsigned int j = 0;
                for (j = 0; j < 10 && j < words - 1; j++) {
                    token = strtok_r(NULL, " \t", &memory);
                    addQList(qlist, token);
                }

                if(flagForInitialize == 0) initialize(NUM_OF_WORKERS, filePath, &flagForInitialize); // in first search => call initialize function
                searchWCommand(qlist, active_fd, terminalSize);
            }
        }else{
            memset(communicationBuf, '\0', BUF_SIZE);
            strcpy(communicationBuf, "Invallid Command!!\n");
            myWrite(active_fd, communicationBuf, strlen(communicationBuf)); //and send them to socket
        }
        free(tempBuf);     
    }
    printf("> Closing command connection.\n");
    shutdown(active_fd, SHUT_RDWR);
    close(active_fd); // close connection
    return code;
}

int main(int argc, char** argv) {
    int err;
    int serverPort = -1;
    int commandPort = -1;
    int numOfThreads;
    char * saveDir = NULL;
    char * endPtrStrtolCheck;
    char * startURL;
    char * hostOrIP;
    char * host;

    gettimeofday(&t1, NULL);
    struct sockaddr_in client = {0};
    struct sockaddr *clientptr = (struct sockaddr *) &client;
    struct hostent * rem;
    socklen_t clientlen = sizeof(client);

    static struct sigaction actSP;
    actSP.sa_handler = SIG_IGN;
    sigfillset(&(actSP.sa_mask));
    sigaction(SIGPIPE, &actSP, NULL);

    if (argc != 12) { // typical controls
        printf("> Use correct arguments!!\n");
        exit(EXIT_FAILURE);
    } else {
        for (int i = 0; i < argc; i++) {
            if ((i + 1 < argc) && (strcmp(argv[i], "-h") == 0)) {
                hostOrIP = argv[i + 1];
                host = hostOrIP;
                printf("> host_or_IP = %s\n", hostOrIP);
                
                // https://stackoverflow.com/questions/8289845/how-to-detect-your-ip-name-using-gethostbyname
                if(isValidIP(hostOrIP)){ // if is vallid IP
                    struct sockaddr_in sa;
                    char currentHost[BUF_SIZE];
                    int gni_err;
                    
                    sa.sin_family = AF_INET;
                    sa.sin_port = 0;
                    sa.sin_addr.s_addr = inet_addr(hostOrIP);

                    gni_err = getnameinfo((struct sockaddr *)&sa, sizeof sa, currentHost, sizeof currentHost, NULL, 0, NI_NAMEREQD | NI_NOFQDN);

                    if (gni_err == 0) {
                        host = currentHost; // find host name
                    } else {
                        fprintf(stderr, "Error looking up host: %s\n", gai_strerror(gni_err));
                        exit(EXIT_FAILURE);
                    }
                }else{
                    if ((rem = gethostbyname(hostOrIP)) == NULL) { // else hostOrIp deeming host
                        herror("gethostbyname");
                        exit(EXIT_FAILURE);
                    }
                }
            } else if ((i + 1 < argc) && (strcmp(argv[i], "-p") == 0)) {
                endPtrStrtolCheck = NULL;
                serverPort = strtol(argv[i + 1], &endPtrStrtolCheck, 10); // check if argument is incorect
                if (serverPort < 0 || argv[i + 1] == endPtrStrtolCheck) {
                    fprintf(stderr, "Incorrect port!!\n");
                    exit(EXIT_FAILURE);
                }
                printf("> port = %d\n", serverPort);
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
                saveDir = argv[i + 1];
                printf("save_dir = %s\n", saveDir); 
                purgeOrCreateDir(saveDir); // purge or create save_directory
            } else if (i == argc - 1) {
                startURL = argv[i];
                printf("> starting_URL = %s\n", startURL);
            }
        }
    }

    // https://stackoverflow.com/questions/1022957/getting-terminal-width-in-c 
    struct winsize terminalSize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminalSize); // get terminal size (= colums)

    int fd = open("docFile", O_WRONLY | O_TRUNC | O_APPEND); // purge + write + append in docFile
    if(errno == ENOENT) fd = open("docFile", O_WRONLY | O_CREAT | O_APPEND, 0644); // if no such file on directory => create docFile   

    Queue * queue = createQueue(numOfThreads); // create Queue (Buffer)
    Stats * stats = createStats(); // Create Statistics (Stats) Structure
    HashTable * hashTable = createHashTable(HASH_SIZE);
    InfoWList * ilist = malloc(sizeof(InfoWList));
    ilist->headIWL = NULL;
    InfoWNode * current = NULL;
    pthread_mutex_t * fileMtx = malloc(sizeof(pthread_mutex_t)); // mutex for write in docFile
    pthread_mutex_init(fileMtx, 0);

    struct stat fileStat = {0};
    if (stat("/", &fileStat) < 0) {
            perror("critical error, hdd access failed ");
            exit(EXIT_FAILURE);
    }
    int blockSize = fileStat.st_blksize; // find Disk Block Size
    
    char * urlClipboard1 = calloc(strlen(startURL) + 1, sizeof(char));
    strcpy(urlClipboard1, startURL);

    char * urlClipboard2 = calloc(strlen(startURL) + 1, sizeof(char));
    strcpy(urlClipboard2, startURL);

    if(strstr(startURL, "http:") != NULL){ // if starting_URL is "full" URL ...
        char * s = NULL;
        multiTok(urlClipboard1, &s, "//", true); // find from URL -> Host, Server Port & Required Page to Download
        char * hostByStartingURL = multiTok(NULL, &s, ":", true);
        char * portStr = multiTok(NULL, &s, "/", true);
        int portByStartingURL = atoi(portStr);
        char * temp = multiTok(NULL, &s, " ", true);
        char * page = calloc(strlen(temp) + 2, sizeof(char)); // add '/' before temp (= page) string
        sprintf(page, "/%s", temp); 

        if((portByStartingURL != serverPort) || (strcmp(hostByStartingURL, host) != 0))
        {
            printf("> Host or Port in Starting URL is invallid!!\n");
            exit(EXIT_FAILURE);
        }
        produceQueue(queue, urlClipboard2, hashTable);
        free(page);
    }else{ // if no "full" URL ...
        free(urlClipboard2);
        char * URL = calloc(URL_SIZE, sizeof(char));
        sprintf(URL, "http://%s:%d%s", host, serverPort, urlClipboard1); // create it
        produceQueue(queue, URL, hashTable); // + try to produce ib Queue (Buffer)
    }

    free(urlClipboard1);

    for (int i = 0; i < numOfThreads; i++) {
        if (ilist->headIWL == NULL) {
            ilist->headIWL = malloc(sizeof(InfoWNode));
            current = ilist->headIWL;
            current->fileMtx = fileMtx;
            current->tid = -1;
            current->fd = fd;
            current->blockSize = blockSize;
            current->next = NULL;
        } else {
            current->next = malloc(sizeof(InfoWNode));
            current = current->next;
            current->fileMtx = fileMtx;
            current->tid = -1;
            current->fd = fd;
            current->blockSize = blockSize;
            current->next = NULL;
        }
    }

    InfoWNode * curWork = ilist->headIWL;
    while (curWork != NULL) {
        curWork->queue = queue;
        curWork->saveDir = saveDir;
        curWork->stats = stats;
        curWork->hashTable = hashTable;

        if ((err = pthread_create(&curWork->tid, NULL, mainThread, (void *) curWork)) > 0) { // Create a thread
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        curWork = curWork->next;
    }

    static struct sigaction act; // handler for SIGINT
    act.sa_handler = catchInterrupt;
    sigfillset(&(act.sa_mask));
    sigaction(SIGINT, &act, NULL);

    int commandFd = createSocket(commandPort);
    int clientFd;

    while (shutdownPending == 0 && ((clientFd = accept(commandFd, clientptr, &clientlen)) > 0)) {
        int code;
        
        curWork = ilist->headIWL;
        while (curWork != NULL) { // restart thread process
            curWork->queue = queue;
            curWork->saveDir = saveDir;
            curWork->stats = stats;

            err = pthread_tryjoin_np(curWork->tid, NULL);

            if (err == 0) { // join successful, thus thread has died           
                if ((err = pthread_create(&curWork->tid, NULL, mainThread, (void *) curWork)) > 0) { // regenerate termited thread/s
                    perror("pthread_create");
                    exit(EXIT_FAILURE);
                }
            }
            curWork = curWork->next;
        }

        printf("> connection from commandFd\n");
        code = command(clientFd, stats, queue, "docFile", terminalSize.ws_col);

        if (code == 0) break; // received "SHUTDOWN" command in command port => break
    }

    if(flagForInitialize != 0){ // if received at least one "SEARCH" command
        WordQList * qlist1 = createWordQList();
        addQList(qlist1, "/exit");

        InfoPNode * currWork = plist->headP;
        while (currWork != NULL) {
            sendToPipe(qlist1, currWork->fd1); // send exit command to workers
            currWork = currWork->next;
        }

        PollArgsList * pal = createPollArgsList(); // collect response

        currWork = plist->headP;
        while (curWork != NULL) {
            addPollArgsList(pal, currWork->fd2);
            curWork = curWork->next;
        }

        int workerFd = 0;
        int countArgs = 0;

        while ((countArgs = countPollArgsList(pal)) > 0 && (workerFd = pollWMonitoring(pal)) > 0) {
            WordQList * responseData = receiveQList(workerFd);
            destroyWordQList(&responseData);
            removePollArgsList(pal, workerFd);                  
        }

        destroyWordQList(&qlist1);
        destroyPollArgsList(&pal);

        // wait workers
        currWork = plist->headP;
        while (currWork != NULL) {
            int status;
            wait(&status);
            currWork->status = status;
            currWork = currWork->next;
        }

        // close pipes
        currWork = plist->headP;
        while (currWork != NULL) {
            close(currWork->fd1);
            close(currWork->fd2);
            currWork = currWork->next;
        }

        // delete pipes
        currWork = plist->headP;
        while (currWork != NULL) {

            if (unlink(currWork->strTransmit) == -1) {
                printf("> pipe %s does not exist or could not be deleted.\n", currWork->strTransmit);
            } else {
                printf("> pipe %s deleted .\n", currWork->strTransmit);
            }
            if (unlink(currWork->strReceive) == -1) {
                printf("> pipe %s does not exist or could not be deleted.\n", currWork->strReceive);
            } else {
                printf("> pipe %s deleted .\n", currWork->strReceive);
            }
            currWork = currWork->next;
        }

        destroyInfoPList(&plist);
    }

    curWork = ilist->headIWL;
    while (curWork != NULL) {
        produceQueue(curWork->queue, NULL, NULL); // sent "signal" to the workers for smooth termation
        curWork = curWork->next;
    }

    // wait threads workers
    curWork = ilist->headIWL;
    while (curWork != NULL) {
        pthread_join(curWork->tid, NULL);

        printf("> Thread %lu finished\n", (unsigned long int) curWork->tid);
        curWork = curWork->next;
    }

    close(commandFd);
    close(fd);

    pthread_mutex_destroy(fileMtx);
    free(fileMtx);

    destroyInfoWList(&ilist);
    destroyQueue(&queue);
    destroyStats(&stats);
    destroyHashTable(&hashTable);
    printf("> GoodBye !!\n");

    return (EXIT_SUCCESS);
}
