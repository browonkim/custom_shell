//시스템 프로그래밍/ 2021/ myshell
//B735137/ 김형원

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>

//debug option
//#define DEBUG
#define _CRT_SECURE_NO_WARNINGS
//constant
#define infinite 1
#define BufferSize 1024
#define MAXSTRING 256

//error Flag
#define PIPE_FUN_ERROR 400
#define MALLOC_ALLOCATION_ERROR 401
#define FORK_ERROR 402
//run Flag
#define NO_PIPE 200
#define PIPE_BEGIN 201
#define PIPE_END 202
#define PIPE_MID 203

//option Flag
#define NO_STRING_RETURN_NULL 0
#define NOT_OPTION 100
#define BACKGROUND 101
#define INPUT_REDIRECTION 102
#define OUTPUT_REDIRECTION_W 103
#define OUTPUT_REDIRECTION_A 104
#define ERROR_REDIRECTION_W 105
#define ERROR_REDIRECTION_A 106
#define OUTPUT_AND_ERROR_REDIRECTION_W 107
#define OUTPUT_AND_ERROR_REDIRECTION_A 108

#define SUCCESS 1
#define FAILED 0

enum boolean
{
    False = 0,
    True = 1
} typedef boolean;

struct range
{
    int start;
    int end;
} typedef range;

FILE *opened_files[64] = {
    NULL,
};
int num_of_opened;
void close_all_files();

int backup_stdin;
int backup_stdout;
int backup_stderr;

void todo()
{
    printf("NOT IMPLEMENTED YET!\n");
}

//functions
int syntax_error(char *string);
void exception_cleanse(char **list, int listSize);
void get_command(char *buffer, size_t bufferSize);
void split_pipe(char *buffer, char **list, int *listSize);
void tokenizeCommand(char *buffer, char **list, int *listSize);
void split_command(char *command, char **list, int *listSize);
int run_command(char **list, int flag);
void parse_command();
void pop_pipe(char **pipeQueue, int queueSize);
void execute_command(char **list, char option);
void clear_list(char **list);
void error(int errorFlag);
int check_redirection(char *string);
int redirection_file(char *filename, const char *mode, int replaced);

int main(void)
{
    char commandBuffer[BufferSize];
    const size_t commandBufSize = sizeof(commandBuffer);

    size_t commonSize = sizeof(char *) * MAXSTRING;

    char **pipeQueue = (char **)malloc(commonSize);
    memset(pipeQueue, 0, commonSize);
    int pipeSize = 0;

    int i = 0;

    while (infinite)
    {
        backup_stdin = dup(STDIN_FILENO);
        backup_stdout = dup(STDOUT_FILENO);
        backup_stderr = dup(STDERR_FILENO);

        memset(commandBuffer, '\0', commandBufSize);
        printf("$ ");
        get_command(commandBuffer, commandBufSize);
        split_pipe(commandBuffer, pipeQueue, &pipeSize);
        pop_pipe(pipeQueue, pipeSize);
        clear_list(pipeQueue);
        close_all_files();

        dup2(backup_stdin, STDIN_FILENO);
        dup2(backup_stdout, STDOUT_FILENO);
        dup2(backup_stderr, STDERR_FILENO);
        close(backup_stdin);
        close(backup_stdout);
        close(backup_stderr);
    }
    free(pipeQueue);
}

void get_command(char *buffer, size_t bufferSize)
{
    if (fgets(buffer, bufferSize, stdin) == NULL)
    {
        if (feof(stdin))
            exit(0);
        else if (ferror(stdin))
        {
            fprintf(stderr, "FATAL ERROR!\n");
            exit(1);
        }
    }
    //remove '\n'
    buffer[strlen(buffer) - 1] = '\0';
#ifdef DEBUG
    printf("%s\n", buffer);
#endif
}

void clear_list(char **list)
{
    int i = 0;
    while (list[i] != NULL)
    {
        free(list[i]);
        list[i] = NULL;
        i++;
    }
}

void split_pipe(char *buffer, char **pipeQueue, int *queueSize)
{
    int i;
    *queueSize = 0;
    char *token = strtok(buffer, "|");
    while (token != NULL)
    {
        pipeQueue[*queueSize] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(pipeQueue[*queueSize], token);
        (*queueSize)++;
        token = strtok(NULL, "|");
    }
    pipeQueue[*queueSize] = NULL;

#ifdef DEBUG
    for (i = 0; i < (*queueSize); i++)
    {
        printf("%d) pipe: %s\n", i + 1, pipeQueue[i]);
    }
#endif
}

