//  CITS2002 Project 1 2024
//  Student1:   24169259   Leon Li
//  Platform:   Linux  (or Apple)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "runml.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("必须要有一个文件输入\n");
        return EXIT_FAILURE;
    }

    FILE *programFile = fopen(argv[1], "r");
    // FILE *programFile = fopen("./program.ml", "r");
    FILE *tempFile = fopen("./temp.c", "w");
    FILE *tempHFile = fopen("./temp.h", "w");
    char buffer[1024];
    char *token = "";
    int isInFunction = 0;
    fputs("#include <stdio.h>\n", tempFile);
    fputs("#include \"temp.h\"\n\n", tempFile);
    fputs("int main(int argc, char *argv[]) {\n", tempFile);

    while (fgets(buffer, sizeof(buffer), programFile))
    {

        // 跳过注释行
        if (buffer[0] == '#')
        {
            continue;
        }

        // 执行function里面的语句
        if (isInFunction && (firstFourCharsAreWhitespace(buffer) || buffer[0] == '\t'))
        {
            doFunctionBody(token, buffer, tempHFile);
        }

        // 判断是否function结束
        if (isInFunction == 1 && !firstFourCharsAreWhitespace(buffer) && buffer[0] != '\t')
        {
            fprintf(tempHFile, "%s", "}\n");
            isInFunction = 0;
        }

        // 判断是否是函数调用语句
        if (isFunctionInvoke(buffer))
        {
            // 1.是print multiply(12, 6)
            if (strncmp(buffer, "print ", 6) == 0)
            {
                char *invokeString = strstr(buffer, "print ");
                invokeString += strlen("print ");
                fprintf(tempFile, "printf(\"%%d\\n\",%s);\n", invokeString);
            }
            // 2.是printsum (12, 6)
            else
            {
                fprintf(tempFile, "%s;\n", buffer);
            }
        }

        // 解析 <- 赋值行
        else if (strstr(buffer, "<-"))
        {
            isInFunction = 0;
            doAssignValue(token, buffer, tempHFile);
        }

        // 解析 print 语句
        else if (isFirstWord(buffer, "print") && strstr(buffer, "print"))
        {
            isInFunction = 0;
            doPrintValue(token, buffer, tempFile);
        }

        // 解析 function 语句
        else if (isFirstWord(buffer, "function") && strstr(buffer, "function"))
        {
            isInFunction = 1;
            doFunctionName(token, buffer, tempHFile);
        }
    }

    fputs("\treturn 0;\n", tempFile);
    fputs("}\n", tempFile);

    fclose(programFile);
    fclose(tempFile);
    fclose(tempHFile);

    system("gcc temp.c -o temp");
    system("./temp");
    // system("rm -rf temp");
    // system("rm -rf temp.c");
    // system("rm -rf temp.h");
    return EXIT_SUCCESS;
}

int isFirstWord(const char *str, const char *target)
{
    char firstWord[1024]; // 存储第一个单词
    int i = 0;

    // 提取第一个单词，直到遇到空格或字符串结束
    while (str[i] != ' ' && str[i] != '\0')
    {
        firstWord[i] = str[i];
        i++;
    }
    firstWord[i] = '\0'; // 添加字符串结束符

    // 判断第一个单词是否是 "target", print或者function
    return strcmp(firstWord, target) == 0;
}

int firstFourCharsAreWhitespace(const char *str)
{
    // 检查前四个字符是否都是空白字符
    for (int i = 0; i < 4; i++)
    {
        if (str[i] == '\0')
        {
            // 字符串长度小于4，不满足条件
            return 0;
        }
        if (!isspace(str[i]))
        {
            // 如果发现一个字符不是空白字符，则返回 false (0)
            return 0;
        }
    }
    // 如果前四个字符都是空白字符，返回 true (1)
    return 1;
}

int isFunctionInvoke(const char *str)
{
    // 分别检查字符 '('、',' 和 ')' 是否存在
    if (strchr(str, '(') != NULL && strchr(str, ',') != NULL && strchr(str, ')') != NULL)
    {
        return 1; // 如果都存在，返回 true (1)
    }
    return 0; // 如果有一个不存在，返回 false (0)
}

