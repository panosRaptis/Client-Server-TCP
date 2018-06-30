#include "workerThread.h"

char * GETRequest(const char * host, const char * page) { // string for GET Request
    char * buf = calloc(BUF_SIZE, sizeof(char));
    sprintf(buf, "GET %s HTTP/1.1\r\n"
            "User-Agent: Mozilla/4.0\r\n"
            "Host: %s\r\n"
            "Accept-Language: en-us\r\n"
            "Accept-Encoding: gzip, deflate\r\n"
            "Connection: Keep-Alive\r\n\r\n", page, host);
    return buf;
}

void findLinksInFile(const char * filePath, Queue * queue, HashTable * hashTable, const char * host, int serverPort) { // find alls URL in file with filePath, 
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * s = NULL;
    FILE * fp = NULL;

    if ((fp = fopen(filePath, "r")) == NULL) { // open file (filePath)
        perror("find_links: fopen"); // fopen failed
        return;
    }

    while ((read = getline(&line, &len, fp)) > 0) { // read line by line
        line[strcspn(line, "\n")] = '\0';
        char * firstPart = multiTok(line, &s, "<a href=\"", false);
        if (firstPart != NULL) {
            char * page = multiTok(NULL, &s, "\"", true); // for each page
            char * URL = calloc(URL_SIZE, sizeof(char));
            sprintf(URL, "http://%s:%d%s", host, serverPort, page); // create full URL
            produceQueue(queue, URL, hashTable); // if not already exists, put in Queue and in Histroy structure
        }
        free(line);
        line = NULL;
    }

    if (line != NULL) {
        free(line);
        line = NULL;
    }

    fclose(fp);
    fp = NULL;
}

char * getCodeRequest(char str[ ]) { // get HTTP Code for GET Request
    char * request = calloc(strlen(str) + 1, sizeof(char));
    strcpy(request, str);
    char * memory;
    strtok_r(request, " ", &memory);
    char * code = strtok_r(NULL, " ", &memory);
    char * res = calloc(strlen(code) + 1, sizeof(char));
    strcpy(res, code);
    free(request);
    return res;
}

long int getSizeFile(char str[ ]) { // get value of Content-Length Field
    char * request = calloc(strlen(str) + 1, sizeof(char));
    strcpy(request, str);
    long int result = -1;
    char * s = NULL;
    char * line = NULL;
    for (line = multiTok(request, &s, "\r\n", true); line != NULL; line = multiTok(NULL, &s, "\r\n", true)) {
        char * memory;
        if (strcmp(line, "") != 0) {
            char * lineCPY = calloc(strlen(line) + 1, sizeof(char));
            strcpy(lineCPY, line);
            char * firstWord = strtok_r(lineCPY, " ", &memory);
            if (strcmp(firstWord, "Content-Length:") == 0) {
                char * sizeStr = strtok_r(NULL, " ", &memory);
                result = atoi(sizeStr);
                free(lineCPY);
                break;
            }
            free(lineCPY);
        } else break;
    }
    free(request);
    return result;
}

