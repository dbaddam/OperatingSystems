#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[], char* envp[])
{
   if (argc == 1)
   {
      printf("Should pass atleast one argument\n");
      exit(-1);
   }

   int count;
   if (!atoi(argv[1], &count))
   {
      printf("Invalid number\n");
      exit(-1);
   }
   for (int i = 0;i < count ;i++)
   {
      sleep(5);
      int pid = getpid();
      int ppid = getppid();
      printf("pid %d, ppid %d AWAKE\n", pid, ppid);
      yield();
   } 
   return 0;
}
