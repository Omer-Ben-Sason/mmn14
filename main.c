#include "main.h"

int main(int argc,char** argv)
{
    reservedNode* root = NULL; /* root of macro tree */
    FILE* outputFileAm = NULL;
    memory mem = {0};
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
    mem = firstPass(root,outputFileAm);
    printf("\n\n\n");
    printSymbols(mem.headDC);
    printSymbols(mem.headIC);
    free(inputFileName);
    fclose(outputFileAm);
    fclose(inputFile);
    free(root);
    free(mem.headDC);
    free(mem.headIC);
    return 0;
}