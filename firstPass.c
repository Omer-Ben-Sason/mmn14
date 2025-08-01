#include "firstPass.h"

int foundErrorFirstPass = 0; /* global error flag */


/*
the func: buildSymbols
input: root of reserved words, and the source file
output: memory struct containing symbol tables for DC and IC
*/
memory buildSymbols(reservedNode* root, FILE* file) /* build symbol tables from source */
{
    int i = 0; /* index variable */
    int DC = DC_START; /* data counter */
    int IC = IC_START; /* instruction counter */
    char** commands = NULL; /* command tokens */
    char* PC = NULL; /* current line pointer */
    int lineNum = 0; /* current line number */

    int thereIsLabal = 0; /* flag for label presence */
    reservedNode* rNode = NULL; /* current reserved word */
    char* inst = NULL; /* instruction */
    char* labal = NULL; /* label */
    memory mem = {0}; /* final memory output */
    symbolNode* headDC = NULL; /* DC list head */
    symbolNode* headIC = NULL; /* IC list head */
    symbolNode* lastIC = NULL; /* last node in IC */
    symbolNode* curr = headDC; /* current node */
    int inCode = 0; /* flag if inside code */
    char* lastReg = malloc(TEN_BITS + 1); /* register tracker */

    for (i = 0; i < TEN_BITS; i++) /* initialize reg binary to zero */
    {
        lastReg[i] = '0'; /* fill with zeros */
    }
    lastReg[TEN_BITS] = '\0'; /* null terminate */

    fseek(file, 0, SEEK_SET); /* rewind file */

    while ((PC = loadLine(file)) != NULL && *PC != EOF && *PC != 0) /* read each line */
    {
        lineNum++; /* increase line number */

        if (PC == NULL || *PC == '\0' || strspn(PC, " \t\r\n") == strlen(PC)) /* skip empty/whitespace lines */
        {
            continue; /* next iteration */
        }

        commands = breakLine(PC); /* split line */

        if ((labal = checkIfLabal(commands[0], root, lineNum, headDC, headIC))) /* check label */
        {
            thereIsLabal = 1; /* mark label presence */
        }

        inst = thereIsLabal ? commands[1] : commands[0]; /* get instruction */
        rNode = findNode(root, inst); /* find reserved */

        if (rNode && *(rNode->reserved->name) == DOT && !inCode) /* directive line */
        {
            if (strcmp(inst, SYMBOL_DATA) == 0 || strcmp(inst, SYMBOL_MAT) == 0 || strcmp(inst, SYMBOL_STRING) == 0) /* data types */
            {
                if (thereIsLabal) /* label before directive */
                {
                    putInDC(inst, countRestOfLine(PC, thereIsLabal), lineNum, &headDC, labal, &DC); /* insert to DC */
                }
            }
            else if (strcmp(inst, SYMBOL_EXT) == 0) /* extern case */
            {
                if (commands[2]) /* extra label */
                {
                    labal = commands[2]; /* override */
                    printf("wanring at line %d: a labal in .extern is useless!\n", lineNum); /* warning */
                }
                else if (commands[1]) /* get external label */
                {
                    labal = commands[1]; /* assign */
                }
                else /* invalid extern */
                {
                    printf("at line %d: invalid extern command!\n", lineNum); /* error */
                    foundErrorFirstPass = 1; /* set error */
                }

                addSymbol(&headDC, labal, DC++, SYMBOL_EXT, strdup(CLEAR_MEM)); /* add extern */
            }
            else if (strcmp(inst, SYMBOL_ENT) == 0)
            {
                if (commands[2])
                {
                    printf("warning at line %d: a label in .entry is useless!\n", lineNum);
                }
                else if (commands[1])
                {
                    labal = commands[1];
                    addSymbol(&headDC, NULL, DC++, SYMBOL_ENT, labal); /* store for second pass */
                }
                else
                {
                    printf("at line %d: invalid entry command!\n", lineNum);
                    foundErrorFirstPass = 1;
                }
            }
            else /* unknown directive */
            {
                printf("at line %d: invalid directive: %s\n", lineNum, inst); /* error */
                foundErrorFirstPass = 1; /* mark error */
            }
        }
        else if (findNode(root, inst) && strcmp(findNode(root, inst)->reserved->type, SYMBOL_CODE) == 0) /* instruction */
        {
            putInIC(&IC, countRestOfLine(PC, thereIsLabal), lineNum, &headIC, labal, root, &lastReg, inst); /* insert to IC */
        }
        else /* invalid */
        {
            printf("at line %d: invalid directive: %s\n", lineNum, inst); /* error */
            foundErrorFirstPass = 1; /* mark error */
        }

        thereIsLabal = 0; /* reset label flag */
    }

    if (commands) free(commands); /* cleanup commands */
    if (PC) free(PC); /* cleanup PC */

    curr = headDC; /* start DC shifting */

    while (curr) /* add IC to DC address */
    {
        curr->symbol->addr += IC; /* shift address */
        curr = curr->next; /* move forward */
    }

    if (headIC) /* if IC exists */
    {
        lastIC = headIC; /* start at head */
        while (lastIC->next) /* go to end */
        {
            lastIC = lastIC->next; /* advance */
        }
        lastIC->next = headDC; /* connect DC after IC */
    }

    mem.headDC = headDC; /* save DC */
    mem.headIC = headIC; /* save IC */
    return mem; /* return full memory */
}

