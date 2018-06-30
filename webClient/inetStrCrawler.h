#ifndef INETSTRCRAWLER_H
#define INETSTRCRAWLER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  
#include <netdb.h>      
#include <unistd.h>    
#include <stdlib.h>    
#include <ctype.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "constants.h"

int createSocket(int port);   

int createSocketWithServer(char * host, int port);

#endif /* INETSTRCRAWLER_H */