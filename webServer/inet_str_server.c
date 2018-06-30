// Course Slides - Topic 5

#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>

#include "inet_str_server.h"	     /* internet sockets */

int createSocket(int port) {
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

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    if (bind(sock, serverptr, sizeof(server)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock, CONNECTIONS_BUF_SIZE) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("> Setup server port at  %d completed \n", port);

    return sock;
}