void service(char * url, const char * saveDir, pthread_mutex_t * fileMtx, Stats * stats, Queue * queue, HashTable * hashTable, int docfileFd, int blockSize) {
    char header[HTTP_BUF_SIZE];
    memset(header, '\0', HTTP_BUF_SIZE);
    char * memory;
    char * urlClipboard = calloc(strlen(url) + 1, sizeof(char));
    strcpy(urlClipboard, url);

    char * s = NULL;
    multiTok(urlClipboard, &s, "//", true); // find from URL -> Host, Server Port & Required Page to Download
    char * host = multiTok(NULL, &s, ":", true);
    char * portStr = multiTok(NULL, &s, "/", true);
    int serverPort = atoi(portStr);
    char * temp = multiTok(NULL, &s, " ", true);
    char * page = calloc(strlen(temp) + 2, sizeof(char)); // add '/' before temp (= page) string
    sprintf(page, "/%s", temp);

    char * conGETReq = GETRequest(host, page); // construct GET request

    int sock = createSocketWithServer(host, serverPort);

    if (sock == -1) { // goto to try consume again an other URL from Queue (Buffer)
        free(conGETReq);
        free(urlClipboard);
        free(page);
        return;
    }

    myWrite(sock, conGETReq, strlen(conGETReq)); // send GET request (writeall)

    int headerSize = myReadRequest(sock, header, HTTP_BUF_SIZE); // received result
    int firstTimeReadSize = strlen(header);
    char * tempBuf = calloc(firstTimeReadSize - headerSize + 2, sizeof(char));
    int k;
    int c = 0;
    
    for(k = headerSize + 1; k < firstTimeReadSize; k++){
        tempBuf[c] = header[k];
        c++;
    }
    header[headerSize] = '\0';

    char * codeReq = getCodeRequest(header);

    if (strcmp(codeReq, "200") == 0) {
        long int sizeFile = getSizeFile(header);
        if (sizeFile == -1) { // error handling ...
            printf("> Failed Read HTTP Request Header...\n");
            free(conGETReq);
            free(codeReq);
            free(urlClipboard);
            free(page);
            free(tempBuf);
            close(sock); // close connection
            return; // + return
        }

        char * filePath = calloc(strlen(saveDir) + strlen(page) + 4, sizeof(char));

        char * pageCPY = calloc(strlen (page) + 1, sizeof(char));
        strcpy(pageCPY, page);

        char * folderPath = strtok_r(pageCPY, "/", &memory);

        if (saveDir[strlen(saveDir) - 1] != '/') sprintf(filePath, "%s/%s", saveDir, folderPath);
        else sprintf(filePath, "%s%s", saveDir, folderPath);

        struct stat fileStat = {0};
        char * tempDirPath = calloc(strlen(filePath) + 2, sizeof(char));
        sprintf(tempDirPath, "%s\n", filePath);

        pthread_mutex_lock(fileMtx); // lock mutex check folders/write docfile

        if (stat(filePath, &fileStat) == -1) { // check if Sitej forder exist
            mkdir(filePath, 0755); // if no, create folder in save_dir
            myWrite(docfileFd, tempDirPath, strlen(tempDirPath)); // write in docfile, path of folder Sitej
        }
        
        pthread_mutex_unlock(fileMtx); // unlock this mutex

        free(tempDirPath);

        char * finalHTMLFilePath = strtok_r(NULL, "/", &memory);
        char buf[TEMP_BUF_SIZE];
        memset(buf, '\0', TEMP_BUF_SIZE);
        sprintf(buf, "/%s", finalHTMLFilePath);
        strcat(filePath, buf);
        int siteFd = open(filePath, O_CREAT | O_WRONLY | O_APPEND, 0644);

        // download file and store to directory (block by block with myRead/myWrite)
        myWrite(siteFd, tempBuf, strlen(tempBuf));

        char * blockData = (char *) malloc(blockSize);
        sizeFile = sizeFile - (firstTimeReadSize - headerSize - 1);
        int loops = sizeFile / blockSize;
        int lastBlockSize = sizeFile % blockSize;

        for (int i = 0; i < loops; i++) { // read file block by block
            myRead(sock, blockData, blockSize);
            myWrite(siteFd, blockData, blockSize);
        }

        myRead(sock, blockData, lastBlockSize); // read file block by block  (beware of last block incomplete)
        myWrite(siteFd, blockData, lastBlockSize);

        close(siteFd);

        pthread_mutex_lock(&(stats->mtx)); // lock mutex of stats  
        stats->numOfPages++;
        stats->numOfBytes += (sizeFile + (firstTimeReadSize - headerSize - 1));
        pthread_mutex_unlock(&(stats->mtx)); // unlock mutex of stats

        findLinksInFile(filePath, queue, hashTable, host, serverPort);
        free(pageCPY);
        free(blockData);
        free(filePath);
    } else if (strcmp(codeReq, "404") == 0) { // received 404 Not Found HTTP Code
        printf("> Closing connection - 404 HTTP Code!!\n");
    } else if (strcmp(codeReq, "403") == 0) { // received 403 Forbidden HTTP Code
        printf("> Closing connection - 403 HTTP Code!!\n");
    } else if (strcmp(codeReq, "400") == 0) { // received 400 Bad Request HTTP Code
        printf("> Closing connection - 400 HTTP Code!!\n");
    }

    free(tempBuf);
    tempBuf = NULL;

    free(conGETReq);
    conGETReq = NULL;

    free(codeReq);
    codeReq = NULL;

    free(urlClipboard);
    urlClipboard = NULL;

    free(page);
    page = NULL;

    close(sock); // close connection
}

void * mainThread(void * args) {
    InfoWNode * curWork = (InfoWNode *) args;
    pthread_mutex_t * fileMtx = curWork->fileMtx;
    Queue * queue = curWork->queue;
    Stats * stats = curWork->stats;
    HashTable * hashTable = curWork->hashTable;
    const char * saveDir = curWork->saveDir;
    int fd = curWork->fd;
    int blockSize = curWork->blockSize;

    while (1) {
        char * url = consumeQueue(queue);

        if (url == NULL) {
            break;
        }
        
        service(url, saveDir, fileMtx, stats, queue, hashTable, fd, blockSize);
        
        free(url);
    }
    printf("> WorkerThread destroyed with TID: %d,%lu \n", getpid(), (unsigned long int) pthread_self());
    return 0;
}