#include "communication.h"

// Advanced UNIX Programming (2nd Edition), Marc J. Rochkind, pag. 117 (writeall)
ssize_t myWrite(int fd, const void *buf, size_t nbyte) {
    ssize_t nwritten = 0;
    ssize_t n;

    do {
        if ((n = write(fd, &((const char *) buf) [nwritten], nbyte - nwritten)) == -1) {
            if (errno == EINTR) continue;
            else return -1;
        }
        nwritten += n;
    } while (nwritten < nbyte);
    return nwritten;
}

// Advanced UNIX Programming (2nd Edition), Marc J. Rochkind, pag. 118-119 (readall)
ssize_t myRead(int fd, const void *buf, size_t nbyte) {
    ssize_t nread = 0;
    ssize_t n;

    do {
        if ((n = read(fd, &((char *) buf) [nread], nbyte - nread)) == -1) {
            if (errno == EINTR) continue;
            else return -1;
        }
        if (n == 0) return nread;
        nread += n;
    } while (nread < nbyte);
    return nread;
}

ssize_t myReadTerminalCommand(int fd, const void *buf, size_t nbyte) { // read commands in command port
    ssize_t nread = 0;
    ssize_t n;

    do {
        if ((n = read(fd, &((char *) buf) [nread], 1)) == -1) {
            if (errno == EINTR) continue;
            else return -1;
        }
        if (n == 0) return nread;

        if (((char *) buf) [nread] == '\r') {
            return nread;
        }

        if (((char *) buf) [nread] == '\n') {
            return nread;
        }
        nread += n;
    } while (nread < nbyte);
    return nread;
}

ssize_t myReadRequest(int fd, const void *buf, size_t nbyte) { // this function is based all on myRead and it is 
                                                               // slightly modified in order to read 30-30 bytes from service socket
    ssize_t nread = 0;
    ssize_t n;

    do {
        if ((n = read(fd, &((char *) buf) [nread], BUF_SIZE_READ_REQUEST)) == -1) {
            if (errno == EINTR) continue;
            else return -1;
        }
        if (n == 0) return nread;

        nread += n;

        if (nread > 3) {
            if (((char *) buf) [nread - 1] == '\n') {
                if (((char *) buf) [nread - 2] == '\r') {
                    if (((char *) buf) [nread - 3] == '\n') {
                        if (((char *) buf) [nread - 4] == '\r') {
                            return nread;
                        }
                    }
                }
            }
        }
    } while (nread < nbyte);
    return nread;
}

int sendQList(WordQList * list, int senderFd) { // sent list of chars * into the named-pipe (following the communication protocol), use myWrite
    WordQNode * head = NULL;
    if (list == NULL) {
        return 0;
    } else {
        head = list->headWQL;
        if (head == NULL || head->next == NULL) {
            return 0;
        }
    }

    int tokens = countQList(list);
    WordQNode * current = head->next;

    myWrite(senderFd, &tokens, sizeof(tokens));

    while (current != NULL) {
        int length = strlen(current->word);
        myWrite(senderFd, &length, sizeof(length));
        myWrite(senderFd, current->word, (length));
        current = current->next;
    }

    return 0;
}

WordQList * receiveQList(int receiverFd) { // get list of chars * from the named-pipe (following the communication protocol), use myRead
    WordQList * list = createWordQList();

    int tokens = 0;
    if (myRead(receiverFd, &tokens, sizeof(tokens)) <= 0) {
        return NULL;
    }

    for (int i = 0; i < tokens; i++) {
        int length;
        myRead(receiverFd, &length, sizeof(length));
        char * word = calloc(length + 1, sizeof(char));
        myRead(receiverFd, word, (length));

        addQList(list, word);
        free(word);
    }

    return list;
}

void sendToPipe(WordQList * qlist, int fd) {
    sendQList(qlist, fd);
}

WordQList * readFromPipe(int fd) {
    return receiveQList(fd);
}