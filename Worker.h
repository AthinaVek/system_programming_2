#ifndef WORKER_H
#define WORKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include  <sys/types.h>

char *Worker(int *fp, int modNum, pid_t  pid, char *sWord);

#endif /* WORKER_H */