/*
the func: checkIfLabal
input: possible label string, root reserved tree, line number, and current symbol tables
output: label name if valid, otherwise NULL
*/
char* checkIfLabal(char* labal, reservedNode* root, int lineNum, symbolNode* DC, symbolNode* IC) /* check if valid label */
{
    int len = 0; /* length of label */
    reservedNode* check = NULL; /* pointer to reserved word node */

    if (!labal || (len = strlen(labal)) >= LABAL_LEN) /* null or too long */
    {
        return NULL; /* invalid */
    }
    if (!((*labal >= BIG_A_ASCII && *labal <= BIG_Z_ASCII) || (*labal >= SMALL_A_ASCII && *labal <= SMALL_Z_ASCII)||*labal==DOT))
    {
        printf("at line %d: labal need to start with a letter!\n", lineNum);
        foundErrorFirstPass = 1;
        return NULL;
    }

    if (labal[len - 1] != COLOM) /* must end with ':' */
    {
        return NULL; /* not a label */
    }

    labal[len - 1] = '\0'; /* remove ':' */

    check = findNode(root, labal); /* check if reserved */

    if (check || searchSymbolTable(labal, DC, IC,1)) /* check conflicts */
    {
        if (check) /* label is a reserved word */
        {
            printf("at line %d: labal name is taken!\n", lineNum); /* error msg */
            foundErrorFirstPass = 1; /* set error flag */
        }
        return NULL; /* invalid */
    }

    return labal; /* valid label name */
}


/*
the func: putInDC
input: directive type (.data/.string/.mat), content, line number, symbol list head, label name, DC counter
output: number of entries added to DC
*/
int putInDC(char* type, char* restOfLine, int lineNum, symbolNode** head, char* name, int* DC) /* parse and store .data/.string/.mat */
{
    int count = 0; /* counter for entries added */

    if (strcmp(type, SYMBOL_DATA) == 0) /* if .data */
    {
        count = handleData(restOfLine, lineNum, head, name, DC, type); /* handle numeric data */
    }
    else if (strcmp(type, SYMBOL_STRING) == 0) /* if .string */
    {
        count = handleString(restOfLine, lineNum, head, name, DC); /* handle string */
    }
    else if (strcmp(type, SYMBOL_MAT) == 0) /* if .mat */
    {
        count = handleMat(restOfLine, lineNum, head, name, DC); /* handle matrix */
    }

    if (*(restOfLine + strlen(restOfLine) - 1) == COMMA_ASCII) /* check trailing comma */
    {
        printf("at line %d: trailing comma without value\n", lineNum); /* error msg */
        foundErrorFirstPass = 1; /* set error flag */
    }

    return count; /* return number of entries */
}

