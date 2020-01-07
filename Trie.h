#ifndef TRIE_H
#define TRIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ListNode.h"


typedef struct TrieNode{
    struct TrieNode *child;
    struct TrieNode *left;
    struct TrieNode *right;
    char c;
    ListNode *pList;
}TrieNode;



TrieNode *newTrieNode(char c);
ListNode *searchTrie(TrieNode **head, char *word);
void insertTrie(TrieNode **head, char *word, int lineNum, char* path, char *file);
void triePrint(TrieNode *curNode, char *word, int i);
void freeTrie(TrieNode *curNode);

#endif /* TRIE_H */
