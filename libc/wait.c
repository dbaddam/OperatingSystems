#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t wait(int *status)
{
   return wait4(-1, status, 0, NULL);
}
