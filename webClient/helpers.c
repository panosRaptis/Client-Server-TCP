#include "helpers.h"

// https://stackoverflow.com/questions/12698836/counting-words-in-a-string-c-programming
int lineWords(const char sentence[ ]) { // #words of a line (idDoc)
    int counted = 0; // result
    const char* it = sentence;
    int inword = 0;

    do switch(*it) {
        case '\0': 
        case ' ': case '\t': case '\n': case '\r':
            if (inword) { inword = 0; counted++; }
            break;
        default: inword = 1;
    } while(*it++);

    return counted;
}

// http://man7.org/linux/man-pages/man3/realpath.3.html
char * getFullPath(const char * dirPath){
    char * ptr = realpath(dirPath, NULL);
    return ptr;
}

// https://stackoverflow.com/questions/791982/determine-if-a-string-is-a-valid-ip-address-in-c
bool isValidIP(char * ipAddress){
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

// https://stackoverflow.com/questions/12733105/c-function-that-counts-lines-in-file
int numLines(FILE * fp) { // #lines of the file
    int lines = 0;
    char prevCh = '\0';

    while (!feof(fp)) {
        char ch = fgetc(fp);
        if ((ch == '\n' && prevCh != '\n') || (ch == EOF && prevCh != '\n')) {
            lines++;
        }
        prevCh = ch;
    }
    return lines;
}

// https://stackoverflow.com/questions/1514660/how-to-remove-all-spaces-and-tabs-from-a-given-string-in-c-language
void skipExtraSpaces(char * str) { // skip extra Spaces or Tabs from the char * string
    int count = 0; // keep track of non-space character count
    bool flagPrintSpace = true;
    bool flagFirstWord = false;
    
    for (int i = 0; str[i]; i++)

        if (str[i] != ' ' && str[i] != '\t'){
            str[count++] = str[i];
            if(!flagPrintSpace) flagPrintSpace = true;
            if(!flagFirstWord) flagFirstWord = true;
        }else if(flagPrintSpace && flagFirstWord){
            str[count++] = ' ';
            flagPrintSpace = false;
        }
                               
    str[count] = '\0';
}

char * mytrimString(char * str) { // trim char * string
    char * result = calloc(strlen(str)+1, sizeof(char));
    for(int i = 0; i < strlen(str); i++){
        if(str[i] != ' ' && str[i] != '\t'){
            str += i;
            break;
        }
    }   
    int k;
    for(k = strlen(str) - 1; k > 0; k--){
        if(str[k] != ' ' && str[k] != '\t'){
            result[k + 1] = '\0';
            break;
        }
    }
    
    strncpy(result, str, k + 1);
    char * finalResult = calloc(strlen(result) + 1, sizeof(char));
    strcpy(finalResult, result);
    free(result);
    char * s = NULL;
    char * firstPart = multiTok(finalResult, &s, "<a href=\"", true);
    char * finalTrimResult = calloc(strlen(firstPart) + 1, sizeof(char));
    strcpy(finalTrimResult, firstPart);
    free(finalResult);
    return finalTrimResult;
}

// https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
int isRegularFile(const char *path){
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void purgeOrCreateDir(const char * saveDir){
    DIR * dir;
    struct dirent * ent;
    char filePath[BUF_SIZE];
    
    if ((dir = opendir(saveDir)) != NULL) { // open directory
        while ((ent = readdir(dir)) != NULL) { // for each file in directory ... 
            if ((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0)) {
                memset(filePath, ' ', BUF_SIZE);

                if(saveDir[strlen(saveDir) - 1] != '/') sprintf(filePath, "%s/%s", saveDir, ent->d_name);
                else sprintf(filePath, "%s%s", saveDir, ent->d_name);
                    
                if(isRegularFile(filePath) == 0) {
                    purgeOrCreateDir(filePath);
                    rmdir(filePath);
                }
                else {
                    unlink(filePath);
                }
            }
        }
        closedir(dir);
    } else{
        mkdir(saveDir, 0755);
    }
}

// https://stackoverflow.com/questions/29788983/split-char-string-with-multi-character-delimiter-in-c
char * multiTok(char * input, char ** string, char * delimiter, bool flag) {
    if (input != NULL) *string = input;

    if (*string == NULL) return *string;

    char * end = strstr(*string, delimiter);
    if (end == NULL) {
        if(flag){
        char * temp = *string;
        *string = NULL;
        return temp;
        } else return NULL; 
    }

    char * temp = *string;

    *end = '\0';
    *string = end + strlen(delimiter);
    return temp;
}

void prettyPrint(char * path, int idLine, char * strLine, int terminalWidth, WordQList * list, int count, int activeFd, bool flag) { // printf results for search Command                                                                        // with standard format
    if(flag){
        char terminal[terminalWidth + 2];
        int formatChars = sprintf(terminal, "%d.(%s)[%d] ", count, path, idLine);
        int outputLen = formatChars + strlen(strLine) + 1;
        char * str = calloc(outputLen, sizeof(char));
        char * underline = calloc(outputLen, sizeof(char));
        memset(underline, ' ', outputLen * sizeof(char));
        underline[outputLen - 1] = '\0';
        strcpy(str, terminal);
        strcat(str, strLine);

        int start = 0;
        int end = 0;
        int startFlag = 0;
        for(int i = formatChars; i <= strlen(str); i++) {
            if(str[i] == ' ' || str[i] == '\t' || str[i] == '\0') {
                if(startFlag == 0) continue;
                startFlag = 0;
                end = i - 1;
                char * wordToFind = calloc(end - start + 2, sizeof(char));
                strncpy(wordToFind, str + start, end - start + 1);
                if(findWordQList(list, wordToFind) == 0){
                    for(int j = start; j <= end; j++){
                        underline[j] = '^';
                    }
                }
                free(wordToFind);
            }
            else if(startFlag == 0) {
                startFlag = 1;
                start = i;
            }
        }

        snprintf(terminal, terminalWidth + 2, "%s\n", str);
        myWrite(activeFd, terminal, strlen(terminal));
        snprintf(terminal, terminalWidth + 2, "%s\n", underline);
        myWrite(activeFd, terminal, strlen(terminal));

        int newTerminalWidth = terminalWidth - formatChars;
        memset(terminal, ' ', strlen(terminal));

        for(int k = terminalWidth; k < strlen(str); k += newTerminalWidth) {
            snprintf(terminal + formatChars, newTerminalWidth + 2, "%s\n", str + k);
            myWrite(activeFd, terminal, strlen(terminal));
            snprintf(terminal + formatChars, newTerminalWidth + 2, "%s\n", underline + k);
            myWrite(activeFd, terminal, strlen(terminal));
        }

        free(underline);
        free(str);
    }else{
        char * str1 = calloc(16, sizeof(char)); // Integer.toString();
        snprintf(str1, 16, "%d", count);

        char * str2 = calloc(16, sizeof(char)); // Integer.toString();
        snprintf(str2, 16, "%d", idLine);

        char * terminal = calloc(strlen(str1) + strlen(path) + strlen(str2) + strlen(strLine) + 9, sizeof(char));
        sprintf(terminal, "%s.(%s)[%s] %s\n\n", str1, path, str2, strLine);
        myWrite(activeFd, terminal, strlen(terminal));

        free(str1);
        free(str2);
        free(terminal);
    }
}