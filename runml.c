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
int first_n_chars_are_whitespace(const char *str, int n);
int is_function_invoke(const char *str);
int contains_digit(const char *str);
int is_str_length_more_than_12(const char *str);
void remove_new_line_character(char *str);
int is_real_number(const char *str);
void remove_all_temp_files();

void do_assign_value(char *token, char buffer[1024], FILE *outputFile, int isInFunction);
void do_print_value(char *token, char buffer[1024], FILE *outputFile, int isInFunction);
void do_function_name(char *token, char buffer[1024], FILE *outputFile);
void do_function_body(char *token, char buffer[1024], FILE *outputFile, int isInFunction);

static int identifierCount = 0;
static pid_t pid;
static int is_debug_mode = 0;
static char tempCFileName[128];
static char tempHFileName[128];
static char tempExeFileName[128];

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
    if (argc > 1)
    {
        for (int i = 2; i < argc; i++)
        {
            if (!is_real_number(argv[i]))
            {
                fprintf(stderr, "! program's command-line arguments must be real-valued numbers\n");
                return EXIT_FAILURE;
            }
        }
    }
    // enter debug mode
    if (argc > 2 && strcmp(argv[2], "1") == 0)
    {
        is_debug_mode = 1;
    }

    pid = getpid();

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
        // Skip comment lines
        if (buffer[0] == '#')
        {
            if (is_debug_mode)
            {
                fprintf(stdout, "@ Ignore comment line \"#\"\n");
            }
            continue;
        }

        // Execute statements inside the function
        if (isInFunction && (first_n_chars_are_whitespace(buffer, 4) || buffer[0] == '\t'))
        {
            do_function_body(token, buffer, tempHFile, isInFunction);
        }

        // Check if the function has ended
        if (isInFunction && !first_n_chars_are_whitespace(buffer, 4) && buffer[0] != '\t')
        {
            fprintf(tempHFile, "%s", "\treturn 0;\n}\n");
            isInFunction = 0;
        }

        // Statements inside the function must start with '\t'
        if (isInFunction && (!first_n_chars_are_whitespace(buffer, 4) || buffer[0] != '\t') &&
            (first_n_chars_are_whitespace(buffer, 8) || buffer[1] == '\t'))
        {

            fprintf(stderr, "! Usage: statement inside function must start with \\t\n");
            remove_all_temp_files();
            exit(EXIT_FAILURE);
        }

        // Check if it is a function call statement
        if (is_function_invoke(buffer))
        {
            // 1. If the function call is in the form "print multiply(12, 6)"
            if (strncmp(buffer, "print ", 6) == 0)
            {
                char *invokeString = strstr(buffer, "print ");
                invokeString += strlen("print ");
                if (is_debug_mode)
                {
                    fprintf(stdout, "@ Printing function: %s\n", invokeString);
                }
                fprintf(tempCFile, "printf(\"%%d\\n\",%s);\n", invokeString);
            }
            // 2. If the function call is in the form: "printsum (12, 6)"
            else
            {
                if (is_debug_mode)
                {
                    fprintf(stdout, "@ Calling function: %s\n", buffer);
                }
                fprintf(tempCFile, "%s;\n", buffer);
            }
        }

        // Parse the assignment line with "<-"
        else if (strstr(buffer, "<-"))
        {
            isInFunction = 0;
            identifierCount++;
            do_assign_value(token, buffer, tempHFile, isInFunction);
        }

        // Parse the print statement
        else if (is_first_word(buffer, "print") && strstr(buffer, "print"))
        {
            isInFunction = 0;
            do_print_value(token, buffer, tempCFile, isInFunction);
        }

        // Parse the function name and parameters
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

    char compileCommand[256];
    snprintf(compileCommand, sizeof(compileCommand), "gcc %s -o %s", tempCFileName, tempExeFileName);
    system(compileCommand);

    char execCommand[256];
    snprintf(execCommand, sizeof(execCommand), "./%s", tempExeFileName);
    system(execCommand);

    remove_all_temp_files();

    return EXIT_SUCCESS;
}

int is_first_word(const char *str, const char *target)
{
    char firstWord[1024];
    int i = 0;

    while (str[i] != ' ' && str[i] != '\0')
    {
        firstWord[i] = str[i];
        i++;
    }
    firstWord[i] = '\0';

    return strcmp(firstWord, target) == 0;
}

int first_n_chars_are_whitespace(const char *str, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (str[i] == '\0')
        {
            return 0;
        }
        if (!isspace(str[i]))
        {
            return 0;
        }
    }
    return 1;
}

int is_function_invoke(const char *str)
{
    if (strchr(str, '(') != NULL && strchr(str, ',') != NULL && strchr(str, ')') != NULL)
    {
        return 1;
    }
    return 0;
}

