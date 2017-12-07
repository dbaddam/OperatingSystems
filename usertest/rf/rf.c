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
   while(fgets(str, 20, fp) != -1)
   {
      fputs(str, stdout);
   }
//   fclose(fp);

   FILE *fp1;
   char str1[20];

   fp1 = fopen("/usr/kish","r");
   if(fp1==NULL)
   {
      printf("error in opening file = /usr/kish\n");
      return -1;
   }
   while(fgets(str1, 20, fp1) != -1)
   {
      fputs(str1, stdout);
   }
//   fclose(fp1);

   FILE *fp2;
   char str2[20];

   fp2 = fopen("/usr/kishan.txt","r");
   if(fp2==NULL)
   {
      printf("error in opening file = /usr/kishan.txt\n");
      return -1;
   }
   while(fgets(str2, 20, fp2) != -1)
   {
      fputs(str2, stdout);
   }
//   fclose(fp2);

   fclose(fp);
   fclose(fp2);
   fclose(fp1);
   return 0;
}