void pop_pipe(char **pipeQueue, int queueSize)
{
    int i;

    char *list[64][64] = {{
        NULL,
    }};
    int listSize = 0;
    for (i = 0; i < queueSize; i++)
    {
        split_command(pipeQueue[i], list[i], &listSize);
        if (listSize == 0)
        {
            if (i == 0)
            {
                printf("myshell: syntax error near by |\n");
                int j;
                for (j = 0; j < 64; j++)
                {
                    exception_cleanse(list[j], 64);
                }
                return;
            }
            else
            {
                int j;
                for (j = 0; j < 64; j++)
                {
                    exception_cleanse(list[j], 64);
                }
                return;
            }
        }
        //일단 non emplementation이라고 하고 return 시켜버리기
        //즉 어떤 경우에도 listSize는 공백이 아닌 어떤 문자를 가지고 있어야한다는 뜻
    }
    switch (queueSize)
    {
    case 0:
        break;
    case 1:
        run_command(list[0], NO_PIPE);
        break;
    default:
        for (i = 0; i < queueSize; i++)
        {
            if (i == 0)
            {
                run_command(list[i], PIPE_BEGIN);
            }
            else if ((i + 1) < queueSize)
            {
                run_command(list[i], PIPE_MID);
            }
            else
            {
                run_command(list[i], PIPE_END);
            }
        }
        break;
    }
    for (i = 0; i < 64; i++)
    {
        if (list[i] == NULL)
            break;
        clear_list(list[i]);
    }
}

void split_command(char *command, char **list, int *listSize)
{
    int i; //for debug
    *listSize = 0;
    char *token = strtok(command, " \n");
    while (token != NULL)
    {
        list[*listSize] = (char *)malloc(sizeof(char) * strlen(token));
        if (list[*listSize] == NULL)
        {
            error(MALLOC_ALLOCATION_ERROR);
        }
        strcpy(list[*listSize], token);
        (*listSize)++;
        token = strtok(NULL, " \n");
    }
    list[*listSize] = NULL;
#ifdef DEBUG
    i = 0;
    while (list[i] != NULL)
    {
        printf("[ %s ] ", list[i]);
        i++;
    }
    printf("\n");
#endif
}

