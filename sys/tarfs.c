#include <sys/os.h>


int getFileFromTarfs(char *filename, char **file_start_address);

/*
 * memset
 */

void *sbumemset(void *b, int c, int len)
{
   int i;
   char *p = b;
   for(i=0;i<len;i++)
   {
      *p = c;
       p++;
   }
   return b;
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
    for(i=0;i<sbustrlen(str);i++)
    {
        if(str[i] == substring[0])
        {
            char* res = (char*) _get_page();
            if(sbustr(str,i,i+sbustrlen(substring)-1,res) == -1)
            {
                _free_page(res);
                return -1;
            }
            if(sbustrncmp(res,substring,sbustrlen(substring)))
            {
                if(sbustr(str,i,sbustrlen(str)-1,result)==1)
                {
                    _free_page(res);
                    return 1;
                }
            }
            _free_page(res);
        }
    }
    return -1;
}

/* concatenate given two strings */
void sbuconcat(char *str1, char *str2, char *result)
{
   int i;
   int k=0;
   for(i=0;i<sbustrlen(str1);i++)
   {
      result[k] = str1[i];
      k++;
   }
   for(i=0;i<sbustrlen(str2);i++)
   {
      result[k] = str2[i];
      k++;
   }
   result[k]='\0';
   return;
}

/* Returns the FIRST index of given character ch if present in a given string,
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

/* given: /bin/ls/../dinesh
 * return: /bin/dinesh
 */
void handle_dots(char *str, char *result)
{
   if(contains(str, '.') == -1)
   {
        //copy str into result, safe side
        strncpy(result, str, sbustrlen(str)+1);
        return;
   }
   int i;
   for(i=0;i<sbustrlen(str);i++)
   {
      if(str[i] == '.' && str[i+1] == '.')
      {
         int j;
         for(j=i-2;j>=0;j--)
         {
            if(str[j]=='/')
            {
               char* part1 = (char*) _get_page();
               char* part2 = (char*) _get_page();
               sbustr(str, 0, j-1, part1);
               sbustr(str, i+2, sbustrlen(str)-1, part2);
               sbuconcat(part1, part2, result);
               _free_page(part1);
               _free_page(part2);
               handle_dots(result, result);
               return;
            }
         }
         sbustr(str, i+2, sbustrlen(str)-1, result);
         handle_dots(result, result);
         return;
      }
   }
   return;
}

/* given: /bin//dinesh//dude
 * return: /bin/dinesh/dude
 */
void handle_slashes(char *str, char *result)
{
   if(str[sbustrlen(str)-1] == '/')
   {
      str[sbustrlen(str)-1] = '\0';
   }
   int i;
   for(i=0;i<sbustrlen(str)-1;i++)
   {
      if(str[i] == '/' && str[i+1] == '/')
      {
         if(str[i+2]=='\0')
         {
            str[i] = '\0';
            handle_slashes(str, result);
            return;
         }
               char* part1 = (char*) _get_page();
               char* part2 = (char*) _get_page();
               sbustr(str, 0, i, part1);
               sbustr(str, i+2, sbustrlen(str)-1, part2);
               sbuconcat(part1, part2, result);
               _free_page(part1);
               _free_page(part2);
               handle_slashes(result, result);
               return;
      }
   }
   strncpy(result, str, sbustrlen(str)+1);
   return;
}


/* Sanitizes the given path, handling relative or obsolute and also double dots in
 * the given path
 */
int sanitize_path(char *path, char *result)
{
  // path = "/" or "//"
  if(((*path == '/') && (*(path+1)=='\0')) || 
     (*path == '/' && *(path+1)=='/' && *(path+2)=='\0')
    )
  {
     // root
     *result = '/';
     *(result+1) = '\0';
     return 1;
  }
  // obsolute path, no need to get pwd
  // but need to handle the dots (/..)
  if(*path == '/')
  { 
     char* buf = (char*) _get_page(); 
     path++;
     handle_dots(path, buf);
     if(sbustrlen(buf)>0)
     {
        if(buf[sbustrlen(buf)-1] == '/')
        {  
           buf[sbustrlen(buf)-1] = '\0';
        }
     }
     if(sbustrlen(buf)>0)
     {
        if(*buf == '/')
        {
           strncpy(buf, buf+1, sbustrlen(buf)+1);
        }
     }
     char *result1 = (char*) _get_page();
     strncpy(result1, buf, sbustrlen(buf)+1);
     handle_slashes(result1, result);
     _free_page(result1);
     _free_page(buf);
     return 1;
  }
  else
  { // relative path
    // get pwd and append to path and then handle the dots
     char* buf = (char*) _get_page();
     char* cwd = (char*) _get_page();
     getcwd(cwd, 256);
     if(sbustrlen(cwd)>1) // if cwd = "/", we should not add / again
     {  
        cwd[sbustrlen(cwd)] = '/';
        cwd[sbustrlen(cwd)+1] = '\0';
     }
     char* fullpath = (char*) _get_page(); 
     sbuconcat(cwd, path, fullpath);
     handle_dots(fullpath, buf);
     if(sbustrlen(buf)>0)
     {
        if(buf[sbustrlen(buf)-1] == '/')
        {  
           buf[sbustrlen(buf)-1] = '\0';
        }
     }
     if(sbustrlen(buf)>0)
     {
        if(buf[0] == '/')
        {  
           strncpy(buf, buf+1, sbustrlen(buf)+1);
        }
     }
     char *result1 = (char*) _get_page();
     strncpy(result1, buf, sbustrlen(buf)+1);
     handle_slashes(result1, result);
     _free_page(result1);
     _free_page(cwd);
     _free_page(fullpath);
     _free_page(buf);
     return 1;
  }
  return -1;
}

