// Course Slides - Topic 5 (page 45 - 46)

#include "inetStrCrawler.h"    

int createSocket(int port) { // create socket for receiving msg from command port
    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *) &server;

    int enable = 1;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (bind(sock, serverptr, sizeof(server)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock, CONNECTIONS_BUF_SIZE) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("> Setup command port at %d completed \n", port);

    return sock;
}

int createSocketWithServer(char * host, int port) { // create socket (+ connecting process) for communication with webServer
    int sock;
    struct sockaddr_in server;
    struct sockaddr * serverptr = (struct sockaddr *) &server;
    struct hostent * rem;

    int enable = 1;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }
    
    if ((rem = gethostbyname(host)) == NULL) {
        herror("gethostbyname");
        close(sock);
        return -1;
    }

    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);
    
    if (connect(sock, serverptr, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}