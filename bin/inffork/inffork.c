#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


void sleep(int s)
{
  int i,j;
   while(s--)
     for (i = 0;i < 10000;i++)
         for (j = 0;j < 100000;j++);
}

int sbustrlen(char *str)                                                                                      
{                                                                                                             
   char *c = str;                                                                                             
   int   len = 0;                                                                                             
   if (!str)                                                                                                  
      return -1;                                                                                              
                                                                                                              
   while (*c != '\0')                                                                                         
   {                                                                                                          
      c++;                                                                                                    
      len++;                                                                                                  
   }                                                                                                          
                                                                                                              
   return len;                                                                                                
}                    

void rev(char* c)
{
   int low = 0;
   int high = sbustrlen(c) - 1;

   while (low < high)
   {
       char t = c[low];
       c[low] = c[high];
       c[high] = t;
       low++;
       high--;
   }
}

void convert(int n, char* s)
{
   if (n == 0)
   { *s = '0'; s[1] = '\0'; return;}

   int c = 0;
   while (n > 0)
   {
       s[c++] = n%10 + '0';
       n /=10;
   }
   s[c] = '\0';
   rev(s);
}

void fn(int n)
{
    if (n == 0)
       return;
    fn(n-1);
}

int main(int argc, char* argv[], char* envp[])
{
   int i;
   for (i = 0;i  < 1000000;i++)
   {
       if (fork() == 0)
       {
          char c[10];
          fn(i);
          puts("Inffork Child");
          convert(i, c); 
          puts(c);
          //sleep(1);
          puts("Inffork Child");
          exit(i+100);
       }else
       {
          char c[10];
          puts("Inffork Parent");
          int status;
          wait(&status);
          convert(status, c); 
          //sleep(1);
          puts(c);
          puts("Inffork Status");
       }
   }
   return 0;
}
