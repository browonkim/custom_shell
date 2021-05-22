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
void parsing_command();
void run_command();
void clearList(char **list, int listSize);
void error(int errorFlag);
int main()
{
    char commandBuffer[BufferSize];
    const size_t commandBufSize = sizeof(commandBuffer);

    size_t commonSize = sizeof(char *) * MAXSTRING;

    char **pipeQueue = (char **)malloc(commonSize);
    memset(pipeQueue, 0, commonSize);
    int pipeSize = 0;

    char **list = (char **)malloc(commonSize);
    memset(list, 0, commonSize);
    int listSize = 0;

    int i = 0;

    while (infinite)
    {
        listSize = 0;
        memset(commandBuffer, '\0', commandBufSize);
        printf("$ ");
        getCommand(commandBuffer, commandBufSize);
        parsing_pipe(commandBuffer, pipeQueue, &pipeSize);
        //pipe각각의 원소들에 대해 parsing & // > >> n> < <0 &> &>> >& >>& 처리해주기
        //dup2써서 redirection구현
        //마지막으로 "&" 있는지 체크해서 background fork 구현하기
        //그 후 foreground fork 구현하기
        clearList(list, listSize);
        clearList(pipeQueue, pipeSize);
    }
    free(list);
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

void clearList(char **list, int listSize)
{
    int i = 0;
    for (i = 0; i < listSize; i++)
    {
        free(list[i]);
        list[i] = NULL;
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

void popPipe(char **pipeQueue, int queueSize){
    int i;
    int fdList[MAXSTRING][2];
    int until = queueSize - 1;
    for(i=0;i<until;i++)
    {
        if(pipe(fdList[i]) < 0)
            error(PIPE_FUN_ERROR);
    }
    for(i=0;i<queueSize;i++)
    {
        //char *** list = [a["","","",""] | b["","","",""] | c["","","",""], ...]
        parsing_command();
    }
    for(i=0;i<queueSize;i++)
    {
        if(i == 0 && (i+1) == queueSize)
        {
            //no pipe. just run
            run_command(); //no pipe
        }
        else if(i == 0 && (i+1) < queueSize)
        {
            run_command(); //just write pipe
        }
        else if((i+1) == queueSize)
        {
            run_command(); //just read Pipe
        }
        else if((i+1) < queueSize)
        {
            run_command(); //read Pipe and write Pipe
        }
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