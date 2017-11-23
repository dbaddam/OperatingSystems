#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

int dup2(int oldfd, int newfd)
{
   syscall2(dup2, oldfd, newfd);

   return (int)sysret;   
}
