#include <stdio.h>
#include <syscall.h>
#include <unistd.h>


ssize_t read(int fd, void *buf, size_t count)
{
  syscall3(read, fd, buf, count); 

  return (ssize_t) sysret;
}
