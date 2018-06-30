#ifndef INET_STR_SERVER_H
#define INET_STR_SERVER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  
#include <netdb.h>      
#include <unistd.h>    
#include <stdlib.h>    
#include <ctype.h> 

#include "constants.h"

int createSocket(int port);

#endif /* INET_STR_SERVER_H */