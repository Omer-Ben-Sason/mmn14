#include "firstPass.h"

int foundErrorFirstPass = 0; 
char** breakToParams(char* line, int breakBrackets) 
{
    int paramCount = 0;
    char* token = NULL;
    char* lineCopy = NULL;
    char** params = (char**)malloc(MAX_PARAMS * sizeof(char*)); 

    if (!params) 
    {
        printf("Memory allocation failed for params\n");
        return NULL;
    }

    lineCopy = strdup(line);
    if (!lineCopy)
    {
        printf("Memory allocation failed for line copy\n");
        free(params);
        return NULL;
    }

    if (breakBrackets)
    {
        token = strtok(lineCopy, ",[] \t"); 
    }
    else
    {
        token = strtok(lineCopy, ", \t"); 
    }

    while (token != NULL && paramCount < MAX_PARAMS) 
    {
        params[paramCount] = (char*)malloc(strlen(token) + 1);  
        if (!params[paramCount])
        {
            printf("Memory allocation failed for param\n");
            break;
        }

        strcpy(params[paramCount], token);  
        paramCount++;

        if (breakBrackets)
        {
            token = strtok(NULL, ",[] \t"); 
        }
        else
        {
            token = strtok(NULL, ", \t"); 
        } 
    }

    params[paramCount] = NULL; 
    free(lineCopy);
    return params;
}

