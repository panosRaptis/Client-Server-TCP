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
    return result;
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

char * getWord(const char * line, int i, int numOfWords){
    char * str = calloc(strlen(line) + 1, sizeof(char));
    strcpy(str, line);
    int count = 1;
    char * s = NULL;
    char * res = NULL;
    char * token = NULL;
    for(int k = 0; k < numOfWords; k++){

        if(k == 0) token = multiTok(str, &s, " ", true);
        else if(k > 0 && k < numOfWords - 1) token = multiTok(NULL, &s, " ", true);
        else if (k == numOfWords - 1) token = multiTok(NULL, &s, "\r\n", true);

        if(count == i) {
            res = calloc(strlen(token) + 1, sizeof(char));
            strcpy(res, token);
            break;
        }
        count++;
    }
    free(str);
    return res;
}

bool isValidGETRequest(char str[ ]){ // check if GET Request is vallid (+ return true/false)
    char * request = calloc(strlen(str) + 1, sizeof(char));
    strcpy(request, str);
    int countLine = 1;
    bool itsOK = true;
    char * s = NULL;
    bool isHost = false, flag = false;
    char * firstWord = NULL;
    for (char * line = multiTok(request, &s, "\r\n", true); line != NULL; line = multiTok(NULL, &s, "\r\n", true)) 
    {
        int words;
        if(strcmp(line, "") != 0){
            flag = true;
            words = lineWords(line);
            firstWord = getWord(line, 1, words);
        } else break;

        if(firstWord == NULL){
            itsOK = false;
            break;
        }
        if(countLine == 1){
            char * thirdWord = getWord(line, 3, words); 
            if(strcmp(firstWord, "GET") != 0) {
                itsOK = false;
                free(thirdWord);
                break;
            }else if(words != 3){
                itsOK = false;
                free(thirdWord);
                break;
            }else if(strcmp(thirdWord, "HTTP/1.1") != 0){
                itsOK = false;
                free(thirdWord);
                break;
            }
            free(thirdWord);
        }else if(strcmp(firstWord, "Host:") == 0) {
            isHost = true;
            if(words <= 1){
                itsOK = false;             
                break;
            }       
        }else if((strcmp(firstWord, "User-Agent:") == 0) || (strcmp(firstWord, "Accept-Language:") == 0) || (strcmp(firstWord, "Accept-Encoding:") == 0) || (strcmp(firstWord, "Connection:") != 0)){
            if(words <= 1){
                break;
            }
        }

        free(firstWord);
        countLine++;
    }

    if(!itsOK && flag) free(firstWord);
    
    if(str[strlen(str) - 1] != '\n' || str[strlen(str) - 2] != '\r' || str[strlen(str) - 3] != '\n' || str[strlen(str) - 4] != '\r'){
        itsOK = false;
    }
    
    if(!isHost) itsOK = false;

    free(request);
    return itsOK;
}