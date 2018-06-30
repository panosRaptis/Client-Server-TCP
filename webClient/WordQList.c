#include "WordQList.h"

void addQList(WordQList * list, char * val) { // add a Node (word) in the list of chars *
    WordQNode * last = list->headWQL;

    while (last->next != NULL) {
        last = last->next;
    }

    last->next = malloc(sizeof(WordQNode));
    last->next->word = calloc(strlen(val) + 1, sizeof(char));
    strcpy(last->next->word, val);
    last->next->next = NULL;
}

int countQList(WordQList * list) { // #Nodes into List
    int c = 0;
    WordQNode * head = NULL;
    if (list == NULL) {
        return 0;
    } else {
        head = list->headWQL;
        if (head == NULL || head->next == NULL) {
            return 0;
        }
    }

    WordQNode * current = head->next;

    c = 0;

    while (current != NULL) {
        c++;
        current = current->next;
    }
    return c;
}

void printQList(WordQList * list) { // printf WordQList -> oprional (is not used in the latest version of the app, only for my debug)
    WordQNode * head = NULL;
    if (list == NULL) {
        printf("> List is empty!!\n");
        return;
    } else {
        head = list->headWQL;
        if (head == NULL || head->next == NULL) {
            printf("> List is empty!!\n");
            return;
        }
    }

    WordQNode * current = head->next;

    while (current != NULL) {
        printf("%s\n", current->word);
        current = current->next;
    }
}

char * serializeQList(WordQList * list) { // serialize received worker command from a named-pipe
    char * buffer = calloc(BUF_SIZE, 1);
    int ctr = 0;
    WordQNode * current = list->headWQL->next;

    while (current != NULL) {
        if (ctr == 0) {
            strcpy(buffer, current->word);
        } else {
            strcat(buffer, " ");
            strcat(buffer, current->word);
        }
        current = current->next;
        
        ctr++;
    }
    
    return buffer;
}

WordQList * createWordQList() { // create header of WordQList
    WordQList * l = (WordQList *) malloc(sizeof(WordQList));
    l->headWQL = malloc(sizeof(WordQNode));
    l->headWQL->word = NULL;
    l->headWQL->next = NULL;

    return l;
}

void destroyWordQList(WordQList ** list) { // delete (free) WordQList
    if(*list == NULL) return;
    WordQNode ** head = &((*list)->headWQL);
    if (*head == NULL) {
        free(*list);
        *list = NULL;
        return;
    }

    WordQNode * current = *head;
    current = current->next;
    WordQNode * next = NULL;

    while (current != NULL) {
        next = current->next;
        free(current->word);
        free(current);
        current = next;
    }
    free(*head);
    *head = NULL;
    
    free(*list);
    *list = NULL;
}

int findWordQList(WordQList * list, char* word) { // find Node in List with str == word
    WordQNode * head = list->headWQL;
    if (head == NULL || head->next == NULL) {
        return -1;
    }

    WordQNode * current = head->next;
    while (current != NULL) {
        if (strcmp(word, current->word) == 0) {
            return 0;
        }
        current = current->next;
    }
    return -1;
}

void deleteLastQNode(WordQList ** list) // delete (free) WordQList
{
    WordQNode * head = (*list)->headWQL;
    if (head == NULL || head->next == NULL) {
        return;
    }

    WordQNode * current = head->next;
    WordQNode * t = NULL;
    while(current->next != NULL)
    {
        t = current;
        current = current->next;
    }

    free(t->next->word);    
    free(t->next);
    t->next=NULL; 
}  