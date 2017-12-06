#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void lsleep(int secs)
{
    while(secs--)
      for (int i = 0;i < 3000;i++)
         for (int j = 0;j < 100000;j++);
}

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


   int pid = getpid();
   int ppid = getppid();
   printf("pid %d, ppid %d START\n", pid, ppid);
   for (int i = 0;i < count ;i++)
   {
      lsleep(1);
      //int pid = getpid();
      //int ppid = getppid();
      //printf("pid %d, ppid %d cnt - %dAWAKE\n", pid, ppid, i);
      //yield();
   } 
   return 0;
}
