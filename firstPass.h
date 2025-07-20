#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "preassembler.h"
#include "linkedList.h"
#include "utils.h"

#define DC_START 0
#define IC_START 100
#define DOT "."
#define SYMBOL_DATA ".data"
#define SYMBOL_MAT ".mat"
#define SYMBOL_STRING ".string"
#define SYMBOL_EXT ".extern"
#define SYMBOL_ENT ".entry"
#define SYMBOL_CODE "code"

#define LABAL_LEN 31
#define COLOM ':'
#define BUFF_SIZE 81
#define ZERO_ASCII '0'
#define NINE_ASCII '9'
#define LEFT_BRACKET '['
#define RIGHT_BRACKET ']'
#define MINUS_ASCII '-'
#define PLUS_ASCII '+'
#define COMMA_ASCII ','
#define ASCII_BUF_SIZE 12

symbolNode* firstPass(reservedNode* root, FILE* file);
int isNumeric(char* str);
int putInMem(char* type, char* restOfLine, int lineNum, symbolNode** head, char* name, int* DC, int* IC);
char* checkIfLabal(char* labal, reservedNode* root);
symbolNode* buildSymbols(reservedNode* root, FILE* file);
void putInIC(int* IC, char** line, int lineNum, symbolNode** head);
#endif 
