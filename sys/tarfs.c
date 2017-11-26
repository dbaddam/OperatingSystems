#include <sys/os.h>
//#include <sys/util.h>
#include <sys/kprintf.h>
#include <sys/tarfs.h>
#include <sys/elf64.h>

#define MAX 256


int getFileFromTarfs(char *filename, char **file_start_address);


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

/* Returns the substring from start to end in given string
 * else return -1
 */
int sbustr(char *str, int start, int end, char *target)
{
    if(start < 0 || end >= sbustrlen(str))
    {
        return -1;
    }
    int i;
    int j=0;
    for(i=start;i<=end;i++)
    {
        target[j] = str[i];
        j++;
    }
    target[j]='\0';
    return 1;
}

/* If there is *substring in *str, then returns *subtring plus rest of string in *str
 * else returns -1
 */
int sbustrstr(char *str, char *substring, char *result)
{
    int i;
    for(i=0;i<sbustrlen(str);i++){
        if(str[i] == substring[0])
        {
            char res[MAX];
            if(sbustr(str,i,i+sbustrlen(substring)-1,res) == -1)
            {
                return -1;
            }
            if(sbustrncmp(res,substring,sbustrlen(substring)))
            {
                if(sbustr(str,i,sbustrlen(str)-1,result)==1)
                {
                    return 1;
                }
            }
        }
    }
    return -1;
}

/* Returns index of given character ch if present in a given string,
 * else return -1.
 */
int contains(char *str, char ch)
{
   int i;
   for(i=0;i<sbustrlen(str);i++)
   {
      if(str[i] == ch)
      {
         return i;
      }
   }
   return -1;
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

/* checks if given name is directory or not */
int is_directory(char *dirname)
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
      if(sbustrncmp(header->name, dirname, sbustrlen(dirname)))
      {   
         if((char)(header->typeflag[0]) == '5')
         {
            return 1;
         }
      }   
      addr += (((filesize+511)/512)+1)*512;
   }   
   return -1;
}


/* This function lists all the files/directories present in the given directory
 * NOTE: The input argument should be a full path
 * 
 */
void read_directory(char *dirname)
{
   if(is_directory(dirname)==-1)
   {
      kprintf("%s is not a directory name\n",dirname);
      return;
   }
   uint64_t addr = (uint64_t) &_binary_tarfs_start;
   int i=0;
   kprintf("found below files for dir=%s\n",dirname);
   for(i=0;;i++)
   {   
      struct posix_header_ustar *header;
      header = (struct posix_header_ustar *)addr;
      if(header->name[0]=='\0')
      {   
         break;
      }
      unsigned int filesize = oct2bin((unsigned char *)header->size, 11);
      char dircontent[MAX];
      if(sbustrstr(header->name, dirname, dircontent)==1)
      {   
       //  kprintf("hdr name = %s,",header->name);
       //  kprintf("dirname = %s,",dirname);
       //  kprintf("dircontent = %s,", dircontent);
         char content[MAX];
         sbustr(dircontent, sbustrlen(dirname)+1, sbustrlen(dircontent)-1, content);
         if(sbustrlen(content)>0)
         {
            int index = contains(content,'/');
            if(index != -1)
            {
               char content1[MAX];
               sbustr(content, 0, index-1, content1);
               kprintf("%s\n",content1);
            }else
            {
               kprintf("%s\n",content);
            }
         }
      }   
      addr += (((filesize+511)/512)+1)*512;
   }   
   return;
}

/*  This function reads the next directory entry from given fd, and returns 1 on success,
 *  returns directory entry into buf
 * 
 *  else returns -1, if there are no more entries or on failure
 */
int readdir_tarfs(int fd, char *buf)
{
   task* t = cur_task;
   char dirname[256];
   sbustrncpy(dirname, (char *)t->file[fd].name, sbustrlen((char *)t->file[fd].name));
   dirname[sbustrlen((char *)t->file[fd].name)]='\0';
   if(is_directory(dirname)==-1)
   {
      kprintf("%s is not a directory name\n",dirname);
      return -1;
   }
   uint64_t addr = t->file[fd].offset;
   int i;
   for(i=0;;i++)
   {
      struct posix_header_ustar *header;
      header = (struct posix_header_ustar *)addr;
      if(header->name[0]=='\0')
      {
         kprintf("No more entries available in the given directory name!\n");
         return -1;
      }
      unsigned int filesize = oct2bin((unsigned char *)header->size, 11);
      char dircontent[MAX];
      if(sbustrstr(header->name, dirname, dircontent)==1)
      {
         char content[MAX];
         sbustr(dircontent, sbustrlen(dirname)+1, sbustrlen(dircontent)-1, content);
         if(sbustrlen(content)>0)
         {
            int index = contains(content,'/');
            if(index != -1)
            {
               char content1[MAX];
               sbustr(content, 0, index-1, content1);
               sbustrncpy(buf, content1, sbustrlen(content1));
               buf[sbustrlen(content1)] = '\0';
               addr += (((filesize+511)/512)+1)*512;
               t->file[fd].offset = addr;
               return 1;
            }else
            {
               sbustrncpy(buf, content, sbustrlen(content));
               buf[sbustrlen(content)] = '\0';
               addr += (((filesize+511)/512)+1)*512;
               t->file[fd].offset = addr;
               return 1;
            }
         }
      }
      addr += (((filesize+511)/512)+1)*512;
   }
   return -1;
}


