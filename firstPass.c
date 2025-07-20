#include "firstPass.h"

int foundErrorFirstPass = 0; 
symbolNode* buildSymbols(reservedNode* root, FILE* file)
{
    int DC = DC_START;
    int IC = IC_START;
    char** commands = NULL, *line = NULL;
    int lineNum = 0;
    int thereIsLabal = 0;
    reservedNode* rNode = NULL;
    char* inst = NULL, *labal = NULL;
    char* restOfLine = NULL;
    symbolNode* head = NULL;
    fseek(file, 0, SEEK_SET);

    while ((line = loadLine(file)) != NULL)
    {
        lineNum++;
        commands = breakLine(line);

        if ((labal = (checkIfLabal(commands[0], root))))
        {
            thereIsLabal = 1;
        }

        inst = commands[1];
        rNode = findNode(root, inst);
        printf("%s\n",inst);
        if (rNode && strncmp(rNode->reserved->name, DOT, 1) == 0)
        {
            if (strcmp(inst, SYMBOL_DATA) == 0 || strcmp(inst, SYMBOL_MAT) == 0 || strcmp(inst, SYMBOL_STRING) == 0)
            {
                if (thereIsLabal)
                {
                    restOfLine = line + strlen(commands[0]) + strlen(commands[1]) + 2;
                    putInMem(inst, restOfLine, lineNum, &head, labal, &DC, &IC);
                }
            }
            else if (strcmp(inst, SYMBOL_EXT) == 0)
            {
                addSymbol(&head, labal, DC, SYMBOL_EXT, 0);
            }
            else if (strcmp(inst, SYMBOL_ENT) == 0)
            {
                continue;
            }
            else if (findNode(root, inst)&&strcmp(findNode(root, inst)->reserved->type, "code") == 0)
            {
                putInIC(&IC, commands, lineNum, &head);
            }
            else
            {
                printf("at line %d: invalid directive: %s\n", lineNum, inst);
                foundErrorFirstPass = 1;
            }
        }
        

        thereIsLabal = 0;

        free(commands);
        free(line);
    }

    return head;
}

char* checkIfLabal(char* labal, reservedNode* root)
{
    int len = 0;

    if (!labal || (len = strlen(labal)) >= LABAL_LEN)
    {
        return NULL;
    }

    if (labal[len - 1] != COLOM)
    {
        return NULL;
    }

    labal[len - 1] = '\0';

    if (findNode(root, labal))
    {
        return NULL;
    }

    return labal;
}

