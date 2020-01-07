#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include "Trie.h"

#define MAXRBUF 1024
#define MAXWBUF 1024

int alarmFlag=0;

typedef struct arrayNode{
    char name[256];
    int num;
    struct arrayNode *next;
}Node;

typedef struct wc_count {
    int nChars;
    int nWords;
    int nLines;
}WcNode;


void alarmHandler(int sig){
    alarmFlag = 1;
}


char * getFileline(char * filepath, int num) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int k = 0, lineFound = 0;
    static char szLine[256];
    
    fp = fopen(filepath, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    
    while ((read = getline(&line, &len, fp)) != -1) {
        k++;
        if (k == num) {
            lineFound = 1;
            break;
        }
    }
    fclose(fp);
    
    if (lineFound) {
        strcpy(szLine, line);
        szLine[read-1]='\0';
    }
    else
        szLine[0] = '\0';
    
    if (line)
        free(line);
    
    return szLine;
}


Node *maxcount(TrieNode *trie, char *word, Node *maxNode, Node *maxArray){
    ListNode *pL1;
    Node *tmp;
    
    pL1 = searchTrie(&trie, word);
    if(pL1){
        while (pL1 != NULL){
            tmp = maxArray;
            while (tmp != NULL) {
                if (strcmp(tmp->name, pL1->file) == 0) {
                    tmp->num += pL1->freq;
                    if (tmp->num > maxNode->num) {
                        strcpy(maxNode->name, tmp->name);
                        maxNode->num = tmp->num;
                    }
                    break;
                }
                tmp = tmp->next;
            }
            pL1 = pL1->next;
        }
    }
    return maxNode;
}


Node *mincount(TrieNode *trie, char *word, Node *minNode, Node *minArray){
    ListNode *pL1;
    Node *tmp;
    int flag=0;
    
    pL1 = searchTrie(&trie, word);
    if(pL1){
        while (pL1 != NULL){
            tmp = minArray;
            while (tmp != NULL) {
                if (strcmp(tmp->name, pL1->file) == 0){
                    tmp->num += pL1->freq;
                    if ((flag == 0) && (tmp->num > 0)){
                        strcpy(minNode->name, tmp->name);
                        minNode->num = tmp->num;
                        flag = 1;
                    }
                    else if (strcmp(tmp->name,minNode->name)==0)
                        minNode->num = tmp->num;
                    else if ((tmp->num > 0) && (tmp->num < minNode->num)){
                        printf("mphke\n");
                        strcpy(minNode->name, tmp->name);
                        minNode->num = tmp->num;
                    }
                    break;
                }
                tmp = tmp->next;
            }
            pL1 = pL1->next;
        }
    }
    return minNode;
}


