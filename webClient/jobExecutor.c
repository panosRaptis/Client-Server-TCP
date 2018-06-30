#include "jobExecutor.h"

void initialize(int w, char * fileName, int * flagForInitialize){
    *flagForInitialize = 1;
    FILE * filePointer = NULL;
    InfoPNode * currWork = NULL;

    static struct sigaction action_ignore;
    action_ignore.sa_handler = SIG_IGN;
    sigfillset(&(action_ignore.sa_mask));

    if ((filePointer = fopen(fileName, "r")) == NULL) { // open file (fileName)
        perror("build docfile: fopen"); // fopen failed
        exit(EXIT_FAILURE);
    }

    int numOfDirs = numLines(filePointer);
    if (numOfDirs < w) {
        w = numOfDirs;
    }

    plist = malloc(sizeof(InfoPList));
    plist->headP = NULL;

    int dirsPerWorker = numOfDirs / w;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;
    int countw = 0;
    InfoPNode * current = NULL;
    int randomWorker;

    if (numOfDirs % w > 0) {
        long seed = time(NULL);
        srand((unsigned int) seed);
    }

    rewind(filePointer);

    while ((read = getline(&line, &len, filePointer)) != -1) // read line by line docfile
    {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) != 0) {
            if (count < dirsPerWorker) {
                if (plist->headP == NULL) { // create 1st Node of InfoWorkersList
                    plist->headP = malloc(sizeof(InfoPNode));
                    current = plist->headP;
                    current->pid = -1;
                    current->fd1 = 0;
                    current->fd2 = 0;
                    current->status = 0;
                    current->strTransmit = NULL;
                    current->strReceive = NULL;
                    current->next = NULL;
                    current->paths = malloc(sizeof(Paths));
                    current->numOfPaths = 1;
                    current->paths->str = calloc(strlen(line) + 1, sizeof(char));
                    strcpy(current->paths->str, line);
                    current->paths->next = NULL;
                    count++;
                    countw++;
                } else {
                    addPath(current->paths, line); // add Path in a Info Node (worker)
                    current->numOfPaths++;
                    count++;
                }
            } else {
                if (countw == w && numOfDirs % w > 0) { // the remaining dirs are randomly distributed
                    randomWorker = rand() % w;
                    InfoPNode * temp = GetPNth(plist, randomWorker); // get a work randomly
                    temp->numOfPaths++;
                    addPath(temp->paths, line);
                } else if (countw < w) { // add new Info NOde (worker)
                    count = 0;
                    countw++;
                    current->next = malloc(sizeof(InfoPNode));
                    current = current->next;
                    current->pid = -1;
                    current->fd1 = 0;
                    current->fd2 = 0;
                    current->status = 0;
                    current->strTransmit = NULL;
                    current->strReceive = NULL;
                    current->next = NULL;
                    current->paths = malloc(sizeof(Paths));
                    current->numOfPaths++;
                    current->paths->str = calloc(strlen(line) + 1, sizeof(char));
                    strcpy(current->paths->str, line);
                    current->paths->next = NULL;
                    count++;
                }
            }
        }
        free(line);
        line = NULL;
    }
    free(line);
    line = NULL;

    fclose(filePointer);
    filePointer = NULL;

    int workerId = 0;
    currWork = plist->headP;
    while (currWork != NULL) {
        char * strTransmit = calloc(100, sizeof(char)); // create pipes
        char * strReceive = calloc(100, sizeof(char));
        snprintf(strTransmit, 100, "transmit%d", workerId);
        snprintf(strReceive, 100, "receive%d", workerId);

        currWork->strTransmit = calloc(strlen(strTransmit) + 1, sizeof(char));
        strcpy(currWork->strTransmit, strTransmit);

        currWork->strReceive = calloc(strlen(strReceive) + 1, sizeof(char));
        strcpy(currWork->strReceive, strReceive);

        if (mkfifo(strTransmit, 0644) == -1) {
            printf("> pipe %s already exists .\n", strTransmit);
        } else {
            printf("> pipe %s created .\n", strTransmit);
        }
        if (mkfifo(strReceive, 0644) == -1) {
            printf("> pipe %s already exists .\n", strReceive);
        } else {
            printf("> pipe %s created.\n", strReceive);
        }
        free(strTransmit);
        free(strReceive);

        workerId++;
        currWork = currWork->next;
    }

    // create workers
    currWork = plist->headP;
    pid_t childpid;
    while (currWork != NULL) {
        childpid = fork();

        if (childpid < 0) {
            printf("Out of memory error\n");
            exit(EXIT_FAILURE);
        }
        if (childpid == 0) {
            sigaction(SIGINT, &action_ignore, NULL);
            sigaction(SIGQUIT, &action_ignore, NULL);
            mainWorker(currWork->strTransmit, currWork->strReceive);
            exit(EXIT_SUCCESS);
        }

        currWork->pid = childpid;
        currWork = currWork->next;
    }

    // open pipes
    currWork = plist->headP;
    while (currWork != NULL) {
        if ((currWork->fd1 = open(currWork->strTransmit, O_WRONLY)) == -1) {
            printf("pipe %s could no be opened.\n", currWork->strTransmit);
            exit(EXIT_FAILURE);
        }
        if ((currWork->fd2 = open(currWork->strReceive, O_RDONLY)) == -1) {
            printf("pipe %s could no be opened.\n", currWork->strReceive);
            exit(EXIT_FAILURE);
        }
        currWork = currWork->next;
    }

    // send paths to workers
    currWork = plist->headP;
    WordQList * tempQList;
    Paths * currPath;
    while (currWork != NULL) {
        tempQList = createWordQList();
        currPath = currWork->paths;
        while (currPath != NULL) {
            addQList(tempQList, currPath->str);
            currPath = currPath->next;
        }
        sendToPipe(tempQList, currWork->fd1);
        destroyWordQList(&tempQList);
        currWork = currWork->next;
    }

    currWork = plist->headP;
    WordQList * codeList = NULL;
    WordQNode * codeNode = NULL;
    while (currWork != NULL) {
        printf("> Waiting for worker %d to become ready ... \n", currWork->pid);
        codeList = receiveQList(currWork->fd2);
        if (strcmp(codeList->headWQL->next->word, "2") == 0) {
            codeNode = codeList->headWQL->next->next;
            while (codeNode != NULL) {
                printf("> Worker %d not found Path: %s\n", currWork->pid, codeNode->word);
                codeNode = codeNode->next;
            }
        }
        destroyWordQList(&codeList);
        currWork = currWork->next;
    }
}

