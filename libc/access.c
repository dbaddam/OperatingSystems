#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int access(const char *pathname, int mode)
{
   syscall2(access, pathname, mode);

   return (int)sysret;
}
