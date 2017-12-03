#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

FILE files[MAX_FILES];

/* TODOKISHAN - For now fopen won't create a file if you open
 * in write mode. You can open only in READ mode
 * for now. We may have add additional functionality
 * as necessary in the future. */
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
         files[fd].fd = fd;
         files[fd].offset = 0;
         return files + fd;
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
