#include <sys/kprintf.h>
#include <sys/tarfs.h>


/*
* This function is used to convert octal value to int.
* The file size in the struct is stored as an octal string instead of binary format.
* Hence we need this function to get the size of each file in integer format.
*/
int oct2bin(unsigned char *str, int size)
{
   int n=0;
   unsigned char *c = str;
   while(size-- > 0)
   {
      n = n*8;
      n = n+(*c - '0');
      c++;
   }
   return n;
}


void init_tarfs()
{
   kprintf("First file name in tarfs = %s\n",&_binary_tarfs_start);   
   //struct posix_header_ustar headers[32];//some number of headers
   uint64_t addr = (uint64_t) &_binary_tarfs_start;
//0xffffffff80206400;   
   unsigned int i=0;
   for(i=0;;i++)
   {
      struct posix_header_ustar *header;
      header = (struct posix_header_ustar *)addr;
      //struct posix_header_ustar *header = (struct posix_header_ustar *)_binary_tarfs_start;
      if(header->name[0]=='\0')
      {
         break;
      }
      kprintf("filename = %s\n",header->name);
      unsigned int filesize = oct2bin((unsigned char *)header->size, 11);
      kprintf("filesize = %d\n",filesize);
      //headers[i]=(struct posix_header_ustar *)header;
      addr += (((filesize+511)/512)+1)*512;
      kprintf("addr value = %p\n",addr);
   }
}
