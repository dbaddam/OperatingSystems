#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 1024


int main(int argc, char *argv[])
{
   char buffer[MAX_BUFFER_SIZE];
   FILE *fp;
   if(argc > 0)
   {
      fp = fopen(argv[1], "r");
   }
   
   while(fgets(buffer, MAX_BUFFER_SIZE, fp))
        fputs(buffer, stdout);  
   
   fclose(fp);
   return 0;
}
