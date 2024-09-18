//  CITS2002 Project 1 2024
//  Student1:   24169259   Leon Li
//  Platform:   Linux  (or Apple)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

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

int is_first_word(const char *str, const char *target);
int first_four_chars_are_whitespace(const char *str);
int is_function_invoke(const char *str);
int contains_digit(const char *str);
int is_str_length_more_than_12(const char *str);
void remove_new_line_character(char *str);

void do_assign_value(char *token, char buffer[1024], FILE *outputFile, int isInFunction);
void do_print_value(char *token, char buffer[1024], FILE *outputFile, int isInFunction);
void do_function_name(char *token, char buffer[1024], FILE *outputFile);
void do_function_body(char *token, char buffer[1024], FILE *outputFile, int isInFunction);

static int identifierCount = 0;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <ml-program>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (strstr(argv[1], ".ml") == NULL)
    {
        fprintf(stderr, "! Input Error: file must have a '.ml' extension.\n");
        return EXIT_FAILURE;
    }
    pid_t pid = getpid();
    char tempCFileName[128];
    char tempHFileName[128];
    char tempExeFileName[128];
    snprintf(tempCFileName, sizeof(tempCFileName), "ml-%d.c", pid);
    snprintf(tempHFileName, sizeof(tempHFileName), "ml-%d.h", pid);
    snprintf(tempExeFileName, sizeof(tempExeFileName), "ml-%d", pid);

    FILE *programFile = fopen(argv[1], "r");
    FILE *tempCFile = fopen(tempCFileName, "w");
    FILE *tempHFile = fopen(tempHFileName, "w");
    char buffer[1024];
    char *token = "";
    int isInFunction = 0;
    fputs("#include <stdio.h>\n", tempCFile);
    fprintf(tempCFile, "#include \"ml-%d.h\"\n\n", pid);
    fputs("int main(int argc, char *argv[]) {\n", tempCFile);

    while (fgets(buffer, sizeof(buffer), programFile))
    {
        // 跳过注释行
        if (buffer[0] == '#')
        {
            fprintf(stdout, "@ Ignore comment line \"#\"\n");
            continue;
        }

        // 执行function里面的语句
        if (isInFunction && (first_four_chars_are_whitespace(buffer) || buffer[0] == '\t'))
        {
            do_function_body(token, buffer, tempHFile, isInFunction);
        }

        // 判断是否function结束
        if (isInFunction == 1 && !first_four_chars_are_whitespace(buffer) && buffer[0] != '\t')
        {
            fprintf(tempHFile, "%s", "\treturn 0;\n}\n");
            isInFunction = 0;
        }

        if (isInFunction && !(first_four_chars_are_whitespace(buffer) || buffer[0] == '\t'))
        {
            fprintf(stdout, "! statement inside function should start with \\t\n");
            exit(EXIT_FAILURE);
        }
        // 判断是否是函数调用语句
        if (is_function_invoke(buffer))
        {
            // 1.是print multiply(12, 6)
            if (strncmp(buffer, "print ", 6) == 0)
            {
                char *invokeString = strstr(buffer, "print ");
                invokeString += strlen("print ");
                fprintf(stdout, "@ Printing function: %s\n", invokeString);
                fprintf(tempCFile, "printf(\"%%d\\n\",%s);\n", invokeString);
            }
            // 2.是printsum (12, 6)
            else
            {
                fprintf(stdout, "@ Calling function: %s\n", buffer);
                fprintf(tempCFile, "%s;\n", buffer);
            }
        }

        // 解析 <- 赋值行
        else if (strstr(buffer, "<-"))
        {
            isInFunction = 0;
            identifierCount++;
            do_assign_value(token, buffer, tempHFile, isInFunction);
        }

        // 解析 print 语句
        else if (is_first_word(buffer, "print") && strstr(buffer, "print"))
        {
            isInFunction = 0;
            do_print_value(token, buffer, tempCFile, isInFunction);
        }

        // 解析 function 语句
        else if (is_first_word(buffer, "function") && strstr(buffer, "function"))
        {
            isInFunction = 1;
            do_function_name(token, buffer, tempHFile);
        }
    }

    fputs("\treturn 0;\n", tempCFile);
    fputs("}\n", tempCFile);

    fclose(programFile);
    fclose(tempCFile);
    fclose(tempHFile);
    // 编译生成的 .c 文件
    char compileCommand[256];
    snprintf(compileCommand, sizeof(compileCommand), "gcc %s -o %s", tempCFileName, tempExeFileName);
    system(compileCommand);

    // 执行生成的可执行文件
    char execCommand[256];
    snprintf(execCommand, sizeof(execCommand), "./%s", tempExeFileName);
    system(execCommand);

    // 删除生成的.c, .h 和可执行文件
    remove(tempCFileName); // 删除 .c 文件
    remove(tempHFileName); // 删除 .h 文件

    char removeExeFileCommand[1024];
    snprintf(removeExeFileCommand, sizeof(removeExeFileCommand), "rm -rf %s", tempExeFileName);
    system(removeExeFileCommand); // 删除可执行文件
    return EXIT_SUCCESS;
}

