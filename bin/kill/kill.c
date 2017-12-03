#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[], char* envp[])
{
   if (argc < 3)
   {
      printf("Invalid arguments : Usage - kill -9 <pid>\n");
      exit(-1);
   }

   if (strcmp(argv[1], "-9") != 0 )
   {
      printf("Invalid arguments : Usage - kill -9 <pid>\n");
      exit(-1);
   }

   int pid;
   if (!atoi(argv[2], &pid))
   {
      printf("Invalid pid number : Usage - kill -9 <pid>\n");
      exit(-1);
   }
   kill(pid);
   return 0;
}
