#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define COUNT 20

int main(int argc, char* argv[], char* envp[])
{
   int i;

   for (i = 0;i < COUNT;i++)
   {
      int pid = fork();
      if (pid == 0)
      {
         char* argv[10] = {"/bin/test1", "5", 0};
         execve("/bin/test1", argv, envp);
      }
   }

   for (i = 0;i < COUNT;i++)
   {
      int pid = wait(NULL);
      printf("pid exited - %d\n",pid);
   }

   return 0;
}