/* given a filename and a start address, searches in tarfs and returns the filesize and file content 
 * start address if found, or 0 if not found
 */
int getFileFromTarfs(char *unsanitized_filename, char **file_start_address)
{
   char* filename = (char*) _get_page();
   sanitize_path(unsanitized_filename, filename);
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
         _free_page(filename);
         return filesize;
      }
      addr += (((filesize+511)/512)+1)*512;
   }
   _free_page(filename);
   return 0; 
}

/* checks if given name is directory or not */
//int is_directory(char *unsanitized_dirname)
int is_directory(char *dirname)
{
   /*char *dirname = (char*) _get_page();
   sanitize_path(unsanitized_dirname, dirname);
   kprintf("in is_directory, dirname = %s\n",dirname);
  */ uint64_t addr = (uint64_t) &_binary_tarfs_start;
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
  //          _free_page(dirname);
            return 1;
         }
      }   
      addr += (((filesize+511)/512)+1)*512;
   }   
 //  _free_page(dirname);
   return -1;
}

/* checks if given name is file or not */
int is_file(char *filename)
{  
   /*char* filename = (char*) _get_page();
   sanitize_path(unsanitized_filename, filename);*/
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
         if((char)(header->typeflag[0]) == '\0' || 
            (char)(header->typeflag[0]) == '0')
         {  
            //_free_page(filename);
            return 1;
         } 
      }    
      addr += (((filesize+511)/512)+1)*512;
   }   
   //_free_page(filename);
   return -1;
}

/* This function lists all the files/directories present in the given directory
 * NOTE: The input argument should be a full path
 * 
 */
void read_directory(char *unsanitized_dirname)
{
   char *dirname = (char*) _get_page();
   sanitize_path(unsanitized_dirname, dirname);
   if(is_directory(dirname)==-1)
   {
      kprintf("%s is not a directory name\n",dirname);
      _free_page(dirname);
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
      char *dircontent = (char*) _get_page();
      if(sbustrstr(header->name, dirname, dircontent)==1)
      {   
         char *content = (char*) _get_page();
         sbustr(dircontent, sbustrlen(dirname)+1, sbustrlen(dircontent)-1, content);
         if(sbustrlen(content)>0)
         {
            int index = contains(content,'/');
            if(index != -1)
            {
               char *content1 = (char*) _get_page();
               sbustr(content, 0, index-1, content1);
               kprintf("%s\n",content1);
               _free_page(content1);
            }else
            {
               kprintf("%s\n",content);
            }
         }
         _free_page(content);
      }
      _free_page(dircontent);
      addr += (((filesize+511)/512)+1)*512;
   }   
   _free_page(dirname);
   return;
}

/* This function returns the directories/files in the root
 */
int readdir_tarfs_root(int fd, char *buf)
{
   task* t = cur_task;
   uint64_t addr = t->file[fd].offset;
   int i;
   for(i=0;;i++)
   {  
      struct posix_header_ustar *header;
      header = (struct posix_header_ustar *)addr;
      if(header->name[0]=='\0')
      {  
         return -1;
      }
      unsigned int filesize = oct2bin((unsigned char *)header->size, 11);
      int index = contains(header->name, '/');
      if(index != -1)
      {
         if(header->name[index+1] == '\0')
         {
            header->name[index] = '\0';
            strncpy(buf, header->name, sbustrlen(header->name)+1);
            addr += (((filesize+511)/512)+1)*512;
            t->file[fd].offset = addr;
            return 1;
         }      
      }
      if(index == -1)
      {
         strncpy(buf, header->name, sbustrlen(header->name)+1);
         addr += (((filesize+511)/512)+1)*512;
         t->file[fd].offset = addr;
         return 1;
      }
      addr += (((filesize+511)/512)+1)*512;
   }
   return -1;
}


/*  This function reads the next directory entry from given fd, and returns 1 on success,
 *  returns directory entry into buf
 * 
 *  else returns -1, if there are no more entries or on failure
 */
