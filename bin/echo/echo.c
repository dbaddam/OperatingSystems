#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sbuutil.h>


#define MAX_BUFFER 4096

int main(int argc, char *argv[], char *envp[])
{
   //char buffer[MAX_BUFFER];
   if(argc > 1)
   {
      int i;
      for(i=1;i<argc;i++)
      {
         fputs(argv[i], stdout);
         fputs(" ", stdout);
      }
   }
   puts("");
   return 0;
}