/* Opens a given filepath and returns a fd
 * else, returns 0
 */
uint32_t open(char* path, uint64_t flags)
{
   task* t = cur_task;
   /* t->file[i] i-th descriptor
    * bit(t->file[i].flags, ALLOCATED_FD)
    * bis()
    */
   char *file_start_addr;
   int fsize = getFileFromTarfs((char *)path, &file_start_addr);
   int i;
   for(i=0; i<MAX_FILES; i++)
   {
      // if fd is not allocated, go ahead and assign it
      if(!bit(t->file[i].flags, ALLOCATED_FD))
      {
         bis(t->file[i].flags, ALLOCATED_FD);
         t->file[i].offset = (uint64_t)(file_start_addr);
         t->file[i].start_addr = (uint64_t)(file_start_addr); 
         t->file[i].size = (uint64_t)fsize;
         return (uint32_t)i;
      }
   }
   return 0;
}

/* returns number of bytes read and returns the content in buf
 * if EOF is reached, reads till that point and return those many bytes read
 */
uint64_t read(uint32_t fd, char *buf, uint64_t count)
{
   task* t  = cur_task;
   if(!bit(t->file[fd].flags, ALLOCATED_FD))
   {
      return -1;
   }
   if(count <= 0)
   {
      return 0;
   }
   
   uint64_t offset_addr = t->file[fd].offset;
   uint64_t fsize = t->file[fd].size;
   uint64_t start_addr = t->file[fd].start_addr;
   uint64_t end_addr = start_addr + fsize;
   
   //kprintf("start=%p,end=%p,offset=%p\n",start_addr,end_addr,offset_addr); 
   //kprintf("fsize=%d\n",fsize);
   int i=0;
   if(offset_addr + count <= end_addr)
   {
      for(i=0; i<count; i++)
      {
         buf[i] = *((char *)(offset_addr + (uint64_t)i));
      }
   }
   if(offset_addr + count > end_addr)
   {
      for(i=0; i<(end_addr - offset_addr); i++)
      {
         buf[i] = *((char *)(offset_addr + (uint64_t)i));
      }
   }
   t->file[fd].offset += i;
   return (uint64_t)(i);
}

/* Returns 1 on succesful closure of fd
 * else returns -1
 */
int close(uint32_t fd)
{
   task* t = cur_task;
   if(!bit(t->file[fd].flags, ALLOCATED_FD))
   {
      return -1;
   }
   bic(t->file[fd].flags, ALLOCATED_FD);
   return 1;
}

/* Opens the given filepath and returns an fd on success
 * else returns -1 on failure
 */

int opendir_tarfs(char *path, uint64_t flags)
{
   task* t = cur_task;
   int i;
   for(i=0; i<MAX_DIR; i++)
   {
      if(!bit(t->file[i].flags, ALLOCATED_FD))
      {
         bis(t->file[i].flags, ALLOCATED_FD);
         sbustrncpy((char *)(t->file[i].name), path, sbustrlen(path));
        
         t->file[i].offset = (uint64_t) &_binary_tarfs_start; 
         return i;
      }
   }
   return -1;
}

/* Closes the given fd and returns 1 on success
 * else -1 on failure
 */

int closedir_tarfs(int fd)
{
   task* t = cur_task;
   if(!bit(t->file[fd].flags, ALLOCATED_FD))
   {
      return -1;
   }
   bic(t->file[fd].flags, ALLOCATED_FD);
   return 1;
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
   /*
   char *filecontent;
   int fsize = getFileFromTarfs("bin/ls", &filecontent);
   if(fsize != 0)
   {
      kprintf("bin/ls file found\n");
      //kprintf("file content = %s\n",filecontent);
      //kprintf("last char = %d\n", *(filecontent+5)); 
      //elf_load_file(filecontent);
   }*/

   char *name = "usr";
   int fd = opendir_tarfs(name, 1);
   kprintf("fd=%d\n",fd);
   char buf[256];
   int k;
   kprintf("The entries in %s are: \n",name);
   for(k=0; readdir_tarfs(fd, buf)==1; k++)
   {
      kprintf("file %d = %s\n",k, buf);
   }
   closedir_tarfs(fd);   
 
   char *s = "usr/some.txt";
   int fd1 = open(s, 1);
   kprintf("fd=%d\n",fd1);
   char buffer[256];
   int bytes1 = read(fd1, buffer, 256);
   kprintf("buffer = %s\n",buffer);
   kprintf("bytes1 = %d\n",bytes1);
   int bytes2 = read(fd1, buffer, 12);
   kprintf("buffer = %s\n",buffer);
   kprintf("bytes2 = %d\n",bytes2);
   close(fd1);
   
}