char *currentDateTime(){
    static char buf[80];
    time_t currentTime;
    struct tm *t;

    currentTime = time(NULL);     // diavazw to current time
    t = localtime(&currentTime);  // fortwnw to current time sth struct t

    sprintf (buf, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

    return buf;
}


int writeToLogFile(char *filename, char *sBuf){
    FILE *fp;

    fp = fopen(filename, "a");
    if(fp == NULL)
        return -1;

    fprintf(fp, "%s", sBuf);
    fclose(fp);

    return 0;
}


Node *searchWords(TrieNode *trie, char **sWords, Node *inFiles, int sWordsCount, char *id){
    int flag=0, flag2=0;   //gia thn 1h fora
    ListNode *pL;
    Node *head;
    char logFile[128], allPaths[1024], sCommand[50], wBuf[MAXWBUF];
    
    for (int i=1; i<sWordsCount; i++){     //gia kathe leksh pou anazhtw
        pL = searchTrie(&trie, sWords[i]);
        if (pL){      //an uparxei
            while (pL != NULL){     //diavazw ola ta posting lists
                head = inFiles;
                while (head != NULL){ 
                    if ((strcmp(head->name, pL->path) == 0) && (head->num == pL->lineNum)){
                        flag = 1;
                        break;
                    }
                    head = head->next;
                }
                
                if (flag == 0){
                    Node *new;
                    new = malloc(sizeof(Node));
                    strcpy(new->name, pL->path);
                    new->num = pL->lineNum;
                    new->next = NULL;
                    
                    if (inFiles == NULL)
                        inFiles = new;
                    else{
                        head = inFiles;
                        while (head->next != NULL)
                            head = head->next;
                        head->next = new;
                    }
                }
                else
                    flag = 0;
                
                if (flag2 == 0){   //to prwto path gia to log
                    strcpy(allPaths, pL->path);
                    flag2 = 1;
                }
                else{
                    strcat(allPaths, " : ");
                    strcat(allPaths, pL->path);
                }
                pL = pL->next;
            }
            sprintf(logFile, "log/Worker_%03d", atoi(id));
            strcpy(sCommand, "search");
            sprintf(wBuf, "%s : %s : %s : %s\n", currentDateTime(), sCommand, sWords[i], allPaths);  
            writeToLogFile(logFile, wBuf);
        }
    }
    return inFiles;
} 


int isWhiteSpace(char c){
    if (c==' ' || c=='\t' || c=='\n')
        return 1;
    
    return 0;
}


void myWc(WcNode *wc, char* path, DIR *dr){
    struct dirent *de;
    FILE *fp;
    char c, *pathName;
    int isWord, isWhite;
    
    wc->nChars = 0;
    wc->nWords = 0;
    wc->nLines = 0;
    
    while ((de = readdir(dr)) != NULL){
        if ((strcmp(de->d_name,".") != 0) && (strcmp(de->d_name,".."))){   //gia kathe arxeio
            sprintf(pathName, "%s/%s", path, de->d_name);  
            
            isWord = 0;
            fp = fopen(pathName, "r");
            if(fp){
                while ((c = fgetc(fp)) != EOF) {
                    wc->nChars++;
                    isWhite = isWhiteSpace(c);
                    
                    if (c == '\n')
                        wc->nLines++;
                    if (isWhite == 1) {
                        if(isWord == 0) 
                            wc->nWords++;
                        isWord = 1;
                    }
                    if(isWhite == 0 && isWord == 1)
                        isWord = 0;
                }
                fclose(fp);
            }
        }
    }
}


int main(int argc, char** argv){
    int readfd, writefd, r, n;
    int fCount=0, lineCount=0, i, f=0, sumLines=0;
    char pathName[1024], c, **temp, *word, rBuf[MAXRBUF], wBuf[MAXWBUF], lBuf[MAXWBUF];
    char *w, **sWords, logFile[128], allPaths[1024], sCommand[50];
    struct dirent *de;
    size_t lineLength;
    DIR *dr;
    FILE *fp;
    TrieNode *trie=NULL;
    Node *inFiles=NULL;
    WcNode *wc;
    char szLine[512];
    
    signal(SIGUSR1, alarmHandler);   //orizei sunarthsh pou tha ektelestei an erthei to shma
    
    if((writefd = open(argv[2], O_WRONLY)) < 0)   //anoigw ta named pipes
        perror("w OPEN R PIPE");
    if((readfd = open(argv[3], O_RDONLY)) < 0)   
        perror("w OPEN R PIPE");
    
    Node *mNode, *mArray, *tmp, *newNode;
    mNode = malloc(sizeof(Node));
    mNode->num = -1;
    mNode->next = NULL;
    mArray = NULL;
    
    
    
    for (n=4; n<argc; n++){
        dr = opendir(argv[n]);
        if (dr == NULL){
            printf("work ERROR\n");
            return 0;
        }
        
        while ((de = readdir(dr)) != NULL){
            lineCount=0;
            if ((strcmp(de->d_name,".") != 0) && (strcmp(de->d_name,".."))){   //gia kathe arxeio
                fCount++;
                sprintf(pathName, "%s/%s", argv[n], de->d_name);        
                
                fp = fopen(pathName, "r");
                if(fp){
                    while ((c = fgetc(fp)) != EOF){
                        if (c == '\n'){
                            lineCount++;
                            sumLines++;
                        }
                    }
                    
                    newNode = malloc(sizeof(Node));
                    strcpy(newNode->name, de->d_name);
                    newNode->num = 0;
                    newNode->next = NULL;
                    
                    if (mArray == NULL)
                        mArray = newNode;
                    else{
                        tmp = mArray;
                        while (tmp->next != NULL)
                            tmp = tmp->next;
                        tmp->next = newNode;
                    }
                    
                    temp = malloc(lineCount*sizeof(char*));
                    rewind(fp);
                    
                    for (i=0; i<lineCount; i++){    
                        getline(&temp[i], &lineLength, fp);
                    }
                    for (i=0; i<lineCount; i++){ 
                        word = strtok(temp[i], " \n ");    
                        while (word != NULL){    //spasimo se lekseis kai insert sto trie
                            insertTrie(&trie, word, i, pathName, de->d_name);
                            word = strtok(NULL, " '\n'");
                        }
                    }
                    fclose(fp);
                }
            }
        }
        closedir(dr);
    }
    
    while(1){
        while (alarmFlag == 0)    //perimenei mexri na dextei shma SIGUSR1 apo jobExecutor
            pause();
        alarmFlag = 0;
        mNode->num = -1;
        
        r = read(readfd, rBuf, MAXRBUF);  //diavasma apo pipe
        if (r > 0) {
            int j=0;
            w = strtok(rBuf, " \n\t");    //spaw tis lekseis pou dinw apo plhktrologio
            sWords = malloc(20*sizeof(char*));
            while (w != NULL){
                sWords[j] = malloc(strlen(w)*sizeof(char));
                sWords[j] = w;
                w = strtok(NULL, " \n");
                j++;
            }
            
            sleep(0.5);
            if (strncmp(sWords[0],"/search",7) == 0){             //SEARCH
                strcpy(sCommand, "search");
                inFiles = searchWords(trie, sWords, inFiles, j, argv[1]);
                if (inFiles){
                    while (inFiles){
                        strcpy(szLine, getFileline(inFiles->name, inFiles->num+1));
                        sprintf(wBuf, "%s %d %s\n", inFiles->name, inFiles->num+1, szLine);
                        
                        r = write(writefd, wBuf, strlen(wBuf));
                        if (r == -1)
                            printf("WRITE ERROR!!\n");
                        inFiles = inFiles->next;
                    }
                }
                else
                    write(writefd, " ", 1);
            }
            else if (strncmp(sWords[0],"/maxcount",9) == 0){       //MAXCOUNT
                strcpy(sCommand, "maxcount");
                tmp = mArray;
                while (tmp != NULL){
                    tmp->num = 0;
                    tmp = tmp->next;
                }
                mNode = maxcount(trie, sWords[1], mNode, mArray);
                sprintf(wBuf, "%s %d", mNode->name, mNode->num);
                if (mNode->num > 0){
                    sprintf(logFile, "log/Worker_%03d", atoi(argv[1]));
                    sprintf(lBuf, "%s : %s : %s : %s\n", currentDateTime(), "maxcount", sWords[1], mNode->name);  
                    writeToLogFile(logFile, lBuf);
                    
                    r = write(writefd, wBuf, strlen(wBuf));
                    if (r == -1)
                        printf("WRITE ERROR!!\n");
                }
                else
                    write(writefd, " ", 1);
            }
            else if (strncmp(sWords[0],"/mincount",9) == 0){        //MINCOUNT
                strcpy(sCommand, "mincount");
                tmp = mArray;
                while (tmp != NULL){
                    tmp->num = 0;
                    tmp = tmp->next;
                }
                mNode = mincount(trie, sWords[1], mNode, mArray);
                sprintf(wBuf, "%s %d", mNode->name, mNode->num);
                if (mNode->num > 0){
                    sprintf(logFile, "log/Worker_%03d", atoi(argv[1]));
                    sprintf(lBuf, "%s : %s : %s : %s\n", currentDateTime(), "mincount", sWords[1], mNode->name);  
                    writeToLogFile(logFile, lBuf);
                    
                    r = write(writefd, wBuf, strlen(wBuf));
                    if (r == -1)
                        printf("WRITE ERROR!!\n");
                }
                else
                    write(writefd, " ", 1);
            }
            else if (strncmp(sWords[0],"/wc",3) == 0){              //WC
                strcpy(sCommand, "wc");
                printf(" \n");
                for (n=4; n<argc; n++){
                    dr = opendir(argv[n]);
                    if (dr != NULL){
                        wc = malloc(sizeof(WcNode));
                        myWc(wc, argv[n], dr);
                        sprintf(wBuf, "characters: %d, words: %d, lines: %d\n", wc->nChars, wc->nWords, wc->nLines);
                        free(wc);
                        r = write(writefd, wBuf, strlen(wBuf));
                        if (r == -1)
                            printf("WRITE ERROR!!\n");
                    }
                    closedir(dr);
                }
                sprintf(logFile, "log/Worker_%03d", atoi(argv[1]));
                sprintf(lBuf, "%s : %s\n", currentDateTime(), "wc");  
                writeToLogFile(logFile, lBuf);
            }
            else if (strncmp(sWords[0],"/exit",5) == 0){
                close(writefd);
                close(readfd);
                free(mNode);
                if ((mArray != NULL) && (temp)){
                    for (i=0; i<fCount; i++)
                        free(temp[i]);
                    free(temp);
                }
                free(wc);
                freeTrie(trie);
                return 0;
            }   
        }
    }
}

