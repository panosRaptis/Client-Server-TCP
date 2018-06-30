#include "PollArgsList.h"

void addPollArgsList(PollArgsList * list, int val) { // add a node (word) into the PollArgsueryList
    PollArgsNode * last = list->headWPAL;

    while (last->next != NULL) {
        last = last->next;
    }

    last->next = malloc(sizeof(PollArgsNode));
    last->next->fd = val;
    last->next->next = NULL;
}

int countPollArgsList(PollArgsList * list) { // #nodes in PollArgsist (without header)
    int c = 0;
    PollArgsNode * head = NULL;
    if (list == NULL) {
        return 0;
    } else {
        head = list->headWPAL;
        if (head == NULL || head->next == NULL) {
            return 0;
        }
    }

    PollArgsNode * current = head->next;

    c = 0;
    while (current != NULL) {
        c++;
        current = current->next;
    }
    return c;
}

void printPollArgsList(PollArgsList * list) { // printf query's words (node of PollArgsueryList) -> oprional (is not used in the latest version of the app, only for my debug)
    PollArgsNode * head = NULL;
    if (list == NULL) {
        printf("> No workers pending !!\n");
        return;
    } else {
        head = list->headWPAL;
        if (head == NULL || head->next == NULL) {
            printf("> No workers pending !!\n");
            return;
        }
    }

    PollArgsNode * current = head->next;
    int panos = 0;
    while (current != NULL) {
        printf("#%d Now in PollArgsList %d\n", panos, current->fd);
        panos++;
        current = current->next;
    }
}

struct pollfd * serializePollArgsList(PollArgsList * list) { // create fdarray
    int N = countPollArgsList(list);
    int ctr = 0;

    struct pollfd * fdarray = malloc(sizeof(struct pollfd)*N);

    PollArgsNode * current = list->headWPAL->next;

    while (current != NULL) {
        fdarray[ctr].fd = current->fd;
        fdarray[ctr].events = POLLIN | POLLHUP;
        fdarray[ctr].revents = 0;

        current = current->next;
        ctr++;
    }

    return fdarray;
}

PollArgsList * createPollArgsList() { // create header of PollArgsueryList
    PollArgsList * l = (PollArgsList *) malloc(sizeof(PollArgsList));
    l->headWPAL = malloc(sizeof(PollArgsNode));
    l->headWPAL->fd = 0;
    l->headWPAL->next = NULL;

    return l;
}

void destroyPollArgsList(PollArgsList ** list) { // delete (free) PollArgsueryList
    PollArgsNode ** head = &((*list)->headWPAL);
    if (*head == NULL) {
        return;
    }

    PollArgsNode * current = *head;
    current = current->next;
    PollArgsNode * next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    free(*head);
    *head = NULL;

    free(*list);
    *list = NULL;
}

int findPollArgsList(PollArgsList * list, int fd) { // oprional (is not used in the latest version of the app, only for my debug)
    PollArgsNode * head = list->headWPAL;
    if (head == NULL || head->next == NULL) {
        return -1;
    }

    PollArgsNode * current = head->next;
    while (current != NULL) {
        if (current->fd == fd) {
            return 0;
        }
        current = current->next;
    }
    return -1;
}

void deleteLastPollArgsNode(PollArgsList ** list) {
    PollArgsNode * head = (*list)->headWPAL;
    if (head == NULL || head->next == NULL) {
        return;
    }

    PollArgsNode * current = head->next;
    PollArgsNode * t = NULL;
    while (current->next != NULL) {
        t = current;
        current = current->next;
    }

    free(t->next);
    t->next = NULL;
}

void removePollArgsList(PollArgsList * list, int val) { // remove val (fd) from PollArgsList
    if (list == NULL) {
        return;
    } else {
        PollArgsNode * head = list->headWPAL;
        if (head == NULL || head->next == NULL) {
            return;
        }
    }
    
    PollArgsNode * current = list->headWPAL->next;
    PollArgsNode * prevNode = NULL;

    if (current != NULL && current->fd == val)
    {
        list->headWPAL->next = current->next;   
        free(current);            
        return;
    }

    while(current != NULL && current->fd != val)
    {
        prevNode = current;
        current = current->next;
        
    }   

    if (current == NULL) return; 

    prevNode->next = current->next;
    free(current);
}