memory buildSymbols(reservedNode* root, FILE* file)
{
    int i = 0;
    int DC = DC_START;
    int IC = IC_START;
    char** commands = NULL, *line = NULL;
    int lineNum = 0;
    int thereIsLabal = 0;
    reservedNode* rNode = NULL;
    char* inst = NULL, *labal = NULL;
    memory mem = {0};
    symbolNode* headDC = NULL,*headIC = NULL;
    int inCode = 0;
    char* lastReg = malloc(TEN_BITS + 1);
    for (i = 0; i < TEN_BITS; i++)
    {
        lastReg[i] = '0';
    }
    lastReg[TEN_BITS] = '\0';
    fseek(file, 0, SEEK_SET);

    while ((line = loadLine(file)) != NULL&&*line!=EOF&&*line!=0)
    {
        lineNum++;
        if (line == NULL || *line == '\0' || strspn(line, " \t\r\n") == strlen(line))     
        {
            continue;
        }
        commands = breakLine(line);
        
        if ((labal = checkIfLabal(commands[0], root)))
        {
            thereIsLabal = 1;
        }
        inst = thereIsLabal ? commands[1] : commands[0];

        rNode = findNode(root, inst);
        if (rNode && *(rNode->reserved->name) == DOT &&!inCode)
        {
            if (strcmp(inst, SYMBOL_DATA) == 0 || strcmp(inst, SYMBOL_MAT) == 0 || strcmp(inst, SYMBOL_STRING) == 0)
            {
                if (thereIsLabal)
                {
                    putInDC(inst, countRestOfLine(line,thereIsLabal), lineNum, &headDC, labal, &DC);
                }
            }
            else if (strcmp(inst, SYMBOL_EXT) == 0)
            {
                addSymbol(&headDC, labal, DC, SYMBOL_EXT, 0);
            }
            else if (strcmp(inst, SYMBOL_EXT) == 0)
            {
                continue;
            }
            else
            {
                printf("at line %d: invalid directive: %s\n", lineNum, inst);
                foundErrorFirstPass = 1;
            }
        }
        else if (findNode(root, inst)&& strcmp(findNode(root, inst)->reserved->type, SYMBOL_CODE) == 0)
        {
            putInIC(&IC, countRestOfLine(line,thereIsLabal), lineNum, &headIC, labal, root, &lastReg,inst);
        }
        else
        {
            putInIC(&IC, countRestOfLine(line,thereIsLabal), lineNum, &headIC, labal, root, &lastReg,inst);
        }
        

        thereIsLabal = 0;

    }
    if (commands) free(commands);
    if (line) free(line);
    mem.headDC = headDC;
    mem.headIC = headIC;
    return mem;
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

int putInDC(char* type, char* restOfLine, int lineNum, symbolNode** head, char* name, int* DC)
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
                    addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL, intToBinary(atoi(tmp)));
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
                    addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL, intToBinary(atoi(tmp)));
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
                
                addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL,  intToBinary(atoi(strdup(asciiStr))));
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
                    putInDC(SYMBOL_DATA, tmp, lineNum, head, name, DC);
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
void putInIC(int* IC, char* restOfLine, int lineNum, symbolNode** head, char* name, reservedNode* root, char** lastReg,char* inst)
{
    int i = 0, j = 0;
    char** params = breakToParams(restOfLine,1),**paramsBackets = breakToParams(restOfLine,0);
    reservedNode* rNode = NULL, * rNodeNext = NULL;
    char* currBinary = NULL,*currBinaryInst = NULL;
    char fullRegBin[TEN_BITS + 1];
    int currParamMat = 0;
    int icStart=*IC;
    char addressingTypeSrc = -1,addressingTypeDst = -1;
    char* addressingTypeString = NULL;
    char regBin[TEN_BITS + 1];
    char* currOpDst = NULL,*currOpSrc = NULL,*currName = name;
    (*IC)++;
    if (findNode(root,inst)->reserved->type && strcmp(findNode(root,inst)->reserved->type, SYMBOL_CODE) == 0)
    {
        currBinary = strdup(CLEAR_MEM);
        currOpSrc = findNode(root,inst)->reserved->opSrc;
        currOpDst = findNode(root,inst)->reserved->opDst;
        for (j = 0; j < FOUR_BITS; j++)
        {
            currBinary[j] = findNode(root,inst)->reserved->binary[j+SIX_BITS];
        }
        currBinary[TEN_BITS]= '\0';
        currBinaryInst = currBinary;

    }
    for (i = 0; params[i] != NULL; i++)
    {
        printf("%s",params[i]);
        if ((currParamMat = checkIfValidMatParam(params[i])) == 0)
        {
            printf("at line %d: invalid mat format, expected [rows][cols]\n", lineNum);
            foundErrorFirstPass = 1;
        }

        if (params[i] && strchr(params[i], '\n'))
        {
            *strchr(params[i], '\n') = '\0';
        }
        
        if ((*params[i]=='#') && isNumeric(params[i]+1))
        {
            addSymbol(head, name, (*IC)++, SYMBOL_CODE,intToBinary(atoi(params[i]+1)));
            if (!addressingTypeSrc)
            {
                addressingTypeSrc = ZERO_ASCII;
            }
            else
            {
                addressingTypeDst = ZERO_ASCII;
            }        
         }
        else if ((rNode = findNode(root, params[i])))
        {
            if (rNode->reserved->type && strcmp(rNode->reserved->type, REGISTER_SYMBOL) == 0)
            {
                currBinary = rNode->reserved->binary + SIX_BITS;
                if (params[i + 1])
                {
                    rNodeNext = findNode(root, params[i + 1]);
                }
                if (rNodeNext && rNodeNext->reserved && strcmp(rNodeNext->reserved->type, REGISTER_SYMBOL) == 0)
                {
                    strncpy(fullRegBin, currBinary, FOUR_BITS);
                    strncpy(fullRegBin + FOUR_BITS, rNodeNext->reserved->binary + SIX_BITS, FOUR_BITS);
                    fullRegBin[8] = '0';
                    fullRegBin[9] = '0';
                    fullRegBin[10] = '\0';
                    if (name)
                    {
                        name = NULL;
                    }
                    addSymbol(head, name, (*IC)++, SYMBOL_CODE, strdup(fullRegBin));
                    i++; /* skip next param (already used) */
                }
                else
                {
                    /* write just one register */
                    if (!addressingTypeSrc)
                    {
                        addressingTypeDst = THREE_ASCII;
                    }
                    else
                    {
                        addressingTypeSrc = THREE_ASCII;
                    }                    
                    strcpy(regBin, CLEAR_MEM);
                    strncpy(regBin, currBinary, 4);
                    for (j = FOUR_BITS; j < TEN_BITS; j++) regBin[j] = '0';
                    regBin[10] = '\0';
                    if (name)
                    {
                        name = NULL;
                    }
                    addSymbol(head, name, (*IC)++, SYMBOL_CODE, strdup(regBin));
                }
            }
        }


        else if (!rNode)
        {

            if (!addressingTypeSrc)
            {
                addressingTypeSrc = ONE_ASCII;
            }
            else
            {
                addressingTypeDst = ONE_ASCII;
            } 
            addSymbol(head, NULL, (*IC)++, SYMBOL_CODE, strdup(params[i]));
        }
        
        printf("ddddddd %d\n",addressingTypeSrc);

        if (addressingTypeSrc==ONE_ASCII)
        {
            
            currParamMat = checkIfValidMatParam(paramsBackets[0]);
            printf("%d %s\n",currParamMat,paramsBackets[0]);
        }
        else
        {
            currParamMat = checkIfValidMatParam(paramsBackets[1]);
        }

        if (currParamMat==1)
        {
            if (addressingTypeDst)
            {
                addressingTypeDst = TWO_ASCII;
            }
            else
            {
                addressingTypeSrc = TWO_ASCII;
            } 
        }
        if (currBinaryInst)
        {
            addressingTypeString = intToBinary(addressingTypeDst-ZERO_ASCII);
            printf("at line %d: %s\n",lineNum,addressingTypeString);

            if (currOpDst&&strchr(currOpDst,addressingTypeDst))
            {               
                currBinaryInst[6] = addressingTypeString[8];
                currBinaryInst[7] = addressingTypeString[9];
            }
            else if (currOpDst)
            {
                printf("at line %d: addressing type of the dst is invalid\n",lineNum);
                foundErrorFirstPass = 1;
            }
            addressingTypeString = intToBinary(addressingTypeSrc-ZERO_ASCII);

            if (currOpSrc&&strchr(currOpSrc,addressingTypeSrc))
            {
                currBinaryInst[4] = addressingTypeString[8];
                currBinaryInst[5] = addressingTypeString[9];
            }
            else if (currOpSrc)
            {
                printf("at line %d: addressing type of the src is invalid\n",lineNum);
                foundErrorFirstPass = 1;
            }


        }
        free(params[i]);
        free(rNode);
        printf("ddddddd %d\n",addressingTypeSrc);
            printf("\n\n\n");

    }
    insertSymbolSortedByAddr(head, currName, icStart, SYMBOL_CODE, strdup(currBinaryInst));

    free(params);
    free(addressingTypeString);

    printf("\n\n\n");

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

memory firstPass(reservedNode* root, FILE* file)
{
    memory mem = {0};
    if (foundErrorFirstPass)
    {
        printf("Errors found in first pass, aborting symbol table creation.\n");
        mem.headDC = NULL;
        mem.headIC = NULL;
        return mem;/* return empty memory struct */
    }
    else
    {
        return buildSymbols(root, file);
    }
}

int checkIfValidMatParam(char* param)
{
    int leftBrackets = 0;
    int rightBrackets = 0;
    char* paramTmp = param;
    if (!paramTmp)
    {
        return -1;
    }

    while (*paramTmp)
    {
        if (*paramTmp == LEFT_BRACKET)
        {
            leftBrackets++;
        }
        else if (*paramTmp == RIGHT_BRACKET)
        {
            rightBrackets++;
        }
        paramTmp++;
    }

    if (leftBrackets == 2 && rightBrackets == 2)
    {
        return 1;
    }
    else if (leftBrackets == 0 && rightBrackets == 0)
    {
        return -1;
    }
    return 0;
}

char* countRestOfLine(char* line, int isLabal)
{
    int wordCount = isLabal ? 2 : 1;
    while (*line && wordCount > 0)
    {
        while (*line && (*line != ' ' && *line != '\t')) line++; 
        while (*line && (*line == ' ' || *line == '\t')) line++; 
        wordCount--;
    }

    return line;
}