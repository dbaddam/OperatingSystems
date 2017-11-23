#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

off_t lseek(int fd, off_t offset, int whence)
{
   syscall3(lseek, fd, offset, whence);
   return (off_t)sysret;
}
