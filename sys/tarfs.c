#include <sys/os.h>

/* return the length of a given string */
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

/* compares the given strings and returns 1 if same, 0 if not same*/
int sbustrncmp(char *str1, char *str2, int size)
{
   int i;

   if (str1 == NULL && str2 == NULL)
      return 1;

   if (str1 == NULL || str2 == NULL)
      return 0;

   for (i = 0; i < size;i++)
   {   
      if (*(str1 + i) != *(str2 + i)) 
         return 0;  
   }   

   return 1;
}

/* always str2 size > str1 size */
int sbusubstr(char *str1, char *str2)
{
   int size = sbustrlen(str2);
   return sbustrncmp(str1, str2, size);
}



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

/* given a filename and a start address, searches in tarfs and returns the filesize and file content 
 * start address if found, or 0 if not found
 */
int getFileFromTarfs(char *filename, char **file_start_address)
{
   uint64_t addr = (uint64_t) &_binary_tarfs_start;
   int i=0;
   for(i=0;;i++)
   {
      struct posix_header_ustar *header;
      header = (struct posix_header_ustar *)addr;
      if(header->name[0]=='\0')
      {   
         break;
      }
      unsigned int filesize = oct2bin((unsigned char *)header->size, 11);
      if(sbustrncmp(header->name, filename, sbustrlen(filename)))
      {
         *file_start_address = (char *)(addr + 512);
         return filesize;
      }
      addr += (((filesize+511)/512)+1)*512;
   }
   return 0; 
}


void list_files_directory(char *dirname)
{
   uint64_t addr = (uint64_t) &_binary_tarfs_start;
   int i=0;
   for(i=0;;i++)
   {   
      struct posix_header_ustar *header;
      header = (struct posix_header_ustar *)addr;
      if(header->name[0]=='\0')
      {   
         break;
      }
      kprintf("found below files for dir=%s\n",dirname);
      unsigned int filesize = oct2bin((unsigned char *)header->size, 11);
      if(sbusubstr(header->name, dirname))
      {   
         kprintf("%s\n",header->name);

      }   
      addr += (((filesize+511)/512)+1)*512;
   }   
   return;
}

void init_tarfs()
{
   kprintf("First file name in tarfs = %s\n",&_binary_tarfs_start);   
   //struct posix_header_ustar headers[32];//some number of headers
   uint64_t addr = (uint64_t) &_binary_tarfs_start;
   unsigned int i=0;
   for(i=0;;i++)
   {
      struct posix_header_ustar *header;
      header = (struct posix_header_ustar *)addr;
      if(header->name[0]=='\0')
      {
         break;
      }
      kprintf("filename = %s,",header->name);
      unsigned int filesize = oct2bin((unsigned char *)header->size, 11);
      kprintf("filesize = %d,",filesize);
      // headers[i] = *header;
      addr += (((filesize+511)/512)+1)*512;
      kprintf("addr value = %p\n",addr);
   }
   
   char *filecontent;
   int fsize = getFileFromTarfs("bin/sbush", &filecontent);
   if(fsize != 0)
   {
      kprintf("bin/sbush file found\n");
      //kprintf("file content = %s\n",filecontent);
      //kprintf("last char = %d\n", *(filecontent+5)); 
      elf_load_file(filecontent, 0);
   } 
}

