#include "utils.h"



/* 
 * Frees dynamically allocated array of strings 
 * Input: array of strings
 * Output: none
 */
void freeWords(char** words) 
{
    int i = 0;
    if (words) 
    {
        for (i = 0; words[i] != NULL; i++) 
        {
            free(words[i]); /* free each word */
        }
        free(words); /* free array */
    }
}

/* 
 * Splits a line into tokens (words), returns a dynamically allocated array of words 
 * Input: string line
 * Output: dynamically allocated array of strings (tokens)
 */
char** breakLine(char* line) 
{
    int capacity = 10; /* initial capacity for words array */
    int numWords = 0; /* number of words found */
    char* token = NULL, *tmp = strdup(line); /* temporary line copy for tokenizing */
    char** words = (char**)malloc(capacity * sizeof(char*)); /* array of word pointers */

    if (!words) 
    {
        printf("Memory allocation failed\n");
        return NULL;
    }

    token = strtok(tmp, " \t\n"); /* first token */
    while (token != NULL) 
    {
        if (numWords >= capacity) 
        {
            capacity *= 2; /* increase capacity */
            words = (char**)realloc(words, capacity * sizeof(char*));
            if (!words) 
            {
                printf("Memory reallocation failed\n");
                return NULL;
            }
        }

        words[numWords] = (char*)malloc(strlen(token) + 1); /* allocate memory for word */
        if (!words[numWords]) 
        {
            printf("Memory allocation failed for word\n");
            return NULL;
        }
        strcpy(words[numWords], token); /* copy token */
        numWords++;

        token = strtok(NULL, " \t\n"); /* next token */
    }

    free(tmp);
    words[numWords] = NULL; /* mark end of words */

    return words;
}

/* 
 * Reads a line from a file and returns a dynamically allocated string 
 * Input: pointer to input file
 * Output: dynamically allocated line string or NULL
 */
char* loadLine(FILE* file)
{
    char* line = (char*)malloc(LEN * sizeof(char)); /* buffer to hold the line */
    if (!line)
    {
        printf("Memory allocation failed\n");
        return 0;
    }

    if (fgets(line, LEN, file)) /* read line from file */
    {
        return line;
    }

    free(line);
    return NULL;
}
/* 
 * turns an integer into its binary representation
 * Input: integer number
 *  * Output: integer representing the binary number
 */
char* intToBinary(int num)
{
    char* binary = (char*)malloc(EVLEVEN_BIT); /* 10 bits + '\0' */
    int bits = EVLEVEN_BIT-1;
    unsigned int mask = (1 << bits) - 1; 
    int i = 0;

    if (!binary)
    {
        return NULL;
    }

    num &= mask; 

    binary[bits] = '\0';
    for (i = bits - 1; i >= 0; i--)
    {
        binary[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }

    return binary;
}

/*
the func: breakToParams
input: a line of text, and a flag whether to break by brackets or not
output: an array of parameter strings (tokens), NULL-terminated
*/
char** breakToParams(char* line, int breakBrackets) /* main tokenizer */
{
    int paramCount = 0; /* number of tokens */
    char* token = NULL; /* current token */
    char* lineCopy = NULL; /* copy of the line */
    char** params = (char**)malloc(MAX_PARAMS * sizeof(char*)); /* array of tokens */

    if (!params) /* malloc check for params */
    {
        printf("Memory allocation failed for params\n"); /* error message */
        return NULL; /* return on fail */
    }

    lineCopy = strdup(line); /* duplicate line */
    if (!lineCopy) /* malloc check for line copy */
    {
        printf("Memory allocation failed for line copy\n"); /* error message */
        free(params); /* free previously allocated params */
        return NULL; /* return on fail */
    }

    if (breakBrackets) /* decide delimiter set */
    {
        token = strtok(lineCopy, ",[] \t"); /* split with brackets */
    }
    else
    {
        token = strtok(lineCopy, ", \t"); /* split without brackets */
    }

    while (token != NULL && paramCount < MAX_PARAMS) /* loop tokens */
    {
        params[paramCount] = (char*)malloc(strlen(token) + 1); /* allocate for token */
        if (!params[paramCount]) /* malloc check */
        {
            printf("Memory allocation failed for param\n"); /* error message */
            break; /* exit loop */
        }

        strcpy(params[paramCount], token); /* copy token */
        paramCount++; /* next index */

        if (breakBrackets) /* next token with brackets */
        {
            token = strtok(NULL, ",[] \t"); /* continue split */
        }
        else
        {
            token = strtok(NULL, ", \t"); /* continue split */
        } 
    }

    params[paramCount] = NULL; /* null-terminate array */
    free(lineCopy); /* free duplicated line */
    return params; /* return array */
}
/*
the func: searchSymbolTable
input: label name, head of DC and IC symbol lists
output: pointer to symbol node if found, otherwise NULL
*/
symbolNode* searchSymbolTable(char* name, symbolNode* DC, symbolNode* IC,int searchIC) /* search for label in symbol tables */
{
    symbolNode* curr = DC; /* start with DC list */

    while (curr) /* iterate over DC */
    {
        if (curr->symbol->name && strcmp(curr->symbol->name, name) == 0) return curr; /* found match in DC */
        curr = curr->next; /* go to next */
    }
    if (searchIC)
    {
        curr = IC; /* switch to IC list */

        while (curr) /* iterate over IC */
        {
            if (curr->symbol->name && strcmp(curr->symbol->name, name) == 0) return curr; /* found match in IC */
            curr = curr->next; /* go to next */
        }
    }
    return NULL; /* not found */
}
/* 
the func: isNumeric
input: a string
output: 1 if numeric, else 0
*/
int isNumeric(char* str) /* checks if a string is a valid integer */
{
    if (*str == '+' || *str == '-') /* optional sign */
    {
        str++; /* skip sign */
    }

    if (*str == '\0') /* string was only '+' or '-' */
    {
        return 0; /* not numeric */
    }

    while (*str) /* loop through each character */
    {
        if (*str < '0' || *str > '9') /* not a digit */
        {
            return 0; /* invalid */
        }
        str++; /* next character */
    }

    return 1; /* valid number */
}
