#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


DIR* opendir(const char *path)
{
   int fd_dir = opendir_tarfs(&path);
   sbustrncpy(dir[fd_dir].name, path, sbustrlen(path));
   dir[fd_dir].name[sbustrlen(path)] = '\0';
   return &dir[fd_dir];
}


dirent* readdir(DIR *dirp)
{
   dirp.name   
} 

