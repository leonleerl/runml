//  CITS2002 Project 1 2024
//  Student1:   24169259   Leon Li
//  Platform:   Linux  (or Apple)

#include <stdio.h>
union DataType
{
    double DataTypeDouble;
    int DataTypeInt;
};

typedef struct
{
    char x[1024];
    char operator[3];
    union DataType DataType;
} Statement;

typedef struct
{
    char parameters[1024][1024];
    char functionName[1024];
    int parameterCount;
} FunctionStatement;

int isFirstWord(const char *str, const char *target);
int firstFourCharsAreWhitespace(const char *str);
int isFunctionInvoke(const char *str);
int containsDigit(const char *str);

void doAssignValue(char *token, char buffer[1024], FILE *outputFile);
void doPrintValue(char *token, char buffer[1024], FILE *outputFile);
void doFunctionName(char *token, char buffer[1024], FILE *outputFile);
void doFunctionBody(char *token, char buffer[1024], FILE *outputFile);