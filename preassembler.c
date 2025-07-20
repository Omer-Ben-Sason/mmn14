#include "preassembler.h"

int foundErrorMacro = 0; /* flag indicating if an error was found in macro definition */

/* 
 * Initializes reserved words and inserts them into the macro tree 
 * Input: pointer to root of macro tree
 * Output: none
 */
void initReservedWords(reservedNode** root)
{
    char* reserved[] = { /* list of reserved keywords */
        "r3", "r1", "r5", "r0", "r4", "r7", "r2", "r6", 
        ".data", ".entry", ".extern", ".mat", ".string",
        "mov", "cmp", "add", "sub", "not", 
        "clr", "lea", "inc", "dec", "jmp",
        "bne", "red", "prn", "jsr", "rts", "stop" 
    };

    int count = sizeof(reserved) / sizeof(reserved[0]); /* total number of reserved words */
    int i = 0; /* loop index */
    char* type = NULL; /* type of reserved word, NULL for most */
    char* opDst = NULL; /* destination operand, NULL for most */
    char* opSrc = NULL; /* source operand, NULL for most */
    char* pData = NULL; /* data for reserved word, NULL for most */
    char* binary = NULL; /* binary representation, NULL for most */
    for (i = 0; i < count; i++)
    {
        if (*reserved[i]=='r')
        {
            type = "register"; /* register type for r0-r7 */
        }
        else if (strcmp(reserved[i], ".data") == 0 || strcmp(reserved[i], ".mat") == 0 || strcmp(reserved[i], ".string") == 0)
        {
            type = "data"; /* data type for .data, .mat, .string */
        }
        else if (strcmp(reserved[i], ".entry") == 0 || strcmp(reserved[i], ".extern") == 0)
        {
            type = "symbol"; /* symbol type for .entry, .extern */
        }
        else
        {
            binary = intToBinary(i);
            if (strcmp(reserved[i],"mov")||strcmp(reserved[i],"add")||strcmp(reserved[i],"sub"))
            {
                opDst = "123"; /* destination operand for mov */
                opSrc = "0123"; /* source operand for mov */
            }
            else if (strcmp(reserved[i],"cmp"))
            {
                opDst = "0123"; /* destination operand for mov */
                opSrc = "0123"; /* source operand for mov */
            }
            else if (strcmp(reserved[i],"lea"))
            {
                opDst = "123"; /* destination operand for mov */
                opSrc = "12"; /* source operand for mov */
            }
            else if (strcmp(reserved[i],"clr")||strcmp(reserved[i],"not")||strcmp(reserved[i],"inc")||strcmp(reserved[i],"dec")||strcmp(reserved[i],"jmp")||strcmp(reserved[i],"jsr")||strcmp(reserved[i],"bne")||strcmp(reserved[i],"red"))
            {
                opDst = "123"; /* destination operand for mov */
            }
            else if (strcmp(reserved[i],"prn"))
            {
                opDst = "0123"; /* destination operand for mov */
            }
            type = "code"; /* command type for other reserved words */
        }
        *root = insertNode(*root, reserved[i], pData,type,opDst,opSrc,binary); /* insert each reserved word into the tree */
    }
}
/* 
 * turns an integer into its binary representation
 * Input: integer number
 *  * Output: integer representing the binary number
 */
char* intToBinary(int num)
{
    char* binary = (char*)malloc(9); /* 8 bits + '\0' */
    int i = 7;
    if (!binary)
    {
        return NULL;
    }

    binary[8] = '\0';

    

    while (i >= 0)
    {
        binary[i] = (num & 1) ? '1' : '0';
        num >>= 1;
        i--;
    }

    return binary;
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
 * Replaces macro usages in the file with their actual content 
 * Input: input file pointer, macro tree root
 * Output: writes modified content to "file.am"
 */
void replaceMcro(FILE* file, reservedNode* root)
{
    char* line = NULL; /* holds current line */
    char** commands = NULL; /* holds split line */
    reservedNode* node = NULL; /* current macro node */
    FILE* outputFile = fopen("file.am", "w"); /* output file */
    fseek(file, 0, SEEK_SET); /* reset file pointer to start */

    while ((line = loadLine(file)) != NULL)
    {     
        commands = breakLine(line);
        node = findNode(root, commands[0]);

        if (node && node->reserved->pData)
        {
            fputs(node->reserved->pData, outputFile); /* write macro content */
        }
        else
        {
            fputs(line, outputFile); /* write line as-is */
        }

        freeWords(commands);
        free(line);
    }

    fclose(outputFile);
}

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
 * Opens a file for writing with ".am" extension, returns file pointer 
 * Input: base output file name (without extension)
 * Output: pointer to opened .am file
 */
FILE* openOutputFile(char* outputFileName)
{
    char* fullName = (char*)malloc(strlen(outputFileName) + FOUR_BYTES); /* output file name */
    FILE* outputFile = NULL;
    if (!fullName)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }

    sprintf(fullName, "%s.am", outputFileName); /* create file name */
    outputFile = fopen(fullName, "w"); /* open file */

    free(fullName); 
    return outputFile;
}

