#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 100

int main(int argc, char *argv[], char *envp[])
{
    puts("$$$$$$Adding XYZ=KISHAN");
    setenv("XYZ", "KISHAN", 1);
    printenv(); 
    puts("$$$$$$Changing XYZ=NERELLA");
    setenv("XYZ", "NERELLA", 1);    
    printenv();
    puts("$$$$$$Changing XYZ=KISHAN without overwrite FAIL");
    setenv("XYZ", "KISHAN", 0);    
    printenv();
    puts("$$$printing PATH value");
    puts(getenv("PATH"));
    puts(getenv("XYZ"));
    return 0;
    puts("Hello World");
    char cwd[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];

    if (getcwd(cwd, MAX_BUFFER_SIZE))
    {
       puts(cwd);
    }

    if (chdir("../.."))
    {
       puts("error ");
    }

    if (getcwd(cwd, MAX_BUFFER_SIZE))
    {
       puts(cwd);
    }

    int i;
    puts("*****Printing arguments****");
    for (i = 0;i < argc;i++)
    {
       puts(argv[i]);
    }
    puts("*****End of Printing arguments****");

    puts("*****Printing env****");
    for (i = 0;envp[i];i++)
    {
       puts(envp[i]);
    }
    puts("*****End of Printing env****");

    //puts(envp[0]);

    FILE* fp = fopen("/home/knerella/workdir/bin/sbush/a.txt","r");
    while (fgets(buffer, MAX_BUFFER_SIZE, fp))
    {
       puts(buffer);
    }
    
    fclose(fp);

    char* args[] = {"ls", "-l", NULL};
    char* envpp[] = {NULL};

    if (fork() == 0)
    {
       execve("/bin/ls", args, envpp);
       puts("******************EXECVE FAILED");
       exit(0);
    }
    else
    {
       int status;
       puts("Parent");
       wait(&status);
       puts("Parent wait done");
    }
    return 0;
}
