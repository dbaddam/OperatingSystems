#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[], char* envp[])
{
   if (argc <= 1)
   {
      puts("error :should provide the number of seconds to sleep");
      exit(-1);
   }

   puts("Sleep not implemented *******");
   exit(-1);
   sleep(10); 
   return 0;
}
