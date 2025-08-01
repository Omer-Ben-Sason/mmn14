#ifndef LINKED_LIST_H
#define LINKED_LIST_H


typedef struct symbol {
    char* name;
    int addr;
    char* type;
    char* data;
} symbol;

typedef struct symbolNode {
    symbol* symbol;
    struct symbolNode* next;
} symbolNode;
#include "utils.h"
void addSymbol(symbolNode** head, char* name, int address, char* type,char* data);
symbol* findSymbol(symbolNode* head, char* name);
void freeSymbolTable(symbolNode* head);
void printSymbols(symbolNode* head);
void insertSymbolSortedByAddr(symbolNode** head, char* name, int address, char* type, char* data);
#endif