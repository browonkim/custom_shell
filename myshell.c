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
void parseCommand(char *buffer, char list[][MAXSTRING], int *listSize);
void executeCommand(char list[][MAXSTRING], int listSize);


int main()
{
    char commandBuffer[BufferSize];
    const size_t commandBufSize = sizeof(commandBuffer);
    char list[MAXSTRING][MAXSTRING];
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
    }
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

void parseCommand(char *buffer, char list[][MAXSTRING], int *listSize)
{
    char *token = strtok(buffer, " \n");
    *listSize = 0;
    while (token != NULL)
    {
        strcpy(list[(*listSize)], token);
        (*listSize)++;
        token = strtok(NULL, " \n");
    }
}

void executeCommand(char list[][MAXSTRING], int listSize)
{
    pid_t pid;
    int status;
    pid = fork();
    if(pid == 0){
        //child
    }
    else{
        //parent
        wait(&status);
    }
}
