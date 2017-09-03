#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_ARG_COUNT 100
#define MAX_ARG_SIZE 1000

void printMessage(char *str)
{
   fputs(str, stdout);
}

void printLine(char* str)
{
   printMessage(str);
   printMessage("\n");
}

void printError(char *str)
{
   printLine(str);
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

/********************** sbusplit ************************/
/* sbusplit - This function splits the input buffer with space the delimiter.
      printf("New env %s\n", getenv("PATH"));
 *
 * Arguments :
 * buf  - input string
 * args - the output set of strings
 * Returns the number of arguments
 */
int sbusplit(char* buf, char args[][MAX_ARG_SIZE])
{
   char  *c = buf;
   char  *ch;
   int    argcnt = 0;
   int    i;
   char  *argstart, *argend;
 
   while (*c != '\0')
   {
      while (*c == ' ' || *c == '\t')
         c++;
      argstart = c;

      while (*c != ' ' && *c != '\t' && *c != '\0')
         c++;
      argend = c;

      for (i=0, ch = argstart; ch < argend; ch++, i++)
         args[argcnt][i] = *ch;
      args[argcnt][i] = '\0';

      argcnt++;
      if (argcnt >= MAX_ARG_COUNT)
      {
         printError("error - too many arguments");
         break;
      }
   }
   return argcnt;
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

   if (iscmd(c, "#!")) 
   {
      return 0;
   }
   else if ((iscmd(c, "pwd") && !*(c+3))||
        iscmd(c, "pwd "))
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
   else if (iscmd(c, "cd "))
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
      c += sbustrlen("cd ");
      while ( *c == ' ' || *c == '\t')
         c++;
      pathstart = c;

      while ( *c != ' ' && *c != '\t' && *c != '\0')
         c++;
      pathend = c;

      for ( i=0, ch = pathstart; ch < pathend; ch++, i++)
         path[i] = *ch;
      path[i] = '\0';
   
      if (chdir(path))
      {
         printError("error - unable to change directory");
      } 
   }  
   else if (iscmd(c, "export "))
   {
      char *keystart;
      c += sbustrlen("export ");

      while (*c == ' ' || *c == '\t')
         c++;
      keystart = c;

      while (*c != '=' && *c != '\0')
         c++;

      /* incomplete command - 'export PATH' */
      if (*c == '\0')
         return 0;
      else
      {
         /* *c == '=' */
         *c = '\0';
      }

      if (setenv(keystart, c+1, 1))
      {
         printError("error - unable to set environment variable");
      }
      //printf("%s\n", getenv("PATH"));
      //printf("%s\n", getenv("PS1"));
   }
   else
   {
      char *args[MAX_ARG_COUNT];
      char  argscontent[MAX_ARG_COUNT][MAX_ARG_SIZE];
      char *env[] = {NULL};
      int   background = 0;
      int   status;
      int   argcount;
      int   pid;
      int   i;

      argcount = sbusplit(c, argscontent);
      for (i = 0;i < argcount;i++)
         args[i] = argscontent[i];

      /* If the command needs be run in the background, the last argument
       * should be '&' and we are not supposed to pass that to execvp*()
       * If '&' is not set, we make the LAST argument as NULL. */ 
      if (argcount > 0 && iscmd(argscontent[argcount-1], "&"))
      {
         background = 1;
         args[argcount-1] = NULL;
      }
      else
      {
         args[argcount] = NULL;
      }

      pid = fork();
      if (pid == 0)
      {
         if (!execvpe(args[0], args, env))
         {
            printError("error - invalid command/unable to execute\n");
         }
 
         printError("error - unable to execute");
         exit(1);
      } 
      else
      {
         if (!background)
            waitpid(pid, &status, 0);
      }
   }

   return 0;
}


int main(int argc, char *argv[], char *envp[])
{
   char  buffer[MAX_BUFFER_SIZE];
   char *c;
   FILE *fp = stdin;

   if (argc > 1)
   {
      fp = fopen(argv[1], "r");
   }

   setenv("PS1", "sbush>", 1);
   while(1)
   {
      if (argc == 1)
         printMessage(getenv("PS1"));

      /* fgets stores '\n' AND '\0' at the end of the buffer unlike
       * gets which stores only '\0'. So, we find '\n' and replace
       * it with '\0' before starting to process to make our lives easier.
       * We are going back and forth between fgets and gets. You may
       * have to uncomment the following code.
       */
      if (!fgets(buffer, MAX_BUFFER_SIZE, (argc > 1 )? fp:stdin))
      {
         break;
      }

      c = buffer;
      while (*c != '\n' && *c != '\0')
         c++;
      *c = '\0';

      if (runcmd(buffer))
      {
         break;
      }
   }

   fclose(fp); 
   return 0;
}
