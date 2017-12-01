#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

void yield()
{
   syscall0(sched_yield);
}
