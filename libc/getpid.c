#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

pid_t getpid()
{
   syscall0(getpid);

   return (pid_t)sysret;
}
