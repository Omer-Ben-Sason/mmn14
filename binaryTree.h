#ifndef BINARY_TREE_H
#define BINARY_TREE_H
#include "utils.h"
typedef struct reserved {
    char* name;
    char* pData;
} reserved;

typedef struct reservedNode {
    reserved* reserved;
    struct reservedNode* left;
    struct reservedNode* right;
    int height;
} reservedNode;

int height(reservedNode* node);
int getBalance(reservedNode* node);
reservedNode* rotateRight(reservedNode* y);
reservedNode* rotateLeft(reservedNode* x);
reservedNode* createNode(char* name, char* pData);
reservedNode* insertNode(reservedNode* node, char* name, char* pData);
reservedNode* findNode(reservedNode* root, char* name);
void printInOrder(reservedNode* root);
void freeTree(reservedNode* root);

#endif 
