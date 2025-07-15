#include "linkedList.h"
void addSymbol(symbolNode** head, char* name, int address, char* type,char* data)
{
    symbolNode* node = (symbolNode*)malloc(sizeof(symbolNode));
    node->symbol = (symbol*)malloc(sizeof(symbol));
    node->symbol->name = name ? strdup(name) : NULL;
    node->symbol->addr = address;
    node->symbol->type = type;
    node->symbol->data = data;
    node->next = *head;
    *head = node;
}
void printSymbols(symbolNode* head)
{
    if (!head)
    {
        return;
    }
    printf("%d %s\t%s\t%s",head->symbol->addr,(head->symbol->name)?head->symbol->name:"\0",head->symbol->data,(head->symbol->type)?head->symbol->type:"\0");
    printSymbols(head->next);
}

symbol* findSymbol(symbolNode* head,char* name)
{
    while (head) 
    {
        if (strcmp(head->symbol->name, name) == 0)
        {
            return head->symbol;
        }
        head = head->next;
    }
    return 0;
}

void freeSymbolTable(symbolNode* head)
{
    while (head) 
    {
        symbolNode* next = head->next;
        free(head->symbol->name);
        free(head->symbol);
        free(head);
        head = next;
    }
}
