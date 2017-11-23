#include <stdio.h>
#include <syscall.h>
#include <unistd.h>


ssize_t write(int fd, const void *buf, size_t count)
{
  syscall3(write, fd, buf, count); 

  return (ssize_t) sysret;
}

void uyield()
{
  syscall0(sched_yield);

  return;
}
