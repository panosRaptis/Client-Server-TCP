#include "worker.h"

// https://stackoverflow.com/questions/20619236/how-to-get-utc-time
char * dateHTTP(pthread_mutex_t * dateMtx) { // create string for Date Field in HTTP Request
    char * buf = calloc(DATE_BUF_SIZE, sizeof(char));
    time_t now = time(0); 
    pthread_mutex_lock(dateMtx); // lock here
    struct tm tm = *gmtime(&now); // because "gmtime" is non thread safe function :(
    pthread_mutex_unlock(dateMtx); // unclock here
    strftime(buf, DATE_BUF_SIZE, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return buf;
}

char * HTTPCode_200(int contentLength, pthread_mutex_t * dateMtx) { // header for HTTP Code 200 (OK)
    char * buf = calloc(BUF_SIZE, sizeof(char));
    char * dateStr = dateHTTP(dateMtx);
    sprintf(buf, "HTTP/1.1 200 OK\r\n"
            "Date: %s\r\n"
            "Server: myhttpd/1.0.0 (Ubuntu 64)\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: text/html\r\n"
            "Connection: Closed\r\n\r\n", dateStr, contentLength);
    free(dateStr);
    return buf;
}

char * HTTPCode_400(pthread_mutex_t * dateMtx) { // Bad Request
    char * buf = calloc(BUF_SIZE, sizeof(char));
    char * dateStr = dateHTTP(dateMtx);
    sprintf(buf, "HTTP/1.1 400 Bad Request\r\n"
            "Date: %s\r\n"
            "Server: myhttpd/1.0.0 (Ubuntu 64)\r\n"
            "Content-Length: 92\r\n"
            "Content-Type: text/html\r\n"
            "Connection: Closed\r\n\r\n"
            "<html>Bad Request. Your browser sent a request that this server could not understand.</html>", dateStr);
    free(dateStr);
    return buf;
}

char * HTTPCode_404(pthread_mutex_t * dateMtx) { // Not Found
    char * buf = calloc(BUF_SIZE, sizeof(char));
    char * dateStr = dateHTTP(dateMtx);
    sprintf(buf, "HTTP/1.1 404 Not Found\r\n"
            "Date: %s\r\n"
            "Server: myhttpd/1.0.0 (Ubuntu 64)\r\n"
            "Content-Length: 49\r\n"
            "Content-Type: text/html\r\n"
            "Connection: Closed\r\n\r\n"
            "<html>Sorry dude, couldn't find this file.</html>", dateStr);
    free(dateStr);
    return buf;
}

char * HTTPCode_403(pthread_mutex_t * dateMtx) { // Forbidden
    char * buf = calloc(BUF_SIZE, sizeof(char));
    char * dateStr = dateHTTP(dateMtx);
    sprintf(buf, "HTTP/1.1 403 Forbidden\r\n"
            "Date: %s\r\n"
            "Server: myhttpd/1.0.0 (Ubuntu 64)\r\n"
            "Content-Length: 70\r\n"
            "Content-Type: text/html\r\n"
            "Connection: Closed\r\n\r\n"
            "<html>Trying to access this file but don't think I can make it.</html>", dateStr);
    free(dateStr);
    return buf;
}

void service(int sock, const char * rootDir, pthread_mutex_t * dateMtx, Stats * stats) {
    char header[HTTP_BUF_SIZE] = {0};
    char * memory;
    printf("> Starting service handler \n");

    myReadRequest(sock, header, HTTP_BUF_SIZE); // read 30-30 bytes from socket

    if (!isValidGETRequest(header)) { // validation of received HTTP Request
        char * msg = HTTPCode_400(dateMtx); // send 400 HTTP Code
        myWrite(sock, msg, strlen(msg));
        free(msg);
        printf("> Closing service connection.\n");
        close(sock);
        return; // + return
    }

    char * tempBuf = calloc(strlen(header) + 1, sizeof(char));
    strcpy(tempBuf, header);
    strtok_r(tempBuf, " ", &memory);
    char * URL = strtok_r(NULL, " ", &memory); // parse URL

    char * requestedPath = calloc(strlen(rootDir) + strlen(URL) + 2, sizeof(char));
    
    if(rootDir[strlen(rootDir) - 1] == '/'){
        char buf[BUF_SIZE];
        memset(buf, '\0', BUF_SIZE);
        strncpy(buf, rootDir, strlen(rootDir));
        buf[strlen(buf) - 1] = '\0'; 
        sprintf(requestedPath, "%s%s", buf, URL);
    }else sprintf(requestedPath, "%s%s", rootDir, URL);
    
    // check if file exists 
    int fd = open(requestedPath, O_RDONLY); // try to open file
    if (fd < 0){
        if (errno == EACCES){ // Permission denied
            char * msg = HTTPCode_403(dateMtx); // send 403 HTTP Code
            myWrite(sock, msg, strlen(msg));
            free(msg);
        } else if(errno == ENOENT){ // No such file or directory 
            char * msg = HTTPCode_404(dateMtx); // send 404 HTTP Code
            myWrite(sock, msg, strlen(msg));
            free(msg);
        }
    } else { // open succ this file
        struct stat fileStat; // error handling... on error 403
        int res = fstat(fd, &fileStat);

        if(res < 0){
            char * msg = HTTPCode_403(dateMtx); // send 403 HTTP Code
            myWrite(sock, msg, strlen(msg));
            free(msg);
            free(requestedPath);
            close(sock);
            return;
        }

        long int size = fileStat.st_size;
        int blockSize = fileStat.st_blksize;

        char * blockData = (char *) malloc(blockSize);
        int loops = size / blockSize;
        int lastBlockSize = size % blockSize;

        char * headerHTTPResponse = HTTPCode_200(size, dateMtx);
        myWrite(sock, headerHTTPResponse, strlen(headerHTTPResponse));

        for (int i = 0; i < loops; i++) { // read file block by block
            myRead(fd, blockData, blockSize);
            myWrite(sock, blockData, blockSize);
        }

        myRead(fd, blockData, lastBlockSize); // send file block by block  (beware of last block incomplete)
        myWrite(sock, blockData, lastBlockSize);

        pthread_mutex_lock(&(stats->mtx)); // lock mutex of stats  
        stats->numOfPages++;
        stats->numOfBytes += size;
        pthread_mutex_unlock(&(stats->mtx)); // unlock mutex of stats

        free(headerHTTPResponse);
        close(fd);
        free(blockData);
    }
    free(tempBuf);
    free(requestedPath);
    printf("> Closing service connection.\n");
    //shutdown(sock, SHUT_RDWR);
    close(sock);
}

void * mainWorker(void * args) {
    InfoWNode * curWork = (InfoWNode *) args;
    pthread_mutex_t * dateMtx = curWork->dateMtx;
    Stats * stats = curWork->stats;
    const char * rootDir = curWork->rootdir;

    while (1) {
        int clientFd = consumeQueue(curWork->queue); // consume for Queue (Buffer)

        if (clientFd == 0) { // termited worker thread
            break;
        }        
        
        service(clientFd, rootDir, dateMtx, stats); // service command
    }

    printf("> Worker destroyed with PID: %d,%lu \n", getpid(), (unsigned long int) pthread_self());

    int * status = malloc(sizeof(int));
    *status = 0;
    return status;
}