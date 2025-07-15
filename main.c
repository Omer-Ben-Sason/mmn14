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
    {
        char* fullName = malloc(strlen(inputFileName) + FOUR_BYTES);
        if (!fullName)
        {
            printf("Memory allocation failed\n");
            free(inputFileName);
            return 1;
        }
        sprintf(fullName, "%s.as", inputFileName);
        inputFile = fopen(fullName, "r");
        free(fullName);
    }
    outputFileAm = preassmble(inputFile,inputFileName,root);
    symbolNodeHead = firstPass(root,outputFileAm);
    printSymbols(symbolNodeHead);
    free(inputFileName);
    return 0;
}
