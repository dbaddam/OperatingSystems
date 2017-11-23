#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int close(int fd)
{
   syscall1(close, fd);

   return (int)sysret;
}