int is_first_word(const char *str, const char *target)
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

int first_four_chars_are_whitespace(const char *str)
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

int is_function_invoke(const char *str)
{
    // 分别检查字符 '('、',' 和 ')' 是否存在
    if (strchr(str, '(') != NULL && strchr(str, ',') != NULL && strchr(str, ')') != NULL)
    {
        return 1; // 如果都存在，返回 true (1)
    }
    return 0; // 如果有一个不存在，返回 false (0)
}

int contains_digit(const char *str)
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

int is_str_length_more_than_12(const char *str)
{
    return strlen(str) > 12;
}

void remove_new_line_character(char *str)
{
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = '\0'; // 将最后一个字符替换为 '\0'
    }
}

void do_assign_value(char *token, char buffer[1024], FILE *outputFile, int isInFunction)
{
    if (identifierCount > 50)
    {
        fprintf(stderr, "! identifier number should be no more than 50\n");
        exit(EXIT_FAILURE);
    }
    identifierCount++;
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
        if (is_str_length_more_than_12(token))
        {
            fprintf(stderr, "! length of identifier should be no more than 12\n");
            exit(EXIT_FAILURE);
            return;
        }
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
        if (!contains_digit(token))
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
                remove_new_line_character(variable);
                if (isInFunction)
                {
                    fprintf(stdout, "@ (Inside function) Assigned value %s to %s\n", variable, stat.x);
                }
                else
                {
                    fprintf(stdout, "@ Assigned value %s to %s\n", variable, stat.x);
                }
                // 输出到文件
                fprintf(outputFile, "\tint %s = %s;\n", stat.x, variable);
            }
        }

        else if (strchr(token, '.'))
        {
            stat.DataType.DataTypeDouble = atof(token);
            remove_new_line_character(token);
            if (isInFunction)
            {
                fprintf(stdout, "@ (Inside function) Assigned value %s to %s\n", token, stat.x);
            }
            else
            {
                fprintf(stdout, "@ Assigned value %s to %s\n", token, stat.x);
            }
            fprintf(outputFile, "\tconst double %s = %f;\n", stat.x, stat.DataType.DataTypeDouble);
        }
        else
        {
            stat.DataType.DataTypeInt = atoi(token);
            remove_new_line_character(token);
            if (isInFunction)
            {
                fprintf(stdout, "@ (Inside function) Assigned value %s to %s\n", token, stat.x);
            }
            else
            {
                fprintf(stdout, "@ Assigned value %s to %s\n", token, stat.x);
            }
            fprintf(outputFile, "\tconst int %s = %d;\n", stat.x, stat.DataType.DataTypeInt);
        }
        token = strtok(NULL, " ");
    }
}

void do_print_value(char *token, char buffer[1024], FILE *outputFile, int isInFunction)
{
    token = strtok(buffer, " ");
    char *x = strtok(NULL, "");
    if (x)
    {
        remove_new_line_character(x);
        fprintf(outputFile,
                "\tif ((%s) == (int)(%s)) {\n"
                "\t\tprintf(\"%%d\\n\", (int)(%s));\n"
                "\t} else {\n"
                "\t\tprintf(\"%%.6f\\n\", (double)(%s));\n"
                "\t}\n",
                // "return 0;\n ",
                x,
                x, x, x);
        if (isInFunction)
        {
            fprintf(stdout, "@ (Inside function) printing value: %s\n", x);
        }
        else
        {
            fprintf(stdout, "@ printing value: %s\n", x);
        }
    }
}

void do_function_name(char *token, char buffer[1024], FILE *outputFile)
{
    fprintf(stdout, "@ Parsing function's name and parameters\n");

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

    // 在 ml-xxx.h 中生成函数声明
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

void do_function_body(char *token, char buffer[1024], FILE *outputFile, int isInFunction)
{
    // 这一句是return
    if (strstr(buffer, "return"))
    {
        fprintf(stdout, "@ (Inside function) Parsing statement: %s", buffer);
        fprintf(outputFile, "%s;", buffer);
        return;
    }
    if (strstr(buffer, "<-"))
    {
        do_assign_value(token, buffer, outputFile, isInFunction);
    }
    else if ((first_four_chars_are_whitespace(buffer) || buffer[0] == '\t') && strstr(buffer, "print"))
    {
        do_print_value(token, buffer, outputFile, isInFunction);
    }
}