int run_command(char **list, int flag)
{
    
    char output_filename[1024] = {'\0'};
    int argc = 0;
    int i = 0;
    //FILE *file = NULL;
    int pipeid[2];
    int status;
    if (pipe(pipeid) < 0)
        error(PIPE_FUN_ERROR);

#ifdef DEBUG
    printf("debug: %d %d\n", pipeid[0], pipeid[1]);
#endif
    
    //fork
    pid_t pid = fork();
    
    if (pid < 0)
        error(FORK_ERROR);
    
    else if (pid > 0)
    {
        //parent
        int is_there_an_output_redirection = 0;
        while (list[i] != NULL)
        {
            switch (check_redirection(list[i]))
            {
            case BACKGROUND:
                todo();
                break;
            case OUTPUT_REDIRECTION_W:
                i++;
                if (check_redirection(list[i]) != NOT_OPTION)
                {
                    //syntaxerror
                }
                else
                {
                    is_there_an_output_redirection = 1;
                    strcpy(output_filename, list[i]);
                }
                break;
            case OUTPUT_REDIRECTION_A:
                i++;
                if (check_redirection(list[i]) != NOT_OPTION)
                {
                    //syntaxerror
                }
                else
                {
                    is_there_an_output_redirection = 1;
                    strcpy(output_filename, list[i]);
                }
                break;
            case OUTPUT_AND_ERROR_REDIRECTION_W:
                todo();
                i++;
                break;
            case OUTPUT_AND_ERROR_REDIRECTION_A:
                todo();
                i++;
                break;
            }
            i++;
        }

#ifdef DEBUG
        printf("debug: command arguments without redirection options\n");
        for (i = 0; argv[i] != NULL; i++)
        {
            printf("[ %s ] ", argv[i]);
        }
        printf("\n");
#endif
        
        if (flag == NO_PIPE)
        {
            wait(&status);
            return SUCCESS;
        }
        else if (flag == PIPE_BEGIN)
        {
            //close write pipe
            close(pipeid[1]);
            while (wait(&status) > 0)
                ;
            //close(STDIN_FILENO);
            if (is_there_an_output_redirection == 0)
                dup2(pipeid[0], STDIN_FILENO);
            else
            {
                int fd = open(output_filename, O_RDONLY | O_NONBLOCK);
                if (fd < 0)
                {
                    printf("error! %s\n", strerror(errno));
                    exit(9);
                }
                else
                    dup2(fd, STDIN_FILENO);
            }
        }
        else if (flag == PIPE_END)
        {
            close(pipeid[1]);
            while (wait(&status) > 0)
                ;
            return SUCCESS;
        }
        else if (flag == PIPE_MID)
        {
            close(pipeid[1]);
            while (wait(&status) < 0)
                ;
            if (is_there_an_output_redirection == 0)
                dup2(pipeid[0], STDIN_FILENO);
            else
            {
                int fd = open(output_filename, O_RDONLY | O_NONBLOCK);
                if (fd < 0)
                {
                    printf("error! %s\n", strerror(errno));
                    exit(9);
                }
                else
                    dup2(fd, STDIN_FILENO);
            }
        }
    }
    else
    {
        //child
        int redirection_fileno[3] = {-1, -1, -1}; //i o e
        char *argv[64] = {
            NULL,
        };
        while (list[i] != NULL)
        {
            switch (check_redirection(list[i]))
            {
            case BACKGROUND:
                todo();
                break;
            case INPUT_REDIRECTION:
                i++;
                if (check_redirection(list[i]) != NOT_OPTION)
                {
                    exception_cleanse(argv, 64);
                    return syntax_error(list[i]);
                }
                else
                    redirection_fileno[0] = redirection_file(list[i], "r", STDIN_FILENO);
                break;
            case OUTPUT_REDIRECTION_W:
                i++;
                if (check_redirection(list[i]) != NOT_OPTION)
                {
                    exception_cleanse(argv, 64);
                    return syntax_error(list[i]);
                }
                else
                {
                    strcpy(output_filename, list[i]);
                    redirection_fileno[1] = redirection_file(list[i], "w", STDOUT_FILENO);
                }
                break;
            case OUTPUT_REDIRECTION_A:
                i++;
                if (check_redirection(list[i]) != NOT_OPTION)
                {
                    exception_cleanse(argv, 64);
                    return syntax_error(list[i]);
                }
                else
                {
                    strcpy(output_filename, list[i]);
                    redirection_fileno[1] = redirection_file(list[i], "a", STDOUT_FILENO);
                }
                break;
            case ERROR_REDIRECTION_W:
                i++;
                if (check_redirection(list[i]) != NOT_OPTION)
                {
                    exception_cleanse(argv, 64);
                    return syntax_error(list[i]);
                }
                else
                    redirection_fileno[2] = redirection_file(list[i], "w", STDERR_FILENO);
                break;
            case ERROR_REDIRECTION_A:
                i++;
                if (check_redirection(list[i]) != NOT_OPTION)
                {
                    exception_cleanse(argv, 64);
                    return syntax_error(list[i]);
                }
                else
                    redirection_fileno[2] = redirection_file(list[i], "a", STDERR_FILENO);
                break;
            case OUTPUT_AND_ERROR_REDIRECTION_W:
                todo();
                i++;
                break;
            case OUTPUT_AND_ERROR_REDIRECTION_A:
                todo();
                i++;
                break;
            default:
                argv[argc] = (char *)malloc(sizeof(char) * strlen(list[i]));
                strcpy(argv[argc], list[i]);
                argc++;
            }
            i++;
        }
        argv[argc] = NULL;

#ifdef DEBUG
        printf("debug: command arguments without redirection options\n");
        for (i = 0; argv[i] != NULL; i++)
        {
            printf("[ %s ] ", argv[i]);
        }
        printf("\n");
#endif

        if (flag == NO_PIPE)
        {
            if (execvp(argv[0], argv) < 0)
            {
                printf("myshell: %s\n", strerror(errno));
                clear_list(argv);
                exit(4);
            }
        }
        else if (flag == PIPE_BEGIN)
        {
            //close read pipe
            close(pipeid[0]);
            //close(STDOUT_FILENO);
            if (redirection_fileno[1] == -1)
                dup2(pipeid[1], STDOUT_FILENO);
            else
            {
                dup2(redirection_fileno[1], STDOUT_FILENO);
            }
            execvp(argv[0], argv);
            FILE *fp = fdopen(backup_stderr, "a");
            fprintf(fp, "myshell: %s\n", strerror(errno));
            //fclose(fp);
            clear_list(argv);
            exit(4);
        }
        else if (flag == PIPE_END)
        {
            close(pipeid[0]);
            execvp(argv[0], argv);
            FILE *fp = fdopen(backup_stderr, "a");
            fprintf(fp, "myshell: %s\n", strerror(errno));
            //fclose(fp);
            clear_list(argv);
            exit(4);
        }
        else if (flag == PIPE_MID)
        {
            close(pipeid[0]);
            if (redirection_fileno[1] == -1)
                dup2(pipeid[1], STDOUT_FILENO);
            else
            {
                dup2(redirection_fileno[1], STDOUT_FILENO);
            }
            execvp(argv[0], argv);
            FILE *fp = fdopen(backup_stderr, "a");
            fprintf(fp, "myshell: %s\n", strerror(errno));
            //fclose(fp);
            clear_list(argv);
            exit(4);
        }
    }
}

