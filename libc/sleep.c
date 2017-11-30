#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

unsigned int sleep(unsigned int secs)
{
   syscall1(sleep, secs);

   return (unsigned int)sysret;
}
