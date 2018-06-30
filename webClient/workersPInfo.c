#include "workersPInfo.h"

void addPath(Paths * path, char * val) { // add Path Node under standard File Node
    Paths * last = path;

    while (last->next != NULL) {
        last = last->next;
    }

    last->next = malloc(sizeof(Paths));
    last->next->str = calloc(strlen(val) + 1, sizeof(char));
    strcpy(last->next->str, val);
    last->next->next = NULL;
}

InfoPNode * GetPNth(InfoPList * head, int index){ // get a Info Node (worker) with value = index
    if (head == NULL){
        return NULL;
    }
    InfoPNode * current = head->headP;
    int count = 0; 
    while (current != NULL)
    {
       if (count == index) return current;
       count++;
       current = current->next;
    }
    return NULL;        
}

void destroyInfoPList(InfoPList ** head){ // delete (free) InfoPList
    if(*head == NULL){
        return;
    }else if((*head)->headP == NULL)
    {
        free(*head);
        *head = NULL;
        return;
    }

    InfoPNode * current = (*head)->headP;
    current = current->next;
    InfoPNode * next;

    while(current != NULL){
        next = current->next;
        destroyPaths(&(current->paths));

        free(current->strTransmit);
        current->strTransmit = NULL;

        free(current->strReceive);
        current->strReceive = NULL;

        free(current);
        current = next;
    }

    destroyPaths(&((*head)->headP->paths));

    free((*head)->headP->strTransmit);
    (*head)->headP->strTransmit = NULL;

    free((*head)->headP->strReceive);
    (*head)->headP->strReceive = NULL;

    free((*head)->headP);
    (*head)->headP = NULL;

    free(*head);
    (*head) = NULL;
}

void destroyPaths(Paths ** path){ // delete (free) Paths
    if (*path == NULL) return;

    Paths * current = *path;
    Paths * next;

    while(current != NULL){
        next = current->next;
        free(current->str);
        free(current);
        current = next;
    }

    *path = NULL;
}