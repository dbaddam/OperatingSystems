#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>

#define MAX_ENV_KEYS 200

extern char* _sbush_env[MAX_ENV_KEYS];

int main(int argc, char *argv[], char *envp[]);
void exit(int status);

void *malloc(size_t size);
void free(void *ptr);

void setupenv(char* envp[]);
void printenv();
int setenv(const char *name, const char *value, int overwrite);
char *getenv(const char *name);
char **getenvp();

#endif
