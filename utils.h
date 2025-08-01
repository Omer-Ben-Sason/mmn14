#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedList.h"
#define LEN 81
#define EVLEVEN_BIT 11
#define MAX_PARAMS 10  

void freeWords(char** words);
char** breakLine(char* line);
char* loadLine(FILE* file);
char* intToBinary(int num);
symbolNode* searchSymbolTable(char* name, symbolNode* DC, symbolNode* IC,int searchIC);
char** breakToParams(char* line,int breakBrackets);
int isNumeric(char* str);
#endif 