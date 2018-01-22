#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <syscall.h>

FILE _files_int[MAX_FILES];

FILE* fopen(const char *path, const char *mode)
{

   int   flags  = 0;
   int   rflag  = 0;
   int   wflag  = 0;
   int   fd;

   stdin->fd = 0;
   stdout->fd = 1;
   stderr->fd = 2;

   for (; *mode;mode++)
   {
       if (*mode == 'r')
          rflag = 1;
       else if(*mode == 'w')
          wflag = 1;
       else
       {
          puts("I/O error - we don't support this I/O mode");
          return NULL;
       }
   }

   if (rflag && wflag)
      flags = O_RDWR;
   else if (rflag)
      flags = O_RDONLY;
   else if (wflag)
      flags = O_WRONLY;
   else
   {
      puts("I/O error - we don't support this I/O mode");
      return NULL;
   }

   fd = open(path, flags);

   if (fd >= 0)
   {
      if (fd < MAX_FILES)
      {
         _files_int[fd].fd = fd;
         _files_int[fd].offset = 0;
         return _files_int + fd;
      }
      else
      {
         return NULL;
      }
   }
   else
   {
      return NULL;
   }

   return 0;
}

/* At the end of fgets, the caller would get a NULL-terminated string.
 * The execption to this is input is from stdin OR size = 1
 * Also, fgets returns at most one line. */
int fgets(char *s, int size, FILE * fp)
{
    int rcount = 0;

    stdin->fd = 0;
    stdout->fd = 1;
    stderr->fd = 2;

    if (fp == stdin)
       return read(fp->fd, s, size);

    if (size > 1)
    {
       long i = 0;

       rcount = read(fp->fd, s, size-1);

       if (rcount == 0)
          return -1; 
       /* We see if there is a new line character and return only
        * till that point. We use lseek to correct the file offset
        * so that the next 'read' starts from the right place */
       while (i < rcount && s[i] != '\n' && s[i] != '\0')
          i++;

       if (i == rcount)
       {
          fp->offset += rcount;
          /* do nothing */
       }
       else if (s[i] == '\n')
       {
          fp->offset += i+1;
 
          if (lseek(fp->fd, fp->offset, SEEK_SET) < 0)
          {
             puts("I/O error : lseek failure");
             return -1;
          }
          rcount = i+1;
       } 
       else if(s[i] == '\0')
       {
          fp->offset += i+1;
          /* do nothing */
       }
    }
    else
    {
       /* If someone wants to read one character only, let's not interfere */
       rcount = read(fp->fd, s, size);
    }

    if (rcount >= 0)
    {
       s[rcount] = '\0';
    }

    return rcount;
}

int fclose(FILE *stream)
{
   return close(stream->fd); 
}

int putchar(int c)
{
  syscall3(write, 1, &c, 1);   
  return (sysret == 1) ? c:0;
}

int puts(const char *s)
{
  for( ; *s; ++s) if (putchar(*s) != *s) return EOF;
  return (putchar('\n') == '\n') ? 0 : EOF;
}

char* gets(char* buf)
{
   stdin->fd = 0;
   stdout->fd = 1;
   stderr->fd = 2;
   int rcount = read(0, buf, 1000); 
   if (rcount >= 0)
      return buf;
   return NULL; 
}

int fputs(const char *s, FILE * fp)
{
    stdin->fd = 0;
    stdout->fd = 1;
    stderr->fd = 2;
    const char *c = s;
    int len = 0;

    while (*c)
    {
       len++;
       c++;
    }

    return write(fp->fd, s, len);
}
