#include "binaryTree.h"

int height(reservedNode* node)
{
    return node ? node->height : 0;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int getBalance(reservedNode* node)
{
    return node ? height(node->left) - height(node->right) : 0;
}

reservedNode* rotateRight(reservedNode* y)
{
    reservedNode* x = y->left;
    reservedNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

reservedNode* rotateLeft(reservedNode* x)
{
    reservedNode* y = x->right;
    reservedNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

reservedNode* createNode(char* name, char* pData,char* type,char* opDst,char* opSrc,char* binary)
{
    reservedNode* node = (reservedNode*)malloc(sizeof(reservedNode));
    node->reserved = (reserved*)malloc(sizeof(reserved));

    node->reserved->name = strdup(name);
    node->reserved->pData = pData ? strdup(pData) : 0;
    node->reserved->type = type ? strdup(type) : 0;
    node->reserved->opDst = opDst ? strdup(opDst) : 0;
    node->reserved->opSrc = opSrc ? strdup(opSrc) : 0;
    node->reserved->binary = binary ? strdup(binary) : 0;
    node->left = 0;
    node->right = 0;
    node->height = 1;

    return node;
}

reservedNode* insertNode(reservedNode* node, char* name, char* pData, char* type, char* opDst, char* opSrc,char* binary)
{
    int cmp = 0,balance=0;

    if (!node)
    {
        return createNode(name, pData, type, opDst, opSrc, binary);
    }

    cmp = strcmp(name, node->reserved->name);

    if (cmp < 0)
    {
        node->left = insertNode(node->left, name, pData,type, opDst, opSrc, binary);
    }
    else if (cmp > 0)
    {
        node->right = insertNode(node->right, name, pData, type, opDst, opSrc, binary);
    }
    else
    {
        return node; 
    }

    node->height = 1 + max(height(node->left), height(node->right));
    balance = getBalance(node);

    if (balance > 1 && strcmp(name, node->left->reserved->name) < 0)
        return rotateRight(node);

    if (balance < -1 && strcmp(name, node->right->reserved->name) > 0)
        return rotateLeft(node);

    if (balance > 1 && strcmp(name, node->left->reserved->name) > 0)
    {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }

    if (balance < -1 && strcmp(name, node->right->reserved->name) < 0)
    {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

reservedNode* findNode(reservedNode* root, char* name)
{
    int cmp = 0;
    if (!root)
    {
        return 0;
    }
    cmp = strcmp(name, root->reserved->name);

    if (cmp == 0)
    {
        return root;
    }
    else if (cmp < 0)
    {
        return findNode(root->left, name);
    }
    else
    {
        return findNode(root->right, name);
    }
}

void printInOrder(reservedNode* root)
{
    if (!root)
    {
        return;
    }
    printInOrder(root->left);
    printf("%s %s %s\n", root->reserved->name, root->reserved->type ? root->reserved->type : "NULL", 
           root->reserved->binary ? root->reserved->binary : "NULL");
    printInOrder(root->right);
}

void freeTree(reservedNode* root)
{
    if (!root)
    {
        return;
    }

    freeTree(root->left);
    freeTree(root->right);

    free(root->reserved->name);
    free(root->reserved->pData);
    free(root->reserved);
    free(root);
}