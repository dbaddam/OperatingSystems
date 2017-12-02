#include <unistd.h>
#include <stdio.h>



/* At the end of fgets, the caller would get a NULL-terminated string.
 * The execption to this is input is from stdin OR size = 1
 * Also, fgets returns at most one line. */
int fgets(char *s, int size, FILE * fp)
{
    int rcount = 0;

    if (fp == stdin)
       return read(fp->fd, s, size);

    if (size > 1)
    {
       long i = 0;

       rcount = read(fp->fd, s, size-1);

       /* We see if there is a new line character and return only
        * till that point. We use lseek to correct the file offset
        * so that the next 'read' starts from the right place */
       while (i < rcount && s[i] != '\n' && s[i] != '\0')
          i++;

       if (i == rcount)
       {
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

char* gets(char* buf)
{
   int rcount = read(0, buf, 1000); 
   if (rcount >= 0)
      return buf;
   return NULL; 
}
