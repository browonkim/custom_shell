#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define infinite 1
#define BufferSize 1024
#define MAXSTRING 256
#define DEBUG

enum boolean
{
    False = 0,
    True = 1
} typedef boolean;

void getCommand(char *buffer, size_t bufferSize);
void parseCommand(char *buffer, char** list, int *listSize);
void executeCommand(char** list, int listSize);
void clearList(char** list, int listSize);
int main()
{
    char commandBuffer[BufferSize];
    const size_t commandBufSize = sizeof(commandBuffer);
    char** list = (char **)malloc(sizeof(char *)*MAXSTRING);    //char * list[MAXSTRING]
    memset(list, 0, sizeof(char*)*MAXSTRING);
    int listSize = 0;
    int i = 0;
    while (infinite)
    {
        listSize = 0;
        memset(commandBuffer, '\0', commandBufSize);
        printf("$ ");
        getCommand(commandBuffer, commandBufSize);
#ifdef DEBUG
        printf("%s", commandBuffer);
#endif
        parseCommand(commandBuffer, list, &listSize);
#ifdef DEBUG
        for (i = 0; i < listSize; i++)
        {
            printf("%s ",list[i]);
        }
        printf("token size = %d\n", listSize);
#endif
        executeCommand(list, listSize);
        clearList(list, listSize);
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
}

void parseCommand(char *buffer, char** list, int *listSize)
{
    char *token = strtok(buffer, " \n");
    *listSize = 0;
    while (token != NULL)
    {
        list[*listSize] = (char*)malloc(sizeof(char)*(strlen(token)+1));
        strcpy(list[(*listSize)], token);
        (*listSize)++;
        token = strtok(NULL, " \n");
    }
    list[*listSize] = NULL;
}

void executeCommand(char** list, int listSize)
{
    pid_t pid;
    int status;
    pid = fork();
    if(pid == 0){
        //child
        execv(list[0],list);
        printf("myshell: command not found: %s\n", list[0]);
        exit(0);
    }
    else{
        //parent
        wait(&status);
    }
}

void clearList(char** list, int listSize){
    int i = 0;
    for(i=0;i<listSize;i++)
        free(list[i]);
}