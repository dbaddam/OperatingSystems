#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 100

int main(int argc, char *argv[], char *envp[])
{
    puts("Hello World");
    char cwd[MAX_BUFFER_SIZE];

    if (getcwd(cwd, MAX_BUFFER_SIZE))
    {
       puts(cwd);
    }

    if (chdir(".."))
    {
       puts("error ");
    }

    if (getcwd(cwd, MAX_BUFFER_SIZE))
    {
       puts(cwd);
    }

    //puts(envp[0]);

    char* args[] = {"ls", "-l", NULL};
    char* envpp[] = {NULL};

    if (fork() == 0)
    {
       execve("/bin/ls", args, envpp);
    }
    else
    {
       puts("Parent");
    }
    return 0;
}
