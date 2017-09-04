#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

pid_t fork()
{
   syscall0(fork);

   return (pid_t)sysret;
}