/* 
 * Scans the input file, collects macro definitions and validates them.  
 * Stores macro content and later outputs to new file without macros. 
 * Input: input file pointer, macro tree root, output file name
 * Output: writes final processed file (if no errors)
 */
void findMcro(FILE* file, reservedNode* root, char* outputFileName)
{
    char* line = NULL; /* holds current line */
    int flag = 0; /* 1 = inside macro definition */
    int lineNum = 0; /* line counter */
    char* macroName = NULL; /* current macro name */
    char* toWrite = NULL; /* line/macro to write */
    char* data = NULL, *temp = NULL; /* holds macro data and temp buffers */
    char** commands = NULL; /* tokenized line */
    reservedNode* node = NULL; /* current macro node */
    int len = 0; /* length of line to append */

    char* fileContent = NULL; /* content of final output file */
    int contentSize = 0; /* total size of content */

    foundErrorMacro = 0;

    while ((line = loadLine(file)) != NULL)
    {
        commands = breakLine(line);
        if (commands[0] == NULL)
        {
            if (foundErrorMacro == 0)
            {
                len = strlen(line);
                temp = realloc(fileContent, contentSize + len + 1);
                if (!temp)
                {
                    printf("Memory allocation failed\n");
                    free(fileContent);
                    freeWords(commands);
                    free(line);
                    return;
                }
                fileContent = temp;
                strcpy(fileContent + contentSize, line);
                contentSize += len;
            }

            freeWords(commands);
            free(line);
            lineNum++;
            continue;
        }

        if (strchr(line, '\n') == NULL && !feof(file))
        {
            printf("at line %d: line cannot be more than 80 chars!\n", lineNum);
            foundErrorMacro = 1;
        }

        if (strcmp(commands[0], MCRO_START) == 0)
        {
            if (commands[2])
            {
                printf("at line %d: mcro only takes 1 arg (mcro <name>)\n", lineNum);
                flag = 0;
                foundErrorMacro = 1;
            }
            else
            {
                flag = 1;
                if (!commands[1])
                {
                    flag = 0;
                    printf("at line %d: every macro needs to have a name!\n", lineNum);
                    foundErrorMacro = 1;
                }
                else
                {
                    macroName = commands[1];
                    macroName = strdup(macroName);
                    if (findNode(root, macroName))
                    {
                        printf("at line %d: the name \"%s\" is taken or reserved!\n", lineNum, macroName);
                        flag = 0;
                        foundErrorMacro = 1;
                    }
                    else
                    {
                        insertNode(root, macroName, NULL,"mcro",NULL,NULL,NULL); /* add macro to tree */
                    }
                }
            }
        }
        else if (flag && strcmp(commands[0], MCRO_END) == 0)
        {
            flag = 0;
            if (commands[1])
            {
                printf("at line %d: mcroend doesn't take an argument!\n", lineNum);
                foundErrorMacro = 1;
            }
            if (macroName)
            {
                free(macroName);
                macroName = NULL;
            }
        }
        else if (flag) /* inside macro */
        {
            node = findNode(root, macroName);
            data = node->reserved->pData;

            if (!data)
            {
                data = (char*)malloc(strlen(line) + 1);
                if (data)
                {
                    strcpy(data, line);
                }
            }
            else
            {
                char* temp = realloc(data, strlen(data) + strlen(line) + 1);
                if (temp)
                {
                    strcat(temp, line);
                    data = temp;
                }
            }

            node->reserved->pData = data;
        }
        else if (!flag && foundErrorMacro == 0)
        {
            node = findNode(root, commands[0]);
            toWrite = (node && node->reserved->pData) ? node->reserved->pData : line;

            len = strlen(toWrite);
            temp = realloc(fileContent, contentSize + len + 1);
            if (!temp)
            {
                printf("Memory allocation failed\n");
                free(fileContent);
                freeWords(commands);
                free(line);
                return;
            }
            fileContent = temp;
            strcpy(fileContent + contentSize, toWrite);
            contentSize += len;
        }

        freeWords(commands);
        free(line);
        lineNum++;
    }

    if (!foundErrorMacro && fileContent)
    {
        FILE* outputFile = openOutputFile(outputFileName);
        if (outputFile)
        {
            fputs(fileContent, outputFile); /* write output to file */
            fclose(outputFile);
        }
        else
        {
            printf("Failed to open output file for writing\n");
        }
    }

    free(fileContent);
}

FILE* preassmble(FILE* inputFile,char* inputFileName,reservedNode* root)
{

    FILE* output = NULL;
    char* outputFileNameAm = NULL;

    findMcro(inputFile, root, inputFileName);

    fclose(inputFile);

    outputFileNameAm = (char*)malloc(strlen(inputFileName) + FOUR_BYTES);
    if (!outputFileNameAm)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }

    sprintf(outputFileNameAm, "%s.am", inputFileName);
    output = fopen(outputFileNameAm, "r");
    free(outputFileNameAm);

    return output;
}
