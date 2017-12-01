#include <sbuutil.h>



void strncpy(char* dest, char* src, int len)
{
   int i;
  
   for (i = 0;i < len && src[i];i++)
   {   
      dest[i] = src[i];
   }   

   if (i == len)
   {   
      dest[i-1] = '\0';
   }   
   else
   {   
      dest[i] = '\0';
   }   
}


void strcpy(char* dest, char* src)
{
   int i;

   for (i = 0;src[i];i++)
   {
      dest[i] = src[i];
   }
   dest[i] = '\0';
}


int strlen(char *str)
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
