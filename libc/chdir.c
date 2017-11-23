#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int chdir(const char* path)
{
   syscall1(chdir, path);
   return (int)sysret;
}
