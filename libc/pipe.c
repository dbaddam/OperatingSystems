#define <stdlib.h>
#define <syscall.h>
#define <unistd.h>

int pipe(int pipefd[2])
{
   syscall1(pipe, pipefd);

   return (int)sysret;
}