int contains_digit(const char *str)
{
    while (*str)
    {
        if (isdigit(*str))
        {
            return 1;
        }
        str++;
    }
    return 0;
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
        str[len - 1] = '\0';
    }
}

int is_real_number(const char *str)
{
    int has_decimal_point = 0;
    int has_digit = 0;

    if (*str == '+' || *str == '-')
    {
        str++;
    }

    while (*str)
    {
        if (isdigit(*str))
        {
            has_digit = 1;
        }
        else if (*str == '.')
        {
            if (has_decimal_point)
            {
                return 0;
            }
            has_decimal_point = 1;
        }
        else
        {
            return 0;
        }
        str++;
    }
    return has_digit;
}

void remove_all_temp_files()
{
    remove(tempCFileName);
    remove(tempHFileName);

    char removeExeFileCommand[1024];
    snprintf(removeExeFileCommand, sizeof(removeExeFileCommand), "rm -rf %s", tempExeFileName);
    system(removeExeFileCommand);
}

void do_assign_value(char *token, char buffer[1024], FILE *outputFile, int isInFunction)
{
    if (identifierCount > 50)
    {
        fprintf(stderr, "! identifier number should be no more than 50\n");
        remove_all_temp_files();
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
            fprintf(stderr, "! Usage: length of identifier should be no more than 12\n");
            remove_all_temp_files();
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
        // If a variable is assigned the result of other variables
        if (!contains_digit(token))
        {
            // Find the position of "<- "
            char *tempToken = strstr(tempBuffer, "<- ");
            if (tempToken != NULL)
            {
                // Move the pointer to the character after "<- "
                tempToken += strlen("<- ");

                // Copy the content after "<- " to the variable
                char variable[1024];
                strcpy(variable, tempToken);
                remove_new_line_character(variable);
                if (is_debug_mode)
                {
                    if (isInFunction)
                    {
                        fprintf(stdout, "@ (Inside function) Assigned value %s to %s\n", variable, stat.x);
                    }
                    else
                    {
                        fprintf(stdout, "@ Assigned value %s to %s\n", variable, stat.x);
                    }
                }
                fprintf(outputFile, "\tint %s = %s;\n", stat.x, variable);
            }
        }

        else if (strchr(token, '.'))
        {
            stat.DataType.DataTypeDouble = atof(token);
            remove_new_line_character(token);
            if (is_debug_mode)
            {
                if (isInFunction)
                {
                    fprintf(stdout, "@ (Inside function) Assigned value %s to %s\n", token, stat.x);
                }
                else
                {
                    fprintf(stdout, "@ Assigned value %s to %s\n", token, stat.x);
                }
            }
            fprintf(outputFile, "\tconst double %s = %f;\n", stat.x, stat.DataType.DataTypeDouble);
        }
        else
        {
            stat.DataType.DataTypeInt = atoi(token);
            remove_new_line_character(token);
            if (is_debug_mode)
            {
                if (isInFunction)
                {
                    fprintf(stdout, "@ (Inside function) Assigned value %s to %s\n", token, stat.x);
                }
                else
                {
                    fprintf(stdout, "@ Assigned value %s to %s\n", token, stat.x);
                }
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
                x,
                x, x, x);
        if (is_debug_mode)
        {
            if (isInFunction)
            {
                fprintf(stdout, "@ (Inside function) printing value %s\n", x);
            }
            else
            {
                fprintf(stdout, "@ printing value %s\n", x);
            }
        }
    }
}

void do_function_name(char *token, char buffer[1024], FILE *outputFile)
{
    if (is_debug_mode)
    {
        fprintf(stdout, "@ Parsing function's name and parameters\n");
    }
    FunctionStatement funcStat;
    funcStat.parameterCount = 0;

    token = strtok(buffer, " ");
    // Retrieve the function name
    token = strtok(NULL, " ");
    strcpy(funcStat.functionName, token);

    // Retrieve the parameters
    while ((token = strtok(NULL, " ")) != NULL)
    {
        strcpy(funcStat.parameters[funcStat.parameterCount], token);
        funcStat.parameterCount++;
    }

    // Generate function declaration in ml-xxx.h
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
    // If this statement is a return
    if (strstr(buffer, "return"))
    {
        if (is_debug_mode)
        {
            fprintf(stdout, "@ (Inside function) Parsing statement: %s", buffer);
        }
        fprintf(outputFile, "%s;", buffer);
        return;
    }
    // If this statement is "<-"
    if (strstr(buffer, "<-"))
    {
        do_assign_value(token, buffer, outputFile, isInFunction);
    }
    // If this statement is print
    else if ((first_n_chars_are_whitespace(buffer, 4) || buffer[0] == '\t') && strstr(buffer, "print"))
    {
        do_print_value(token, buffer, outputFile, isInFunction);
    }
}
