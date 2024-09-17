//  CITS2002 Project 1 2024
//  Student1:   24169259   Leon Li
//  Platform:   Apple Silicon
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
    int isFunction;
    int parameterCount;
} FunctionStatement;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("必须要有一个文件输入\n");
        return EXIT_FAILURE;
    }
    FILE *programFile = fopen(argv[1], "r");
    FILE *tempFile = fopen("./temp.c", "w");
    FILE *toolsFile = fopen("./tools.h", "a");
    char buffer[1024];
    char *token;
    fputs("#include <stdio.h>\n\n", tempFile);
    fputs("int main(int argc, char *argv[]) {\n", tempFile);
    while (fgets(buffer, sizeof(buffer), programFile))
    {
        // 跳过注释行
        if (buffer[0] == '#')
        {
            continue;
        }
        // <-赋值行
        if (strstr(buffer, "<-"))
        {
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
        // print行
        else if (strstr(buffer, "print"))
        {
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
        // function 行
        if (strstr(buffer, "function"))
        {
            FunctionStatement funcStat;
            token = strtok(buffer, " ");
            if (token)
            {
                funcStat.isFunction = 1;
                token = strtok(NULL, " ");
            }
            if (token)
            {
                strcpy(funcStat.functionName, token);
                token = strtok(NULL, " ");
            }
            if (token)
            {
                char *parameter;
                while ((parameter = strtok(NULL, " ")) != NULL)
                {
                    strcpy(funcStat.parameters[funcStat.parameterCount], parameter);
                    funcStat.parameterCount++;
                }
            }

            fprintf(toolsFile, "int %s(int %s, int %s){", funcStat.functionName, funcStat.parameters[0], funcStat.parameters[1]);
        }
    }

    fputs("\treturn 0;\n", tempFile);
    fputs("}\n", tempFile);

    fclose(programFile);
    fclose(tempFile);

    system("gcc temp.c -o temp");
    system("./temp");

    return EXIT_SUCCESS;
}
