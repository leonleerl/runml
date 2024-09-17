#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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

int isFirstWord(const char *str, char *target)
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

int main(int argc, char *argv[])
{
    // if (argc < 2)
    // {
    //     printf("必须要有一个文件输入\n");
    //     return EXIT_FAILURE;
    // }

    // FILE *programFile = fopen(argv[1], "r");
    FILE *programFile = fopen("./program.ml", "r");
    FILE *tempFile = fopen("./temp.c", "w");
    FILE *toolsFile = fopen("./tools.h", "w");
    char buffer[1024];
    char *token;
    int isInFunction = 0;
    fputs("#include <stdio.h>\n\n", tempFile);
    fputs("int main(int argc, char *argv[]) {\n", tempFile);

    while (fgets(buffer, sizeof(buffer), programFile))
    {

        // 跳过注释行
        if (buffer[0] == '#')
        {
            continue;
        }

        if (isInFunction && (firstFourCharsAreWhitespace(buffer) || buffer[0] == '\t'))
        {
            printf("Hello World");
        }

        // 解析 <- 赋值行
        else if (strstr(buffer, "<-"))
        {
            isInFunction = 0;
            Statement stat;
            token = strtok(buffer, " ");
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
                if (strchr(token, '.'))
                {
                    stat.DataType.DataTypeDouble = atof(token);
                    fprintf(tempFile, "\tdouble %s = %f;\n", stat.x, stat.DataType.DataTypeDouble);
                }
                else
                {
                    stat.DataType.DataTypeInt = atoi(token);
                    fprintf(tempFile, "\tint %s = %d;\n", stat.x, stat.DataType.DataTypeInt);
                }
                token = strtok(NULL, " ");
            }
        }

        // 解析 print 语句
        else if (isFirstWord(buffer, "print") && strstr(buffer, "print"))
        {
            isInFunction = 0;
            token = strtok(buffer, " ");
            char *x = strtok(NULL, "");
            if (x)
            {
                fprintf(tempFile,
                        "\tif ((%s) == (int)(%s)) {\n"
                        "\t\tprintf(\"%%d\\n\", (int)(%s));\n"
                        "\t} else {\n"
                        "\t\tprintf(\"%%.6f\\n\", (double)(%s));\n"
                        "\t}\n",
                        x, x, x, x);
            }
        }

        // 解析 function 语句
        else if (isFirstWord(buffer, "function") && strstr(buffer, "function"))
        {
            isInFunction = 1;
            printf("进入函数了!\n"); // 用于调试的输出
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

            // 在 tools.h 中生成函数声明
            fprintf(toolsFile, "int %s(", funcStat.functionName);
            for (int i = 0; i < funcStat.parameterCount; i++)
            {
                fprintf(toolsFile, "int %s", funcStat.parameters[i]);
                if (i < funcStat.parameterCount - 1)
                {
                    fprintf(toolsFile, ", ");
                }
            }
            fprintf(toolsFile, "){\n");

            // // 在 temp.c 中插入函数调用
            // fprintf(tempFile, "\t%s(", funcStat.functionName);
            // for (int i = 0; i < funcStat.parameterCount; i++)
            // {
            //     fprintf(tempFile, "%s", funcStat.parameters[i]);
            //     if (i < funcStat.parameterCount - 1)
            //     {
            //         fprintf(tempFile, ", ");
            //     }
            // }
            // fprintf(tempFile, ");\n");
        }
    }

    fputs("\treturn 0;\n", tempFile);
    fputs("}\n", tempFile);

    fclose(programFile);
    fclose(tempFile);
    fclose(toolsFile);

    system("gcc temp.c -o temp");
    system("./temp");

    return EXIT_SUCCESS;
}
