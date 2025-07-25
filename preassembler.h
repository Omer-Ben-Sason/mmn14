#ifndef PREASSEMBLER_H
#define PREASSEMBLER_H

#include "binaryTree.h"
#include "utils.h"
#include <stdio.h>


#define LEN 81
#define EOL '\n'
#define EVLEVEN_BIT 11
#define MCRO_START "mcro"
#define MCRO_END "mcroend"
#define FOUR_BYTES 4
#define ZERO_ASCII '0'
#define SEVEN_ASCII '7'
char* intToBinary(int num);
void initReservedWords(reservedNode** root);
char* loadLine(FILE* file);
char** breakLine(char* line);
void replaceMcro(FILE* file, reservedNode* root);
void freeWords(char** words);
void findMcro(FILE* file, reservedNode* root, char* outputFileName);
FILE* preassmble(FILE* inputFile, char* inputFileName, reservedNode* root);

#endif 
