#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int open(const char *pathname, int flags)
{
   syscall2(open, pathname, flags);

   return (int)sysret;
}
