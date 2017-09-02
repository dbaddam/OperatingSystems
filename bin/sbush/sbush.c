#include <stdio.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024


void printError(char *str)
{
   fputs(str, stdout);
}

void printMessage(char *str)
{
   fputs(str, stdout);
}

void printLine(char* str)
{
   printMessage(str);
   printMessage("\n");
}

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

int iscmd(char *buf, char *cmd)
{
   return sbustrncmp(buf, cmd, sbustrlen(cmd));
}

void sbustrncpy(char* dest, char* src, int len)
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

int runcmd(char *buf)
{
   char *c = buf;
   
   /* Consider EOF as well */ 
   while( *c == ' ' || *c == '\t')
   {
      c++;
   }

   if (*c == '\0')
   {
      return 0;
   }

   if (iscmd(c, "pwd"))
   {
      char cwd[MAX_BUFFER_SIZE];
      if (getcwd(cwd, sizeof(cwd)))
      {
         printLine(cwd);
      }
      else
      {
         printLine("error - unable to get present working directory");
      }
   }
   /* (exit is matched and we are at the end of the buffer) OR 
    * (exit is matched and there is a space next */
   else if ((iscmd(c, "exit") && !*(c+4))||
            iscmd(c, "exit "))
   {
      return 1; 
   }
   else if (iscmd(c, "cd"))
   {
      char *pathstart;
      char *pathend;
      char  path[MAX_BUFFER_SIZE];
      char *ch;
      int   i;

      /* Extract the path out of the buffer by doing the following.
       * 0. Start with the character after cd
       * 1. Search for the first non space character. Mark it as pathstart
       * 2. Search for the first space or the end of buf after pathstart.
       *    Mark it as pathend
       * 3. Copy [pathstart, pathend) to 'path' array. Now call chdir
       */
      c += sbustrlen("cd");
      while ( *c == ' ' || *c == '\t')
         c++;
      pathstart = c;

      while ( *c != ' ' && *c != '\0')
         c++;
      pathend = c;

      for ( i=0, ch = pathstart; ch < pathend; ch++, i++)
         path[i] = *ch;
      path[i] = '\0';
   
      if (chdir(path))
      {
         printError("error - unable to change directory\n");
      } 
   }
   else
   {
      char *args[] = {"ls", NULL};
      char *env[] = {"PATH=/bin", NULL};

      if (!execvpe(args[0], args, env))
      {
         printError("error - invalid command/unable to execute\n");
      }

      //printf("invalid command\n");
   }

   return 0;
}


int main(int argc, char *argv[], char *envp[]) {
   char  buffer[MAX_BUFFER_SIZE];

   while(1)
   {
      printMessage("sbush> ");
      if (!gets(buffer))
      {
         printError("Unable to process input");
      }

      if (runcmd(buffer))
      {
         break;
      }
   } 
   return 0;
}
