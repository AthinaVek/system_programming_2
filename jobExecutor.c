#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include "Trie.h"

pid_t forkPid;
int alarmJobFlag = 0;

void onAlarm(int sig){
    kill(forkPid, SIGKILL);
}

//void alarmJobHandler(int sig){
//    alarmJobFlag = 1;
//}

int main(int argc, char** argv) {
    int workers, j, lineCount=0, r, flag=0;
    int i, fd, argCount, *readfd, *writefd;
    char *searchWords, **sWords, *w, c, **paths, **namedRPipes, **namedWPipes;
    char rBuf[2048], szBuf[1024], **mCount;
    size_t size, lineLength;
    FILE *fp;
    pid_t pid, *pids;
    
    if (argc > 1) {
        for (i=1; i<argc-1; i++){
            if (strcmp(argv[i],"-d") == 0)
                fp = fopen(argv[i+1], "r");
            else if (strcmp(argv[i],"-w") == 0)
                workers = atoi(argv[i+1]);
        }
        
        if (workers < 1)
            return 0;
        
//        signal(SIGUSR2, alarmJobHandler);
        
        if (fp){
            namedRPipes = malloc(workers*sizeof(char*));
            namedWPipes = malloc(workers*sizeof(char*));
            for (i=0; i<workers; i++){
                namedRPipes[i] = malloc(128*sizeof(char));
                sprintf(namedRPipes[i], "/tmp/fifoa.%d", i);
                namedWPipes[i] = malloc(128*sizeof(char));
                sprintf(namedWPipes[i], "/tmp/fifob.%d", i);
            }

            while ((c = fgetc(fp)) != EOF){  //metraw ta paths tou  docfile
                if (c == '\n')
                    lineCount++;
            }
            
            readfd = malloc(workers*sizeof(int));   //pinakes me fd twn named pipes
            writefd = malloc(workers*sizeof(int));
            pids = malloc(workers*sizeof(pid_t));
            
            paths = malloc(lineCount*sizeof(char*));  //pinakas me paths
            rewind(fp);

            int n;
            for (i=0; i<lineCount; i++){     //vazw kathe grammh sto path[]
                n= getline(&paths[i], &lineLength, fp);
                paths[i][n-1] = '\0';
            }

            char **ar;
            argCount = 3+lineCount/workers+3;  //oi theseis tou pinaka pou stelnw stous workers
            
            ar = malloc(argCount*sizeof(char*));   //pinakas me tis metavlhtes gia tous workers
            for (i=0; i<argCount; i++)
                ar[i] = malloc(128*sizeof(char));

            if (lineCount < workers)   //an ta files einai ligotera ap tous workers den dhmiourgw parapanw
                workers = lineCount;
            
            for (i=0; i<workers; i++){
                int m=4;
                strcpy(ar[0],"./worker");
                sprintf(ar[1], "%d", i);
                strcpy(ar[2], namedRPipes[i]);
                strcpy(ar[3], namedWPipes[i]);
                for (int k=0; k<lineCount; k++){
                    if((k == 0) && (i == 0)){
                        strcpy(ar[m], paths[k]);
                        m++;
                    }
                    else if(k%workers == i){
                        strcpy(ar[m], paths[k]);
                        m++;
                    }
                }
                ar[m] = NULL;

                if (mkfifo(namedRPipes[i], 0666) < 0)   //ftiaxnw ta pipes
                    perror("FIFO");
                if (mkfifo(namedWPipes[i], 0666) < 0)
                    perror("FIFO");
                
                pid = fork();       //ftiaxnw th fork
                if (pid == -1){
                    perror("fork");
                    exit(1);
                }
                if (pid == 0){       //child
                    execvp(ar[0], ar);        //kalw worker
                }
                
                pids[i] = pid;
            }
            fclose(fp);
            
            for (i=0; i<workers; i++){
                if((readfd[i] = open(namedRPipes[i], O_RDONLY)) < 0)   //anoigw ta named pipes
                    perror("OPEN R PIPE");
                if((writefd[i] = open(namedWPipes[i], O_WRONLY)) < 0)
                        perror("OPEN R PIPE");
            }
        
            mCount = malloc(workers*sizeof(char*));
            for (i=0; i<workers; i++)
                mCount[i] = malloc(1024*sizeof(char));

            while(1){
                searchWords = NULL;
                szBuf[0] = '\0';
                
                printf("GIVE COMMAND: ");
                getline(&searchWords, &size, stdin);   //eisagwgh leksewn anazhthshs

                if ((strncmp(searchWords,"/search",7)==0) || (strncmp(searchWords,"/maxcount",9)==0) || 
                        (strncmp(searchWords,"/mincount",9)==0) || (strncmp(searchWords,"/wc",3)==0) || 
                        (strncmp(searchWords,"/exit",5)==0)){
                    
                    for (i=0; i<workers; i++)
                         kill(pids[i], SIGUSR1);    //stelnei se kathe worker shma oti tha stalei entolh
                    
                    for (i=0; i<workers; i++){             //stelnw thn entolh stous workers
                        r = write(writefd[i], searchWords, strlen(searchWords));
                        if (r == -1)
                            printf("WRITE ERROR!!\n");
                    }
                    
                    sleep(1);
                    int maxC=0, minC=0;
                    for (i=0; i<workers; i++){             //dexomai apotelesmata apo workers
                        r = read(readfd[i], rBuf, sizeof(rBuf));
                        if ((r > 0) && (strncmp(rBuf," ", 1)!=0)){
                            flag = 1;
                            strncpy(szBuf, rBuf, r);
                            szBuf[r] = '\0';
                            if (strncmp(searchWords,"/maxcount",9) == 0){    //ftiaxnw pinaka gia maxcount
                                strcpy(mCount[maxC], szBuf);
                                maxC++;
                            }
                            else if (strncmp(searchWords,"/mincount",9) == 0){    //ftiaxnw pinaka gia mincount
                                strcpy(mCount[minC], szBuf);
                                minC++;
                            }
                            else      //gia search/wc
                                printf("%s\n", szBuf);
                        }
                        sleep(1);
                    }
                    if (strncmp(searchWords,"/maxcount",9) == 0){
                        int len1, len2;
                        char max[128];
                
                        strcpy(max,mCount[0]);
                        for(i=1; i<maxC; i++){
                            len1 = strlen(mCount[i-1]);
                            len2 = strlen(mCount[i]);
                            if (mCount[i-1][len1] < mCount[i][len2])
                                strcpy(max, mCount[i]);
                        }
                        if (flag)
                            printf("%s\n", max);
                    }
                    else if (strncmp(searchWords,"/mincount",9) == 0){
                        int len1, len2;
                        char min[128];
                        
                        strcpy(min,mCount[0]);
                        for(i=1; i<minC; i++){
                            len1 = strlen(mCount[i-1]);
                            len2 = strlen(mCount[i]);
                            if (mCount[i-1][len1] > mCount[i][len2])
                                strcpy(min, mCount[i]);
                        }
                        if (flag)
                            printf("%s\n", min);
                    }

                    flag = 0;
                    if (strncmp(searchWords,"/exit",5) == 0)
                        break;
                }
            }
            for (i=0; i<workers; i++){
                close(readfd[i]);
                close(writefd[i]);
                unlink(namedRPipes[i]);
                unlink(namedWPipes[i]);
            }
            free(readfd);
            free(writefd);
            free(pids);
            for (i=0; i<lineCount; i++)
                free(paths[i]);
            free(paths);
            for (i=0; i<argCount; i++)
                free(ar[i]);
            free(ar);
            for (i=0; i<workers; i++)
                free(mCount[i]);
            free(mCount);
        }
    }
    return (EXIT_SUCCESS);
}

