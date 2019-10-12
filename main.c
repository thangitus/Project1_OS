#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LINE 80
#define LSH_TOK_BUFSIZE 10
char *builtin_str[] = {"help", "exit"};

int lsh_num_builtins();
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int (*builtin_func[])(char **) = {&lsh_help, &lsh_exit}; ///

char *lsh_read_line();
char **lsh_split_line(char *line, int *count);
int lsh_launch(char **args, int *countArgs);
int lsh_execute(char **args, int *countArgs);
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

int main(int argc, char **argv)
{
    char **args, **previousArgs = NULL;
    int countArgs;
    int status = 1;
    char *line;

    do
    {
        printf("\nosh> ");
        line = lsh_read_line();
        args = lsh_split_line(line, &countArgs);

        if (strcmp(args[0], "!!") == 0)
        {
            if (previousArgs)
                status = lsh_execute(previousArgs, &countArgs);
            else
            {
                printf("There is no history in memory\n");
            }
        }
        else
            status = lsh_execute(args, &countArgs);

        if (!previousArgs)
            previousArgs = (char **)realloc(previousArgs, countArgs * sizeof(char *));
        else
            previousArgs = (char **)malloc(countArgs * sizeof(char *));

        for (int i = 0; i < countArgs; i++)
        {
            previousArgs[i] = (char *)malloc(strlen(args[i]) + 1);
            strcpy(previousArgs[i], args[i]);
        }

        free(line);
        for (int i = 0; i < countArgs; i++)
            free(args[i]);
        free(args);

    } while (status);
    for (int i = 0; i < countArgs; i++)
        free(previousArgs[i]);
    free(previousArgs);
    return 0;
}

char *lsh_read_line()
{
    char *line;
    size_t bufsize = MAX_LINE;
    line = (char *)malloc(bufsize * sizeof(char));
    if (line == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    getline(&line, &bufsize, stdin);
    return line;
}

char **lsh_split_line(char *line, int *count)
{
    int bufsize = 80, position = 0;
    char **tokens = (char **)malloc(bufsize * sizeof(char *));
    char *token;

    token = strtok(line, " \t\r\n\a");
    while (token != NULL)
    {
        tokens[position] = (char *)malloc((strlen(token) + 1) * sizeof(char));
        strcpy(tokens[position], token);
        position++;

        if (position >= bufsize)
        {
            bufsize += 80;
            tokens = (char **)realloc(tokens, bufsize * sizeof(char *));
        }

        token = strtok(NULL, " \t\r\n\a");
    }
    tokens = (char **)realloc(tokens, position * sizeof(char *));
    *count = position;
    return tokens;
}

int lsh_launch(char **args, int *countArgs)
{
    pid_t pid, wpid;
    int status;
    int count = *countArgs;
    pid = fork();

    if (pid == 0)
    {

        // Child process
        if (strcmp(args[count - 1], "&") == 0)
        {
            count--;
            char **temp = (char **)malloc((count) * sizeof(char *));
            for (int i = 0; i < count; i++)
            {
                temp[i] = (char *)malloc(strlen(args[i]) + 1);
                strcpy(temp[i], args[i]);
            }

            execvp(temp[0], temp);
            for (int i = 0; i < count; i++)
                free(temp[i]);
            free(temp);
        }
        else if (execvp(args[0], args) == -1)
        {
            perror("lsh");
        }

        return 0;
    }
    else if (pid < 0)
    {
        // Error forking
        perror("lsh");
    }
    else
    {

        // Parent process
        if (strcmp(args[count - 1], "&") == 0)
        {
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            --(*countArgs);
        }
    }
    return 1;
}

int lsh_execute(char **args, int *countArgs)
{
    int i;

    if (args[0] == NULL)
    {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args, countArgs);
}

int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_help(char **args)
{
    int i;
    printf("The following are built in:\n");

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        printf("%s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char **args)
{
    return 0;
}
