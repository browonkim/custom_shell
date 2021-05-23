//시스템 프로그래밍/ 2021/ myshell
//B735137/ 김형원

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

//debug option
//#define DEBUG
#define ERRNO_CHECK

//constant
#define infinite 1
#define BufferSize 1024
#define MAXSTRING 256

//error Flag
#define PIPE_FUN_ERROR 400
#define MALLOC_ALLOCATION_ERROR 401


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

//functions
void get_command(char *buffer, size_t bufferSize);
void parsing_pipe(char *buffer, char **list, int *listSize);
void tokenizeCommand(char *buffer, char **list, int *listSize);
void parsing_command(char* command, char** list);
void run_command();
void pop_pipe(char **pipeQueue, int queueSize);
void clear_list(char **list);
void error(int errorFlag);

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
        memset(commandBuffer, '\0', commandBufSize);
        printf("$ ");
        get_command(commandBuffer, commandBufSize);
        parsing_pipe(commandBuffer, pipeQueue, &pipeSize);
        pop_pipe(pipeQueue, pipeSize);
        clear_list(pipeQueue);
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

void clear_list(char **list)
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
        printf("%d) pipe: %s | \n",i+1, pipeQueue[i]);
    }
#endif
}

void pop_pipe(char **pipeQueue, int queueSize){
    int i;
    int fdList[MAXSTRING][2];
    int until = queueSize - 1;
    char* list[32][32]= {{NULL,}};
    for(i=0;i<until;i++)
    {
        if(pipe(fdList[i]) < 0)
            error(PIPE_FUN_ERROR);
    }
    for(i=0;i<queueSize;i++)
    {
        //char *** list = [a["","","",""] | b["","","",""] | c["","","",""], ...]
        split_command(pipeQueue[i], list[i]);
    }
    for(i=0;i<queueSize;i++)
    {
        if(i == 0 && (i+1) == queueSize)
        {
            pipe(fdList[i]);//no pipe. just run
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
        clear_list(list[i]);
    }
}

void split_command(char* command, char **list)
{
    int i = 0;
    char* token = strtok(command, " \n");
    while(token != NULL)
    {
        list[i] = (char*)malloc(sizeof(char)*strlen(token));
        if(list[i] == NULL)
        {
            error(MALLOC_ALLOCATION_ERROR);
        }
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
    int i = 0;
    while(list[i] != NULL)
    {
        int j = 0;
        char c;
        while((c=list[i][j]) != '\0')
        {
            if(c == '<')
            {
                // < 
                // <0
            }
            else if(c == '>')
            {
                //> 만약 뒤에 원소가 있다면 output redirection 구현하기
                //&> error redirection 이랑 output redirection 이랑 같은것을 가르키기
                //2> error 를 리디렉션해주기
                //>& &>랑 똑같이 구현해주기
                //>> 이어쓰기
                //&n> 
                //>&n
            }
            else if(c == '&')
            {
                // & 만 허용됨 string전체가 &여야만함
            }
            j++;
        }
        i++;
    }

    pid_t pid;
    if((pid = fork()) == 0)
    {
        if(execv(list[0],list) < 0)
        {
#ifdef ERRNO_CHECK            
            fprintf(stderr, "%s\n", strerror(errno));
#endif
            fprintf(stderr, "myshell: %s: command not found\n", list[i]);
            exit(0);
        }
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
        case MALLOC_ALLOCATION_ERROR:
            fprintf(stderr, "myshell: fatal! memory allocation error!\n");
            exit(4);
            break;
        default:
            fprintf(stderr, "myshell: unknown error!\n");
            exit(4);
    }
}
