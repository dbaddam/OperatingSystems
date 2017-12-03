#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[], char* envp[])
{

   int max = 2;
   int lev = 3 * 1000;
   char** p = (char**) malloc(lev*sizeof(char*));

   for (int i = 0;i < max;i++)
   {
       for (int j = 0;j < lev;j++)
       {
           p[j] = (char*)malloc(j*500 + 1);
           if (p[j] != NULL)
           {
              for (int k = 0;k < j;k++)
              {
                 p[j][k*500] = k % 255;
              }
           }
           else
           {
              printf("Out of memory\n");
              while(1);
           }
       }

       if (i == 0)
       {
     
           printf("addr - p[%d]=%p\n", 0, p[0]);
           printf("addr - p[%d]=%p\n", lev-1, p[lev-1]);
       }

       free(p[1]);

       for (int j = lev-1;j >= 0 ;j--)
       {
           free(p[j]);
       }
   }

   free(p);
   return 0;
}
