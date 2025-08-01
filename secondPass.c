#include "secondPass.h"

memory replaceSymbolWithAddr(symbolNode** headIC,symbolNode** headDC)
{
    memory mem = {NULL,NULL};
    symbolNode* curr = *headIC,*paramNode = NULL;
    char* currData = NULL;    
    char* currBianry = CLEAR_MEM;
    int paramAddr = 0;
    while (curr)
    {
        currBianry = strdup(CLEAR_MEM);
        currData = curr->symbol->data;
        if (currData && ((currData[0] >= BIG_A_ASCII && currData[0] <= BIG_Z_ASCII) || (currData[0] >= SMALL_A_ASCII && currData[0] <= SMALL_Z_ASCII)))
        {
            if ((paramNode = searchSymbolTable(currData, *headDC, *headIC,0))&&strcmp(curr->symbol->type,SYMBOL_ENT)!=0)
            {
                paramAddr = paramNode->symbol->addr<<2;

                if (strcmp(paramNode->symbol->type,SYMBOL_DATA)==0)
                {
                    strcpy(currBianry,intToBinary(paramAddr));
                    currBianry[8]='1';
                    currBianry[9]='0';
                    curr->symbol->data = currBianry;
                }
                else if (strcmp(paramNode->symbol->type,SYMBOL_EXT)==0)
                {
                    currData = strdup(CLEAR_MEM);
                    currBianry[8]='0';
                    currBianry[9]='1';
                    curr->symbol->data = currBianry;
                }
            }
        }
        curr = curr->next;
    }
    mem.headDC = *headDC;
    mem.headIC = *headIC;
    return mem;
}

memory secondPass(symbolNode** headIC,symbolNode** headDC)
{
    return replaceSymbolWithAddr(headIC,headDC);
}