#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H     1

typedef struct rusage
{
   int dummy_rusage;
}rusage;

pid_t wait4(pid_t pid, int *status, int options, rusage* usage);

#endif
