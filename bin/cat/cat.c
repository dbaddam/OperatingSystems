#include <stdio.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024


int main(int argc, char *argv[], char* envp[])
{
   char buffer[MAX_BUFFER_SIZE];
   FILE *fp = NULL;

   if (argc == 1)
   {
      fputs("cat: file not specified", stdout);
      return -1;
   }
   else if (argc > 1)
   {
      fp = fopen(argv[1], "r");
   }
   
   while (fgets(buffer, MAX_BUFFER_SIZE, fp))
      puts(buffer);  
   
   fclose(fp);
   return 0;
}
