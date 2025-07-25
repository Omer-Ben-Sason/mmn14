#include "linkedList.h"
void addSymbol(symbolNode** head, char* name, int address, char* type, char* data)
{
    symbolNode* node = malloc(sizeof(symbolNode));
    symbolNode* temp = NULL;
    node->symbol = malloc(sizeof(symbol));
    node->symbol->name = name ? strdup(name) : NULL;
    node->symbol->addr = address;
    node->symbol->type = type ? strdup(type) : NULL;
    node->symbol->data = data ? strdup(data) : NULL;
    node->next = NULL;

    if (*head == NULL)
    {
        *head = node;
    }
    else
    {
        temp = *head;
        while (temp->next)
        {
            temp = temp->next;
        }
        temp->next = node;
    }
}
void insertSymbolSortedByAddr(symbolNode** head, char* name, int address, char* type, char* data)
{
    symbolNode* node = malloc(sizeof(symbolNode));
    symbolNode* curr =NULL;
    node->symbol = malloc(sizeof(symbol));
    node->symbol->name = name ? strdup(name) : NULL;
    node->symbol->addr = address;
    node->symbol->type = type ? strdup(type) : NULL;
    node->symbol->data = data ? strdup(data) : NULL;
    node->next = NULL;

    if (*head == NULL || (*head)->symbol->addr > address)
    {
        node->next = *head;
        *head = node;
        return;
    }

    curr= *head;
    while (curr->next && curr->next->symbol->addr < address)
    {
        curr = curr->next;
    }

    node->next = curr->next;
    curr->next = node;
}

void printSymbols(symbolNode* head)
{
    if (!head)
    {
        return;
    }
    printf("addr: %d\tname: %s\tdata: %s\ttype: %s\n",head->symbol->addr,(head->symbol->name)?head->symbol->name:"\0",(head->symbol->data)?head->symbol->data:"0",(head->symbol->type)?head->symbol->type:"\0");
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
