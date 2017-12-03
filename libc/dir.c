#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>
#include <dirent.h>

int opendirt(const char *pathname, int flags)
{
   syscall2(opendirt, pathname, flags);

   return (int)sysret;
}

int readdirt(int fd, void *buf)
{
  syscall2(readdirt, fd, buf); 

  return (ssize_t) sysret;
}

int closedirt(int fd)
{
   syscall1(closedirt, fd);

   return (int)sysret;
}

DIR* opendir(const char *path)
{
   int fd_dir = opendirt(path, 1);
   if(fd_dir == -1)
   {
      return NULL;
   }
   dir[fd_dir].fd = fd_dir;
   //strncpy(dir[fd_dir].name, path, sbustrlen(path));
   return &dir[fd_dir];
}


dirent* readdir(DIR *dirp)
{
   int r = readdirt(dirp->fd, (char *)(dirp->d_entry.d_name));
   if(r==1)
   {
      return &(dirp->d_entry);
   }
   else
   {
      return NULL;
   }
}

int closedir(DIR *dirp)
{
   int r = closedirt(dirp->fd);
   if(r==1)
   {
      return 0;
   }
   else
   {
      return -1;
   }
}


 