int check_redirection(char *string)
{
    if (string == NULL)
        return NO_STRING_RETURN_NULL;
    if (strcmp(string, "&") == 0)
        return BACKGROUND;
    else if (strcmp(string, "&>") == 0)
        return OUTPUT_AND_ERROR_REDIRECTION_W;
    else if (strcmp(string, ">&") == 0)
        return OUTPUT_AND_ERROR_REDIRECTION_W;
    else if (strcmp(string, "2>") == 0)
        return ERROR_REDIRECTION_W;
    else if (strcmp(string, ">2") == 0)
        return ERROR_REDIRECTION_W;
    else if (strcmp(string, ">>") == 0)
        return OUTPUT_REDIRECTION_A;
    else if (strcmp(string, "2>>") == 0)
        return ERROR_REDIRECTION_A;
    else if (strcmp(string, "&>>") == 0)
        return OUTPUT_AND_ERROR_REDIRECTION_A;
    else if (strcmp(string, "<") == 0)
        return INPUT_REDIRECTION;
    else if (strcmp(string, "<0") == 0)
        return INPUT_REDIRECTION;
    else if (strcmp(string, "0<") == 0)
        return INPUT_REDIRECTION;
    else if (strcmp(string, ">") == 0)
        return OUTPUT_REDIRECTION_W;
    else if (strcmp(string, "1>") == 0)
        return OUTPUT_REDIRECTION_W;
    else if (strcmp(string, "1>>") == 0)
        return OUTPUT_REDIRECTION_A;
    else
        return NOT_OPTION;
}

void error(int errorFlag)
{
    switch (errorFlag)
    {
    case PIPE_FUN_ERROR:
        fprintf(stderr, "myshell: pipe creation error!\n");
        exit(3);
        break;
    case MALLOC_ALLOCATION_ERROR:
        fprintf(stderr, "myshell: fatal! memory allocation error!\n");
        exit(4);
        break;
    default:
        fprintf(stderr, "myshell: unknown error!\n");
        exit(5);
    }
}

void close_all_files()
{
    int i;
    for (i = 0; i < 64; i++)
    {
        if (opened_files[i] == NULL)
            continue;
        else
        {
            fclose(opened_files[i]);
            opened_files[i] = NULL;
        }
    }
    num_of_opened = 0;
}

int syntax_error(char *string)
{
    char *temp = string;
    if (temp == NULL)
        temp = "newline";
    printf("myshell: syntax error near unexpected token '%s'\n", temp);
    return FAILED;
}

int redirection_file(char *filename, const char *mode, int replaced)
{
    FILE *file;
    file = fopen(filename, mode);
    if (file == NULL)
    {
        printf("%s\n", strerror(errno));
        return -1;
    }
    else
    {
        opened_files[num_of_opened] = file;
        num_of_opened++;
        int fno = fileno(file);
        close(replaced);
        dup2(fno, replaced);
        return fno;
    }
}

void exception_cleanse(char **list, int listSize)
{
    int i;
    for (i = 0; i < listSize; i++)
    {
        if (list[i] != NULL)
        {
            free(list[i]);
            list[i] = NULL;
        }
    }
}