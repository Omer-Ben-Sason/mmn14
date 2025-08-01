#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include "preassembler.h"
#include "linkedList.h"
#include "utils.h"

#define DC_START 0
#define IC_START 100
#define DOT '.'
#define FOUR_BITS 4
#define SYMBOL_DATA ".data"
#define SYMBOL_MAT ".mat"
#define SYMBOL_STRING ".string"
#define SYMBOL_EXT ".extern"
#define SYMBOL_ENT ".entry"
#define SYMBOL_CODE "code"
#define REGISTER_SYMBOL "register"
#define LABAL_LEN 31
#define COLOM ':'
#define BUFF_SIZE 81
#define ZERO_ASCII '0'
#define NINE_ASCII '9'
#define LEFT_BRACKET '['
#define RIGHT_BRACKET ']'
#define MINUS_ASCII '-'
#define PLUS_ASCII '+'
#define CLEAR_MEM "0000000000"
#define TEN_BITS 10
#define SIX_BITS 6
#define EIGHT_BITS 8
#define COMMA_ASCII ','
#define ASCII_BUF_SIZE 12
#define MAX_PARAMS 10  
#define ONE_ASCII '1'
#define TWO_ASCII '2'
#define THREE_ASCII '3'
#define BIG_A_ASCII 'A'
#define BIG_Z_ASCII 'Z'
#define SMALL_A_ASCII 'a'
#define SMALL_Z_ASCII 'z'

typedef struct memory {
    symbolNode* headDC;
    symbolNode* headIC;
} memory;
int checkIfValidMatParam(char* params);
memory firstPass(reservedNode* root, FILE* file);
int isNumeric(char* str);
int putInDC(char* type, char* restOfLine, int lineNum, symbolNode** head, char* name, int* DC);
char* checkIfLabal(char* labal, reservedNode* root,int lineNum,symbolNode* DC,symbolNode* IC);
memory buildSymbols(reservedNode* root, FILE* file);
void putInIC(int* IC, char* restOfLine, int lineNum, symbolNode** head,char* name,reservedNode* root,char** lastReg,char* inst);
char* countRestOfLine(char* line,int isLabal);
int handleData(char* curr, int lineNum, symbolNode** head, char* name, int* DC, char* type);
int handleString(char* curr, int lineNum, symbolNode** head, char* name, int* DC);
int handleMat(char* curr, int lineNum, symbolNode** head, char* name, int* DC);

#endif 
