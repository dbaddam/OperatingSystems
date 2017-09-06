#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>
#include <dirent.h>

int getdents(unsigned int fd, struct dirent *dirp, unsigned int count)
{
   syscall3(getdents, fd, dirp, count);
   return (int)sysret;
}
