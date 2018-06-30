#include "FileList.h"

void pushFileQList(FileQNode ** node, Files * file) { // find (and add Lines Node) or add new File Node into FileQList
    FileQNode * current = *node;
    Lines * currentL = file->headLines;
    char * filePath = file->filePath;

    if (*node == NULL) { // add 1st File node
        *node = malloc(sizeof(FileQNode));
        (*node)->filePath = calloc(strlen(filePath) + 1, sizeof(char));
        strcpy((*node)->filePath, filePath);
        (*node)->next = NULL;
        (*node)->ptr = NULL;
        (*node)->ptr = malloc(sizeof(LineQNode));
        (*node)->ptr->idLine = currentL->idLine;
        (*node)->ptr->startOffset = currentL->startOffset;
        (*node)->ptr->next = NULL;

        LineQNode * currentLL = (*node)->ptr; // add Lines of 1st File node
        currentL = currentL->next;
        while (currentL != NULL) {
            currentLL->next = malloc(sizeof(LineQNode));
            currentLL = currentLL->next;
            currentLL->idLine = currentL->idLine;
            currentLL->startOffset = currentL->startOffset;
            currentLL->next = NULL;

            currentL = currentL->next;
        }
        return;
    } else {
        if (strcmp(current->filePath, filePath) == 0) { // find 1st File Node -> return
            pushQLines(&(current->ptr), file->headLines); // push Lines
            return;
        }

        while (current->next != NULL) { // find nodej, with j >= 2 -> return
            current = current->next;
            if (strcmp(current->filePath, filePath) == 0) {
                pushQLines(&(current->ptr), file->headLines); //push Lines
                return;
            }
        }

        current->next = malloc(sizeof(FileQNode)); // add new File Node -> return 
        current = current->next;
        current->filePath = calloc(strlen(filePath) + 1, sizeof(char));
        strcpy(current->filePath, filePath);
        current->next = NULL;
        current->ptr = malloc(sizeof(LineQNode));
        current->ptr->idLine = currentL->idLine;
        current->ptr->startOffset = currentL->startOffset;
        current->ptr->next = NULL;

        LineQNode * currentLL = current->ptr; // add Lines Nodes of current node
        currentL = currentL->next;
        while (currentL != NULL) {
            currentLL->next = malloc(sizeof(LineQNode));
            currentLL = currentLL->next;
            currentLL->idLine = currentL->idLine;
            currentLL->startOffset = currentL->startOffset;
            currentLL->next = NULL;

            currentL = currentL->next;
        }
        return;
    }
}

void pushQLines(LineQNode ** headQLines, Lines * headLines) { // find or add new Line Node under a standard File Node
    LineQNode * current = NULL;
    Lines * temp = headLines;
    while (temp != NULL) {
        current = *headQLines;
        if (current->idLine == temp->idLine) {
            temp = temp->next;
            continue;
        }
        int found = 0;
        while (current->next != NULL) {
            current = current->next;
            if (current->idLine == temp->idLine) {
                found = 1;
                break;
            }
        }
        if (found == 0) {
            current->next = malloc(sizeof(LineQNode)); // add node -> return
            current = current->next;
            current->idLine = temp->idLine;
            current->startOffset = temp->startOffset;
            current->next = NULL;
        }
        temp = temp->next;
    }
}

void destroyFileQList(FileQList ** head) { // delete (free) FileQueryList (search command) of the query
    if (*head == NULL) {
        return;
    } else if((*head)->headFQL == NULL) {
        free(*head);
        *head = NULL;
        return;
    }

    FileQNode * current = (*head)->headFQL;
    current = current->next;
    FileQNode * next;

    while (current != NULL) {
        next = current->next;
        destroyLineQList(&(current->ptr));
        free(current->filePath);
        current->filePath = NULL;
        free(current);
        current = NULL;
        current = next;
    }

    destroyLineQList(&((*head)->headFQL->ptr));

    free((*head)->headFQL->filePath);
    (*head)->headFQL->filePath = NULL;

    free((*head)->headFQL);
    (*head)->headFQL = NULL;

    free(*head);
    (*head) = NULL;
}

void destroyLineQList(LineQNode ** headLines) { // delete (free) LineQuetyList of FileQueryList (search command)
    if (*headLines == NULL) {
        return;
    }

    LineQNode * current = *headLines;
    LineQNode * next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    *headLines = NULL;
}

void printFileQlist(FileQList * list) { // printf FileQList (for each search command) -> oprional (is not used in the latest version of the app, only for my debug)
    FileQNode * node = NULL;
    if (list == NULL) {
        return;
    } else {
        node = list->headFQL;
        if (node == NULL) {
            return;
        }
    }

    LineQNode * lines = NULL;
    while (node != NULL) {
        printf("Path is: %s\n", node->filePath);
        FILE * fp = fopen(node->filePath, "r");
        char * line = NULL;
        lines = node->ptr;
        while (lines != NULL) {
            fseek(fp, lines->startOffset, SEEK_SET);

            size_t len = 0;
            ssize_t read;
            if ((read = getline(&line, &len, fp)) != -1) {
                line[strcspn(line, "\n")] = '\0';
                line = mytrimString(line);
            }

            lines = lines->next;
            free(line);
        }
        fclose(fp);
        node = node->next;
        printf("\n");
    }
}

void sendToPipeFileQList(FileQList * list, int fd) { // sent FileQList (search command acks from workers to jobExecutor) via named-pipe
    FileQNode * node = NULL;
    if (list == NULL) {
        WordQList * qlist = createWordQList();
        addQList(qlist, "0"); // send error code
        sendToPipe(qlist, fd);
        destroyWordQList(&qlist);
        return;
    } else {
        node = list->headFQL;
        if (node == NULL) {
            WordQList * qlist = createWordQList();
            addQList(qlist, "0"); // send error code
            sendToPipe(qlist, fd);
            destroyWordQList(&qlist);
            return;
        }
    }

    LineQNode * lines = NULL;
    while (node != NULL) {
        WordQList * qlist = createWordQList();
        addQList(qlist, node->filePath);
        FILE * fp = fopen(node->filePath, "r");
        char * line = NULL;
        lines = node->ptr;
        while (lines != NULL) {
            char * str = calloc(16, sizeof(char)); // Integer.toString();
            snprintf(str, 16, "%d", lines->idLine);
            addQList(qlist, str);
            free(str);

            fseek(fp, lines->startOffset, SEEK_SET); // read line use fseek with offset (lines->startOffset)

            size_t len = 0;
            ssize_t read;
            if ((read = getline(&line, &len, fp)) != -1) {
                line[strcspn(line, "\n")] = '\0';
                char * trimLine = mytrimString(line);
                addQList(qlist, trimLine);
                free(trimLine);
            }

            if (node->next == NULL && lines->next == NULL) {
                addQList(qlist, "END");
            }

            sendToPipe(qlist, fd);
            deleteLastQNode(&qlist); // delete 2 last Nodes of List, for send
            deleteLastQNode(&qlist); // other line in the same file (keep onle 1st node [Path Str])

            lines = lines->next;
            free(line);
        }
        fclose(fp);
        node = node->next;
        destroyWordQList(&qlist);
    }
}