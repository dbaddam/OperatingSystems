#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>
#include <string.h>

#define MAX_FILE_SIZE 1024

int execvp(const char *filename, char *const argv[])
{
/*
   char  file[MAX_FILE_SIZE] ;
   char *path = getenv("PATH");

   while (*path)
   {
      char* keystart = path;
      char* keyend = keystart;

      while (*keyend && *keyend != ':')
         keyend++;

      strncpy(file, path, (keyend-keystart));
      file[keyend-keystart] = '\0';

      strcat(file, filename);

      if (!access(file, F_OK))
         break;

      if (!*keyend)
         return 1;

      path += (keyend-keystart)+1;
   }
*/
   return execve(filename /*file*/, argv, _sbush_env);
}
