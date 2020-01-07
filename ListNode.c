#include "ListNode.h"

ListNode *appendNode(ListNode *head, char *path, char *file, int lineNum){
    ListNode *newNode, *lastNode;
    
    newNode = malloc(sizeof(ListNode));
    newNode->lineNum = lineNum;
    strcpy(newNode->path, path);
    strcpy(newNode->file, file);
    newNode->freq = 1;
    newNode->count = 1;
    newNode->next = NULL;
    
    if (head == NULL){
       head = newNode;
    }  
    else{
        head->count++;
        lastNode = head;
        while (lastNode->next != NULL)
            lastNode = lastNode->next;
        lastNode->next = newNode;
    }
    return newNode;  
}

ListNode *searchNode(ListNode* head, char *path, int lineNum){
    ListNode* curNode = head;
    
    while (curNode != NULL){
        if ((strcmp(path, curNode->path) == 0) && (lineNum == curNode->lineNum)){
            return curNode;
        }
        curNode = curNode->next;
    }
    
    return NULL;
}

void freeList(ListNode *head){
    ListNode* curNode = head;
    while (curNode != NULL){
        curNode = head->next;
        free (head);
    }
}