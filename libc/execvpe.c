#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int execvpe(const char *filename, char *const argv[], char *const envp[])
{
   return execve(filename, argv, envp);
}
