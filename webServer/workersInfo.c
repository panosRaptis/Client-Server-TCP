#include "workersInfo.h"

InfoWNode * GetNth(InfoWList * head, int index){ // get a Info Node (worker) with value = index
    if (head == NULL){
        return NULL;
    }
    InfoWNode * current = head->headIWL;
    int count = 0; 
    while (current != NULL)
    {
       if (count == index) return current;
       count++;
       current = current->next;
    }
    return NULL;        
}

void destroyInfoWList(InfoWList ** head){ // delete (free) InfoWList
    if(*head == NULL){
        return;
    }else if((*head)->headIWL == NULL)
    {
        free(*head);
        *head = NULL;
        return;
    }

    InfoWNode * current = (*head)->headIWL;
    current = current->next;
    InfoWNode * next;

    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }

    free((*head)->headIWL);
    (*head)->headIWL = NULL;

    free(*head);
    (*head) = NULL;
}