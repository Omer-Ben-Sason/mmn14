#include "buildFiles.h"



char* buildEntryFile(char* fileName,symbolNode* headIC,symbolNode* headDC)
{
    symbolNode* curr = headIC;
    int addr = 0;
    
    char* fileText = NULL;
    while(curr)
    {
        if (strcmp(curr->symbol->type,SYMBOL_ENT))
        {
            addr = searchSymbolTable(curr->symbol->data,headDC,headIC,1);
            sprintf("%s %s\n",curr->symbol->data,intToFourBase(addr));
        }
    }
}