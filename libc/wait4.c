#include <stdlib.h>
#include <syscall.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t wait4(pid_t pid, int *status, int options, rusage* usage)
{
   syscall4(wait4, pid, status, options, usage);

   return (pid_t)sysret;
}
