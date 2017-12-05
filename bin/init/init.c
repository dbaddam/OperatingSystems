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
      wait(NULL);
   }

   char* arg[] = {"/bin/sbush", 0};
   char* env[] = {"PATH=/bin", 0};
   execve("/bin/sbush", arg, env);
   return 0;
}