char prebuf[256]="";
uint32_t readdir_tarfs(uint32_t fd, char *buf)
{
   task* t = cur_task;
   char *dirname = (char*) _get_page();
   strncpy(dirname, (char *)t->file[fd].name, sbustrlen((char *)t->file[fd].name)+1);
   
   // if dirname is root, just call readdir root function
   if(*dirname == '/' && *(dirname+1) == '\0')
   {
      int r = readdir_tarfs_root(fd, buf);
      return (r==1)?1:-1;   
   } 
   if(is_directory(dirname)==-1)
   {
      kprintf("%s is not a directory name\n",dirname);
      _free_page(dirname);
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
 //        kprintf("No more entries available in the given directory name!\n");
         _free_page(dirname);
         sbumemset(prebuf, 0, 256);
         return -1;
      }
      unsigned int filesize = oct2bin((unsigned char *)header->size, 11);
      char *dircontent = (char*) _get_page();
      if(sbustrstr(header->name, dirname, dircontent)==1)
      {
         char *content = (char*) _get_page();
         sbustr(dircontent, sbustrlen(dirname)+1, sbustrlen(dircontent)-1, content);
         if(sbustrlen(content)>0)
         {
            int index = contains(content,'/');
            if(index != -1)
            {
               char *content1 = (char*) _get_page();
               sbustr(content, 0, index-1, content1);
               if(sbustrncmp(prebuf, content1, sbustrlen(content1))==0)
               {
                  strncpy(buf, content1, sbustrlen(content1)+1);
                  strncpy(prebuf, content1, sbustrlen(content1)+1);
                  addr += (((filesize+511)/512)+1)*512;
                  t->file[fd].offset = addr;
                  _free_page(dircontent);
                  _free_page(content1);
                  _free_page(content);
                  _free_page(dirname);
                  return 1;
                }
            }else
            {
               if(sbustrncmp(prebuf, content, sbustrlen(content))==0)
               { 
                  strncpy(buf, content, sbustrlen(content)+1);
                  strncpy(prebuf, content, sbustrlen(content)+1);
                  addr += (((filesize+511)/512)+1)*512;
                  t->file[fd].offset = addr;
                  _free_page(dircontent);
                  _free_page(content);
                  _free_page(dirname);
                  return 1;
               }
            }
         }
         _free_page(content);
      }
      _free_page(dircontent);
      addr += (((filesize+511)/512)+1)*512;
   }
   _free_page(dirname);
   return -1;
}


/* Opens a given filepath and returns a fd
 * else, returns 0
 */
uint32_t open(char* unsanitized_path, int32_t mode)
{
   char *path = (char *) _get_page();
   sanitize_path(unsanitized_path, path);
   task* t = cur_task;
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
         _free_page(path);
         return (uint32_t)i;
      }
   }
   _free_page(path);
   return 0;
}

/* returns number of bytes read and returns the content in buf
 * if EOF is reached, reads till that point and return those many bytes read
 */
uint64_t read(uint32_t fd, char *buf, uint64_t count)
{
   task* t  = cur_task;

   if (fd == STDIN)
   {
      return readLine(buf);
   }

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


/* lseek moves the offset to start_addr+seek given
 * return 1 on succuess, -1 on failure
 */
int lseek(uint32_t fd, uint64_t seek)
{
   task* t = cur_task;
   if(bit(t->file[fd].flags, ALLOCATED_FD))
   {   
      uint64_t fsize = t->file[fd].size;
      uint64_t start_addr = t->file[fd].start_addr;
      uint64_t end_addr = start_addr + fsize;
      if(start_addr + seek <= end_addr)
      {
         t->file[fd].offset = start_addr + seek;
         return 1; 
      }
   } 
   return -1;
}



/* Opens the given filepath and returns an fd on success
 * else returns -1 on failure
 */

int opendir_tarfs(char *unsanitized_path, uint64_t flags)
{
   char *path = (char *) _get_page();
   sanitize_path(unsanitized_path, path);
   task* t = cur_task;
   int i;
   for(i=0; i<MAX_FILES; i++)
   {
      if(!bit(t->file[i].flags, ALLOCATED_FD))
      {
         bis(t->file[i].flags, ALLOCATED_FD);
         strncpy((char *)(t->file[i].name), path, sbustrlen(path)+1);
         t->file[i].offset = (uint64_t) &_binary_tarfs_start;
         _free_page(path); 
         return i;
      }
   }
   _free_page(path);
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
/*
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
   }*/
   /*
   char *filecontent;
   int fsize = getFileFromTarfs("bin/sbush", &filecontent);
   if(fsize != 0)
   {
      kprintf("bin/sbush file found\n");
      //kprintf("file content = %s\n",filecontent);
      //kprintf("last char = %d\n", *(filecontent+5)); 
      //elf_load_file(filecontent);
   }*/


   char *name = "//usr//dinesh//..//";
   int fd = opendir_tarfs(name, 1);
   char buf[256];
   int k;
   kprintf("The entries in %s are: \n",name);
   for(k=0; readdir_tarfs(fd, buf)==1; k++)
   {
      kprintf("file %d = %s\n",k, buf);
   }
   closedir_tarfs(fd);   

/*
   char *name = "dinesh";
   //sanitize_path(path, name);
   //kprintf("given path = %s\n",path);
   //kprintf("sanitized path = %s\n",name);
   //char *name = "bin";
   int fd = opendir_tarfs(name, 1);
   char buf[256];
   int k;
   kprintf("The entries in %s are: \n",name);
   for(k=0; readdir_tarfs(fd, buf)==1; k++)
   {
      kprintf("file %d = %s\n",k, buf);
   }
   closedir_tarfs(fd); 
*/
}
