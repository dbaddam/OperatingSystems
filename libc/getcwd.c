#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

char *getcwd(char *buf, size_t size)
{
   syscall2(getcwd, buf, size);

   return (char*)sysret;
}
