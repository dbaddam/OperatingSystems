#define <stdlib.h>
#define <syscall.h>
#define <unistd.h>

int close(int fd)
{
   syscall1(close, fd);

   return (int)sysret;
}
