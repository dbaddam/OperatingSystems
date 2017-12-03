#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[], char* envp[])
{

   int pid1 = fork();   
   if (pid1 == 0)
   {
      char* argv[10] = {"/bin/test1", "5", 0};
      execve("/bin/test1", argv, envp);
   }
   else
   {
      int pid2 = fork();
      if (pid2 == 0)
      {
         char* argv[10] = {"/bin/test1", "10", 0};
         execve("/bin/test1", argv, envp);
      }
      else
      {
         waitpid(pid1, NULL);
         printf("About to exit\n");
      }
   }
   return 0;
}
