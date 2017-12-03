#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[], char* envp[])
{
   if (argc <= 1)
   {
      puts("error :should provide the number of seconds to sleep");
      exit(-1);
   }

   int secs;
   if (!atoi(argv[1], &secs))
   {
      puts("error :invalid number");
      exit(-1);
   }

   if (secs > 0)
      sleep(secs);
   return 0;
}
