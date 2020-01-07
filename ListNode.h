#ifndef LISTNODE_H
#define LISTNODE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Node{
    char path[256];
    char file[20];
    int lineNum;
    int freq;
    int count; //sunolo
    struct Node *next;
}ListNode;

ListNode *appendNode(ListNode *head, char *path, char *file, int lineNum);
ListNode *searchNode(ListNode *head, char *path, int lineNum);
void freeList(ListNode *head);

#endif /* LISTNODE_H */