/*
the func: handleData
input: data string, line number, DC list, label name, DC counter, type
output: number of symbols added
*/
int handleData(char* curr, int lineNum, symbolNode** head, char* name, int* DC, char* type) /* handles .data directive */
{
    char* commanChecker = curr; /* pointer to scan for commas */
    char tmp[BUFF_SIZE] = {0}; /* buffer for number strings */
    int count = 0; /* how many added */
    int lastComma = 0; /* flag for multiple commas */
    int i = 0; /* index for tmp */

    while (commanChecker && *commanChecker != EOL && *commanChecker != EOF && *commanChecker != 0) /* scan for errors */
    {
        if (*commanChecker == COMMA_ASCII) /* comma found */
        {
            if (lastComma) /* double comma */
            {
                printf("at line %d: multiple commas found\n", lineNum); /* error */
                foundErrorFirstPass = 1; /* set error */
            }
            lastComma = 1; /* update state */
        }
        else if (*commanChecker != ' ' && *commanChecker != '\t') lastComma = 0; /* reset on non-whitespace */
        commanChecker++; /* move forward */
    }

    while (*curr == ' ' || *curr == '\t') curr++; /* skip whitespace */

    while (*curr != EOL && *curr != EOF && *curr != 0) /* parse numbers */
    {
        i = 0; /* reset index */
        while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == '-' || *curr == '+') tmp[i++] = *curr++; /* collect number */
        
        if (i > 0) /* if something was collected */
        {
            tmp[i] = '\0'; /* null terminate */
            if (isNumeric(tmp)) /* check valid number */
            {
                addSymbol(head, (name) ? name : NULL, (*DC)++, (type) ? type : NULL, intToBinary(atoi(tmp))); /* add to DC */
                count++; /* increase count */
                name = NULL; /* use label only once */
            }
            else /* invalid number */
            {
                printf("at line %d: invalid number: %s\n", lineNum, tmp); /* error */
                foundErrorFirstPass = 1; /* flag */
            }
        }

        while (*curr == ' ' || *curr == '\t' || *curr == COMMA_ASCII) curr++; /* skip delimiters */
    }

    return count; /* return how many added */
}

