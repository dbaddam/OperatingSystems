#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int pipe(int pipefd[2])
{
   syscall1(pipe, pipefd);

   return (int)sysret;
}