int putInMem(char* type, char* restOfLine, int lineNum, symbolNode** head, char* name, int* DC, int* IC)
{
    int count = 0;
    char* curr = restOfLine;
    char* commanChecker = curr;
    char tmp[BUFF_SIZE] = {0};
    char asciiStr[ASCII_BUF_SIZE] ={0};
    int i = 0;
    int rows = 0, cols = 0;
    int lastComma = 0;
    if (strcmp(type, SYMBOL_DATA) == 0)
    {
        while (1)
        {
            
            while (commanChecker && *commanChecker != EOL && *commanChecker != EOF && *commanChecker != 0)
            {
                if (*commanChecker == COMMA_ASCII)
                {
                    if (lastComma)
                    {
                        printf("at line %d: multiple commas found\n", lineNum);
                        foundErrorFirstPass = 1;
                    }
                    lastComma = 1;
                }
                else if (*commanChecker != ' ' && *commanChecker != '\t')
                {
                    lastComma = 0; 
                }

                commanChecker++;
            }
            if(*curr == EOL||*curr == EOF||*curr == 0)
            {
                break;
            }

            while (*curr == ' ' || *curr == '\t')
            {
                curr++;
            }

            i = 0;
            while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == '-' || *curr == '+')
            {
                tmp[i++] = *curr;
                curr++;
            }
            if (i > 0)
            {

                tmp[i] = '\0';
                if (isNumeric(tmp))
                {
                    addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL, tmp);
                    count++;
                    name = NULL;
                }
                else
                {
                    printf("at line %d: invalid number: %s\n", lineNum, tmp);
                    foundErrorFirstPass = 1; 
                }
            }
            while (*curr == ' ' || *curr == '\t' || *curr == COMMA_ASCII)
            {
                curr++;
            }
            
            i = 0;
            while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == '-' || *curr == '+')
            {
                tmp[i++] = *curr;
                curr++;
            }

            if (i > 0)
            {
                tmp[i] = '\0';
                if (isNumeric(tmp))
                {
                    addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL, tmp);
                    count++;
                    name = NULL;
                }
                else
                {
                    printf("at line %d: invalid number: %s\n", lineNum, tmp);
                    foundErrorFirstPass = 1;
                }
            }

            while (*curr == ' ' || *curr == '\t')
            {
                curr++;
            }
        }
    }
    else if (strcmp(type, SYMBOL_STRING) == 0)
    {
        while (*curr == ' ' || *curr == '\t')
        {
            curr++;
        }

        if (*curr == '"')
        {
            curr++;
            while (*curr != '"' && *curr != '\0')
            {
                if (*curr == EOL || *curr == EOF)
                {
                    printf("at line %d: missing closing quote\n", lineNum);
                    foundErrorFirstPass = 1;
                }
                sprintf(asciiStr, "%d", *curr);
                
                addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL, strdup(asciiStr));
                count++;
                name = NULL;
                curr++;
            }

            if (*curr == '"')
            {
                addSymbol(head, NULL, (*DC)++, type, 0);
                count++;
            }
            else
            {
                printf("at line %d: missing closing quote\n", lineNum);
                foundErrorFirstPass = 1;
            }
        }
        else
        {
            printf("at line %d: invalid string format\n", lineNum);
            foundErrorFirstPass = 1;
        }
    }

    else if (strcmp(type, SYMBOL_MAT) == 0)
    {
        while (commanChecker && *commanChecker != EOL && *commanChecker != EOF && *commanChecker != 0)
        {
            if (*commanChecker == COMMA_ASCII)
            {
                if (lastComma)
                {
                    printf("at line %d: multiple commas found\n", lineNum);
                    foundErrorFirstPass = 1;
                }
                lastComma = 1;
            }
            else if (*commanChecker != ' ' && *commanChecker != '\t')
            {
                lastComma = 0; 
            }

            commanChecker++;
        }
        while (*curr == ' ' || *curr == '\t')
        {
            curr++;
        }
        if (*curr == LEFT_BRACKET)
        {
            curr++;
            i = 0;

            while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == '-' || *curr == '+')
            {
                tmp[i++] = *curr;
                curr++;
            }
            curr++;
            if (i > 0)
            {
                tmp[i] = '\0';
                if (isNumeric(tmp))
                {
                    rows = atoi(tmp);
                }
                else
                {
                    printf("at line %d: invalid number of rows: %s\n", lineNum, tmp);
                    foundErrorFirstPass = 1;
                }
            }
            while (*curr == ' ' || *curr == '\t'|| *curr == LEFT_BRACKET)
            {
                curr++;
            }            
            i = 0;
            while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == '-' || *curr == '+')
            {
                tmp[i] = *curr;
                curr++;
                i++;
                
            }
            if (i > 0)
            {
                tmp[i] = '\0';
                if (isNumeric(tmp))
                {
                    cols = atoi(tmp);
                }
                else
                {
                    printf("at line %d: invalid number of columns: %s\n", lineNum, tmp);
                    foundErrorFirstPass = 1;
                }
            }
            curr++;
            while (*curr == ' ' || *curr == '\t')
            {
                curr++;
            }
            count = 0;

            while (1)
            {
                if (*curr == EOL||*curr == EOF||*curr == 0 ||  count >= rows * cols)
                {
                    break;
                }
                
                while (*curr == ' ' || *curr == '\t'|| *curr == COMMA_ASCII)
                {
                    curr++;
                }
                

                i = 0;
                

                while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == MINUS_ASCII || *curr == PLUS_ASCII)
                {
                    tmp[i++] = *curr;
                    curr++;
                }
                
                if (i > 0)
                {
                    tmp[i] = '\0';
                    putInMem(SYMBOL_DATA, tmp, lineNum, head, name, DC, IC);
                    name = NULL;  
                    count++;          
                }
            }
            if (count < rows * cols)
            {
                while (count < rows * cols)
                {
                    addSymbol(head, (name) ? name : NULL, (*DC)++, SYMBOL_DATA, 0);
                    count++;
                    name = NULL;
                }
            }
        }
        else
        {
            printf("at line %d: invalid mat format\n", lineNum);
            foundErrorFirstPass = 1;
        }
    }
    if (*(curr - 1) == COMMA_ASCII)
    {
        printf("at line %d: trailing comma without value\n", lineNum);
        foundErrorFirstPass = 1;
    }
    return count;
}
void putInIC(int* IC, char** line, int lineNum, symbolNode** head)
{
    int i = 0;
    printf("11111111111");
    for (i=0;line[i]!=NULL;i++)
    {
        printf("11111111111");
    }
    printf("\n");
    
}
int isNumeric(char* str)
{
    if (*str == '+' || *str == '-')
    {
        str++;
    }
    if (*str == '\0')
    {
        return 0;
    }
    while (*str)
    {
        if (*str < '0' || *str > '9')
        {
            return 0;
        }
        str++;
    }
    return 1;
}

symbolNode* firstPass(reservedNode* root, FILE* file)
{
    if (foundErrorFirstPass)
    {
        printf("Errors found in first pass, aborting symbol table creation.\n");
        return NULL;
    }
    else
    {
        return buildSymbols(root, file);
    }
}