void searchWCommand(WordQList * qlist, int activeFd, int terminalSize){   
    InfoPNode * currWork = plist->headP;
    int count = 1;
    while (currWork != NULL) {
        sendToPipe(qlist, currWork->fd1); // send search command to workers
        currWork = currWork->next;
    }

    PollArgsList * pal = createPollArgsList(); // collect response

    currWork = plist->headP;
    while (currWork != NULL) {
        addPollArgsList(pal, currWork->fd2);
        currWork = currWork->next;
    }

    int workerFd = 0;
    char * path = NULL;
    int idLine = 0;
    char * strLine = NULL;
    bool flagPrint = false;

    while (countPollArgsList(pal) > 0 && (workerFd = pollWMonitoring(pal)) > 0) {
        WordQList * responseData = receiveQList(workerFd);
        WordQNode * n = responseData->headWQL;
        n = n->next;
        path = n->word;
        if (strcmp(path, "0") != 0) {
            if (!flagPrint) flagPrint = true; 
            n = n->next;
            idLine = atoi(n->word);
            n = n->next;
            strLine = n->word;
            prettyPrint(path, idLine, strLine, terminalSize, qlist, count, activeFd, false);
            count++;
            if (n->next != NULL) {
                if (strcmp(n->next->word, "END") == 0) {
                    destroyWordQList(&responseData);
                    removePollArgsList(pal, workerFd);
                    continue;
                }
            }
        } else {
            if(!flagPrint) {
                myWrite(activeFd, "> Word Not Found !!\n", strlen("> Word Not Found !!\n"));
                flagPrint = true;
            }
            destroyWordQList(&responseData);
            removePollArgsList(pal, workerFd);
            continue;
        }

        while (1) {
            WordQList * responseData2 = receiveQList(workerFd);
            WordQNode * n = responseData2->headWQL;

            n = n->next;
            path = n->word;

            if (!flagPrint) flagPrint = true;
            n = n->next;
            idLine = atoi(n->word);

            n = n->next;
            strLine = n->word;
            prettyPrint(path, idLine, strLine, terminalSize, qlist, count, activeFd, false);
            count++;
                    
            if (n->next != NULL) {
                if (strcmp(n->next->word, "END") == 0){
                    destroyWordQList(&responseData2);
                    break;
                }
            }
            destroyWordQList(&responseData2);
        }        
        removePollArgsList(pal, workerFd);
        destroyWordQList(&responseData);
    }

    if (!flagPrint) myWrite(activeFd, "> Word Not Found !!\n", strlen("> Word Not Found !!\n"));
    destroyWordQList(&qlist); // free qlist (WordQueryList)
    destroyPollArgsList(&pal); // free pal (PollArgsList)
}
