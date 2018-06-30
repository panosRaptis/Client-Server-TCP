#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "WordQList.h"

ssize_t myWrite(int fd, const void *buf, size_t nbyte);
    
ssize_t myRead(int fd, const void *buf, size_t nbyte);

ssize_t myReadTerminalCommand(int fd, const void *buf, size_t nbyte);

ssize_t myReadRequest(int fd, const void *buf, size_t nbyte);

void sendCode(int fd, char c);

char receiveCode(int fd);

void sendToPipe(WordQList * list, int fd);

WordQList * readFromPipe(int fd);

#endif /* COMMUNICATION_H */