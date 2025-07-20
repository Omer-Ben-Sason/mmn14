#include "main.h"

int main(int argc,char** argv)
{
    reservedNode* root = NULL; /* root of macro tree */
    FILE* outputFileAm = NULL;
    symbolNode* symbolNodeHead = NULL;
    FILE* inputFile = NULL;
    char* inputFileName = malloc(100); 
    if (!inputFileName)
    {
        printf("Memory allocation failed\n");
        return 1;
    }
    initReservedWords(&root);
    strcpy(inputFileName, "file"); 
    inputFile = fopen(strcat(strdup(inputFileName),".as"),"r"); 
    outputFileAm = preassmble(inputFile,inputFileName,root);
    symbolNodeHead = firstPass(root,outputFileAm);
    printf("\n\n\n");
    printSymbols(symbolNodeHead);
    free(inputFileName);
    fclose(outputFileAm);
    fclose(inputFile);
    free(root);
    free(symbolNodeHead);
    return 0;
}