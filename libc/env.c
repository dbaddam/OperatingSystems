#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_ENV_VALUE_SIZE 1000
#define MAX_KEY_SIZE 100

static char env[MAX_ENV_KEYS][MAX_ENV_VALUE_SIZE];
static char keys[MAX_ENV_KEYS][MAX_KEY_SIZE];
char* _sbush_env[MAX_ENV_KEYS] = {NULL};

void printenv()
{
   int i;

   puts("***********Print environment************");
   for (i = 0; _sbush_env[i];i++)
   {
      puts(keys[i]);
      puts(env[i]);
   }
   puts("***********Print environment done*********");
}
void setupenv(char* envp[])
{
   int i;
   
   for (i = 0; envp[i]; i++)
   {
       char* keystart = envp[i];
       char* keyend = keystart;

       strcpy(env[i], envp[i]);
       _sbush_env[i] = env[i];

       while (*keyend && *keyend != '=')
          keyend++;

       strncpy(keys[i], envp[i], keyend - keystart);
       keys[i][keyend-keystart] = '\0';
   }

   /*printenv();*/
}

int setenv(const char *name, const char *value, int overwrite)
{
    int len = strlen(name);
    int i;

    for (i = 0; _sbush_env[i]; i++)
    {
       if (!strcmp(keys[i], name))
       {
          if (overwrite)
          {
             strcpy(env[i], name);
             env[i][len] = '=';
             strcpy(&env[i][len+1], value);
          }
          return 0;
       }
    }

    strcpy(env[i], name);
    strcpy(keys[i], name);
    env[i][len] = '=';
    strcpy(&env[i][len+1], value);
    _sbush_env[i] = env[i];
    return 0;
}

char *getenv(const char *name)
{
    int len = strlen(name);
    int i;

    for (i = 0; _sbush_env[i]; i++)
    {
       if (!strcmp(keys[i], name))
       {
          return &env[i][len+1];
       }
    }

    return NULL; 
}


