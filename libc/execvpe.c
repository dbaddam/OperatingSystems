#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int execvp(const char *filename, char *const argv[])
{
   return execvpe(filename, argv, _sbush_env);
}
