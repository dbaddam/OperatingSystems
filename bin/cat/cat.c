#include <stdio.h>
#include <stdlib.h>

//TODOKISHAN maxe this buffer
//#define MAX_BUFFER_SIZE 1024
#define MAX_BUFFER_SIZE 5


int main(int argc, char *argv[], char* envp[])
{
   char buffer[MAX_BUFFER_SIZE];
   FILE *fp = NULL;

   if (argc == 1)
   {
      fputs("error: file not specified\n", stdout);
      return -1;
   }
   else if (argc > 1)
   {
      fp = fopen(argv[1], "r");
   }

   if (fp == NULL)
   {
      printf("error: file not exists\n");
      exit(-1);
   }

   while (fgets(buffer, MAX_BUFFER_SIZE, fp) >= 0)
   {
      //fputs(buffer,  stdin);
//      puts(buffer); 
      printf("%s", buffer); 
   }
   fclose(fp);
//   while(1);
   return 0;
}
