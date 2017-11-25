#include <sys/defs.h>
#include <sys/util.h>

void sleep(uint32_t secs)
{
   uint32_t i,j;
   //while(1);
   while(secs--)
      for (i = 0;i < 10000;i++)
         for (j = 0;j < 10000;j++); 
}

uint64_t min(uint64_t a, uint64_t b)
{
   if (a < b)
      return a;
   return b;
}

void memcpy(char* dest, char* src, uint64_t bytes){
   while(bytes--)
   {
     *dest = *src;
     dest++;
     src++;
   }
}

void strncpy(char* dest, char* src, uint32_t len)
{
   int i;
  
   for (i = 0;i < len && *src;i++)
   {
      *(dest+i) = *(src + i);
   }

   if (i != len)
   {
      *(dest + i) = '\0';
   }
}
