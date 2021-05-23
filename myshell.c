//시스템 프로그래밍/ 2021/ myshell
//B735137/ 김형원

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#define DEBUG

#define infinite 1
#define BufferSize 1024
#define MAXSTRING 256

//error Flag
#define PIPE_FUN_ERROR 400

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

typedef int todo;

// get a command from standard input stream
void getCommand(char *buffer, size_t bufferSize);
void parsing_pipe(char *buffer, char **list, int *listSize);
void tokenizeCommand(char *buffer, char **list, int *listSize);
void parsing_command(char* command, char** list);
void run_command();
void pop_pipe(char **pipeQueue, int queueSize);
void clearList(char **list);
void error(int errorFlag);

int main()
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
        memset(commandBuffer, '\0', commandBufSize);
        printf("$ ");
        getCommand(commandBuffer, commandBufSize);
        parsing_pipe(commandBuffer, pipeQueue, &pipeSize);
        pop_pipe(pipeQueue, pipeSize);
        clearList(pipeQueue);
    }
    free(pipeQueue);
}

void getCommand(char *buffer, size_t bufferSize)
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

void tokenizeCommand(char *buffer, char **list, int *listSize)
{
    int i;
    char *token = strtok(buffer, " \n");
    *listSize = 0;
    while (token != NULL)
    {
        list[*listSize] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(list[(*listSize)], token);
        (*listSize)++;
        token = strtok(NULL, " \n");
    }
    list[*listSize] = NULL;

#ifdef DEBUG
    for (i = 0; i < *listSize; i++)
    {
        printf("%s ", list[i]);
    }
    printf("token size = %d\n", *listSize);
#endif
}

void clearList(char **list)
{
    int i = 0;
    while(list[i] != NULL)
    {
        free(list[i]);
        list[i] = NULL;
        i++;
    }
}

void parsing_pipe(char *buffer, char **pipeQueue, int *queueSize)
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
        printf("first pipe: %s | \n", pipeQueue[i]);
    }
#endif
}

void pop_pipe(char **pipeQueue, int queueSize){
    int i;
    int fdList[MAXSTRING][2];
    int until = queueSize - 1;
    char* list[32][32]= {{NULL,}};
    char** listPointer = NULL;

    for(i=0;i<until;i++)
    {
        if(pipe(fdList[i]) < 0)
            error(PIPE_FUN_ERROR);
    }
    for(i=0;i<queueSize;i++)
    {
        //char *** list = [a["","","",""] | b["","","",""] | c["","","",""], ...]
        listPointer = list[i];
        parsing_command(pipeQueue[i], list[i]);
    }
    for(i=0;i<queueSize;i++)
    {
        if(i == 0 && (i+1) == queueSize)
        {
            //no pipe. just run
            run_command(list[i]); //no pipe
        }
        else if(i == 0 && (i+1) < queueSize)
        {
            run_command(list[i]); //just write pipe
        }
        else if((i+1) == queueSize)
        {
            run_command(list[i]); //just read Pipe
        }
        else if((i+1) < queueSize)
        {
            run_command(list[i]); //read Pipe and write Pipe
        }
    }
    for(i=0;i<32;i++)
    {
        if(list[i] == NULL)
            break;
        clearList(list[i]);
    }
}

void parsing_command(char* command, char **list)
{
    int i = 0;
    char* token = strtok(command, " \n");
    while(token != NULL)
    {
        list[i] = (char*)malloc(sizeof(char)*strlen(token));
        strcpy(list[i],token);
        i++;
        token = strtok(NULL, " \n");
    }
    list[i] = NULL;
#ifdef DEBUG
    i = 0;
    while(list[i] != NULL)
    {
        printf("[ %s ] ", list[i]);
        i++;
    }
    printf("\n");
#endif
}

void run_command(char** list)
{
    pid_t pid;
    if((pid = fork()) == 0)
    {
        execv(list[0],list);
        fprintf(stderr, "myshell: error!\n");
        exit(0);
    }
    else
    {
        int status, waitPid;
        waitPid = wait(&status);
    }
}

void error(int errorFlag)
{
    switch(errorFlag){
        case PIPE_FUN_ERROR:
            fprintf(stderr, "myshell: pipe creation error!\n");
            exit(3);
            break;
        default:
            fprintf(stderr, "myshell: unknown error!\n");
            exit(4);
    }
}
