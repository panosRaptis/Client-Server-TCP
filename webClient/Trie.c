#include "Trie.h"

void destroyTrieRec(Node ** n) {
    if ((*n) != NULL) {
        destroyTrieRec(&(*n)->down);
        destroyTrieRec(&(*n)->right);

        if ((*n)->headPL != NULL) { // if there is posting lists to the current node,
            destroyPL(&((*n)->headPL)); // then only it will have to be destroyed
        }
        free(*n);
        *n = NULL;
    }
}

void destroyTrie(Trie ** trie) { // delete (free) Trie
    destroyTrieRec(&((*trie)->headTrie));
    free(*trie);

    *trie = NULL;
}

FileQList * searchCommand(Trie * trie, WordQList * qlist) {
    if (trie->headTrie == NULL) {
        FileQList * fqlist = malloc(sizeof(FileQList));
        fqlist->headFQL = NULL;
        return fqlist;
    }

    WordQNode * node = qlist->headWQL->next;

    FileQList * fqlist = malloc(sizeof(FileQList));
    fqlist->headFQL = NULL;

    Lines * tempLine = NULL;
    LineQNode * currentLineNode = NULL;

    while (node != NULL) { // for each word in WordQList
        char * w = node->word;
        Node * t = searchWord(trie, w); // search word into Trie
        if (t == NULL) {

        } else {
            if (t->headPL != NULL) { // if current PL != NULL => ok word found !!
                if (!(t->headPL->visitFlag)) {
                    t->headPL->visitFlag = true;
                }

                Files * temp = t->headPL->ptr;

                // create FileQList struct <= result of search command
                while (temp != NULL) { // for each file in the PL of the word
                    if (fqlist->headFQL == NULL) {
                        fqlist->headFQL = malloc(sizeof(FileQNode));
                        fqlist->headFQL->filePath = calloc(strlen(temp->filePath) + 1, sizeof(char));
                        strcpy(fqlist->headFQL->filePath, temp->filePath);
                        fqlist->headFQL->ptr = NULL;
                        fqlist->headFQL->next = NULL;

                        tempLine = temp->headLines;
                        fqlist->headFQL->ptr = malloc(sizeof(LineQNode));
                        currentLineNode = fqlist->headFQL->ptr;
                        currentLineNode->idLine = tempLine->idLine;
                        currentLineNode->startOffset = tempLine->startOffset;
                        currentLineNode->next = NULL;

                        while (tempLine->next != NULL) {
                            tempLine = tempLine->next;
                            currentLineNode->next = malloc(sizeof(LineQNode));
                            currentLineNode = currentLineNode->next;
                            currentLineNode->idLine = tempLine->idLine;
                            currentLineNode->startOffset = tempLine->startOffset;
                            currentLineNode->next = NULL;
                        }
                    } else {
                        pushFileQList(&fqlist->headFQL, temp);
                    }
                    temp = temp->next; // go to the next Files Node in the PL of word
                }
            }
        }
        node = node->next; // go to the next word of WordQueryList (WordQList)
    }
    return fqlist;
}

Node * searchWord(Trie * trie, char * w) { // search word into Trie
    int i = 0;
    Node * current = NULL;
    if (trie->headTrie != NULL) {
        current = trie->headTrie;
    } else {
        return NULL;
    }
    while (i < strlen(w)) {
        if (trie->headTrie != NULL) {
            if (i == 0) {
                while (w[i] != current->c && current->right != NULL) {
                    current = current->right;
                }
                if (w[i] != current->c) {
                    return NULL;
                }
            } else {
                if (current->down == NULL) {
                    return NULL;
                } else {
                    current = current->down;
                    while (w[i] != current->c && current->right != NULL) {
                        current = current->right;
                    }
                    if (w[i] != current->c) {
                        return NULL;
                    }
                }
            }
        }
        i++;
    }
    return current; // return last trieNode of argc word (w)
}

void addTrie(Trie ** trie, int idDoc, char * w, char * filePath, int startOffset) { // add word (token) into Trie
    int i = 0;
    Node * current = NULL;

    if ((*trie)->headTrie != NULL) {
        current = (*trie)->headTrie;
    }
    while (i < strlen(w)) {

        if ((*trie)->headTrie == NULL) { // create 1st trieNode
            Node * node = malloc(sizeof(Node));
            node->right = NULL;
            node->down = NULL;
            node->headPL = NULL;
            node->c = w[i];
            (*trie)->headTrie = node;
            current = node;
        } else {
            if (i == 0) { // 1st character of word
                while (w[i] != current->c && current->right != NULL) {
                    current = current->right;
                }
                if (w[i] != current->c) {
                    current->right = malloc(sizeof(Node));
                    current = current->right;
                    current->right = NULL;
                    current->down = NULL;
                    current->headPL = NULL;
                    current->c = w[i];
                }
            } else { // >= 2nd character of word
                if (current->down == NULL) {
                    current->down = malloc(sizeof(Node));
                    current = current->down;
                    current->right = NULL;
                    current->down = NULL;
                    current->headPL = NULL;
                    current->c = w[i];
                } else {
                    current = current->down;
                    while (w[i] != current->c && current->right != NULL) {
                        current = current->right;
                    }
                    if (w[i] != current->c) {
                        current->right = malloc(sizeof(Node));
                        current = current->right;
                        current->right = NULL;
                        current->down = NULL;
                        current->headPL = NULL;
                        current->c = w[i];
                    }
                }
            }
        }
        i++;
    }

    if (current->headPL == NULL) { // currentNode->headPL == NULL => create PL
        current->headPL = malloc(sizeof(PL));
        current->headPL->sizeOfFiles = 0;
        current->headPL->ptr = NULL;
        current->headPL->visitFlag = false;
    }
    pushPL(&current->headPL->ptr, idDoc, &current->headPL->sizeOfFiles, filePath, startOffset); // find or add a new Statistic into PL
}