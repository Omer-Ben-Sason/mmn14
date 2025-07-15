#include "firstPass.h"
symbolNode* buildSymbols(reservedNode* root,FILE* file)
{
    int DC = DC_START; /* 1 */
    int IC = IC_START;
    char** commands = NULL,*line = NULL;
    int lineNum = 0;
    int thereIsLabal = 0;
    reservedNode* rNode = NULL;
    char* inst = NULL,*labal = NULL;
    char* restOfLine = NULL;
    symbolNode* head = NULL;
    fseek(file,0,SEEK_SET);
    while((line = loadLine(file))!=NULL) /* 2 */
    {
        commands = breakLine(line);
        if ((labal=(checkIfLabal(commands[0],root)))) /* 3 */
        {
            thereIsLabal = 1;/* 4 */
        }
        inst = commands[1];
        rNode = findNode(root,inst);
        if (rNode && strncmp(rNode->reserved->name, DOT, 1) == 0)
        {
            if (strcmp(inst, SYMBOL_DATA) == 0 ||strcmp(inst, SYMBOL_MAT) == 0 ||strcmp(inst, SYMBOL_STRING) == 0)/* 5 */
            {
                if (thereIsLabal)/* 6 */
                {
                    restOfLine = line + strlen(commands[0]) + strlen(commands[1]) + 2;
                    putInDC(inst,restOfLine,lineNum,&head,labal,&DC,&IC);
                }
            }
            else if (strcmp(inst,SYMBOL_EXT)==0)/* 8 */
            {

            }
            else if (strcmp(inst,SYMBOL_ENT)==0)/* 8 */
            {
                continue;
            }
        }
    }
    return head;
}
char* checkIfLabal(char* labal, reservedNode* root)
{
    int len=0;

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

int putInDC(char* type, char* restOfLine, int lineNum, symbolNode** head, char* name, int* DC, int* IC)
{
    int count = 0;
    char* curr = restOfLine;
    char tmp[BUFF_SIZE]; 
    int i = 0;
    int rows = 0,cols = 0;
    char* cstr = NULL;
    if (strcmp(type, SYMBOL_DATA) == 0)
    {
        while (*curr != '\0')
        {
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
                    addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL, strdup(tmp));
                    count++;
                    name = NULL;
                }
                else
                {
                    printf("at line %d: invalid number: %s\n", lineNum, tmp);
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
                cstr = (char*)malloc(2);
                cstr[0] = *curr;
                cstr[1] = '\0';

                addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL, cstr);
                count++;
                name = NULL;
                curr++;
            }

            if (*curr == '"')
            {
                addSymbol(head, NULL, (*DC)++, type, strdup("0"));
                count++;
            }
            else
            {
                printf("at line %d: missing closing quote\n", lineNum);
            }
        }
        else
        {
            printf("at line %d: invalid string format\n", lineNum);
        }
    }

    else if (strcmp(type, SYMBOL_MAT) == 0)
    {
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
                    cols = atoi(tmp);
                }
                else
                {
                    printf("at line %d: invalid number of columns: %s\n", lineNum, tmp);
                }
            }
            
            while (*curr == ' ' || *curr == '\t') 
            {
                curr++; 
            }

            for (i = 0; i < rows * cols; i++)
            {
                addSymbol(head, name, (*DC)++, type, 0);
            }

            count = 0;
            while (*curr != '\0' && count < rows * cols)
            {
                while (*curr == ' ' || *curr == '\t' || *curr == COMMA_ASCII)
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
                    if (isNumeric(tmp))
                    {
                        addSymbol(head, name, (*DC)++, type, strdup(tmp));
                        count++;
                    }
                    else
                    {
                            printf("at line %d: invalid matrix value: %s\n", lineNum, tmp);
                    }
                }

                if (count != rows * cols)
                {
                    printf("at line %d: matrix dimensions do not match the number of values\n", lineNum);
                }
            }
        }
        else
        {
            printf("at line %d: invalid mat format\n", lineNum);
        }
    }
    return count;
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
symbolNode* firstPass(reservedNode* root,FILE* file)
{
    return buildSymbols(root,file);
}