/*
the func: handleString
input: string directive, line number, DC list, label, DC counter
output: number of symbols added
*/
int handleString(char* curr, int lineNum, symbolNode** head, char* name, int* DC) /* handles .string directive */
{
    int count = 0; /* counter */
    char asciiStr[ASCII_BUF_SIZE] = {0}; /* ASCII representation */

    while (*curr == ' ' || *curr == '\t') curr++; /* skip whitespace */

    if (*curr == '"') /* opening quote */
    {
        curr++; /* move to first char */

        while (*curr != '"' && *curr != '\0') /* until closing quote or end */
        {
            if (*curr == EOL || *curr == EOF) /* invalid end */
            {
                printf("at line %d: missing closing quote\n", lineNum); /* error */
                foundErrorFirstPass = 1; /* set flag */
                break; /* exit loop */
            }

            sprintf(asciiStr, "%d", *curr); /* convert char to int */
            addSymbol(head, (name) ? name : NULL, (*DC)++, SYMBOL_DATA, intToBinary(atoi(asciiStr))); /* add char */
            count++; /* increase count */
            name = NULL; /* clear label */
            curr++; /* next char */
        }

        if (*curr == '"') /* closing quote */
        {
            addSymbol(head, NULL, (*DC)++, SYMBOL_DATA, strdup(CLEAR_MEM)); /* add null terminator */
            count++; /* increase count */
        }
        else /* missing quote */
        {
            printf("at line %d: missing closing quote\n", lineNum); /* error */
            foundErrorFirstPass = 1; /* flag */
        }
    }
    else /* string doesn't start with " */
    {
        printf("at line %d: invalid string format\n", lineNum); /* error */
        foundErrorFirstPass = 1; /* flag */
    }

    return count; /* total added */
}
/*
the func: handleMat
input: matrix declaration string, line number, DC list, label, DC counter
output: number of values inserted to DC
*/
int handleMat(char* curr, int lineNum, symbolNode** head, char* name, int* DC) /* handles .mat directive */
{
    char tmp[BUFF_SIZE] = {0}; /* temp number buffer */
    int rows = 0; /* matrix rows */
    int cols = 0; /* matrix cols */
    int count = 0; /* count of inserted values */
    int i = 0; /* index */
    char* commanChecker = curr; /* comma checker */
    int lastComma = 0; /* double comma flag */

    while (commanChecker && *commanChecker != EOL && *commanChecker != EOF && *commanChecker != 0) /* check for double commas */
    {
        if (*commanChecker == COMMA_ASCII) /* comma found */
        {
            if (lastComma) /* previous was also comma */
            {
                printf("at line %d: multiple commas found\n", lineNum); /* error */
                foundErrorFirstPass = 1; /* flag */
            }
            lastComma = 1; /* set flag */
        }
        else if (*commanChecker != ' ' && *commanChecker != '\t') lastComma = 0; /* reset if not space */
        commanChecker++; /* next char */
    }

    while (*curr == ' ' || *curr == '\t') curr++; /* skip whitespace */

    if (*curr != LEFT_BRACKET) /* must start with [ */
    {
        printf("at line %d: invalid mat format\n", lineNum); /* error */
        foundErrorFirstPass = 1; /* flag */
        return 0; /* stop */
    }

    curr++; i = 0; /* move past [ and reset index */

    while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == '-' || *curr == '+') tmp[i++] = *curr++; /* read row */
    curr++; /* skip ] */
    if (i > 0 && isNumeric(tmp)) rows = atoi(tmp); /* valid row */
    else
    {
        printf("at line %d: invalid number of rows: %s\n", lineNum, tmp); /* error */
        foundErrorFirstPass = 1; /* flag */
    }

    while (*curr == ' ' || *curr == '\t' || *curr == LEFT_BRACKET) curr++; /* skip until cols */
    i = 0; /* reset index */

    while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == '-' || *curr == '+') tmp[i++] = *curr++; /* read col */

    if (i > 0 && isNumeric(tmp)) cols = atoi(tmp); /* valid col */
    else
    {
        printf("at line %d: invalid number of columns: %s\n", lineNum, tmp); /* error */
        foundErrorFirstPass = 1; /* flag */
    }

    curr++; /* skip ] */
    while (*curr == ' ' || *curr == '\t') curr++; /* skip whitespace */

    while (*curr != EOL && *curr != EOF && *curr != 0 && count < rows * cols) /* parse matrix values */
    {
        while (*curr == ' ' || *curr == '\t' || *curr == COMMA_ASCII) curr++; /* skip */

        i = 0; /* reset index */
        while ((*curr >= ZERO_ASCII && *curr <= NINE_ASCII) || *curr == MINUS_ASCII || *curr == PLUS_ASCII) tmp[i++] = *curr++; /* collect value */

        if (i > 0) /* valid number */
        {
            tmp[i] = '\0'; /* terminate */
            putInDC(SYMBOL_DATA, tmp, lineNum, head, name, DC); /* insert */
            name = NULL; /* reset label */
            count++; /* increment */
        }
    }

    while (count < rows * cols) /* fill remaining with 0 */
    {
        addSymbol(head, (name) ? name : NULL, (*DC)++, SYMBOL_DATA, strdup(CLEAR_MEM)); /* insert zero */
        count++; /* increment */
        name = NULL; /* clear */
    }

    return count; /* return added values */
}
/* 
the func: putInIC
input: IC counter, rest of line, line number, IC list, label name, reserved root, lastReg tracker, instruction name
output: none (adds encoded instructions to IC)
*/
void putInIC(int* IC, char* restOfLine, int lineNum, symbolNode** head, char* name, reservedNode* root, char** lastReg, char* inst)
{
    int i = 0; /* index */
    int j = 0; /* secondary index */
    char** params = breakToParams(restOfLine, 1); /* tokenize with brackets */
    char** paramsBackets = breakToParams(restOfLine, 0); /* tokenize without brackets */
    reservedNode* rNode = NULL; /* current param */
    reservedNode* rNodeNext = NULL; /* next param */
    char* currBinary = NULL; /* instruction bits */
    char* currBinaryInst = NULL; /* original copy of instruction word */
    char fullRegBin[TEN_BITS + 1]; /* for double register encoding */
    int currParamMat = 0; /* matrix flag */
    int icStart = *IC; /* save start address */
    int len = strlen(restOfLine); /* length of line */
    char addressingTypeSrc = -1; /* source addr type */
    char addressingTypeDst = -1; /* dest addr type */
    char* addressingTypeString = NULL; /* holds binary of addressing type */
    char regBin[TEN_BITS + 1]; /* reg bits single */
    char* currOpDst = NULL; /* allowed dst addressing types */
    char* currOpSrc = NULL; /* allowed src addressing types */
    char* currName = name; /* save label */
    char* endOfLine = NULL; /* used to check trailing comma */
    (*IC)++; /* for the main instruction word */

    if (findNode(root, inst)->reserved->type && strcmp(findNode(root, inst)->reserved->type, SYMBOL_CODE) == 0) 
    {
        currBinary = strdup(CLEAR_MEM);
        currOpSrc = findNode(root, inst)->reserved->opSrc;
        currOpDst = findNode(root, inst)->reserved->opDst;

        for (j = 0; j < FOUR_BITS; j++)
        {
            currBinary[j] = findNode(root, inst)->reserved->binary[j + SIX_BITS];
        }

        currBinary[TEN_BITS] = '\0';
        currBinaryInst = currBinary;

    }

    for (i = 0; params[i] != NULL; i++)/*looping tho params*/
    {
        if (params[i] && strchr(params[i], '\n'))
        {
            *strchr(params[i], '\n') = '\0';
        }
        if (params[i + 1] && strchr(params[i + 1], '\n'))
        {
            *strchr(params[i + 1], '\n') = '\0';
        }
        
        currParamMat = (addressingTypeSrc==-1)?checkIfValidMatParam(paramsBackets[0]):checkIfValidMatParam(paramsBackets[1]);/*checking if matrix*/
        if (currParamMat == 0)
        {
            printf("at line %d: invalid mat format, expected [rows][cols]\n", lineNum);
            foundErrorFirstPass = 1;
        }

        if ((*params[i] == '#') && isNumeric(params[i] + 1))/*checking if number*/
        {
            addSymbol(head, NULL, (*IC)++, SYMBOL_CODE, intToBinary(atoi(params[i] + 1)));

            if (addressingTypeSrc == -1)
            {
                if (!(params[1] || strlen(params[1]) == 0))
                {
                    addressingTypeDst = ZERO_ASCII;
                }
                else
                {
                    addressingTypeSrc = ZERO_ASCII;
                }
            }
            else
            {
                addressingTypeDst = ZERO_ASCII;
            }
            continue;
        }

        rNode = findNode(root, params[i]);

        if (rNode && rNode->reserved->type && strcmp(rNode->reserved->type, REGISTER_SYMBOL) == 0)/*checking if reg*/
        {

            currBinary = rNode->reserved->binary + SIX_BITS;

            if (params[i + 1])
            {
                rNodeNext = findNode(root, params[i + 1]);
            }

            if (rNodeNext && rNodeNext->reserved && strcmp(rNodeNext->reserved->type, REGISTER_SYMBOL) == 0)/*checking if next param reg*/
            {

                if (addressingTypeSrc == -1)
                {
                    if (!currParamMat)
                    {
                        addressingTypeSrc = TWO_ASCII;
                    }
                    else
                    {
                        addressingTypeSrc = THREE_ASCII;
                        addressingTypeDst = THREE_ASCII; 
                    }
                }
            

                strncpy(fullRegBin, currBinary, FOUR_BITS);
                strncpy(fullRegBin + FOUR_BITS, rNodeNext->reserved->binary + SIX_BITS, FOUR_BITS);
                fullRegBin[8] = '0';
                fullRegBin[9] = '0';
                fullRegBin[10] = '\0';
                name = NULL;
                addSymbol(head, name, (*IC)++, SYMBOL_CODE, strdup(fullRegBin));
                i++;
            }
            else/*if only one reg*/
            {
                if (addressingTypeSrc == -1 && !(params[1] == NULL || strlen(params[1]) == 0))
                {
                    addressingTypeSrc = THREE_ASCII;

                    /* src goes in bits 0-3 */
                    strncpy(regBin, currBinary, FOUR_BITS);
                    for (j = FOUR_BITS; j < TEN_BITS; j++) regBin[j] = '0';
                }
                else
                {
                    addressingTypeDst = THREE_ASCII;

                    for (j = 0; j < FOUR_BITS; j++) regBin[j] = '0';
                    strncpy(regBin + FOUR_BITS, currBinary, FOUR_BITS);
                    for (j = EIGHT_BITS; j < TEN_BITS; j++) regBin[j] = '0';
                }

                

                regBin[10] = '\0';
                name = NULL;
                addSymbol(head, name, (*IC)++, SYMBOL_CODE, strdup(regBin));
            }

            continue;
        }

        if (addressingTypeSrc == -1)/*if addr type wasnt choosen yet*/
        {
            if (checkIfValidMatParam(paramsBackets[0])==1)
            {
                addressingTypeSrc = TWO_ASCII;
            }
            else if (params[i] != NULL || strlen(params[i]) != 0)
            {
                addressingTypeSrc = ONE_ASCII;
            }

            if ((params[1] == NULL || strlen(params[1]) == 0|| *params[1] == 0|| *params[1] == '\t'|| *params[1] == ' '))
            {
                addressingTypeDst = addressingTypeSrc;
                addressingTypeSrc = -1;
            }
        }
        else /*if not addr type wasnt choosen yet*/
        {                  
            if (checkIfValidMatParam(paramsBackets[1])==1)
            {   
                addressingTypeDst = TWO_ASCII;
            }
            else if ((params[i] != NULL || strlen(params[i]) != 0)&&addressingTypeDst==-1)
            {
                addressingTypeDst = ONE_ASCII;
            }
        }
        if (params[i] && strlen(params[i]) > 0)
        {
            addSymbol(head, NULL, (*IC)++, SYMBOL_CODE, strdup(params[i]));
        }
        if (params[1] == NULL || strlen(params[1]) == 0|| *params[1] == 0)
        { 
            if (*params[0]=='#')
            {
                addressingTypeDst = ZERO_ASCII; 
                addressingTypeSrc = ZERO_ASCII; 
            }

        }
    }

    if ((params[1] == NULL || strlen(params[1]) == 0) || addressingTypeSrc == -1)
    {
        addressingTypeSrc = ZERO_ASCII;
    }
    if (params[0] == NULL || strlen(params[0]) == 0)
    {
        addressingTypeDst = ZERO_ASCII;
    }
    if (currBinaryInst)
    {
        addressingTypeString = intToBinary(addressingTypeDst - ZERO_ASCII);

        if (currOpDst && strchr(currOpDst, addressingTypeDst))/*if addrsessing type is good*/
        {
            currBinaryInst[6] = addressingTypeString[8];
            currBinaryInst[7] = addressingTypeString[9];
        }
        else if (params[1]!=NULL && strlen(params[1]) > 0)/*if addrsessing type is not good*/
        {
            printf("at line %d: dst param is invalid!\n",lineNum);
            foundErrorFirstPass = 1;
        }


        addressingTypeString = intToBinary(addressingTypeSrc - ZERO_ASCII);
        if (currOpSrc && strchr(currOpSrc, addressingTypeSrc))/*if addrsessing type is good*/
        {
            
            currBinaryInst[4] = addressingTypeString[8];
            currBinaryInst[5] = addressingTypeString[9];
        }
        else if (params[0]!=NULL&&currOpSrc)/*if addrsessing type is not good*/
        {
            printf("at line %d: src param is invalid!\n",lineNum);
            foundErrorFirstPass = 1;
        }
    }

    if (currBinaryInst)
    {
        insertSymbolSortedByAddr(head, currName, icStart, SYMBOL_CODE, strdup(currBinaryInst));
    }
    if (len > 0)
    {
        endOfLine = restOfLine + len - 1;
        while ((*endOfLine == ' ' || *endOfLine == '\t')|| *endOfLine == '\n') endOfLine--;

        if (*endOfLine == COMMA_ASCII)
        {
            printf("at line %d: comma at end of line without value after!\n", lineNum);
            foundErrorFirstPass = 1;
        }
    }
    for (i = 0; params[i]; i++) free(params[i]);
    free(params);
    free(paramsBackets);
    free(addressingTypeString);
}

