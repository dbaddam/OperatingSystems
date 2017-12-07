#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MAX_FILE_SIZE 1024

int execvpe(const char *filename, char *const argv[], char *const envp[])
{
   char  file[MAX_FILE_SIZE];
   char *path = getenv("PATH");
   int   i;

   /* If it's a relative path we don't search in PATH */
   for (i = 0; filename[i];i++)
   {
       if (filename[i] == '/')
          return execve(filename, argv, envp);
   }

   /* If the file is in this directory, execute now */
   if (!access(filename, F_OK))
      return execve(filename, argv, envp);

   if (!path)
      return -1;

   while (*path)
   {
      char* keystart = path;
      char* keyend = keystart;

      while (*keyend && *keyend != ':')
         keyend++;

      strncpy(file, path, (keyend-keystart));

      file[keyend-keystart] = '/';
      file[keyend-keystart+1] = '\0';
 
      strcat(file, filename);

      if (!access(file, F_OK))
         break;

      if (!*keyend)
         return 1;

      path += (keyend-keystart)+1;
   }

   return execve(file, argv, envp);
}
