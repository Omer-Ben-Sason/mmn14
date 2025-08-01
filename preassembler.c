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
        "mov", "cmp", "add", "sub", "lea", 
        "clr", "not", "inc", "dec", "jmp",
        "bne", "jsr", "red", "prn", "rts", "stop" 
    };

    int count = sizeof(reserved) / sizeof(reserved[0]); /* total number of reserved words */
    int i = 0,j=0; /* loop index */
    char* type = NULL; /* type of reserved word, NULL for most */
    char* opDst = NULL; /* destination operand, NULL for most */
    char* opSrc = NULL; /* source operand, NULL for most */
    char* pData = NULL; /* data for reserved word, NULL for most */
    char* binary = NULL; /* binary representation, NULL for most */
    for (i = 0; i < count; i++)
    {
        binary = NULL; /* reset binary for each reserved word */
        if (*reserved[i]=='r'&&*(reserved[i]+1)>=ZERO_ASCII&&*(reserved[i]+1)<=SEVEN_ASCII)
        {
            type = "register"; /* register type for r0-r7 */
            binary = intToBinary(*(reserved[i]+1)-ZERO_ASCII); /* binary representation for registers */
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
            binary = intToBinary(j++); /* binary representation for commands */
            if (strcmp(reserved[i],"mov")==0||strcmp(reserved[i],"add")==0||strcmp(reserved[i],"sub")==0)
            {
                opDst = "123"; /* destination operand for mov */
                opSrc = "0123"; /* source operand for mov */
            }
            else if (strcmp(reserved[i],"cmp")==0)
            {
                opDst = "0123"; /* destination operand for mov */
                opSrc = "0123"; /* source operand for mov */
            }
            else if (strcmp(reserved[i],"lea")==0)
            {
                opDst = "123"; /* destination operand for mov */
                opSrc = "12"; /* source operand for mov */
            }
            else if (strcmp(reserved[i],"clr")==0||strcmp(reserved[i],"not")==0||strcmp(reserved[i],"inc")==0||strcmp(reserved[i],"dec")==0||strcmp(reserved[i],"jmp")==0||strcmp(reserved[i],"jsr")==0||strcmp(reserved[i],"bne")==0||strcmp(reserved[i],"red")==0)
            {
                opDst = "123"; /* destination operand for mov */
                opSrc = NULL;
            }
            else if (strcmp(reserved[i],"prn")==0)
            {
                opDst = "0123"; /* destination operand for mov */
                opSrc = NULL;
            }
            type = "code"; /* command type for other reserved words */
        }
        *root = insertNode(*root, reserved[i], pData,type,strdup(opDst),strdup(opSrc),binary); /* insert each reserved word into the tree */
    }
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
