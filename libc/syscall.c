#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

void ps()
{
   syscall0(ps);
}
void kill(int pid)
{
   syscall1(kill, pid);
}