int containsDigit(const char *str)
{
    while (*str)
    {
        if (isdigit(*str)) // 判断当前字符是否为数字
        {
            return 1; // 如果找到数字，返回1
        }
        str++;
    }
    return 0; // 如果没有找到数字，返回0
}

void doAssignValue(char *token, char buffer[1024], FILE *outputFile)
{
    Statement stat;
    char tempBuffer[1024];
    strcpy(tempBuffer, buffer);
    char *ptr = buffer;
    if (*ptr == '\t')
    {
        ptr++;
    }
    token = strtok(ptr, " ");
    if (token)
    {
        strcpy(stat.x, token);
        token = strtok(NULL, " ");
    }
    if (token)
    {
        strcpy(stat.operator, token);
        token = strtok(NULL, " ");
    }
    if (token)
    {
        // 如果是一个变量给x赋值，x <- a * b， 例如token是a * b
        if (!containsDigit(token))
        {
            // 查找 "<- " 的位置
            char *tempToken = strstr(tempBuffer, "<- ");
            if (tempToken != NULL)
            {
                // 移动指针到 "<- " 之后的字符
                tempToken += strlen("<- ");

                // 将 "<- " 后面的内容复制到 variable
                char variable[1024];
                strcpy(variable, tempToken); // 复制 "a * b" 到 variable

                // 输出到文件
                fprintf(outputFile, "\tint %s = %s;\n", stat.x, variable);
            }
        }

        else if (strchr(token, '.'))
        {
            stat.DataType.DataTypeDouble = atof(token);
            fprintf(outputFile, "\tconst double %s = %f;\n", stat.x, stat.DataType.DataTypeDouble);
        }
        else
        {
            stat.DataType.DataTypeInt = atoi(token);
            fprintf(outputFile, "\tconst int %s = %d;\n", stat.x, stat.DataType.DataTypeInt);
        }
        token = strtok(NULL, " ");
    }
}

void doPrintValue(char *token, char buffer[1024], FILE *outputFile)
{
    token = strtok(buffer, " ");
    char *x = strtok(NULL, "");
    if (x)
    {
        fprintf(outputFile,
                "\tif ((%s) == (int)(%s)) {\n"
                "\t\tprintf(\"%%d\\n\", (int)(%s));\n"
                "\t} else {\n"
                "\t\tprintf(\"%%.6f\\n\", (double)(%s));\n"
                "\t}\n"
                "return 0;\n ",
                x,
                x, x, x);
    }
}

void doFunctionName(char *token, char buffer[1024], FILE *outputFile)
{
    FunctionStatement funcStat;
    funcStat.parameterCount = 0;

    // 分析函数名和参数
    token = strtok(buffer, " ");
    token = strtok(NULL, " "); // 获取函数名
    strcpy(funcStat.functionName, token);

    // 获取参数
    while ((token = strtok(NULL, " ")) != NULL)
    {
        strcpy(funcStat.parameters[funcStat.parameterCount], token);
        funcStat.parameterCount++;
    }

    // 在 temp.h 中生成函数声明
    fprintf(outputFile, "int %s(", funcStat.functionName);
    for (int i = 0; i < funcStat.parameterCount; i++)
    {
        fprintf(outputFile, "int %s", funcStat.parameters[i]);
        if (i < funcStat.parameterCount - 1)
        {
            fprintf(outputFile, ", ");
        }
    }
    fprintf(outputFile, "){\n");
}

void doFunctionBody(char *token, char buffer[1024], FILE *outputFile)
{
    // 这一句是return
    if (strstr(buffer, "return"))
    {
        fprintf(outputFile, "%s;", buffer);
        return;
    }
    if (strstr(buffer, "<-"))
    {
        doAssignValue(token, buffer, outputFile);
    }
    else if ((firstFourCharsAreWhitespace(buffer) || buffer[0] == '\t') && strstr(buffer, "print"))
    {
        doPrintValue(token, buffer, outputFile);
    }
}