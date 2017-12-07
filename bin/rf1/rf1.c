#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

//#include <sbuutil.h>

int main(int argc, char* argv[], char* envp[])
{
   FILE *fp;
   char str[20];
   
   fp = fopen("/usr/kish","r");
   if(fp==NULL)
   {
      printf("error in opening file = /usr/kish\n");
      return -1;
   }

   fgets(str, 5, fp);
   fputs(str, stdout);

   if (fork() == 0)
   {
      puts("Child");
      while(fgets(str, 5, fp) != -1)
         fputs(str, stdout);
      exit(1);
   }
   else
   {
      //yield();
      puts("Parent");
      while(fgets(str, 5, fp) != -1)
         fputs(str, stdout);
   }

   fclose(fp);
   return 0;
}