/* 
the func: firstPass
input: reserved words tree, file pointer
output: memory struct after first pass (symbol tables)
*/
memory firstPass(reservedNode* root, FILE* file) /* performs the first pass on the source file */
{
    memory mem = buildSymbols(root, file); /* build symbols from source */

    if (foundErrorFirstPass) /* check if any error was detected */
    {
        printf("Errors found in first pass, aborting symbol table creation.\n"); /* error msg */
        mem.headDC = NULL; /* clear data section */
        mem.headIC = NULL; /* clear code section */
        return mem; /* return empty memory struct */
    }
    else /* no errors */
    {
        return mem; /* return memory with symbol tables */
    }
}

/* 
the func: checkIfValidMatParam
input: a parameter string
output: 1 if valid matrix param, -1 if no brackets, 0 if invalid format
*/
int checkIfValidMatParam(char* param) /* checks if the parameter has valid [row][col] matrix format */
{
    int leftBrackets = 0; /* counter for '[' */
    int rightBrackets = 0; /* counter for ']' */
    char* paramTmp = param; /* temp pointer to iterate */

    if (!paramTmp) /* null check */
    {
        return -1; /* no brackets at all */
    }

    while (*paramTmp) /* go through each char */
    {
        if (*paramTmp == LEFT_BRACKET) /* '[' found */
        {
            leftBrackets++; /* count it */
        }
        else if (*paramTmp == RIGHT_BRACKET) /* ']' found */
        {
            rightBrackets++; /* count it */
        }
        paramTmp++; /* move to next char */
    }

    if (leftBrackets == 2 && rightBrackets == 2) /* correct amount */
    {
        return 1; /* valid matrix format */
    }
    else if (leftBrackets == 0 && rightBrackets == 0) /* no brackets at all */
    {
        return -1; /* no brackets */
    }

    return 0; /* malformed brackets */
}

/* 
the func: countRestOfLine
input: full line, and if it contains label
output: pointer to beginning of content after opcode
*/
char* countRestOfLine(char* line, int isLabal) /* skips 1 or 2 words from line and returns pointer to remaining */
{
    int wordCount = isLabal ? 2 : 1; /* number of words to skip */

    while (*line && (*line == ' ' || *line == '\t')) line++; /* skip leading whitespace */

    while (*line && wordCount > 0) /* loop to skip words */
    {
        while (*line && *line != ' ' && *line != '\t') line++; /* skip over a word */
        while (*line && (*line == ' ' || *line == '\t')) line++; /* skip spaces after word */
        wordCount--; /* decrement word counter */
    }

    return line; /* return pointer to rest of line */
}
