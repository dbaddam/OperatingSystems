#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int execve(const char *filename, char *const argv[], char *const envp[])
{
   syscall3(execve, filename, argv, envp);

   return (int)sysret;
}
