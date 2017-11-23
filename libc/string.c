#include <string.h>

size_t strcmp(const char* str1, const char* str2)
{
    int i;

    if (str1 == NULL && str2 == NULL)
       return 0;

    if (str1 == NULL)
       return -1;

    if (str2 == NULL)
       return 1;

    for (i = 0 ;str1[i] && str2[i];i++)
    {
         if (str1[i] > str2[i])
            return 1;
         else if (str1[i] < str2[i])
            return -1;
    }

    if (str1[i])
       return 1;

    if (str2[i])
       return -1;

    return 0;

 }

char* strcpy(char* dest, const char* src)
{
   char *d = dest;
   while (*src)
   {
      *d = *src;
      src++;
      d++;
   }

   *d = '\0';
   
   return dest;
}

char* strncpy(char* dest, const char* src, size_t n)
{
   char *d = dest; 
   int   i;
  
   for (i = 0;i < n && *src;i++, src++, d++)
   {
      *d = *src; 
   }

   for (;i < n; d++)
   {
      *d = '\0';
   }
   return d;
}

size_t strlen(const char* str)
{
   int len = 0;
   
    while(*str)
    {
       len++;
       str++;
    }

    return len;
}

char *strcat(char *dest, const char *src)
{
   char* c = dest;

   if (!dest || !src)
      return dest;

   while(*c)
      c++;

   while(*src)
   {
      *c = *src;
      c++;
      src++;
   }

   *c = '\0';

   return dest;
}
