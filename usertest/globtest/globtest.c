#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int glob1 = 1;
int glob2 = 2;

int main(int argc, char* argv[], char* envp[])
{
   int cpid = fork();
   int loc = 5;
   if (cpid == 0)
   {
      glob1 = 3;
      printf("child glob1 - %d, glob2 - %d, loc - %d, pid - %d\n", glob1, glob2, loc, getpid());
      exit(1);
   }

   int xpid = wait(NULL);

   printf("glob1 - %d, glob2 - %d, xpid - %d\n", glob1, glob2, xpid);
   
   return 0;
}
