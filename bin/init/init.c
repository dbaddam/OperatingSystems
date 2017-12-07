#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[])
{

   if (fork() == 0)
   {
      execve("/etc/rc", argv,envp);
   }
   else
   {
      while(1)
         wait(NULL);
   }

   while(1);
   return 0;
}
