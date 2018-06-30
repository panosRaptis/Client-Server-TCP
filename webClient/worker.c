#include "workersPInfo.h"
#include "worker.h"

int processDir(Trie * curTrie, const char * pathDir, int fd2) {

    DIR * dir;
    struct dirent * ent;
    FILE * fp = NULL;
    char filePath[BUF_SIZE];

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int counterLine;

    if ((dir = opendir(pathDir)) != NULL) { // open directory
        char * fullPathDir = getFullPath(pathDir);
        while ((ent = readdir(dir)) != NULL) { // for each file in directory ... 
            if ((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0)) {
                memset(filePath, ' ', BUF_SIZE);
                sprintf(filePath, "%s/%s", fullPathDir, ent->d_name);

                if ((fp = fopen(filePath, "r")) == NULL) { // open file (fileName)
                    perror("build_trie: fopen"); // fopen failed
                    exit(EXIT_FAILURE);
                }

                long int startLineOffset = ftell(fp);
                long int endLineOffset = 0;
                counterLine = 0;

                while ((read = getline(&line, &len, fp)) != -1) { // for each line in file
                    line[strcspn(line, "\n")] = '\0';
                    char * doc = calloc(strlen(line) + 1, sizeof(char));
                    strcpy(doc, line);

                    if(strcmp(doc, "<!DOCTYPE html>") != 0 && strcmp(doc, "<html>") != 0 && strcmp(doc, "</html>") != 0 && strcmp(doc, "\t<body>") != 0 && strcmp(doc, "\t</body>") != 0){
                        endLineOffset = ftell(fp);
                        curTrie->numLines++;
                        curTrie->numChars += strlen(doc);
                       
                        char * memory;
                        char * s = NULL;
                        char * firstPart = multiTok(line, &s, "<a href=\"", true);
                        
                        char * temp = calloc(strlen(firstPart) + 1, sizeof(char));
                        strcpy(temp, firstPart);

                        int words = lineWords(firstPart);
                        char * word;
                        
                        for(int j = 0; j < words; j++){
                            if(j == 0) word = strtok_r(temp, "\t ", &memory);
                            else word = strtok_r(NULL, "\t ", &memory);
                            if(strcmp(word, "\t") != 0 || word != NULL){
                                curTrie->numWords++;
                                addTrie(&curTrie, counterLine, word, filePath, startLineOffset); // add word (token) in Trie of Workerj, 1<=j<=N
                            }
                        }

                        free(temp);
                        temp = NULL;

                        counterLine++;
                        startLineOffset = endLineOffset;
                    }
                    free(doc);
                    doc = NULL;

                    if(strcmp(line, "</html>") == 0) break;

                    free(line);
                    line = NULL;

                }
                counterLine += -1;
                free(line);
                line = NULL;

                fclose(fp);
                fp = NULL;
            }
        }
        closedir(dir);
        free(fullPathDir);
        return 1;
    } else if (errno == ENOENT) {
        // does not exist this directory
        // with handler show an error message
        // go open next directory, normaly

        return -1;
    } else {
        perror("Error: opendir");
        exit(EXIT_FAILURE);
    }
}

void network(Trie * curTrie, int fd1, int fd2) {
    char * token;

    srand(getpid());
    printf("> Worker %d up, Network mode:\n", (int) getpid());

    while (1) { //reading from input/CLI
        WordQList * rlist = receiveQList(fd1);
        char * buf = serializeQList(rlist);

        buf[strcspn(buf, "\n")] = '\0';
        int words = lineWords(buf);
        token = strtok(buf, " \t"); // get the first word of the current line on CLI
        if (strcmp(token, "/search") == 0) { // insert function -> search
            WordQList * qlist = createWordQList(); // create Word List of Query
            int j;
            for (j = 0; j < 10 && j < words - 1; j++) {
                token = strtok(NULL, " \t");
                addQList(qlist, token);
            }
            FileQList * l = searchCommand(curTrie, qlist);
            sendToPipeFileQList(l, fd2);
            destroyWordQList(&qlist); // free qlist (WordQueryList)
            destroyFileQList(&l);
        } else if (strcmp(token, "/exit") == 0) // insert function -> exit
        {   WordQList * msg = createWordQList();
            addQList(msg, "OK");

            sendToPipe(msg, fd2);
            destroyWordQList(&msg);
            destroyWordQList(&rlist);
            free(buf);
            buf = NULL;
            break; // break while loop => destroyMap => destroyTrie => return 0;
        }
        destroyWordQList(&rlist);
        free(buf);
        buf = NULL;
    }
}

int mainWorker(char * strTransmit, char * strReceive) {
    const char * pathDir;

    Trie * curTrie = NULL;
    int sucOpenDir = 0;
    curTrie = (Trie *) malloc(sizeof(Trie));
    curTrie->headTrie = NULL;
    curTrie->numWords = 0;
    curTrie->numChars = 0;
    curTrie->numLines = 0;

    if (strTransmit != NULL && strReceive != NULL) {
        int fd1, fd2;

        if ((fd1 = open(strTransmit, O_RDONLY)) == -1) {
            printf("> pipe %s could not be opened by worker.\n", strTransmit);
            exit(EXIT_FAILURE);
        }
        if ((fd2 = open(strReceive, O_WRONLY)) == -1) {
            printf("> pipe %s could no be opened by worker.\n", strReceive);
            exit(EXIT_FAILURE);
        }

        // receive pathdir from pipe
        WordQList * listOfDirs = readFromPipe(fd1);
        WordQNode * tempNode = listOfDirs->headWQL->next;
        WordQList * codeList = createWordQList();
        while(tempNode != NULL){
            pathDir = tempNode->word;
            sucOpenDir = processDir(curTrie, tempNode->word, fd2);
            if (sucOpenDir == 1) printf("> Worker with PID: %d loaded path: %s \n", getpid(), pathDir);
            else {
                if(codeList->headWQL->next == NULL) addQList(codeList, "2"); // notFound a dir => send code 2 & dirs notFound
                addQList(codeList, tempNode->word);
            }
            tempNode = tempNode->next;
        }
        destroyWordQList(&listOfDirs);        
        
        if (codeList->headWQL->next == NULL){ // ack for succsess open all paths -> return 1 to father process
            addQList(codeList, "1");
        }
        sendToPipe(codeList, fd2);
        destroyWordQList(&codeList);
    
        network(curTrie, fd1, fd2);
        close(fd1);
        close(fd2);
    } 

    destroyTrie(&curTrie); // free Trie
    destroyInfoPList(&plist); // free Inwo Workers List (no memory leaks)

    printf("> Worker destroyed with PID: %d \n", getpid());

    return 0;
}